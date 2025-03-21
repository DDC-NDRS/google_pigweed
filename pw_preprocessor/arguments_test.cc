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

// Many of these tests are static asserts. If these compile, they pass. The TEST
// functions are used for organization only.

#include <tuple>

#include "pw_preprocessor/apply.h"
#include "pw_unit_test/framework.h"

namespace pw {
namespace {

#define EMPTY_ARG

TEST(HasArgs, WithoutArguments) {
  static_assert(PW_HAS_ARGS() == 0);
  static_assert(PW_HAS_ARGS(/**/) == 0);
  static_assert(PW_HAS_ARGS(/* uhm, hi */) == 0);
  static_assert(PW_HAS_ARGS(EMPTY_ARG) == 0);

  // Test how the macro handles whitespace and comments.
  // clang-format off
  static_assert(PW_HAS_ARGS(     ) == 0);
  static_assert(PW_HAS_ARGS(
      ) == 0);
  static_assert(PW_HAS_ARGS(
      // wow
      // This is a comment.
      ) == 0);
  // clang-format on

  static_assert(PW_EMPTY_ARGS() == 1);
  static_assert(PW_EMPTY_ARGS(/* hello */) == 1);
  static_assert(PW_EMPTY_ARGS(
                    // hello
                    /* goodbye */) == 1);
}

TEST(HasArgs, WithArguments) {
  static_assert(PW_HAS_ARGS(()) == 1);
  static_assert(PW_HAS_ARGS(0) == 1);
  static_assert(PW_HAS_ARGS(, ) == 1);
  static_assert(PW_HAS_ARGS(a, b, c) == 1);
  static_assert(PW_HAS_ARGS(PW_HAS_ARGS) == 1);
  static_assert(PW_HAS_ARGS(PW_HAS_ARGS()) == 1);

  static_assert(PW_EMPTY_ARGS(0) == 0);
  static_assert(PW_EMPTY_ARGS(, ) == 0);
  static_assert(PW_EMPTY_ARGS(a, b, c) == 0);
  static_assert(PW_EMPTY_ARGS(PW_HAS_ARGS) == 0);
  static_assert(PW_EMPTY_ARGS(PW_HAS_ARGS()) == 0);
}

constexpr int TestFunc(int arg, ...) { return arg; }

#define CALL_FUNCTION(arg, ...) TestFunc(arg PW_COMMA_ARGS(__VA_ARGS__))

template <typename T, typename... Args>
constexpr T TemplateArgCount() {
  return sizeof...(Args);
}

#define COUNT_ARGS_TEMPLATE(...) \
  TemplateArgCount<int PW_COMMA_ARGS(__VA_ARGS__)>()

TEST(CommaVarargs, NoArguments) {
  static_assert(TestFunc(0 PW_COMMA_ARGS()) == 0);
  static_assert(TestFunc(1 /* whoa */ PW_COMMA_ARGS(
                    /* this macro */) /* is cool! */) == 1);

  static_assert(TemplateArgCount<int PW_COMMA_ARGS()>() == 0);
  static_assert(TemplateArgCount<int PW_COMMA_ARGS(/* nothing */)>() == 0);

  static_assert(CALL_FUNCTION(2) == 2);
  static_assert(CALL_FUNCTION(3, ) == 3);
  static_assert(CALL_FUNCTION(4, /* nothing */) == 4);

  static_assert(COUNT_ARGS_TEMPLATE() == 0);
  static_assert(COUNT_ARGS_TEMPLATE(/* nothing */) == 0);
}

TEST(CommaVarargs, WithArguments) {
  static_assert(TestFunc(0 PW_COMMA_ARGS(1)) == 0);
  static_assert(TestFunc(1 PW_COMMA_ARGS(1, 2)) == 1);
  static_assert(TestFunc(2 PW_COMMA_ARGS(1, 2, "three")) == 2);

  static_assert(TemplateArgCount<int PW_COMMA_ARGS(bool)>() == 1);
  static_assert(TemplateArgCount<int PW_COMMA_ARGS(char, const char*)>() == 2);
  static_assert(TemplateArgCount<int PW_COMMA_ARGS(int, char, const char*)>() ==
                3);

  static_assert(CALL_FUNCTION(3) == 3);
  static_assert(CALL_FUNCTION(4, ) == 4);
  static_assert(CALL_FUNCTION(5, /* nothing */) == 5);

  static_assert(COUNT_ARGS_TEMPLATE(int) == 1);
  static_assert(COUNT_ARGS_TEMPLATE(int, int) == 2);
  static_assert(COUNT_ARGS_TEMPLATE(int, int, int) == 3);
}

TEST(CommaVarargs, EmptyFinalArgument) {
  static_assert(COUNT_ARGS_TEMPLATE(EMPTY_ARG) == 0);
  static_assert(COUNT_ARGS_TEMPLATE(int, ) == 1);
  static_assert(COUNT_ARGS_TEMPLATE(int, EMPTY_ARG) == 1);
  static_assert(COUNT_ARGS_TEMPLATE(int, /* EMPTY_ARG */) == 1);
  static_assert(COUNT_ARGS_TEMPLATE(int, int, ) == 2);
  static_assert(COUNT_ARGS_TEMPLATE(int, int, int, ) == 3);
  static_assert(COUNT_ARGS_TEMPLATE(int, int, int, EMPTY_ARG) == 3);
}

// This test demonstrates that PW_COMMA_ARGS behaves unexpectedly when it is
// used when invoking another macro. DO NOT use PW_COMMA_ARGS when invoking
// another macro!
#define BAD_DEMO(fmt, ...) _BAD_DEMO_ADD_123(fmt PW_COMMA_ARGS(__VA_ARGS__))

#define _BAD_DEMO_ADD_123(fmt, ...) \
  _CAPTURE_ARGS_AS_TUPLE("%d: " fmt, 123 PW_COMMA_ARGS(__VA_ARGS__))

#define _CAPTURE_ARGS_AS_TUPLE(...) std::make_tuple(__VA_ARGS__)

TEST(CommaVarargs, MisbehavesWithMacroToMacroUse_NoArgs_ArgsAreOkay) {
  auto [a1, a2] = BAD_DEMO("Hello world");
  EXPECT_STREQ(a1, "%d: Hello world");
  EXPECT_EQ(a2, 123);
}

TEST(CommaVarargs, MisbehavesWithMacroToMacroUse_WithArgs_ArgsOutOfOrder) {
  // If there is an additional argument, the order is incorrect! The 123
  // argument should go before the "world?" argument, but it is inserted after.
  // This would be a compilation error if these arguments were passed to printf.
  // What's worse is that this can silently fail if the arguments happen to be
  // compatible types.
  const auto [a1, a2, a3] = BAD_DEMO("Hello %s", "world?");
  EXPECT_STREQ(a1, "%d: Hello %s");
  EXPECT_STREQ(a2, "world?");
  EXPECT_EQ(a3, 123);
}

TEST(CountArgs, Zero) {
  static_assert(PW_MACRO_ARG_COUNT() == 0);
  static_assert(PW_MACRO_ARG_COUNT(/**/) == 0);
  static_assert(PW_MACRO_ARG_COUNT(/* uhm, hi */) == 0);

  // clang-format off
  static_assert(PW_MACRO_ARG_COUNT(     ) == 0);
  static_assert(PW_MACRO_ARG_COUNT(
      ) == 0);
  static_assert(PW_MACRO_ARG_COUNT(
      // wow
      // This is a comment.
      ) == 0);
  // clang-format on
}

TEST(CountArgs, Commas) {
  // clang-format off
  static_assert(PW_MACRO_ARG_COUNT(,) == 2);
  static_assert(PW_MACRO_ARG_COUNT(,,) == 3);
  static_assert(PW_MACRO_ARG_COUNT(,,,) == 4);
  // clang-format on
  static_assert(PW_MACRO_ARG_COUNT(, ) == 2);
  static_assert(PW_MACRO_ARG_COUNT(, , ) == 3);
  static_assert(PW_MACRO_ARG_COUNT(, , , ) == 4);

  static_assert(PW_MACRO_ARG_COUNT(a, ) == 2);
  static_assert(PW_MACRO_ARG_COUNT(a, , ) == 3);
  static_assert(PW_MACRO_ARG_COUNT(a, b, c, ) == 4);
}

TEST(CountArgs, Parentheses) {
  static_assert(PW_MACRO_ARG_COUNT(()) == 1);
  static_assert(PW_MACRO_ARG_COUNT((1, 2, 3, 4)) == 1);
  static_assert(PW_MACRO_ARG_COUNT((1, 2, 3), (1, 2, 3, 4)) == 2);
  static_assert(PW_MACRO_ARG_COUNT((), ()) == 2);
  static_assert(PW_MACRO_ARG_COUNT((-), (o)) == 2);
  static_assert(PW_MACRO_ARG_COUNT((, , (, , ), ), (123, 4)) == 2);
  static_assert(PW_MACRO_ARG_COUNT(1, (2, 3, 4), (<5, 6>)) == 3);
}

template <typename... Args>
constexpr size_t FunctionArgCount(Args...) {
  return sizeof...(Args);
}

static_assert(FunctionArgCount() == 0);
static_assert(FunctionArgCount(1) == 1);
static_assert(FunctionArgCount(1, 2) == 2);

TEST(CountFunctionArgs, NonEmptyLastArg) {
  static_assert(PW_FUNCTION_ARG_COUNT(a) == 1);
  static_assert(PW_FUNCTION_ARG_COUNT(1, 2) == 2);
  static_assert(PW_FUNCTION_ARG_COUNT(1, 2, 3) == 3);
}

TEST(CountFunctionArgs, EmptyLastArg) {
  static_assert(PW_FUNCTION_ARG_COUNT() == 0);
  static_assert(PW_FUNCTION_ARG_COUNT(a, ) == 1);
  static_assert(PW_FUNCTION_ARG_COUNT(1, 2, ) == 2);
  static_assert(PW_FUNCTION_ARG_COUNT(1, 2, 3, ) == 3);

  static_assert(PW_FUNCTION_ARG_COUNT(a, EMPTY_ARG) == 1);
  static_assert(PW_FUNCTION_ARG_COUNT(1, 2, EMPTY_ARG) == 2);
  static_assert(PW_FUNCTION_ARG_COUNT(1, 2, 3, EMPTY_ARG) == 3);
}

constexpr const char* Value(const char* str = nullptr) { return str; }

TEST(LastArg, NonEmptyLastArg) {
  constexpr const char* last = "last!";
  static_assert(Value(PW_LAST_ARG(last)) == last);
  static_assert(Value(PW_LAST_ARG(1, last)) == last);
  static_assert(Value(PW_LAST_ARG(1, 2, last)) == last);
}

TEST(LastArg, EmptyLastArg) {
  static_assert(Value(PW_LAST_ARG()) == nullptr);
  static_assert(Value(PW_LAST_ARG(1, )) == nullptr);
  static_assert(Value(PW_LAST_ARG(1, 2, )) == nullptr);
  static_assert(Value(PW_LAST_ARG(1, 2, 3, )) == nullptr);
}

TEST(DropLastArg, NonEmptyLastArg) {
  static_assert(FunctionArgCount(PW_DROP_LAST_ARG(1)) == 0);
  static_assert(FunctionArgCount(PW_DROP_LAST_ARG(1, 2)) == 1);
  static_assert(FunctionArgCount(PW_DROP_LAST_ARG(1, 2, 3)) == 2);
}

TEST(DropLastArg, EmptyLastArg) {
  static_assert(FunctionArgCount(PW_DROP_LAST_ARG()) == 0);
  static_assert(FunctionArgCount(PW_DROP_LAST_ARG(1, )) == 1);
  static_assert(FunctionArgCount(PW_DROP_LAST_ARG(1, 2, )) == 2);
  static_assert(FunctionArgCount(PW_DROP_LAST_ARG(1, 2, 3, )) == 3);
}

TEST(DropLastArgIfEmpty, NonEmptyLastArg) {
  static_assert(FunctionArgCount(PW_DROP_LAST_ARG_IF_EMPTY(1)) == 1);
  static_assert(FunctionArgCount(PW_DROP_LAST_ARG_IF_EMPTY(1, 2)) == 2);
  static_assert(FunctionArgCount(PW_DROP_LAST_ARG_IF_EMPTY(1, 2, 3)) == 3);
}

TEST(DropLastArgIfEmpty, EmptyLastArg) {
  static_assert(FunctionArgCount(PW_DROP_LAST_ARG_IF_EMPTY()) == 0);
  static_assert(FunctionArgCount(PW_DROP_LAST_ARG_IF_EMPTY(1, )) == 1);
  static_assert(FunctionArgCount(PW_DROP_LAST_ARG_IF_EMPTY(1, 2, )) == 2);
  static_assert(FunctionArgCount(PW_DROP_LAST_ARG_IF_EMPTY(1, 2, 3, )) == 3);
}

// This test demonstrates that PW_DROP_LAST_ARG_IF_EMPTY behaves unexpectedly
// when it is used when invoking another macro. DO NOT use
// PW_DROP_LAST_ARG_IF_EMPTY when invoking another macro!
#define BAD_DROP_LAST_DEMO(fmt, ...) \
  _BAD_DROP_LAST_DEMO_ADD_123(PW_DROP_LAST_ARG_IF_EMPTY(fmt, __VA_ARGS__))

#define _BAD_DROP_LAST_DEMO_ADD_123(fmt, ...) \
  _CAPTURE_ARGS_AS_TUPLE("%d: " fmt,          \
                         PW_DROP_LAST_ARG_IF_EMPTY(123, __VA_ARGS__))

TEST(DropLastArgIfEmpty, EmptyLastArgArgsLoseOrder) {
  // If there are any additional arguments, the order is incorrect! The 123
  // argument should go before the 3, 2, 1 arguments, but it is inserted after.
  // This would be a compilation error if these arguments were passed to printf.
  // What's worse is that this can silently fail if the arguments happen to be
  // compatible types.
  auto [a1, a2, a3, a4, a5] =
      BAD_DROP_LAST_DEMO("Countdown in %d %d %d", 3, 2, 1, );
  EXPECT_STREQ(a1, "%d: Countdown in %d %d %d");
  EXPECT_EQ(a2, 3);
  EXPECT_EQ(a3, 2);
  EXPECT_EQ(a4, 1);
  EXPECT_EQ(a5, 123);
}

TEST(DropLastArgIfEmpty, NonEmptyLastArgArgsLoseOrder) {
  // If there are any additional arguments, the order is incorrect! The 123
  // argument should go before the 3, 2, 1 arguments, but it is inserted after.
  // This would be a compilation error if these arguments were passed to printf.
  // What's worse is that this can silently fail if the arguments happen to be
  // compatible types.
  auto [a1, a2, a3, a4, a5] =
      BAD_DROP_LAST_DEMO("Countdown in %d %d %d", 3, 2, 1);
  EXPECT_STREQ(a1, "%d: Countdown in %d %d %d");
  EXPECT_EQ(a2, 3);
  EXPECT_EQ(a3, 2);
  EXPECT_EQ(a4, 1);
  EXPECT_EQ(a5, 123);
}

// When PW_DROP_LAST_ARG_IF_EMPTY is used once, and there are no other
// modifications to __VA_ARGS__, then the order is kept.
#define DROP_LAST_DEMO(fmt, arg_a, arg_b, ...) \
  _CAPTURE_ARGS_AS_TUPLE(                      \
      "%d: " fmt, PW_DROP_LAST_ARG_IF_EMPTY(123, arg_a, arg_b, __VA_ARGS__))

TEST(DropLastArgIfEmpty, EmptyLastArgAllArgsInOrder) {
  const auto [a1, a2, a3, a4, a5] =
      DROP_LAST_DEMO("Countdown in %d %d %d", 3, 2, 1, );
  EXPECT_STREQ(a1, "%d: Countdown in %d %d %d");
  EXPECT_EQ(a2, 123);
  EXPECT_EQ(a3, 3);
  EXPECT_EQ(a4, 2);
  EXPECT_EQ(a5, 1);
}

TEST(DropLastArgIfEmpty, NonEmptyLastArgAllArgsInOrder) {
  const auto [a1, a2, a3, a4, a5] =
      DROP_LAST_DEMO("Countdown in %d %d %d", 3, 2, 1);
  EXPECT_STREQ(a1, "%d: Countdown in %d %d %d");
  EXPECT_EQ(a2, 123);
  EXPECT_EQ(a3, 3);
  EXPECT_EQ(a4, 2);
  EXPECT_EQ(a5, 1);
}

#define SOME_VARIADIC_MACRO(...) PW_MACRO_ARG_COUNT(__VA_ARGS__)

#define ANOTHER_VARIADIC_MACRO(arg, ...) SOME_VARIADIC_MACRO(__VA_ARGS__)

#define ALWAYS_ONE_ARG(...) SOME_VARIADIC_MACRO((__VA_ARGS__))

TEST(CountArgs, NestedMacros) {
  static_assert(SOME_VARIADIC_MACRO() == 0);
  static_assert(SOME_VARIADIC_MACRO(X1) == 1);
  static_assert(SOME_VARIADIC_MACRO(X1, X2) == 2);
  static_assert(SOME_VARIADIC_MACRO(X1, X2, X3) == 3);
  static_assert(SOME_VARIADIC_MACRO(X1, X2, X3, X4) == 4);
  static_assert(SOME_VARIADIC_MACRO(X1, X2, X3, X4, X5) == 5);

  static_assert(ANOTHER_VARIADIC_MACRO() == 0);
  static_assert(ANOTHER_VARIADIC_MACRO(X0) == 0);
  static_assert(ANOTHER_VARIADIC_MACRO(X0, X1) == 1);
  static_assert(ANOTHER_VARIADIC_MACRO(X0, X1, X2) == 2);
  static_assert(ANOTHER_VARIADIC_MACRO(X0, X1, X2, X3) == 3);
  static_assert(ANOTHER_VARIADIC_MACRO(X0, X1, X2, X3, X4) == 4);
  static_assert(ANOTHER_VARIADIC_MACRO(X0, X1, X2, X3, X4, X5) == 5);

  static_assert(ALWAYS_ONE_ARG() == 1);
  static_assert(ALWAYS_ONE_ARG(X0) == 1);
  static_assert(ALWAYS_ONE_ARG(X0, X1) == 1);
  static_assert(ALWAYS_ONE_ARG(X0, X1, X2) == 1);
  static_assert(ALWAYS_ONE_ARG(X0, X1, X2, X3) == 1);
  static_assert(ALWAYS_ONE_ARG(X0, X1, X2, X3, X4) == 1);
  static_assert(ALWAYS_ONE_ARG(X0, X1, X2, X3, X4, X5) == 1);
}

/* Tests all supported arg counts. This test was generated by the following
   Python 3 code:
for i in range(256 + 1):
  args = [f'X{x}' for x in range(1, i + 1)]
  print(f'  static_assert(PW_MACRO_ARG_COUNT({", ".join(args)}) == {i});')
*/
// Most tests above 16 arguments were skipped to keep this a reasonable size.
TEST(CountArgs, AllSupported) {
  // clang-format off
  static_assert(PW_MACRO_ARG_COUNT() == 0);
  static_assert(PW_MACRO_ARG_COUNT(X1) == 1);
  static_assert(PW_MACRO_ARG_COUNT(X1, X2) == 2);
  static_assert(PW_MACRO_ARG_COUNT(X1, X2, X3) == 3);
  static_assert(PW_MACRO_ARG_COUNT(X1, X2, X3, X4) == 4);
  static_assert(PW_MACRO_ARG_COUNT(X1, X2, X3, X4, X5) == 5);
  static_assert(PW_MACRO_ARG_COUNT(X1, X2, X3, X4, X5, X6) == 6);
  static_assert(PW_MACRO_ARG_COUNT(X1, X2, X3, X4, X5, X6, X7) == 7);
  static_assert(PW_MACRO_ARG_COUNT(X1, X2, X3, X4, X5, X6, X7, X8) == 8);
  static_assert(PW_MACRO_ARG_COUNT(X1, X2, X3, X4, X5, X6, X7, X8, X9) == 9);
  static_assert(PW_MACRO_ARG_COUNT(X1, X2, X3, X4, X5, X6, X7, X8, X9, X10) == 10);
  static_assert(PW_MACRO_ARG_COUNT(X1, X2, X3, X4, X5, X6, X7, X8, X9, X10, X11) == 11);
  static_assert(PW_MACRO_ARG_COUNT(X1, X2, X3, X4, X5, X6, X7, X8, X9, X10, X11, X12) == 12);
  static_assert(PW_MACRO_ARG_COUNT(X1, X2, X3, X4, X5, X6, X7, X8, X9, X10, X11, X12, X13) == 13);
  static_assert(PW_MACRO_ARG_COUNT(X1, X2, X3, X4, X5, X6, X7, X8, X9, X10, X11, X12, X13, X14) == 14);
  static_assert(PW_MACRO_ARG_COUNT(X1, X2, X3, X4, X5, X6, X7, X8, X9, X10, X11, X12, X13, X14, X15) == 15);
  static_assert(PW_MACRO_ARG_COUNT(X1, X2, X3, X4, X5, X6, X7, X8, X9, X10, X11, X12, X13, X14, X15, X16) == 16);
  static_assert(PW_MACRO_ARG_COUNT(X1, X2, X3, X4, X5, X6, X7, X8, X9, X10, X11, X12, X13, X14, X15, X16, X17) == 17);
  static_assert(PW_MACRO_ARG_COUNT(X1, X2, X3, X4, X5, X6, X7, X8, X9, X10, X11, X12, X13, X14, X15, X16, X17, X18, X19, X20, X21, X22, X23, X24, X25, X26, X27, X28, X29, X30, X31) == 31);
  static_assert(PW_MACRO_ARG_COUNT(X1, X2, X3, X4, X5, X6, X7, X8, X9, X10, X11, X12, X13, X14, X15, X16, X17, X18, X19, X20, X21, X22, X23, X24, X25, X26, X27, X28, X29, X30, X31, X32) == 32);
  static_assert(PW_MACRO_ARG_COUNT(X1, X2, X3, X4, X5, X6, X7, X8, X9, X10, X11, X12, X13, X14, X15, X16, X17, X18, X19, X20, X21, X22, X23, X24, X25, X26, X27, X28, X29, X30, X31, X32, X33) == 33);
  static_assert(PW_MACRO_ARG_COUNT(X1, X2, X3, X4, X5, X6, X7, X8, X9, X10, X11, X12, X13, X14, X15, X16, X17, X18, X19, X20, X21, X22, X23, X24, X25, X26, X27, X28, X29, X30, X31, X32, X33, X34, X35, X36, X37, X38, X39, X40, X41, X42, X43, X44, X45, X46, X47, X48, X49, X50, X51, X52, X53, X54, X55, X56, X57, X58, X59, X60, X61, X62, X63) == 63);
  static_assert(PW_MACRO_ARG_COUNT(X1, X2, X3, X4, X5, X6, X7, X8, X9, X10, X11, X12, X13, X14, X15, X16, X17, X18, X19, X20, X21, X22, X23, X24, X25, X26, X27, X28, X29, X30, X31, X32, X33, X34, X35, X36, X37, X38, X39, X40, X41, X42, X43, X44, X45, X46, X47, X48, X49, X50, X51, X52, X53, X54, X55, X56, X57, X58, X59, X60, X61, X62, X63, X64) == 64);
  static_assert(PW_MACRO_ARG_COUNT(X1, X2, X3, X4, X5, X6, X7, X8, X9, X10, X11, X12, X13, X14, X15, X16, X17, X18, X19, X20, X21, X22, X23, X24, X25, X26, X27, X28, X29, X30, X31, X32, X33, X34, X35, X36, X37, X38, X39, X40, X41, X42, X43, X44, X45, X46, X47, X48, X49, X50, X51, X52, X53, X54, X55, X56, X57, X58, X59, X60, X61, X62, X63, X64, X65) == 65);
  static_assert(PW_MACRO_ARG_COUNT(X1, X2, X3, X4, X5, X6, X7, X8, X9, X10, X11, X12, X13, X14, X15, X16, X17, X18, X19, X20, X21, X22, X23, X24, X25, X26, X27, X28, X29, X30, X31, X32, X33, X34, X35, X36, X37, X38, X39, X40, X41, X42, X43, X44, X45, X46, X47, X48, X49, X50, X51, X52, X53, X54, X55, X56, X57, X58, X59, X60, X61, X62, X63, X64, X65, X66, X67, X68, X69, X70, X71, X72, X73, X74, X75, X76, X77, X78, X79, X80, X81, X82, X83, X84, X85, X86, X87, X88, X89, X90, X91, X92, X93, X94, X95, X96, X97, X98, X99, X100, X101, X102, X103, X104, X105, X106, X107, X108, X109, X110, X111, X112, X113, X114, X115, X116, X117, X118, X119, X120, X121, X122, X123, X124, X125, X126, X127) == 127);
  static_assert(PW_MACRO_ARG_COUNT(X1, X2, X3, X4, X5, X6, X7, X8, X9, X10, X11, X12, X13, X14, X15, X16, X17, X18, X19, X20, X21, X22, X23, X24, X25, X26, X27, X28, X29, X30, X31, X32, X33, X34, X35, X36, X37, X38, X39, X40, X41, X42, X43, X44, X45, X46, X47, X48, X49, X50, X51, X52, X53, X54, X55, X56, X57, X58, X59, X60, X61, X62, X63, X64, X65, X66, X67, X68, X69, X70, X71, X72, X73, X74, X75, X76, X77, X78, X79, X80, X81, X82, X83, X84, X85, X86, X87, X88, X89, X90, X91, X92, X93, X94, X95, X96, X97, X98, X99, X100, X101, X102, X103, X104, X105, X106, X107, X108, X109, X110, X111, X112, X113, X114, X115, X116, X117, X118, X119, X120, X121, X122, X123, X124, X125, X126, X127, X128) == 128);
  static_assert(PW_MACRO_ARG_COUNT(X1, X2, X3, X4, X5, X6, X7, X8, X9, X10, X11, X12, X13, X14, X15, X16, X17, X18, X19, X20, X21, X22, X23, X24, X25, X26, X27, X28, X29, X30, X31, X32, X33, X34, X35, X36, X37, X38, X39, X40, X41, X42, X43, X44, X45, X46, X47, X48, X49, X50, X51, X52, X53, X54, X55, X56, X57, X58, X59, X60, X61, X62, X63, X64, X65, X66, X67, X68, X69, X70, X71, X72, X73, X74, X75, X76, X77, X78, X79, X80, X81, X82, X83, X84, X85, X86, X87, X88, X89, X90, X91, X92, X93, X94, X95, X96, X97, X98, X99, X100, X101, X102, X103, X104, X105, X106, X107, X108, X109, X110, X111, X112, X113, X114, X115, X116, X117, X118, X119, X120, X121, X122, X123, X124, X125, X126, X127, X128, X129) == 129);
  static_assert(PW_MACRO_ARG_COUNT(X1, X2, X3, X4, X5, X6, X7, X8, X9, X10, X11, X12, X13, X14, X15, X16, X17, X18, X19, X20, X21, X22, X23, X24, X25, X26, X27, X28, X29, X30, X31, X32, X33, X34, X35, X36, X37, X38, X39, X40, X41, X42, X43, X44, X45, X46, X47, X48, X49, X50, X51, X52, X53, X54, X55, X56, X57, X58, X59, X60, X61, X62, X63, X64, X65, X66, X67, X68, X69, X70, X71, X72, X73, X74, X75, X76, X77, X78, X79, X80, X81, X82, X83, X84, X85, X86, X87, X88, X89, X90, X91, X92, X93, X94, X95, X96, X97, X98, X99, X100, X101, X102, X103, X104, X105, X106, X107, X108, X109, X110, X111, X112, X113, X114, X115, X116, X117, X118, X119, X120, X121, X122, X123, X124, X125, X126, X127, X128, X129, X130, X131, X132, X133, X134, X135, X136, X137, X138, X139, X140, X141, X142, X143, X144, X145, X146, X147, X148, X149, X150, X151, X152, X153, X154, X155, X156, X157, X158, X159, X160, X161, X162, X163, X164, X165, X166, X167, X168, X169, X170, X171, X172, X173, X174, X175, X176, X177, X178, X179, X180, X181, X182, X183, X184, X185, X186, X187, X188, X189, X190, X191, X192, X193, X194, X195, X196, X197, X198, X199, X200, X201, X202, X203, X204, X205, X206, X207, X208, X209, X210, X211, X212, X213, X214, X215, X216, X217, X218, X219, X220, X221, X222, X223, X224, X225, X226, X227, X228, X229, X230, X231, X232, X233, X234, X235, X236, X237, X238, X239, X240, X241, X242, X243, X244, X245, X246, X247, X248, X249, X250, X251, X252, X253, X254, X255) == 255);
  static_assert(PW_MACRO_ARG_COUNT(X1, X2, X3, X4, X5, X6, X7, X8, X9, X10, X11, X12, X13, X14, X15, X16, X17, X18, X19, X20, X21, X22, X23, X24, X25, X26, X27, X28, X29, X30, X31, X32, X33, X34, X35, X36, X37, X38, X39, X40, X41, X42, X43, X44, X45, X46, X47, X48, X49, X50, X51, X52, X53, X54, X55, X56, X57, X58, X59, X60, X61, X62, X63, X64, X65, X66, X67, X68, X69, X70, X71, X72, X73, X74, X75, X76, X77, X78, X79, X80, X81, X82, X83, X84, X85, X86, X87, X88, X89, X90, X91, X92, X93, X94, X95, X96, X97, X98, X99, X100, X101, X102, X103, X104, X105, X106, X107, X108, X109, X110, X111, X112, X113, X114, X115, X116, X117, X118, X119, X120, X121, X122, X123, X124, X125, X126, X127, X128, X129, X130, X131, X132, X133, X134, X135, X136, X137, X138, X139, X140, X141, X142, X143, X144, X145, X146, X147, X148, X149, X150, X151, X152, X153, X154, X155, X156, X157, X158, X159, X160, X161, X162, X163, X164, X165, X166, X167, X168, X169, X170, X171, X172, X173, X174, X175, X176, X177, X178, X179, X180, X181, X182, X183, X184, X185, X186, X187, X188, X189, X190, X191, X192, X193, X194, X195, X196, X197, X198, X199, X200, X201, X202, X203, X204, X205, X206, X207, X208, X209, X210, X211, X212, X213, X214, X215, X216, X217, X218, X219, X220, X221, X222, X223, X224, X225, X226, X227, X228, X229, X230, X231, X232, X233, X234, X235, X236, X237, X238, X239, X240, X241, X242, X243, X244, X245, X246, X247, X248, X249, X250, X251, X252, X253, X254, X255, X256) == 256);
  // clang-format on
}

TEST(DelegateByArgCount, WithoutAndWithoutArguments) {
#define TEST_SUM0() (0)
#define TEST_SUM1(a) (a)
#define TEST_SUM2(a, b) ((a) + (b))
#define TEST_SUM3(a, b, c) ((a) + (b) + (c))

  static_assert(PW_DELEGATE_BY_ARG_COUNT(TEST_SUM) == 0);
  static_assert(PW_DELEGATE_BY_ARG_COUNT(TEST_SUM, 5) == 5);
  static_assert(PW_DELEGATE_BY_ARG_COUNT(TEST_SUM, 1, 2) == 3);
  static_assert(PW_DELEGATE_BY_ARG_COUNT(TEST_SUM, 1, 2, 3) == 6);
}

#define SEMICOLON(...) ;
#define STRING_THING(index, name, arg) #arg

#define TO_STRING_CASE(index, name, arg) \
  case arg:                              \
    return #arg

#define APPLY_STRING_THING(...) PW_APPLY(STRING_THING, SEMICOLON, , __VA_ARGS__)

constexpr const char* ValueToStr(int value) {
  switch (value) {
    PW_APPLY(TO_STRING_CASE, SEMICOLON, , 100, 200, 300, 400, 500);
  }
  return "Unknown value";
}

TEST(ApplyMacroExpansion, OneStringValue) {
  constexpr const char* test = APPLY_STRING_THING(100);
  EXPECT_STREQ("100", test);
}

TEST(ApplyMacroExpansion, SwitchValue) {
  constexpr const char* test = ValueToStr(300);
  EXPECT_STREQ("300", test);
}

TEST(ApplyMacroExpansion, UnknownSwitchValue) {
  constexpr const char* test = ValueToStr(600);
  EXPECT_STREQ("Unknown value", test);
}

}  // namespace
}  // namespace pw
