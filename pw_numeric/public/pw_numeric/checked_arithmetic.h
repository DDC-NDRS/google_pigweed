// Copyright 2024 The Pigweed Authors
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

#include <optional>
#include <type_traits>

#include "pw_preprocessor/compiler.h"

namespace pw {

/// Adds two numbers, checking for overflow.
///
/// @tparam T The type of the result, which is checked for overflow.
///
/// @tparam A The type of the first addend, `a`.
///
/// @tparam B The type of the second addend, `b`.
///
/// @param[in] a The first addend.
///
/// @param[in] b The second addend.
///
/// @returns The sum (`a + b`) if addition was successful,
/// or `nullopt` if the addition would overflow.
///
/// @note The template argument must be provided, e.g.
/// `pw::CheckedAdd<uint32_t>(...)`.
template <typename T, typename A, typename B>
constexpr std::optional<T> CheckedAdd(A a, B b) {
  T result;

  if (PW_ADD_OVERFLOW(a, b, &result)) {
    return std::nullopt;
  }

  return result;
}

/// Increments a variable by some amount.
///
/// @tparam T The type of the variable to be incremented.
///
/// @tparam Inc The type of the variable to add.
///
/// @param[in] base The variable to be incremented.
///
/// @param[in] inc The number to add to `base`.
///
/// @returns True if the addition was successful and `base` was incremented
/// (`base += inc`); False if the addition would overflow and ``base`` is
/// unmodified.
template <typename T, typename Inc>
constexpr bool CheckedIncrement(T& base, Inc inc) {
  std::optional<T> result =
      CheckedAdd<std::remove_reference_t<decltype(base)>>(base, inc);
  if (!result) {
    return false;
  }
  base = *result;
  return true;
}

/// Subtracts two numbers, checking for overflow.
///
/// @tparam T The type of the result, which is checked for overflow.
///
/// @tparam A The type of the minuend, `a`.
///
/// @tparam B The type of the subtrahend, `b`.
///
/// @param[in] a The minuend (the number from which `b` is subtracted).
///
/// @param[in] b The subtrahend (the number subtracted from `a`).
///
/// @returns The difference (`a - b`) if subtraction was successful,
/// or `nullopt` if the subtraction would overflow.
///
/// @note The template argument must be provided, e.g.
/// `pw::CheckedSub<uint32_t>(...)`.
template <typename T, typename A, typename B>
constexpr std::optional<T> CheckedSub(A a, B b) {
  T result;

  if (PW_SUB_OVERFLOW(a, b, &result)) {
    return std::nullopt;
  }

  return result;
}

/// Decrements a variable by some amount.
///
/// @tparam T The type of the variable to be decremented.
///
/// @tparam Dec The type of the variable to subtract.
///
/// @param[in] base The variable to be decremented.
///
/// @param[in] inc The number to subtract from `base`.
///
/// @returns True if the subtraction was successful and `base` was decremented
/// (`base -= inc`); False if the subtraction would overflow and ``base`` is
/// unmodified.
template <typename T, typename Dec>
constexpr bool CheckedDecrement(T& base, Dec dec) {
  std::optional<T> result =
      CheckedSub<std::remove_reference_t<decltype(base)>>(base, dec);
  if (!result) {
    return false;
  }
  base = *result;
  return true;
}

/// Multiplies two numbers, checking for overflow.
///
/// @tparam T The type of the result, which is checked for overflow.
///
/// @tparam A The type of the first factor, `a`.
///
/// @tparam B The type of the second factor, `b`.
///
/// @param[in] a The first factor.
///
/// @param[in] b The second factor.
///
/// @returns The product (`a * b`) if multiplication was successful,
/// or `nullopt` if the multiplication would overflow.
///
/// @note The template argument must be provided, e.g.
/// `pw::CheckedMul<uint32_t>(...)`.
template <typename T, typename A, typename B>
constexpr std::optional<T> CheckedMul(A a, B b) {
  T result;

  if (PW_MUL_OVERFLOW(a, b, &result)) {
    return std::nullopt;
  }

  return result;
}

}  // namespace pw
