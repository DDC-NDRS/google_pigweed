// Copyright 2021 The Pigweed Authors
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

#include <array>
#include <cstdint>

#include "pw_containers/inline_queue.h"
#include "pw_function/function.h"
#include "pw_metric/metric.h"
#include "pw_span/span.h"
#include "pw_status/status.h"
#include "pw_sync/interrupt_spin_lock.h"
#include "pw_sync/lock_annotations.h"
#include "pw_sync/thread_notification.h"
#include "pw_thread/thread_core.h"

namespace pw::work_queue {

using WorkItem = Function<void()>;

/// Enables threads and interrupts to enqueue work as a
/// `pw::work_queue::WorkItem` for execution by the work queue.
///
/// **Queue sizing**: The number of outstanding work requests is limited
/// based on the internal queue size. The queue size is set through either
/// the size of the `queue_storage` buffer passed into the constructor or by
/// using the templated `pw::work_queue::WorkQueueWithBuffer` helper. When the
/// queue is full, the queue will not accept further work.
///
/// **Cooperative thread cancellation**: The class is a
/// `pw::thread::ThreadCore`, meaning it should be executed as a single thread.
/// To facilitate clean shutdown, it provides a `RequestStop()` method for
/// cooperative cancellation which should be invoked before joining the thread.
/// Once a stop has been requested the queue will no longer accept further work.
///
/// The entire API is thread-safe and interrupt-safe.
class WorkQueue : public thread::ThreadCore {
 public:
  /// @param[in] queue The work entries to enqueue.
  ///
  /// @param[in] queue_capacity The internal queue size which limits the number
  /// of outstanding work requests.
  ///
  /// @note The `ThreadNotification` prevents this from being `constexpr`.
  WorkQueue(InlineQueue<WorkItem>& queue, size_t queue_capacity)
      : stop_requested_(false), queue_(queue) {
    min_queue_remaining_.Set(static_cast<uint32_t>(queue_capacity));
  }

  /// Enqueues a `work_item` for execution by the work queue thread.
  ///
  /// @param[in] work_item The entry to enqueue.
  ///
  /// @returns @rst
  ///
  /// .. pw-status-codes::
  ///
  ///    OK: Success. Entry was enqueued for execution.
  ///
  ///    FAILED_PRECONDITION: The work queue is shutting down. Entries are no
  ///    longer permitted.
  ///
  ///    RESOURCE_EXHAUSTED: Internal work queue is full. Entry was not
  ///    enqueued.
  ///
  /// @endrst
  Status PushWork(WorkItem&& work_item) PW_LOCKS_EXCLUDED(lock_) {
    return InternalPushWork(std::move(work_item));
  }

  /// Queues work for execution. Crashes if the work cannot be queued due to a
  /// full queue or a stopped worker thread.
  ///
  /// This call is recommended where possible since it saves error handling code
  /// at the callsite; and in many practical cases, it is a bug if the work
  /// queue is full (and so a crash is useful to detect the problem).
  ///
  /// @param[in] work_item The entry to enqueue.
  ///
  /// @pre
  /// * The queue must not overflow, i.e. be full.
  /// * The queue must not have been requested to stop, i.e. it must
  ///   not be in the process of shutting down.
  void CheckPushWork(WorkItem&& work_item) PW_LOCKS_EXCLUDED(lock_);

  /// Locks the queue to prevent further work enqueing, finishes outstanding
  /// work, then shuts down the worker thread.
  ///
  /// The `WorkQueue` cannot be resumed after stopping because the `ThreadCore`
  /// thread returns and may be joined. The `WorkQueue` must be reconstructed
  /// for re-use after the thread has been joined.
  void RequestStop() PW_LOCKS_EXCLUDED(lock_);

 private:
  void Run() override PW_LOCKS_EXCLUDED(lock_);
  Status InternalPushWork(WorkItem&& work_item) PW_LOCKS_EXCLUDED(lock_);

  sync::InterruptSpinLock lock_;
  bool stop_requested_ PW_GUARDED_BY(lock_);
  InlineQueue<WorkItem>& queue_ PW_GUARDED_BY(lock_);
  sync::ThreadNotification work_notification_;

  // TODO(ewout): The group and/or its name token should be passed as a ctor
  // arg instead. Depending on the approach here the group should be exposed
  // While doing this evaluate whether perhaps we should instead construct
  // TypedMetric<uint32_t>s directly, avoiding the macro usage given the
  // min_queue_remaining_ initial value requires dependency injection.
  // And lastly when the restructure is finalized add unit tests to ensure these
  // metrics work as intended.
  PW_METRIC_GROUP(metrics_, "pw::work_queue::WorkQueue");
  PW_METRIC(metrics_, max_queue_used_, "max_queue_used", 0u);
  PW_METRIC(metrics_, min_queue_remaining_, "min_queue_remaining", 0u);
};

template <size_t kWorkQueueEntries>
class WorkQueueWithBuffer : public WorkQueue {
 public:
  constexpr WorkQueueWithBuffer() : WorkQueue(queue_, kWorkQueueEntries) {}

 private:
  InlineQueue<WorkItem, kWorkQueueEntries> queue_;
};

}  // namespace pw::work_queue
