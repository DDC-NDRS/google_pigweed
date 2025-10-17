// Copyright 2025 The Pigweed Authors
//
// Licensed under the Apache License, Version 2.0 (the "License"); you may not
// use this file except in compliance with the License. You may obtain a copy of
// the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
// WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
// License for the specific language governing permissions and limitations under
// the License.
#pragma once

#include <mutex>
#include <optional>
#include <type_traits>

#include "pw_assert/assert.h"
#include "pw_async2/context.h"
#include "pw_async2/poll.h"
#include "pw_containers/intrusive_list.h"
#include "pw_sync/interrupt_spin_lock.h"

namespace pw::async2::experimental {

/// A `Future` is an abstract handle to an asynchronous operation that is polled
/// to completion. On completion, futures may return a value representing the
/// result of the operation.
///
/// Futures are single-use and track their completion status. It is an error
/// to poll a future after it has already completed.
///
/// # Implementing
///
/// The future class does not contain any members itself, providing only a core
/// interface and delegating management to specific implementations.
///
/// In practice, developers will rarely derive from `Future` directly. Instead,
/// they should use a more specific abstract future type like
/// `ListableFutureWithWaker`, which manages common behaviors like waker
/// storage.
///
/// Deriving from `Future` directly is necessary when these behaviors are not
/// required; for example, when implementing a future that composes other
/// futures.
///
/// Implementations derived directly from `Future` are required to provide the
/// following member functions:
///
/// - `Poll<T> DoPend(Context& cx)`: Implements the asynchronous operation.
/// - `void DoMarkComplete()`: Marks the future as complete.
/// - `bool DoIsComplete() const`: Returns `true` if `DoMarkCompleted` has
///    previously been called.
///
/// @tparam Derived The concrete class that implements this `Future`.
/// @tparam T       The type of the value returned by `Pend` upon completion.
template <typename Derived, typename T>
class Future {
 public:
  using value_type = T;

  /// Polls the future to advance its state.
  ///
  /// Returns `Pending` if the future is not yet complete, or `Ready` with
  /// its result if it is.
  ///
  /// If this future has already completed, calling `Pend` will trigger an
  /// assertion.
  Poll<value_type> Pend(Context& cx) {
    PW_ASSERT(!is_complete());
    Poll<value_type> poll = derived().DoPend(cx);
    if (poll.IsReady()) {
      derived().DoMarkComplete();
    }
    return poll;
  }

  /// Returns `true` if the future has already returned a `Ready` result.
  ///
  /// Calling `Pend` on a completed future will trigger an assertion.
  bool is_complete() const { return derived().DoIsComplete(); }

 protected:
  constexpr Future() = default;

 private:
  Derived& derived() { return static_cast<Derived&>(*this); }
  const Derived& derived() const { return static_cast<const Derived&>(*this); }
};

namespace internal {

template <typename D, typename V>
std::true_type IsFuture(const volatile Future<D, V>*);

std::false_type IsFuture(...);

}  // namespace internal

template <typename T>
struct is_future : decltype(internal::IsFuture(std::declval<T*>())){};

template <typename T>
constexpr bool is_future_v =
    is_future<std::remove_cv_t<std::remove_reference_t<T>>>::value;

/// Manages a list of futures for a single asynchronous operation.
///
/// An asynchronous operation that vends futures to multiple callers can use a
/// `ListFutureProvider` to track them. This class can be used with any future
/// that derives from a listable type like `ListableFutureWithWaker`. The
/// provider and its futures automatically handle list updates during moves.
///
/// All operations on the list are thread-safe, allowing futures to be modified
/// from outside of an async context (for example, to complete a future on an
/// external signal). The type of lock used is configurable, though it is
/// important to understand that the lock will be acquired within the
/// asynchronous dispatcher's thread. Therefore, it is strongly recommended to
/// avoid long-blocking locks such as mutexes as they will stall other tasks.
///
/// The default lock is a `pw::sync::InterruptSpinLock`, which is a safe
/// choice for use within an async context. If it is certain that futures in
/// the list will only be managed from within an async context (for example,
/// between different async tasks), a no-op lock can be used for efficiency.
///
/// The future list is FIFO: `Pop` returns futures in the order they were added.
///
/// When a future in the list is destroyed, it safely removes itself. The
/// provider is not notified of this event.
template <typename FutureType, typename Lock = sync::InterruptSpinLock>
class ListFutureProvider {
 public:
  constexpr ListFutureProvider() = default;

