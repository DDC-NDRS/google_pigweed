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

#include "pw_bluetooth_proxy/internal/l2cap_channel.h"

#include <mutex>

#include "pw_bluetooth/emboss_util.h"
#include "pw_bluetooth/hci_data.emb.h"
#include "pw_bluetooth/hci_h4.emb.h"
#include "pw_bluetooth/l2cap_frames.emb.h"
#include "pw_bluetooth_proxy/internal/l2cap_channel_manager.h"
#include "pw_log/log.h"
#include "pw_status/status.h"
#include "pw_status/try.h"

namespace pw::bluetooth::proxy {

L2capChannel::L2capChannel(L2capChannel&& other)
    : l2cap_channel_manager_(other.l2cap_channel_manager_),
      connection_handle_(other.connection_handle()),
      transport_(other.transport()),
      local_cid_(other.local_cid()),
      remote_cid_(other.remote_cid()),
      payload_from_controller_fn_(
          std::move(other.payload_from_controller_fn_)) {
  // All L2capChannels share a static mutex, so only one lock needs to be
  // acquired here.
  // TODO: https://pwbug.dev/381942905 - Once mutex no longer needs to be
  // static, elide this operator or acquire a lock on both channels' mutexes.
  std::lock_guard lock(global_send_queue_mutex_);
  send_queue_ = std::move(other.send_queue_);
  l2cap_channel_manager_.ReleaseChannel(other);
  l2cap_channel_manager_.RegisterChannel(*this);
}

L2capChannel& L2capChannel::operator=(L2capChannel&& other) {
  if (this == &other) {
    return *this;
  }
  l2cap_channel_manager_.ReleaseChannel(*this);
  connection_handle_ = other.connection_handle();
  transport_ = other.transport();
  local_cid_ = other.local_cid();
  remote_cid_ = other.remote_cid();
  payload_from_controller_fn_ = std::move(other.payload_from_controller_fn_);
  // All L2capWriteChannels share a static mutex, so only one lock needs to be
  // acquired here.
  // TODO: https://pwbug.dev/369849508 - Once mutex is no longer static,
  // elide this operator or acquire a lock on both channels' mutexes.
  std::lock_guard lock(global_send_queue_mutex_);
  send_queue_ = std::move(other.send_queue_);
  l2cap_channel_manager_.ReleaseChannel(other);
  l2cap_channel_manager_.RegisterChannel(*this);
  return *this;
}

L2capChannel::~L2capChannel() {
  l2cap_channel_manager_.ReleaseChannel(*this);
  ClearQueue();
}

Status L2capChannel::QueuePacket(H4PacketWithH4&& packet) {
  Status status;
  {
    std::lock_guard lock(global_send_queue_mutex_);
    if (send_queue_.full()) {
      status = Status::Unavailable();
    } else {
      send_queue_.push(std::move(packet));
      status = OkStatus();
    }
  }
  ReportPacketsMayBeReadyToSend();
  return status;
}

std::optional<H4PacketWithH4> L2capChannel::DequeuePacket() {
  std::lock_guard lock(global_send_queue_mutex_);
  if (send_queue_.empty()) {
    return std::nullopt;
  }
  H4PacketWithH4 packet = std::move(send_queue_.front());
  send_queue_.pop();
  return packet;
}

void L2capChannel::OnFragmentedPduReceived() {
  PW_LOG_ERROR(
      "(CID 0x%X) Fragmented L2CAP frame received, which is not yet supported.",
      local_cid());
}

L2capChannel::L2capChannel(
    L2capChannelManager& l2cap_channel_manager,
    uint16_t connection_handle,
    AclTransportType transport,
    uint16_t local_cid,
    uint16_t remote_cid,
    pw::Function<void(pw::span<uint8_t> payload)>&& payload_from_controller_fn)
    : l2cap_channel_manager_(l2cap_channel_manager),
      connection_handle_(connection_handle),
      transport_(transport),
      local_cid_(local_cid),
      remote_cid_(remote_cid),
      payload_from_controller_fn_(std::move(payload_from_controller_fn)) {
  l2cap_channel_manager_.RegisterChannel(*this);
}

bool L2capChannel::AreValidParameters(uint16_t connection_handle,
                                      uint16_t local_cid,
                                      uint16_t remote_cid) {
  if (connection_handle > kMaxValidConnectionHandle) {
    PW_LOG_ERROR(
        "Invalid connection handle 0x%X. Maximum connection handle is 0x0EFF.",
        connection_handle);
    return false;
  }
  if (local_cid == 0 || remote_cid == 0) {
    PW_LOG_ERROR("L2CAP channel identifier 0 is not valid.");
    return false;
  }
  return true;
}

pw::Result<H4PacketWithH4> L2capChannel::PopulateTxL2capPacket(
    uint16_t data_length) {
  const size_t l2cap_packet_size =
      emboss::BasicL2capHeader::IntrinsicSizeInBytes() + data_length;
  const size_t acl_packet_size =
      emboss::AclDataFrameHeader::IntrinsicSizeInBytes() + l2cap_packet_size;
  const size_t h4_packet_size = sizeof(emboss::H4PacketType) + acl_packet_size;

  pw::Result<H4PacketWithH4> h4_packet_res =
      l2cap_channel_manager_.GetTxH4Packet(h4_packet_size);
  if (!h4_packet_res.ok()) {
    return h4_packet_res.status();
  }
  H4PacketWithH4 h4_packet = std::move(h4_packet_res.value());
  h4_packet.SetH4Type(emboss::H4PacketType::ACL_DATA);

  PW_TRY_ASSIGN(
      auto acl,
      MakeEmbossWriter<emboss::AclDataFrameWriter>(h4_packet.GetHciSpan()));
  acl.header().handle().Write(connection_handle_);
  // TODO: https://pwbug.dev/360932103 - Support packet segmentation, so this
  // value will not always be FIRST_NON_FLUSHABLE.
  acl.header().packet_boundary_flag().Write(
      emboss::AclDataPacketBoundaryFlag::FIRST_NON_FLUSHABLE);
  acl.header().broadcast_flag().Write(
      emboss::AclDataPacketBroadcastFlag::POINT_TO_POINT);
  acl.data_total_length().Write(l2cap_packet_size);

  PW_TRY_ASSIGN(auto l2cap_header,
                MakeEmbossWriter<emboss::BasicL2capHeaderWriter>(
                    acl.payload().BackingStorage().data(),
                    emboss::BasicL2capHeader::IntrinsicSizeInBytes()));
  l2cap_header.pdu_length().Write(data_length);
  l2cap_header.channel_id().Write(remote_cid_);

  return h4_packet;
}

uint16_t L2capChannel::MaxL2capPayloadSize() const {
  return l2cap_channel_manager_.GetH4BuffSize() - sizeof(emboss::H4PacketType) -
         emboss::AclDataFrameHeader::IntrinsicSizeInBytes() -
         emboss::BasicL2capHeader::IntrinsicSizeInBytes();
}

void L2capChannel::ReportPacketsMayBeReadyToSend() {
  l2cap_channel_manager_.DrainChannelQueues();
}

void L2capChannel::ClearQueue() {
  std::lock_guard lock(global_send_queue_mutex_);
  send_queue_.clear();
}

}  // namespace pw::bluetooth::proxy
