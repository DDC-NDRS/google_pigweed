# Copyright 2025 The Pigweed Authors
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

if(NOT CONFIG_ZEPHYR_PIGWEED_MODULE)
  return()
endif()

pw_add_library(pw_i2c_zephyr STATIC
  HEADERS
    public/pw_i2c_zephyr/initiator.h
  PUBLIC_INCLUDES
    public
  SOURCES
    initiator.cc
  PUBLIC_DEPS
    pw_i2c.initiator
    pw_status
    pw_sync.lock_annotations
    pw_sync.mutex
    zephyr_interface
)
if(CONFIG_PIGWEED_I2C)
  zephyr_append_cmake_library(pw_i2c_zephyr)
endif()

pw_add_test(pw_i2c_zephyr.initiator_test
  SOURCES
    initiator_test.cc
  PRIVATE_DEPS
    pw_i2c_zephyr
    pw_function
    pw_unit_test
    zephyr_interface
  GROUPS
    modules
    pw_i2c_zephyr
)