  ListFutureProvider(const ListFutureProvider&) = delete;
  ListFutureProvider& operator=(const ListFutureProvider&) = delete;

  /// Adds a future to the end of the list.
  void Push(FutureType& future) {
    std::lock_guard lock(lock_);
    futures_.push_back(future);
  }

  /// Removes and returns the first future from the list.
  ///
  /// Triggers an assertion if the list is empty.
  FutureType& Pop() {
    std::lock_guard lock(lock_);
    PW_ASSERT(!futures_.empty());
    FutureType& future = futures_.front();
    futures_.pop_front();
    return future;
  }

  /// Returns `true` if there are no futures listed.
  bool empty() {
    std::lock_guard lock(lock_);
    return futures_.empty();
  }

  /// Provides access to the list's internal lock.
  Lock& lock() { return lock_; }

 private:
  friend FutureType;

  template <typename, typename>
  friend class ListableFutureWithWaker;

  using LockType = Lock;

  IntrusiveList<FutureType> futures_;
  Lock lock_;
};

/// Manages a single future for an asynchronous operation.
///
/// An asynchronous operation which can only have a single caller can use a
/// `SingleFutureProvider` to manage its reference to the future. This can be
/// used with any listable future type, and automatically handles updates
/// during moves.
///
/// All operations on the provider are thread-safe.
///
/// If the future belonging to the provider is destroyed, it safely removes
/// itself. The provider is not notified of this event.
template <typename FutureType>
class SingleFutureProvider {
 public:
  constexpr SingleFutureProvider() = default;

  SingleFutureProvider(const SingleFutureProvider&) = delete;
  SingleFutureProvider& operator=(const SingleFutureProvider&) = delete;

  /// Sets the provider's future. Crashes if a future is already set.
  void Set(FutureType& future) {
    PW_ASSERT(!has_future());
    inner_.Push(future);
  }

  /// Attempts to set the provider's future, returning `true` if successful.
  bool TrySet(FutureType& future) {
    if (has_future()) {
      return false;
    }
    inner_.Push(future);
    return true;
  }

  /// Claims the provider's future, leaving it unset.
  /// Crashes if there is no future set.
  [[nodiscard]] FutureType& Take() { return inner_.Pop(); }

  /// Returns `true` if the provider has a future.
  bool has_future() { return !inner_.empty(); }

 private:
  template <typename, typename>
  friend class ListableFutureWithWaker;
  friend FutureType;

