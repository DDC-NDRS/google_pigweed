// Copyright 2024 The Pigweed Authors
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

#include <optional>

#include "pw_multibuf/multibuf.h"
#include "pw_status/status.h"

namespace pw::bluetooth::proxy {

/// Events returned from all client-facing channel objects in their `event_fn`
/// callback.
enum class L2capChannelEvent {
  /// The channel was closed by something other than `ProxyHost`. The channel is
  /// now `State::kClosed` and should be cleaned up. See logs for details.
  kChannelClosedByOther,
  /// An invalid packet was received. The channel is now `State::kStopped` and
  /// should be closed. See error logs for details.
  kRxInvalid,
  /// During Rx, the channel ran out of memory. The channel is now
  /// `State::kStopped` and should be closed.
  kRxOutOfMemory,
  /// The channel has received a packet while in the `State::kStopped` state.
  /// The channel should have been closed.
  kRxWhileStopped,
  /// `ProxyHost` has been reset. As a result, the channel is now
  /// `State::kStopped` and should be closed. (All channels are
  /// `State::kStopped` on a reset.)
  kReset,
  /// PDU recombination is not yet supported, but a fragmented L2CAP frame has
  /// been received. The channel is now `State::kStopped` and should be closed.
  // TODO: https://pwbug.dev/365179076 - Support recombination.
  kRxFragmented,
  /// Write space is now available after a previous Write on this channel
  /// returned UNAVAILABLE.
  kWriteAvailable,
};

/// Result object with status and optional MultiBuf that is only present if the
/// status is NOT `ok()`.
// `pw::Result` can't be used because it only has a value for `ok()` status.
// `std::expected` can't be used because it only has a value OR a status.
struct StatusWithMultiBuf {
  pw::Status status;
  std::optional<pw::multibuf::MultiBuf> buf = std::nullopt;
};

}  // namespace pw::bluetooth::proxy