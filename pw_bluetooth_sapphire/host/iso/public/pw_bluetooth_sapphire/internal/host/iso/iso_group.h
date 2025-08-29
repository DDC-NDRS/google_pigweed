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

#include <unordered_map>

#include "pw_bluetooth_sapphire/internal/host/hci-spec/constants.h"
#include "pw_bluetooth_sapphire/internal/host/hci-spec/protocol.h"
#include "pw_bluetooth_sapphire/internal/host/iso/iso_common.h"
#include "pw_bluetooth_sapphire/internal/host/iso/iso_stream.h"
#include "pw_bluetooth_sapphire/internal/host/transport/transport.h"

namespace bt::iso {
class IsoGroup {
 public:
  virtual ~IsoGroup() = default;

  using OnClosedCallback = pw::Callback<void(IsoGroup&)>;
  static std::unique_ptr<IsoGroup> CreateCig(
      hci_spec::CigIdentifier id,
      hci::Transport::WeakPtr hci,
      CigStreamCreator::WeakPtr cig_stream_creator,
      OnClosedCallback on_closed_callback);

  [[nodiscard]] hci_spec::CigIdentifier id() const { return id_; }
  const std::unordered_map<hci_spec::CisIdentifier, IsoStream::WeakPtr>&
  streams() const {
    return streams_;
  }

  using WeakPtr = WeakSelf<IsoGroup>::WeakPtr;
  virtual WeakPtr GetWeakPtr() = 0;

 protected:
  IsoGroup(hci_spec::CigIdentifier id,
           hci::Transport::WeakPtr hci,
           CigStreamCreator::WeakPtr cig_stream_creator,
           OnClosedCallback on_closed_callback)
      : id_(id),
        hci_(std::move(hci)),
        cig_stream_creator_(std::move(cig_stream_creator)),
        on_closed_callback_(std::move(on_closed_callback)) {}

  const hci_spec::CigIdentifier id_;
  hci::Transport::WeakPtr hci_;

  CigStreamCreator::WeakPtr cig_stream_creator_;
  std::unordered_map<hci_spec::CisIdentifier, IsoStream::WeakPtr> streams_;

  OnClosedCallback on_closed_callback_;
};

}  // namespace bt::iso
