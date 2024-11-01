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

#include "pw_tokenizer/tokenize.h"

namespace pw::tokenizer {

template <typename T>
constexpr bool ValidEnumerator(T) {
  static_assert(std::is_enum_v<T>, "Must be an enum");
  static_assert(sizeof(T) <= sizeof(Token), "Must fit in a token");
  return true;
}

}  // namespace pw::tokenizer

#define _PW_SEMICOLON(...) ;

#define _PW_TOKENIZE_TO_STRING_CASE(index, name, arg) \
  case name::arg:                                     \
    return #arg;

// Declares an individual tokenized enum value.
#define _PW_TOKENIZE_ENUMERATOR(index, name, enumerator)                   \
  static_assert(                                                           \
      #name[0] == ':' && #name[1] == ':',                                  \
      "Enum names must be fully qualified and start with ::, but \"" #name \
      "\" does not");                                                      \
  static_assert(::pw::tokenizer::ValidEnumerator(name::enumerator));       \
  PW_TOKENIZER_DEFINE_TOKEN(                                               \
      static_cast<::pw::tokenizer::Token>(name::enumerator),               \
      #name,                                                               \
      #enumerator)