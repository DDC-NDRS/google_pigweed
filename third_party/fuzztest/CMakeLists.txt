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

set(dir_pw_third_party_fuzztest "" CACHE PATH
    "Path to the FuzzTest installation.")
option(dir_pw_third_party_fuzztest_ADD_SUBDIRECTORY
    "Whether to add the dir_pw_third_party_fuzztest subdirectory" OFF)

# FuzzTest references RE2, but it is not needed for fuzztest_core.
# Keep this early in the file to get a better error message when
# `dir_pw_third_party_fuzztest` is unset.
add_library(pw_third_party.re2_stub INTERFACE)
add_library(re2::re2 ALIAS pw_third_party.re2_stub)

if("${dir_pw_third_party_fuzztest}" STREQUAL "")
  pw_add_error_target(pw_third_party.fuzztest
    MESSAGE
      "Attempted to build the pw_third_party.fuzztest without configuring it "
      "via dir_pw_third_party_fuzztest. "
      "See https://pigweed.dev/third_party/fuzztest."
  )
  pw_add_error_target(pw_third_party.fuzztest_gtest_main
    MESSAGE
      "Attempted to build the pw_third_party.fuzztest_gtest_main without "
      "configuring it via dir_pw_third_party_fuzztest. "
      "See https://pigweed.dev/third_party/fuzztest."
  )
  return()
endif()

if (dir_pw_third_party_fuzztest_ADD_SUBDIRECTORY)
  # Adapted from @fuzztest//CMakeLists.txt
  include("${dir_pw_third_party_fuzztest}/cmake/FuzzTestFlagSetup.cmake")
  include("${dir_pw_third_party_fuzztest}/cmake/FuzzTestHelpers.cmake")

  add_subdirectory("${dir_pw_third_party_fuzztest}/tools" third_party/fuzztest/tools)
  add_subdirectory("${dir_pw_third_party_fuzztest}/fuzztest" third_party/fuzztest/fuzztest)
endif()

# See also //pw_fuzzer:fuzztest.
add_library(pw_third_party.fuzztest INTERFACE)
target_link_libraries(pw_third_party.fuzztest
  INTERFACE
    fuzztest::fuzztest_core
    pw_unit_test
    pw_unit_test.googletest
)
target_include_directories(pw_third_party.fuzztest
  INTERFACE
    "${dir_pw_third_party_fuzztest}"
)

# See also //target/host/pw_add_test_executable.cmake.
add_library(pw_third_party.fuzztest_gtest_main INTERFACE)
target_link_libraries(pw_third_party.fuzztest_gtest_main
  INTERFACE
    fuzztest::fuzztest_gtest_main
)
