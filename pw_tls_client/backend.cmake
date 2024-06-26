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

include_guard(GLOBAL)

include($ENV{PW_ROOT}/pw_build/pigweed.cmake)

# Backends for the pw_tls_client module.
pw_add_backend_variable(pw_tls_client.pw_tls_client_BACKEND)
pw_add_backend_variable(pw_tls_client.time_BACKEND
  DEFAULT_BACKEND
    pw_chrono.wrap_time_build_time
)
