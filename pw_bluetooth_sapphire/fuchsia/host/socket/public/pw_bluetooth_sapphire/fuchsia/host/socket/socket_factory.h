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

#include <lib/async/dispatcher.h>
#include <lib/zx/socket.h>
#include <pw_assert/check.h>
#include <zircon/status.h>

#include <memory>
#include <unordered_map>

#include "pw_bluetooth_sapphire/internal/host/common/log.h"
#include "pw_bluetooth_sapphire/internal/host/common/macros.h"
#include "pw_bluetooth_sapphire/internal/host/common/weak_self.h"
#include "socket_channel_relay.h"

namespace bt::socket {

// A SocketFactory vends zx::socket objects that an IPC peer can use to
// communicate with l2cap::Channels.
//
// Over time, the factory may grow more responsibility and intelligence. For
// example, the factory might manage QoS by configuring the number of packets a
// SocketChannelRelay can process before yielding control back to the
// dispatcher.
//
// THREAD-SAFETY: This class is thread-hostile. An instance must be
// created and destroyed on a single thread. Said thread must have a
// single-threaded dispatcher. Failure to follow those rules may cause the
// program to abort.
template <typename ChannelT>
class SocketFactory final {
 public:
  SocketFactory();
  ~SocketFactory();

  // Creates a zx::socket which can be used to read from, and write to,
  // |channel|.
  //
  // |channel| will automatically be Deactivated() when the zx::socket is
  // closed, or the creation thread's dispatcher shuts down.
  //
  // |closed_callback| will be called when the channel or socket is closed. This
  // callback may be nullptr to ignore closures.
  //
  // Similarly, the local end corresponding to the returned zx::socket will
  // automatically be closed when |channel| is closed, or the creation thread's
  // dispatcher  shuts down.
  //
  // It is an error to call MakeSocketForChannel() multiple times for
  // the same Channel.
  //
  // Returns the new socket on success, and an invalid socket otherwise
  // (including if |channel| is nullptr).
  zx::socket MakeSocketForChannel(
      typename ChannelT::WeakPtr channel,
      fit::callback<void()> closed_callback = nullptr);

 private:
  using RelayT = SocketChannelRelay<ChannelT>;
  using ChannelIdT = typename ChannelT::UniqueId;

  // TODO(fxbug.dev/42145980): Figure out what we need to do handle the
  // possibility that a channel id is recycled. (See comment in
  // LogicalLink::HandleRxPacket.)
  std::unordered_map<ChannelIdT, std::unique_ptr<RelayT>> channel_to_relay_;

  WeakSelf<SocketFactory> weak_self_;  // Keep last.

  BT_DISALLOW_COPY_AND_ASSIGN_ALLOW_MOVE(SocketFactory);
};

template <typename ChannelT>
SocketFactory<ChannelT>::SocketFactory() : weak_self_(this) {}

template <typename ChannelT>
SocketFactory<ChannelT>::~SocketFactory() {}

template <typename ChannelT>
zx::socket SocketFactory<ChannelT>::MakeSocketForChannel(
    typename ChannelT::WeakPtr channel, fit::callback<void()> closed_callback) {
  if (!channel.is_alive()) {
    return zx::socket();
  }

  const auto unique_id = channel->unique_id();
  if (channel_to_relay_.find(unique_id) != channel_to_relay_.end()) {
    bt_log(ERROR,
           "l2cap",
           "channel %u is already bound to a socket",
           channel->id());
    return zx::socket();
  }

  zx::socket local_socket, remote_socket;
  const auto status =
      zx::socket::create(ZX_SOCKET_DATAGRAM, &local_socket, &remote_socket);
  if (status != ZX_OK) {
    bt_log(ERROR,
           "data",
           "Failed to create socket for channel %u: %s",
           channel->unique_id(),
           zx_status_get_string(status));
    return zx::socket();
  }

  auto relay = std::make_unique<RelayT>(
      std::move(local_socket),
      channel,
      typename RelayT::DeactivationCallback([self = weak_self_.GetWeakPtr(),
                                             id = unique_id,
                                             closed_cb = std::move(
                                                 closed_callback)]() mutable {
        PW_DCHECK(self.is_alive(), "(unique_id=%u)", id);
        size_t n_erased = self->channel_to_relay_.erase(id);
        PW_DCHECK(n_erased == 1, "(n_erased=%zu, unique_id=%u)", n_erased, id);

        if (closed_cb) {
          closed_cb();
        }
      }));

  // Note: Activate() may abort, if |channel| has been Activated() without
  // going through this SocketFactory.
  if (!relay->Activate()) {
    bt_log(ERROR,
           "l2cap",
           "Failed to Activate() relay for channel %u",
           channel->id());
    return zx::socket();
  }

  channel_to_relay_.emplace(unique_id, std::move(relay));
  return remote_socket;
}

}  // namespace bt::socket
