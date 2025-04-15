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

#include "pw_base64/base64.h"

#include <cstring>

#include "pw_unit_test/constexpr.h"
#include "pw_unit_test/framework.h"

namespace pw::base64 {
namespace {

struct EncodedData {
  const size_t binary_size;
  const char* const binary_data;
  const char* const encoded_data;
};

/* The following test data was generated by this Python 3 script.

#!/usr/bin/env python3

import base64
import random


def b64_encode(raw_data):
    encoded = base64.b64encode(raw_data).decode()
    hex_string = ''.join(r'\x{:02x}'.format(b) for b in raw_data)

    print('    {{{size}, "{raw}", "{encoded}"}},'.format(
        size=len(raw_data), raw=hex_string, encoded=encoded))


print('constexpr EncodedData kSingleCharTestData[] = {')

for i in range(256):
    b64_encode(bytes([i]))

print('};')
print()

print('constexpr EncodedData kRandomTestData[] = {')

for length in range(2, 12):
    for _ in range(10):
        b64_encode(bytes(random.randrange(256) for _ in range(length)))

print('};')
print()

*/

constexpr EncodedData kSingleCharTestData[] = {
    {1, "\x00", "AA=="}, {1, "\x01", "AQ=="}, {1, "\x02", "Ag=="},
    {1, "\x03", "Aw=="}, {1, "\x04", "BA=="}, {1, "\x05", "BQ=="},
    {1, "\x06", "Bg=="}, {1, "\x07", "Bw=="}, {1, "\x08", "CA=="},
    {1, "\x09", "CQ=="}, {1, "\x0a", "Cg=="}, {1, "\x0b", "Cw=="},
    {1, "\x0c", "DA=="}, {1, "\x0d", "DQ=="}, {1, "\x0e", "Dg=="},
    {1, "\x0f", "Dw=="}, {1, "\x10", "EA=="}, {1, "\x11", "EQ=="},
    {1, "\x12", "Eg=="}, {1, "\x13", "Ew=="}, {1, "\x14", "FA=="},
    {1, "\x15", "FQ=="}, {1, "\x16", "Fg=="}, {1, "\x17", "Fw=="},
    {1, "\x18", "GA=="}, {1, "\x19", "GQ=="}, {1, "\x1a", "Gg=="},
    {1, "\x1b", "Gw=="}, {1, "\x1c", "HA=="}, {1, "\x1d", "HQ=="},
    {1, "\x1e", "Hg=="}, {1, "\x1f", "Hw=="}, {1, "\x20", "IA=="},
    {1, "\x21", "IQ=="}, {1, "\x22", "Ig=="}, {1, "\x23", "Iw=="},
    {1, "\x24", "JA=="}, {1, "\x25", "JQ=="}, {1, "\x26", "Jg=="},
    {1, "\x27", "Jw=="}, {1, "\x28", "KA=="}, {1, "\x29", "KQ=="},
    {1, "\x2a", "Kg=="}, {1, "\x2b", "Kw=="}, {1, "\x2c", "LA=="},
    {1, "\x2d", "LQ=="}, {1, "\x2e", "Lg=="}, {1, "\x2f", "Lw=="},
    {1, "\x30", "MA=="}, {1, "\x31", "MQ=="}, {1, "\x32", "Mg=="},
    {1, "\x33", "Mw=="}, {1, "\x34", "NA=="}, {1, "\x35", "NQ=="},
    {1, "\x36", "Ng=="}, {1, "\x37", "Nw=="}, {1, "\x38", "OA=="},
    {1, "\x39", "OQ=="}, {1, "\x3a", "Og=="}, {1, "\x3b", "Ow=="},
    {1, "\x3c", "PA=="}, {1, "\x3d", "PQ=="}, {1, "\x3e", "Pg=="},
    {1, "\x3f", "Pw=="}, {1, "\x40", "QA=="}, {1, "\x41", "QQ=="},
    {1, "\x42", "Qg=="}, {1, "\x43", "Qw=="}, {1, "\x44", "RA=="},
    {1, "\x45", "RQ=="}, {1, "\x46", "Rg=="}, {1, "\x47", "Rw=="},
    {1, "\x48", "SA=="}, {1, "\x49", "SQ=="}, {1, "\x4a", "Sg=="},
    {1, "\x4b", "Sw=="}, {1, "\x4c", "TA=="}, {1, "\x4d", "TQ=="},
    {1, "\x4e", "Tg=="}, {1, "\x4f", "Tw=="}, {1, "\x50", "UA=="},
    {1, "\x51", "UQ=="}, {1, "\x52", "Ug=="}, {1, "\x53", "Uw=="},
    {1, "\x54", "VA=="}, {1, "\x55", "VQ=="}, {1, "\x56", "Vg=="},
    {1, "\x57", "Vw=="}, {1, "\x58", "WA=="}, {1, "\x59", "WQ=="},
    {1, "\x5a", "Wg=="}, {1, "\x5b", "Ww=="}, {1, "\x5c", "XA=="},
    {1, "\x5d", "XQ=="}, {1, "\x5e", "Xg=="}, {1, "\x5f", "Xw=="},
    {1, "\x60", "YA=="}, {1, "\x61", "YQ=="}, {1, "\x62", "Yg=="},
    {1, "\x63", "Yw=="}, {1, "\x64", "ZA=="}, {1, "\x65", "ZQ=="},
    {1, "\x66", "Zg=="}, {1, "\x67", "Zw=="}, {1, "\x68", "aA=="},
    {1, "\x69", "aQ=="}, {1, "\x6a", "ag=="}, {1, "\x6b", "aw=="},
    {1, "\x6c", "bA=="}, {1, "\x6d", "bQ=="}, {1, "\x6e", "bg=="},
    {1, "\x6f", "bw=="}, {1, "\x70", "cA=="}, {1, "\x71", "cQ=="},
    {1, "\x72", "cg=="}, {1, "\x73", "cw=="}, {1, "\x74", "dA=="},
    {1, "\x75", "dQ=="}, {1, "\x76", "dg=="}, {1, "\x77", "dw=="},
    {1, "\x78", "eA=="}, {1, "\x79", "eQ=="}, {1, "\x7a", "eg=="},
    {1, "\x7b", "ew=="}, {1, "\x7c", "fA=="}, {1, "\x7d", "fQ=="},
    {1, "\x7e", "fg=="}, {1, "\x7f", "fw=="}, {1, "\x80", "gA=="},
    {1, "\x81", "gQ=="}, {1, "\x82", "gg=="}, {1, "\x83", "gw=="},
    {1, "\x84", "hA=="}, {1, "\x85", "hQ=="}, {1, "\x86", "hg=="},
    {1, "\x87", "hw=="}, {1, "\x88", "iA=="}, {1, "\x89", "iQ=="},
    {1, "\x8a", "ig=="}, {1, "\x8b", "iw=="}, {1, "\x8c", "jA=="},
    {1, "\x8d", "jQ=="}, {1, "\x8e", "jg=="}, {1, "\x8f", "jw=="},
    {1, "\x90", "kA=="}, {1, "\x91", "kQ=="}, {1, "\x92", "kg=="},
    {1, "\x93", "kw=="}, {1, "\x94", "lA=="}, {1, "\x95", "lQ=="},
    {1, "\x96", "lg=="}, {1, "\x97", "lw=="}, {1, "\x98", "mA=="},
    {1, "\x99", "mQ=="}, {1, "\x9a", "mg=="}, {1, "\x9b", "mw=="},
    {1, "\x9c", "nA=="}, {1, "\x9d", "nQ=="}, {1, "\x9e", "ng=="},
    {1, "\x9f", "nw=="}, {1, "\xa0", "oA=="}, {1, "\xa1", "oQ=="},
    {1, "\xa2", "og=="}, {1, "\xa3", "ow=="}, {1, "\xa4", "pA=="},
    {1, "\xa5", "pQ=="}, {1, "\xa6", "pg=="}, {1, "\xa7", "pw=="},
    {1, "\xa8", "qA=="}, {1, "\xa9", "qQ=="}, {1, "\xaa", "qg=="},
    {1, "\xab", "qw=="}, {1, "\xac", "rA=="}, {1, "\xad", "rQ=="},
    {1, "\xae", "rg=="}, {1, "\xaf", "rw=="}, {1, "\xb0", "sA=="},
    {1, "\xb1", "sQ=="}, {1, "\xb2", "sg=="}, {1, "\xb3", "sw=="},
    {1, "\xb4", "tA=="}, {1, "\xb5", "tQ=="}, {1, "\xb6", "tg=="},
    {1, "\xb7", "tw=="}, {1, "\xb8", "uA=="}, {1, "\xb9", "uQ=="},
    {1, "\xba", "ug=="}, {1, "\xbb", "uw=="}, {1, "\xbc", "vA=="},
    {1, "\xbd", "vQ=="}, {1, "\xbe", "vg=="}, {1, "\xbf", "vw=="},
    {1, "\xc0", "wA=="}, {1, "\xc1", "wQ=="}, {1, "\xc2", "wg=="},
    {1, "\xc3", "ww=="}, {1, "\xc4", "xA=="}, {1, "\xc5", "xQ=="},
    {1, "\xc6", "xg=="}, {1, "\xc7", "xw=="}, {1, "\xc8", "yA=="},
    {1, "\xc9", "yQ=="}, {1, "\xca", "yg=="}, {1, "\xcb", "yw=="},
    {1, "\xcc", "zA=="}, {1, "\xcd", "zQ=="}, {1, "\xce", "zg=="},
    {1, "\xcf", "zw=="}, {1, "\xd0", "0A=="}, {1, "\xd1", "0Q=="},
    {1, "\xd2", "0g=="}, {1, "\xd3", "0w=="}, {1, "\xd4", "1A=="},
    {1, "\xd5", "1Q=="}, {1, "\xd6", "1g=="}, {1, "\xd7", "1w=="},
    {1, "\xd8", "2A=="}, {1, "\xd9", "2Q=="}, {1, "\xda", "2g=="},
    {1, "\xdb", "2w=="}, {1, "\xdc", "3A=="}, {1, "\xdd", "3Q=="},
    {1, "\xde", "3g=="}, {1, "\xdf", "3w=="}, {1, "\xe0", "4A=="},
    {1, "\xe1", "4Q=="}, {1, "\xe2", "4g=="}, {1, "\xe3", "4w=="},
    {1, "\xe4", "5A=="}, {1, "\xe5", "5Q=="}, {1, "\xe6", "5g=="},
    {1, "\xe7", "5w=="}, {1, "\xe8", "6A=="}, {1, "\xe9", "6Q=="},
    {1, "\xea", "6g=="}, {1, "\xeb", "6w=="}, {1, "\xec", "7A=="},
    {1, "\xed", "7Q=="}, {1, "\xee", "7g=="}, {1, "\xef", "7w=="},
    {1, "\xf0", "8A=="}, {1, "\xf1", "8Q=="}, {1, "\xf2", "8g=="},
    {1, "\xf3", "8w=="}, {1, "\xf4", "9A=="}, {1, "\xf5", "9Q=="},
    {1, "\xf6", "9g=="}, {1, "\xf7", "9w=="}, {1, "\xf8", "+A=="},
    {1, "\xf9", "+Q=="}, {1, "\xfa", "+g=="}, {1, "\xfb", "+w=="},
    {1, "\xfc", "/A=="}, {1, "\xfd", "/Q=="}, {1, "\xfe", "/g=="},
    {1, "\xff", "/w=="},
};

constexpr EncodedData kRandomTestData[] = {
    {2, "\x74\x6d", "dG0="},
    {2, "\x22\x86", "IoY="},
    {3, "\xc0\xa2\x1c", "wKIc"},
    {3, "\xa9\x67\xfb", "qWf7"},
    {4, "\x77\xe1\x63\x51", "d+FjUQ=="},
    {4, "\x7d\xa6\x8c\x5e", "faaMXg=="},
    {5, "\x68\xaa\x19\x59\xd0", "aKoZWdA="},
    {5, "\x46\x73\xd3\x54\x7e", "RnPTVH4="},
    {6, "\x3f\xe8\x18\x4c\xe8\xf4", "P+gYTOj0"},
    {6, "\x0a\xdd\x39\xbc\x1f\x65", "Ct05vB9l"},
    {7, "\xc4\x5e\x4a\x6d\x4a\x04\xb6", "xF5KbUoEtg=="},
    {7, "\x12\xe9\xf4\xaa\x2e\x4c\x31", "Eun0qi5MMQ=="},
    {8, "\x55\x8c\x60\xcc\xc4\x7d\x99\x1f", "VYxgzMR9mR8="},
    {8, "\xee\x21\x88\x2a\x0f\x7e\x76\xd7", "7iGIKg9+dtc="},
    {9, "\xba\x40\x1d\x06\x92\xce\xc2\x8a\x28", "ukAdBpLOwooo"},
    {9, "\xcc\x89\xf5\xeb\x49\x91\xa6\xa6\x88", "zIn160mRpqaI"},
    {10, "\x55\x6b\x11\xe4\xc2\x22\xb0\x40\x14\x53", "VWsR5MIisEAUUw=="},
    {10, "\xd3\x1e\xc4\xe5\x06\x60\x37\x51\x10\x48", "0x7E5QZgN1EQSA=="},
    {11, "\x98\xae\x09\x8c\x61\x40\xbf\x77\xde\xd9\x0d", "mK4JjGFAv3fe2Q0="},
    {11, "\x86\x39\x06\xa1\xc6\xfc\xcf\x30\x21\xba\xdf", "hjkGocb8zzAhut8="},
};

void ExpectEncodeDecodeSizesMatch(const EncodedData& data) {
  const size_t actual_encoded_size = std::strlen(data.encoded_data);
  const size_t actual_raw_size = data.binary_size;

  // Encoded size.
  ASSERT_EQ(EncodedSize(data.binary_size), actual_encoded_size);
  ASSERT_EQ(PW_BASE64_ENCODED_SIZE(data.binary_size), actual_encoded_size);

  // Max decoded size. Do upper & lower bounds.
  ASSERT_GE(MaxDecodedSize(actual_encoded_size), actual_raw_size);
  ASSERT_GE(PW_BASE64_MAX_DECODED_SIZE(actual_encoded_size), actual_raw_size);
  ASSERT_LE(MaxDecodedSize(actual_encoded_size), actual_raw_size + 2);
  ASSERT_LE(PW_BASE64_MAX_DECODED_SIZE(actual_encoded_size),
            actual_raw_size + 2);
}

// Tests both the C++ constexpr variant and the C macro variant.
TEST(Base64, EncodedAndDecodedSize) {
  for (const EncodedData& data : kSingleCharTestData) {
    ExpectEncodeDecodeSizesMatch(data);
  }
  for (const EncodedData& data : kRandomTestData) {
    ExpectEncodeDecodeSizesMatch(data);
  }
}

TEST(Base64, Encode_SingleChar) {
  char output[32];
  for (const EncodedData& data : kSingleCharTestData) {
    const size_t size = EncodedSize(data.binary_size);
    ASSERT_EQ(std::strlen(data.encoded_data), size);
    Encode(as_bytes(span(data.binary_data, data.binary_size)), output);
    output[size] = '\0';
    EXPECT_STREQ(data.encoded_data, output);
  }
}

TEST(Base64, Encode_RandomData) {
  char output[128];
  for (const EncodedData& data : kRandomTestData) {
    const size_t size = EncodedSize(data.binary_size);
    ASSERT_EQ(std::strlen(data.encoded_data), size);
    Encode(as_bytes(span(data.binary_data, data.binary_size)), output);
    output[size] = '\0';
    EXPECT_STREQ(data.encoded_data, output);
  }
}

TEST(Base64, Encode_BoundaryCheck) {
  constexpr std::byte data[] = {std::byte{'h'}, std::byte{'i'}};
  char output[5] = {};

  EXPECT_EQ(0u, Encode(data, span(output, 3)));
  EXPECT_STREQ("", output);
  EXPECT_EQ(4u, Encode(data, span(output, 4)));
  EXPECT_STREQ("aGk=", output);
}

TEST(Base64, Encode_RandomData_ToInlineString) {
  for (const EncodedData& data : kRandomTestData) {
    auto b64 = Encode<11>(as_bytes(span(data.binary_data, data.binary_size)));
    EXPECT_EQ(data.encoded_data, b64);
  }
}

TEST(Base64, Encode_RandomData_ToInlineStringAppend) {
  for (const EncodedData& data : kRandomTestData) {
    InlineString<32> output("Wow:");

    InlineString<32> expected(output);
    expected.append(data.encoded_data);

    Encode(as_bytes(span(data.binary_data, data.binary_size)), output);
    EXPECT_EQ(expected, output);
  }
}

TEST(Base64, Decode_SingleChar) {
  char output[32];
  for (const EncodedData& data : kSingleCharTestData) {
    size_t binary_size = Decode(data.encoded_data, output);
    ASSERT_EQ(binary_size, data.binary_size);
    EXPECT_EQ(0, std::memcmp(data.binary_data, output, data.binary_size));
  }
}

TEST(Base64, Decode_RandomData) {
  char output[128];
  for (const EncodedData& data : kRandomTestData) {
    size_t binary_size = Decode(data.encoded_data, output);
    ASSERT_EQ(binary_size, data.binary_size);
    EXPECT_EQ(0, std::memcmp(data.binary_data, output, data.binary_size));
  }
}

TEST(Base64, Decode_BoundaryCheck) {
  constexpr const char encoded_data[] = "aGk=";
  std::byte output[4] = {};

  EXPECT_EQ(0u, Decode(encoded_data, span(output, 2)));
  EXPECT_STREQ("", reinterpret_cast<const char*>(output));
  EXPECT_EQ(2u, Decode(encoded_data, span(output, 3)));
  EXPECT_STREQ("hi", reinterpret_cast<const char*>(output));
}

TEST(Base64, Decode_InPlace) {
  constexpr const char expected[] = "This is a secret message";
  char buf[] = "VGhpcyBpcyBhIHNlY3JldCBtZXNzYWdl";
  EXPECT_EQ(sizeof(expected) - 1, Decode(buf, buf));
  EXPECT_EQ(0, std::memcmp(expected, buf, sizeof(expected) - 1));
}

TEST(Base64, DecodeString_InPlace) {
  constexpr const char expected[] = "This is a secret message";
  InlineBasicString buf = "VGhpcyBpcyBhIHNlY3JldCBtZXNzYWdl";
  DecodeInPlace(buf);
  EXPECT_EQ(sizeof(expected) - 1, buf.size());
  EXPECT_EQ(expected, buf);

  buf.clear();
  DecodeInPlace(buf);
  EXPECT_TRUE(buf.empty());
}

TEST(Base64, Decode_UrlSafeDecode) {
  char output[9] = {};

  EXPECT_TRUE(IsValid("+f//WW8h"));
  EXPECT_TRUE(IsValid("-f__WW8h"));

  EXPECT_EQ(6u, Decode("-f__WW8h", output));
  EXPECT_STREQ("\xf9\xff\xffYo!", output);
}

TEST(Base64, Empty) {
  char buffer[] = "DO NOT TOUCH";
  EXPECT_EQ(0u, EncodedSize(0));
  Encode(as_bytes(span("Something cool!!!", 0)), buffer);
  EXPECT_STREQ("DO NOT TOUCH", buffer);

  EXPECT_EQ(0u, MaxDecodedSize(0));
  // NOLINTNEXTLINE(bugprone-string-constructor)
  EXPECT_EQ(0u, Decode(std::string_view("nothing please", 0), buffer));
  EXPECT_STREQ("DO NOT TOUCH", buffer);
}

TEST(Base64, ExampleFromRfc3548Section7) {
  constexpr uint8_t input[] = {0x14, 0xfb, 0x9c, 0x03, 0xd9, 0x7e};
  char output[EncodedSize(sizeof(input)) + 1] = {};

  Encode(as_bytes(span(input)), output);
  EXPECT_STREQ("FPucA9l+", output);
  Encode(as_bytes(span(input, 5)), output);
  EXPECT_STREQ("FPucA9k=", output);
  Encode(as_bytes(span(input, 4)), output);
  EXPECT_STREQ("FPucAw==", output);

  EXPECT_EQ(6u, Decode("FPucA9l+", output));
  EXPECT_EQ(0, std::memcmp(input, output, 6));
  EXPECT_EQ(5u, Decode("FPucA9k=", output));
  EXPECT_EQ(0, std::memcmp(input, output, 5));
  EXPECT_EQ(4u, Decode("FPucAw==", output));
  EXPECT_EQ(0, std::memcmp(input, output, 4));
}

TEST(Base64, ExampleFromRfc4648Section9) {
  char output[EncodedSize(sizeof("foobar")) + 1] = {};
  const std::byte* foobar = reinterpret_cast<const std::byte*>("foobar");

  Encode(span(foobar, 0), output);
  EXPECT_STREQ("", output);
  Encode(span(foobar, 1), output);
  EXPECT_STREQ("Zg==", output);
  Encode(span(foobar, 2), output);
  EXPECT_STREQ("Zm8=", output);
  Encode(span(foobar, 3), output);
  EXPECT_STREQ("Zm9v", output);
  Encode(span(foobar, 4), output);
  EXPECT_STREQ("Zm9vYg==", output);
  Encode(span(foobar, 5), output);
  EXPECT_STREQ("Zm9vYmE=", output);
  Encode(span(foobar, 6), output);
  EXPECT_STREQ("Zm9vYmFy", output);

  std::memset(output, '\0', sizeof(output));
  EXPECT_EQ(0u, Decode("", output));
  EXPECT_STREQ("", output);
  EXPECT_EQ(1u, Decode("Zg==", output));
  EXPECT_STREQ("f", output);
  EXPECT_EQ(2u, Decode("Zm8=", output));
  EXPECT_STREQ("fo", output);
  EXPECT_EQ(3u, Decode("Zm9v", output));
  EXPECT_STREQ("foo", output);
  EXPECT_EQ(4u, Decode("Zm9vYg==", output));
  EXPECT_STREQ("foob", output);
  EXPECT_EQ(5u, Decode("Zm9vYmE=", output));
  EXPECT_STREQ("fooba", output);
  EXPECT_EQ(6u, Decode("Zm9vYmFy", output));
  EXPECT_STREQ("foobar", output);
}

TEST(Base64, DecodeIgnoresOnePaddingByte) {
  std::array<char, 6> decode_buffer{'?', '?', '?', '?', '?', '?'};
  EXPECT_EQ(Decode("AAAAAAA=", decode_buffer.data()), 5u);
  EXPECT_EQ(decode_buffer[0], '\0');
  EXPECT_EQ(decode_buffer[1], '\0');
  EXPECT_EQ(decode_buffer[2], '\0');
  EXPECT_EQ(decode_buffer[3], '\0');
  EXPECT_EQ(decode_buffer[4], '\0');
  EXPECT_EQ(decode_buffer[5], '?');
}

TEST(Base64, DecodeIgnoresTwoPaddingBytes) {
  std::array<char, 6> decode_buffer{'?', '?', '?', '?', '?', '?'};
  EXPECT_EQ(Decode("AAAAAA==", decode_buffer.data()), 4u);
  EXPECT_EQ(decode_buffer[0], '\0');
  EXPECT_EQ(decode_buffer[1], '\0');
  EXPECT_EQ(decode_buffer[2], '\0');
  EXPECT_EQ(decode_buffer[3], '\0');
  EXPECT_EQ(decode_buffer[4], '?');
  EXPECT_EQ(decode_buffer[5], '?');
}

TEST(Base64, IsValid) {
  EXPECT_TRUE(IsValid(""));
  for (const EncodedData& data : kSingleCharTestData) {
    ASSERT_TRUE(IsValid(data.encoded_data));
  }
  for (const EncodedData& data : kRandomTestData) {
    ASSERT_TRUE(IsValid(data.encoded_data));
  }
}

TEST(Base64, IsValidIncorrectLength) {
  EXPECT_FALSE(IsValid("a"));
  EXPECT_FALSE(IsValid("aa"));
  EXPECT_FALSE(IsValid("aaa"));

  EXPECT_FALSE(IsValid("AAAAa"));
  EXPECT_FALSE(IsValid("AAAAaa"));
  EXPECT_FALSE(IsValid("AAAAaaa"));
}

TEST(Base64, IsValidIncorrectPadding) {
  EXPECT_FALSE(IsValid("AAAAaa=a"));
  EXPECT_TRUE(IsValid("AAAAaaa="));

  EXPECT_FALSE(IsValid("aa=a"));
  EXPECT_TRUE(IsValid("aaa="));

  EXPECT_FALSE(IsValid("="));
  EXPECT_FALSE(IsValid("=="));
  EXPECT_FALSE(IsValid("==="));
  EXPECT_FALSE(IsValid("====="));
}

PW_CONSTEXPR_TEST(Base64, DecodedSize_Valid, {
  PW_TEST_EXPECT_EQ(DecodedSize(""), 0u);
  PW_TEST_EXPECT_EQ(DecodedSize("ab=="), 1u);
  PW_TEST_EXPECT_EQ(DecodedSize("abc="), 2u);
  PW_TEST_EXPECT_EQ(DecodedSize("abcd"), 3u);
  PW_TEST_EXPECT_EQ(DecodedSize("1234ab=="), 4u);
  PW_TEST_EXPECT_EQ(DecodedSize("1234abc="), 5u);
  PW_TEST_EXPECT_EQ(DecodedSize("1234abcd"), 6u);
});

PW_CONSTEXPR_TEST(Base64, DecodedSize_Invalid, {
  PW_TEST_EXPECT_EQ(DecodedSize("a"), 0u);
  PW_TEST_EXPECT_EQ(DecodedSize("ab"), 0u);
  PW_TEST_EXPECT_EQ(DecodedSize("abc"), 0u);
  PW_TEST_EXPECT_EQ(DecodedSize("1234ab"), 0u);
  PW_TEST_EXPECT_EQ(DecodedSize("1234abc"), 0u);
});

PW_CONSTEXPR_TEST(Base64, MaxDecodedSize_Valid, {
  PW_TEST_EXPECT_EQ(MaxDecodedSize(0), 0u);
  PW_TEST_EXPECT_EQ(MaxDecodedSize(4), 3u);
  PW_TEST_EXPECT_EQ(MaxDecodedSize(8), 6u);
  PW_TEST_EXPECT_EQ(MaxDecodedSize(12), 9u);
  PW_TEST_EXPECT_EQ(MaxDecodedSize(16), 12u);
});

PW_CONSTEXPR_TEST(Base64, MaxDecodedSize_Invalid, {
  for (unsigned i = 0; i < 20; ++i) {
    if ((i % 4) != 0) {
      PW_TEST_EXPECT_EQ(MaxDecodedSize(i), 0u);
    }
  }
});

// Functions that call the Base64 API from C. These are defined in
// base64_test.c; no point in having a separate header.
extern "C" {

void pw_Base64CallEncode(const void* binary_data,
                         const size_t binary_size_bytes,
                         char* output);

size_t pw_Base64CallDecode(const char* base64,
                           size_t base64_size_bytes,
                           void* output);

bool pw_Base64CallIsValid(const char* base64_data, size_t base64_size);

}  // extern "C"

constexpr const char kBase64[] = "aaaabbbbcc#%";

// Ensure that the C API works correctly from a C-only context.
TEST(Base64, IsValid_Ok) {
  EXPECT_TRUE(IsValid(std::string_view(kBase64, 4)));
  EXPECT_TRUE(IsValid(std::string_view(kBase64, 8)));
}

TEST(Base64, IsValid_IncorrectSize) {
  EXPECT_FALSE(IsValid(std::string_view(kBase64, 5)));
  EXPECT_FALSE(IsValid(std::string_view(kBase64, 6)));
  EXPECT_FALSE(IsValid(std::string_view(kBase64, 7)));
  EXPECT_FALSE(IsValid(std::string_view(kBase64, 10)));
}

TEST(Base64, IsValid_InvalidCharacters) {
  EXPECT_FALSE(IsValid(std::string_view(kBase64, 11)));
  EXPECT_FALSE(IsValid(std::string_view(kBase64, 12)));
}

TEST(Base64CLinkage, IsValid_Ok) {
  EXPECT_TRUE(pw_Base64CallIsValid(kBase64, 4));
  EXPECT_TRUE(pw_Base64CallIsValid(kBase64, 8));
}

TEST(Base64CLinkage, IsValid_IncorrectSize) {
  EXPECT_FALSE(pw_Base64CallIsValid(kBase64, 5));
  EXPECT_FALSE(pw_Base64CallIsValid(kBase64, 6));
  EXPECT_FALSE(pw_Base64CallIsValid(kBase64, 7));
  EXPECT_FALSE(pw_Base64CallIsValid(kBase64, 10));
}

TEST(Base64CLinkage, IsValid_InvalidCharacters) {
  EXPECT_FALSE(pw_Base64CallIsValid(kBase64, 11));
  EXPECT_FALSE(pw_Base64CallIsValid(kBase64, 12));
}

TEST(Base64CLinkage, Encode) {
  char output[EncodedSize(sizeof("foobar")) + 1] = {};

  pw_Base64CallEncode("", 0, output);
  EXPECT_STREQ("", output);
  pw_Base64CallEncode("f", 1, output);
  EXPECT_STREQ("Zg==", output);
  pw_Base64CallEncode("fo", 2, output);
}

TEST(Base64CLinkage, Decode) {
  char output[EncodedSize(sizeof("foobar")) + 1] = {};

  EXPECT_EQ(0u, pw_Base64CallDecode("", 0, output));
  EXPECT_STREQ("", output);
  EXPECT_EQ(1u, pw_Base64CallDecode("Zg==", 4, output));
  EXPECT_STREQ("f", output);
  EXPECT_EQ(2u, pw_Base64CallDecode("Zm8=", 4, output));
  EXPECT_STREQ("fo", output);
}

}  // namespace
}  // namespace pw::base64
