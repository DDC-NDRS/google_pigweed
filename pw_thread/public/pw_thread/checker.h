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

#include <atomic>
#include <optional>

#include "pw_sync/lock_annotations.h"
#include "pw_thread/config.h"
#include "pw_thread/thread.h"

namespace pw {

/// @module{pw_thread}

/// A BaseLockable class that stores the id of a thread and verifies that all
/// `lock()` calls happen on that thread if
/// `PW_THREAD_CHECKER_RUNTIME_ASSERT_ENABLED` config is set.
///
/// Its purpose is to provide a check that data that is meant to only be
/// accessed from a single thread is always accessed from that thread. This
/// is useful on data that isn't synchronized using regular sync primitives.
/// For example, this could be used on data that is always used on an async
/// dispatcher to ensure all data access happens on that dispatcher thread.
///
/// In addition to providing an optional runtime check, this class can be used
/// with static thread safety analysis to ensure that resources are accessed in
/// a context that is checked.
///
/// ```
/// class MyClass {
///  public:
///    // Provide thread id that Foo will be called on. If you don't know the
///    // id, consider the `LazyInitThreadChecker` variant.
///    MyClass(pw::Thread::id thread_to_check)
///      : thread_checker_(thread_to_check) {}
///    void Foo() {
///      std::lock_guard checker(thread_checker_);
///      resource_ = 0;
///    }
///  private:
///    pw::ThreadChecker thread_checker_;
///    int resource_ PW_GUARDED_BY(thread_checker_);
/// };
/// ```
class PW_LOCKABLE("pw::ThreadChecker") ThreadChecker {
 public:
  explicit ThreadChecker(Thread::id id) : self_(id) {}

  // Implementation of the BaseLockable requirement
  void lock() PW_EXCLUSIVE_LOCK_FUNCTION() {
#if PW_THREAD_CHECKER_RUNTIME_ASSERT_ENABLED
    // Hitting this assert means that access to this checker from multiple
    // threads was detected.
    PW_ASSERT(pw::this_thread::get_id() == self_);
#endif  // PW_THREAD_CHECKER_RUNTIME_ASSERT_ENABLED
  }
  void unlock() PW_UNLOCK_FUNCTION() {}

 private:
  Thread::id self_;
};

/// A BaseLockable class that works like `ThreadChecker` but only initializes
/// its thread id when first locked instead of construction time.
///
/// This is useful for cases where you may not have access to right thread id at
/// construction time, but still want to assert fields are always accessed on
/// the same thread.
///
/// Note, this class shouldn't be locked outside of a normal thread context.
/// It's not valid to use it interrupt context or before the scheduler is
/// started.
///
/// ```
/// class MyClass {
///  public:
///    void Foo() {
///      std::lock_guard checker(thread_checker_);
///      resource_ = 0;
///    }
///  private:
///    pw::LazyInitThreadChecker thread_checker_;
///    int resource_ PW_GUARDED_BY(thread_checker_);
/// };
/// ```
class PW_LOCKABLE("pw::LazyInitThreadChecker") LazyInitThreadChecker {
 public:
  constexpr LazyInitThreadChecker()
#if PW_THREAD_CHECKER_RUNTIME_ASSERT_ENABLED
      : thread_id_()
#endif
  {
  }

  // Implementation of the BaseLockable requirement
  void lock() PW_EXCLUSIVE_LOCK_FUNCTION() {
#if PW_THREAD_CHECKER_RUNTIME_ASSERT_ENABLED
    Thread::id stored_id = thread_id_.load(std::memory_order_acquire);
    if (stored_id == Thread::id()) {
      // The checker has not been initialized yet. Attempt to claim it for the
      // current thread.
      const Thread::id current_id = pw::this_thread::get_id();
      // A default constructed id which is used as the expected value.
      Thread::id uninitialized_id;
      // Attempt to swap the uninitialized ID with the current thread's ID.
      // This is done with release semantics to ensure that all prior writes
      // on this thread are visible on other threads.
      if (thread_id_.compare_exchange_strong(
              uninitialized_id, current_id, std::memory_order_acq_rel)) {
        // This thread successfully claimed the checker.
        stored_id = current_id;
      } else {
        // Another thread claimed the checker. `uninitialized_id` is now updated
        // with the stored value.
        stored_id = uninitialized_id;
      }
    }

    // Hitting this assert means that access to this checker from multiple
    // threads was detected.
    PW_ASSERT(pw::this_thread::get_id() == stored_id);
#endif  // PW_THREAD_CHECKER_RUNTIME_ASSERT_ENABLED
  }
  void unlock() PW_UNLOCK_FUNCTION() {}

 private:
#if PW_THREAD_CHECKER_RUNTIME_ASSERT_ENABLED
  std::atomic<Thread::id> thread_id_;
#endif  // PW_THREAD_CHECKER_RUNTIME_ASSERT_ENABLED
};

}  // namespace pw
