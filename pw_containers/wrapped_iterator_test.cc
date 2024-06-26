// Copyright 2021 The Pigweed Authors
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

#include "pw_containers/wrapped_iterator.h"

#include <array>

#include "pw_unit_test/framework.h"

namespace pw::containers {
namespace {

class HalfIterator : public WrappedIterator<HalfIterator, const int*, int> {
 public:
  constexpr HalfIterator(const int* it) : WrappedIterator(it) {}

  int operator*() const { return value() / 2; }
};

constexpr std::array<int, 6> kArray{0, 2, 4, 6, 8, 10};

TEST(WrappedIterator, IterateForwards) {
  int expected = 0;
  for (HalfIterator it(kArray.data());
       it != HalfIterator(kArray.data() + kArray.size());
       ++it) {
    EXPECT_EQ(*it, expected);
    expected += 1;
  }
}

TEST(WrappedIterator, IterateBackwards) {
  HalfIterator it(kArray.data() + kArray.size());

  int expected = 5;
  do {
    --it;
    EXPECT_EQ(*it, expected);
    expected -= 1;
  } while (it != HalfIterator(kArray.data()));
}

TEST(WrappedIterator, PostIncrement) {
  HalfIterator it(kArray.data());
  EXPECT_EQ(it++, HalfIterator(kArray.data()));
  EXPECT_EQ(it, HalfIterator(kArray.data() + 1));
  EXPECT_EQ(*it, 1);
}

TEST(WrappedIterator, PreIncrement) {
  HalfIterator it(kArray.data());
  EXPECT_EQ(++it, HalfIterator(kArray.data() + 1));
  EXPECT_EQ(*it, 1);
}

TEST(WrappedIterator, PostDecrement) {
  HalfIterator it(kArray.data() + kArray.size());
  EXPECT_EQ(it--, HalfIterator(kArray.data() + kArray.size()));
  EXPECT_EQ(it, HalfIterator(kArray.data() + kArray.size() - 1));
  EXPECT_EQ(*it, 5);
}

TEST(WrappedIterator, PreDecrement) {
  HalfIterator it(kArray.data() + kArray.size());
  EXPECT_EQ(--it, HalfIterator(kArray.data() + kArray.size() - 1));
  EXPECT_EQ(*it, 5);
}

}  // namespace
}  // namespace pw::containers
