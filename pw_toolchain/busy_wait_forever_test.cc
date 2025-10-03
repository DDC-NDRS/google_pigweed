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

#include "pw_toolchain/busy_wait_forever.h"

#include "pw_unit_test/framework.h"

extern "C" int pw_TestBusyWaitForeverC(int loop_infinitely_if_0);

namespace {

int TestBusyWaitForever(int loop_infinitely_if_0) {
  if (loop_infinitely_if_0 != 0) {
    return loop_infinitely_if_0;
  }
  // No return statement needed because of infinite loop.
  pw::BusyWaitForever();
}

TEST(BusyWaitForever, Compiles) {
  EXPECT_EQ(TestBusyWaitForever(123), 123);

  if (false) {
    EXPECT_EQ(TestBusyWaitForever(0), 0);  // Loops forever!
  }
}

int TestBusyWaitForeverCAlias(int loop_infinitely_if_0) {
  if (loop_infinitely_if_0 != 0) {
    return loop_infinitely_if_0;
  }
  pw_BusyWaitForever();  // No return statement needed because of infinite loop.
}

TEST(BusyWaitForever, CAliasCompiles) {
  EXPECT_EQ(TestBusyWaitForeverCAlias(123), 123);

  if (false) {
    EXPECT_EQ(TestBusyWaitForeverCAlias(0), 0);  // Loops forever!
  }
}

TEST(BusyWaitForever, CompilesInC) {
  EXPECT_EQ(pw_TestBusyWaitForeverC(123), 123);

  if (false) {
    EXPECT_EQ(pw_TestBusyWaitForeverC(0), 0);  // Loops forever!
  }
}

}  // namespace
