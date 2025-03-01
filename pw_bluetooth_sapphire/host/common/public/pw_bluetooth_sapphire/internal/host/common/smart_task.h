// Copyright 2023 The Pigweed Authors
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
#include <pw_assert/check.h>
#include <pw_async/dispatcher.h>
#include <pw_async/task.h>

namespace bt {

// SmartTask is a utility that wraps a pw::async::Task and adds features like
// cancelation upon destruction and state tracking. It is not thread safe, and
// should only be used on the same thread that the dispatcher is running on.
class SmartTask {
 public:
  explicit SmartTask(pw::async::Dispatcher& dispatcher,
                     pw::async::TaskFunction&& func = nullptr)
      : dispatcher_(dispatcher), func_(std::move(func)) {}

  ~SmartTask() {
    if (pending_) {
      PW_CHECK(Cancel());
    }
  }

  void PostAt(pw::chrono::SystemClock::time_point time) {
    pending_ = true;
    dispatcher_.PostAt(task_, time);
  }

  void PostAfter(pw::chrono::SystemClock::duration delay) {
    pending_ = true;
    dispatcher_.PostAfter(task_, delay);
  }

  void Post() {
    pending_ = true;
    dispatcher_.Post(task_);
  }

  bool Cancel() {
    pending_ = false;
    return dispatcher_.Cancel(task_);
  }

  void set_function(pw::async::TaskFunction&& func) { func_ = std::move(func); }

  bool is_pending() const { return pending_; }

  pw::async::Dispatcher& dispatcher() const { return dispatcher_; }

 private:
  pw::async::Dispatcher& dispatcher_;
  pw::async::Task task_{[this](pw::async::Context& ctx, pw::Status status) {
    pending_ = false;
    if (func_) {
      func_(ctx, status);
    }
  }};
  pw::async::TaskFunction func_ = nullptr;
  bool pending_ = false;
};

}  // namespace bt
