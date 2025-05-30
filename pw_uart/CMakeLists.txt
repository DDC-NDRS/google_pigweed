# Copyright 2023 The Pigweed Authors
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

pw_add_library(pw_uart.uart_base INTERFACE
  HEADERS
    public/pw_uart/uart_base.h
  PUBLIC_INCLUDES
    public
  PUBLIC_DEPS
    pw_status
)

pw_add_library(pw_uart.uart INTERFACE
  HEADERS
    public/pw_uart/uart.h
  PUBLIC_INCLUDES
    public
  PUBLIC_DEPS
    pw_assert
    pw_bytes
    pw_chrono.system_clock
    pw_status
    pw_span
    pw_uart.uart_base
)

pw_add_library(pw_uart.uart_non_blocking INTERFACE
  HEADERS
    public/pw_uart/uart_non_blocking.h
  PUBLIC_INCLUDES
    public
  PUBLIC_DEPS
    pw_assert
    pw_bytes
    pw_function
    pw_status
    pw_span
    pw_uart.uart_base
)

pw_add_library(pw_uart.blocking_adapater STATIC
  HEADERS
    public/pw_uart/blocking_adapter.h
  PUBLIC_INCLUDES
    public
  PUBLIC_DEPS
    pw_assert
    pw_log
    pw_status
    pw_sync.timed_thread_notification
    pw_uart.uart
    pw_uart.uart_non_blocking
  SOURCES
    blocking_adapter.cc
)

pw_add_library(pw_uart.stream INTERFACE
  HEADERS
    public/pw_uart/stream.h
  PUBLIC_INCLUDES
    public
  PUBLIC_DEPS
    pw_stream
    pw_uart.uart
)