  ListFutureProvider<FutureType> inner_;
};

/// An abstract movable future that is stored in an intrusive linked list
/// managed by a `ListFutureProvider`.
///
/// `ListableFutureWithWaker` is extended by concrete future types for
/// specific asynchronous operations. It internally handles list management
/// during moves and stores the `Waker` of the task that polled it.
///
/// # Implementing
///
/// A concrete future that derives from `ListableFutureWithWaker` must provide a
/// `DoPend` method and implement its own move constructor and move assignment
/// operator. It must also provide a
/// `static constexpr const char kWaitReason[]` that is used as the waker's wait
/// reason.
///
/// The move operations must first move any members of the derived class, then
/// call the base `MoveFrom` method to transfer the intrusive list pointers and
/// waker.
///
/// @code{.cpp}
/// class MyFuture : public ListableFutureWithWaker<MyFuture, int> {
///  public:
///   static constexpr const char kWaitReason[] = "MyFuture";
///
///   MyFuture(MyFuture&& other) noexcept
///       : ListableFutureWithWaker(kMovedFrom) {
///     // First, move any derived members.
///     provider_ = std::exchange(other.provider_, nullptr);
///     // Finally, call the base class to handle its state.
///     ListableFutureWithWaker::MoveFrom(other);
///   }
///
///   MyFuture& operator=(MyFuture&& other) noexcept {
///     provider_ = std::exchange(other.provider_, nullptr);
///     ListableFutureWithWaker::MoveFrom(other);
///     return *this;
///   }
///
///  private:
///   // ...
/// };
/// @endcode
///
/// If a listable future is destroyed while it is in a provider's list, it
/// safely removes itself. The provider is not notified of this. Asynchronous
/// operations which require more complex cancellation or cleanup must handle
/// this in their `Derived` future's destructor.
///
/// @tparam Derived The concrete class that implements this future.
/// @tparam T The type of the value returned by `Poll`.
template <typename Derived, typename T>
class ListableFutureWithWaker
    : public Future<ListableFutureWithWaker<Derived, T>, T>,
      public IntrusiveList<Derived>::Item {
 public:
  ListableFutureWithWaker(const ListableFutureWithWaker&) = delete;
  ListableFutureWithWaker& operator=(const ListableFutureWithWaker&) = delete;

 protected:
  /// Wrapper around a future provider pointer which also serves as a
  /// conditional lock, allowing for nullptr.
  class PW_LOCKABLE("pw::async2::experimental::ListableFutureWithWaker::Lock")
      Provider {
   public:
    void lock() PW_EXCLUSIVE_LOCK_FUNCTION() {
      if (provider_ != nullptr) {
        provider_->lock().lock();
      }
    }

    void unlock() PW_UNLOCK_FUNCTION() {
      if (provider_ != nullptr) {
        provider_->lock().unlock();
      }
    }

    Provider& operator=(ListFutureProvider<Derived>* provider) {
      provider_ = provider;
      return *this;
    }

    ListFutureProvider<Derived>* get() const { return provider_; }
    ListFutureProvider<Derived>& operator*() const {
      PW_ASSERT(provider_ != nullptr);
      return *provider_;
    }
    ListFutureProvider<Derived>* operator->() const {
      PW_ASSERT(provider_ != nullptr);
      return provider_;
    }

    explicit operator bool() const { return provider_ != nullptr; }

   private:
    friend class ListableFutureWithWaker<Derived, T>;

    explicit Provider(ListFutureProvider<Derived>* provider)
        : provider_(provider) {}

    ListFutureProvider<Derived>* provider_;
  };

  using Lock = Provider;

  /// Tag to prevent accidental default construction.
  enum ConstructedState { kMovedFrom, kReadyForCompletion };

  /// Initializes a future in an "empty" state.
  /// `state` determines the behavior as follows:
  ///
  /// - `kMovedFrom`: The constructed future appears as one which has been moved
  ///   and is marked completed. This should be used from from derived futures'
  ///   move constructors, followed by a call to `MoveFrom` to set the
  ///   appropriate base state.
  ///
  /// - `kReadyForCompletion`: The constructed future lacks a provider but is
  ///   incomplete and can still be called. This can help to construct futures
  ///   which are initially `Ready`. If a future is constructed in this state,
  ///   its `DoPend` must return `Ready`.
  explicit ListableFutureWithWaker(ConstructedState state)
      : provider_(nullptr), complete_(state == kMovedFrom) {}

  explicit ListableFutureWithWaker(ListFutureProvider<Derived>& provider)
      : provider_(&provider) {
    provider.Push(derived());
  }
  explicit ListableFutureWithWaker(SingleFutureProvider<Derived>& single)
      : ListableFutureWithWaker(single.inner_) {}

  ~ListableFutureWithWaker() {
    if (!provider_) {
      return;
    }
    std::lock_guard guard(lock());
    if (!this->unlisted()) {
      this->unlist();
    }
  }

  void MoveFrom(ListableFutureWithWaker& other) {
    complete_ = std::exchange(other.complete_, true);
    provider_ = std::exchange(other.provider_, nullptr);
    waker_ = std::move(other.waker_);

    if (provider_) {
      std::lock_guard guard(lock());
      if (!other.unlisted()) {
        this->replace(other);
      }
    }
  }

  ListFutureProvider<Derived>& provider() { return *provider_; }

  Lock& lock() { return provider_; }

  /// Wakes the task waiting on the future.
  void Wake() { std::move(waker_).Wake(); }

 private:
  using Base = Future<ListableFutureWithWaker<Derived, T>, T>;

  friend Base;
  friend ListFutureProvider<Derived>;

  Poll<T> DoPend(Context& cx) {
    static_assert(
        std::is_same_v<std::remove_extent_t<decltype(Derived::kWaitReason)>,
                       const char>,
        "kWaitReason must be a character array");

    Poll<T> poll = derived().DoPend(cx);
    if (poll.IsPending()) {
      PW_ASYNC_STORE_WAKER(cx, waker_, Derived::kWaitReason);
    }
    return poll;
  }

  void DoMarkComplete() { complete_ = true; }
  bool DoIsComplete() const { return complete_; }

  Derived& derived() { return static_cast<Derived&>(*this); }

  Provider provider_;
  Waker waker_;
  bool complete_ = false;
};

}  // namespace pw::async2::experimental
