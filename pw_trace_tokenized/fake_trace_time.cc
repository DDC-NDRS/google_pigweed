// Copyright 2020 The Pigweed Authors
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
//==============================================================================
//

#include <cstddef>
#include <cstdint>
#include <type_traits>

#include "pw_trace_tokenized/config.h"

namespace {

PW_TRACE_TIME_TYPE time_counter = 0;

}  // namespace

// Define trace time as a counter for tests.
PW_TRACE_TIME_TYPE pw_trace_GetTraceTime() { return time_counter++; }

// Return 1 for ticks per second, as it doesn't apply to fake timer.
size_t pw_trace_GetTraceTimeTicksPerSecond() { return 1; }

void pw_trace_ResetFakeTraceTimer() { time_counter = 0; }
