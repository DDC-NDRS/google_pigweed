# Copyright 2022 The Pigweed Authors
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
include_guard(GLOBAL)

include($ENV{PW_ROOT}/pw_build/pigweed.cmake)

# The pw_system backend that provides the system RPC server.
pw_add_backend_variable(pw_system.rpc_server_BACKEND)

# The pw_system backend that provides the system target hooks.
pw_add_backend_variable(pw_system.target_hooks_BACKEND)

pw_add_backend_variable(pw_system.device_handler_BACKEND
  DEFAULT_BACKEND
    pw_system.unknown_device_handler
)
pw_add_backend_variable(pw_system.io_BACKEND
  DEFAULT_BACKEND
    pw_system.sys_io_target_io
)
