# Copyright 2024 The Pigweed Authors
#
# Licensed under the Apache License, Version 2.0 (the "License"); you may not
# use this file except in compliance with the License. You may obtain a copy of
# the License at
#
#     https://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
# WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
# License for the specific language governing permissions and limitations under
# the License.

include($ENV{PW_ROOT}/pw_build/pigweed.cmake)

###############################################################################
##          Everything below here is intended to be emboss only              ##
##          and will be skipped if emboss isn't enabled.                     ##
###############################################################################
if("${dir_pw_third_party_emboss}" STREQUAL "")
  # Skip emboss defs if it's not configured
  return()
endif()

pw_add_library(pw_bluetooth_proxy STATIC

# LINT.IfChange
  HEADERS
    public/pw_bluetooth_proxy/basic_l2cap_channel.h
    public/pw_bluetooth_proxy/channel_proxy.h
    public/pw_bluetooth_proxy/direction.h
    public/pw_bluetooth_proxy/gatt_notify_channel.h
    public/pw_bluetooth_proxy/h4_packet.h
    public/pw_bluetooth_proxy/internal/acl_data_channel.h
    public/pw_bluetooth_proxy/internal/gatt_notify_channel_internal.h
    public/pw_bluetooth_proxy/internal/h4_storage.h
    public/pw_bluetooth_proxy/internal/hci_transport.h
    public/pw_bluetooth_proxy/internal/l2cap_aclu_signaling_channel.h
    public/pw_bluetooth_proxy/internal/l2cap_channel_manager.h
    public/pw_bluetooth_proxy/internal/l2cap_channel.h
    public/pw_bluetooth_proxy/internal/l2cap_coc_internal.h
    public/pw_bluetooth_proxy/internal/l2cap_leu_signaling_channel.h
    public/pw_bluetooth_proxy/internal/l2cap_signaling_channel.h
    public/pw_bluetooth_proxy/internal/l2cap_status_tracker.h
    public/pw_bluetooth_proxy/internal/locked_l2cap_channel.h
    public/pw_bluetooth_proxy/internal/logical_transport.h
    public/pw_bluetooth_proxy/internal/recombiner.h
    public/pw_bluetooth_proxy/internal/rfcomm_fcs.h
    public/pw_bluetooth_proxy/l2cap_channel_common.h
    public/pw_bluetooth_proxy/l2cap_coc.h
    public/pw_bluetooth_proxy/l2cap_status_delegate.h
    public/pw_bluetooth_proxy/proxy_host.h
    public/pw_bluetooth_proxy/rfcomm_channel.h
    public/pw_bluetooth_proxy/single_channel_proxy.h
# LINT.ThenChange(BUILD.bazel, BUILD.gn)

  PUBLIC_INCLUDES
    public

# LINT.IfChange
  PUBLIC_DEPS
    pw_allocator.best_fit
    pw_allocator.synchronized_allocator
    pw_bluetooth.emboss_att
    pw_bluetooth.emboss_hci_commands
    pw_bluetooth.emboss_hci_common
    pw_bluetooth.emboss_hci_data
    pw_bluetooth.emboss_hci_events
    pw_bluetooth.emboss_hci_h4
    pw_bluetooth.emboss_l2cap_frames
    pw_bluetooth.emboss_rfcomm_frames
    pw_bluetooth.emboss_util
    pw_containers
    pw_function
    pw_log
    pw_multibuf
    pw_multibuf.simple_allocator
    pw_result
    pw_span
    pw_span.cast
    pw_status
    pw_sync.lock_annotations
    pw_sync.mutex
  SOURCES
    acl_data_channel.cc
    basic_l2cap_channel.cc
    channel_proxy.cc
    gatt_notify_channel.cc
    h4_storage.cc
    l2cap_aclu_signaling_channel.cc
    l2cap_channel_manager.cc
    l2cap_channel.cc
    l2cap_coc.cc
    l2cap_leu_signaling_channel.cc
    l2cap_signaling_channel.cc
    l2cap_status_tracker.cc
    proxy_host.cc
    recombiner.cc
    rfcomm_channel.cc
    rfcomm_fcs.cc
    single_channel_proxy.cc
# LINT.ThenChange(Android.bp, BUILD.bazel, BUILD.gn)

)

pw_add_library(pw_bluetooth_proxy.test_utils STATIC

# LINT.IfChange
  HEADERS
    pw_bluetooth_proxy_private/test_utils.h
  PUBLIC_DEPS
    pw_bluetooth_proxy
    pw_bluetooth.emboss_hci_commands
    pw_bluetooth.emboss_hci_common
    pw_bluetooth.emboss_hci_events
    pw_bluetooth.emboss_hci_h4
    pw_multibuf.testing
    pw_third_party.fuchsia.stdcompat
    pw_unit_test
  SOURCES
    test_utils.cc
# LINT.ThenChange(Android.bp, BUILD.bazel, BUILD.gn)

)

pw_add_test(pw_bluetooth_proxy.pw_bluetooth_proxy_test

# LINT.IfChange
  PRIVATE_DEPS
    pw_bluetooth_proxy
    pw_bluetooth_proxy.test_utils
    pw_bluetooth.emboss_att
    pw_bluetooth.emboss_hci_commands
    pw_bluetooth.emboss_hci_common
    pw_bluetooth.emboss_hci_events
    pw_bluetooth.emboss_hci_h4
    pw_multibuf
    pw_third_party.fuchsia.stdcompat
  SOURCES
    channel_proxy_test.cc
    gatt_notify_test.cc
    h4_packet_test.cc
    l2cap_coc_test.cc
    proxy_host_test.cc
    recombiner_test.cc
    rfcomm_fcs_test.cc
    rfcomm_test.cc
    utils_test.cc
# LINT.ThenChange(BUILD.bazel, BUILD.gn)

  GROUPS
    modules
)


