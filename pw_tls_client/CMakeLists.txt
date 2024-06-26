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
include($ENV{PW_ROOT}/pw_tls_client/backend.cmake)

pw_add_facade(pw_tls_client.pw_tls_client INTERFACE
  BACKEND
    pw_tls_client.pw_tls_client_BACKEND
  HEADERS
    public/pw_tls_client/options.h
    public/pw_tls_client/session.h
    public/pw_tls_client/status.h
  PUBLIC_INCLUDES
    public
  PUBLIC_DEPS
    pw_assert
    pw_bytes
    pw_result
    pw_status
    pw_stream
    pw_string
)

pw_add_facade(pw_tls_client.time INTERFACE
  BACKEND
    pw_tls_client.time_BACKEND
)
