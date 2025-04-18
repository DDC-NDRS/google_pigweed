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
#pragma once

#include "pw_span/span.h"
#include "pw_unit_test/framework.h"

namespace pw::unit_test {

class UnitTestThread;

namespace internal {

// Unit test event handler that streams test events through an RPC service.
class RpcEventHandler : public EventHandler {
 public:
  RpcEventHandler(UnitTestThread& thread);
  void ExecuteTests(span<std::string_view> suites_to_run);

  void TestProgramStart(const ProgramSummary&) override {}
  void EnvironmentsSetUpEnd() override {}
  void TestSuiteStart(const TestSuite&) override {}
  void TestSuiteEnd(const TestSuite&) override {}
  void EnvironmentsTearDownEnd() override {}
  void TestProgramEnd(const ProgramSummary&) override {}

  void RunAllTestsStart() override;
  void RunAllTestsEnd(const RunTestsSummary& run_tests_summary) override;
  void TestCaseStart(const TestCase& test_case) override;
  void TestCaseEnd(const TestCase& test_case, TestResult result) override;
  void TestCaseExpect(const TestCase& test_case,
                      const TestExpectation& expectation) override;
  void TestCaseDisabled(const TestCase& test_case) override;

 private:
  UnitTestThread& thread_;
};

}  // namespace internal
}  // namespace pw::unit_test
