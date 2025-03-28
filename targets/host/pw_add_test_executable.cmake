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

include("$ENV{PW_ROOT}/pw_build/pigweed.cmake")
include("$ENV{PW_ROOT}/pw_unit_test/test.cmake")

# Used by pw_add_test to instantiate unit test executables for the host.
#
# Required Args:
#
#   NAME: name for the desired executable
#   TEST_DEP: the target which provides the tests for this executable
#   TEST_MAIN: The test main to use, can be "" to use defaults.
#
function(pw_add_test_executable_with_main NAME TEST_DEP TEST_MAIN)
  pw_parse_arguments(
    NUM_POSITIONAL_ARGS
      3
  )

  # CMake requires a source file to determine the LINKER_LANGUAGE.
  add_executable("${NAME}" EXCLUDE_FROM_ALL
                 $<TARGET_PROPERTY:pw_build.empty,SOURCES>)

  set(test_backend "${pw_unit_test_BACKEND}")
  if ("${TEST_MAIN}" STREQUAL "")
    if("${test_backend}" STREQUAL "pw_unit_test.light")
      set(main pw_unit_test.logging_main)
    elseif("${test_backend}" STREQUAL "pw_unit_test.googletest")
      set(main pw_third_party.googletest.gmock_main)
    elseif("${test_backend}" STREQUAL "pw_unit_test.fuzztest")
      set(main pw_third_party.fuzztest_gtest_main)
    else()
      message(FATAL_ERROR
              "Unsupported test backend selected for host test executables")
    endif()
  else()
    set(main ${TEST_MAIN})
  endif()

  pw_target_link_targets("${NAME}"
    PRIVATE
      "${main}"
      "${TEST_DEP}"
  )
endfunction(pw_add_test_executable_with_main)

# Used by pw_add_test to instantiate unit test executables for the host.
#
# Required Args:
#
#   NAME: name for the desired executable
#   TEST_DEP: the target which provides the tests for this executable
#
function(pw_add_test_executable NAME TEST_DEP)
  pw_add_test_executable_with_main(${NAME} ${TEST_DEP} "")
endfunction(pw_add_test_executable)
