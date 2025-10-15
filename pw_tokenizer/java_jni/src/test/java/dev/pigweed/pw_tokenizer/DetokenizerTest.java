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

package dev.pigweed.pw_tokenizer;

import static com.google.common.truth.Truth.assertThat;
import static org.junit.Assert.assertThrows;

import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;

@RunWith(JUnit4.class)
public final class DetokenizerTest {
  // This variable contains the binary version of the following token database:
  //
  //   00000001,          ,"","This is message 1"
  //   00000002,1998-09-04,"","This is message 2: %s"
  //   00000003,          ,"","This is message 3: %d, %f"
  //
  // The database was created with the pw_tokenizer Python tooling and converted to source code with
  // `xxd -i`.
  // clang-format off
  private static final byte[] BINARY_DATABASE = {
      0x54, 0x4f, 0x4b, 0x45, 0x4e, 0x53, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x01, 0x00, 0x00, 0x00, (byte) 0xff, (byte) 0xff, (byte) 0xff, (byte) 0xff, 0x02, 0x00,
      0x00, 0x00, 0x04, 0x09, (byte) 0xce, 0x07, 0x03, 0x00, 0x00, 0x00, (byte) 0xff, (byte) 0xff,
      (byte) 0xff, (byte) 0xff, 0x54, 0x68, 0x69, 0x73, 0x20, 0x69, 0x73, 0x20, 0x6d, 0x65, 0x73,
      0x73, 0x61, 0x67, 0x65, 0x20, 0x31, 0x00, 0x54, 0x68, 0x69, 0x73, 0x20, 0x69, 0x73, 0x20,
      0x6d, 0x65, 0x73, 0x73, 0x61, 0x67, 0x65, 0x20, 0x32, 0x3a, 0x20, 0x25, 0x73, 0x00, 0x54,
      0x68, 0x69, 0x73, 0x20, 0x69, 0x73, 0x20, 0x6d, 0x65, 0x73, 0x73, 0x61, 0x67, 0x65, 0x20,
      0x33, 0x3a, 0x20, 0x25, 0x64, 0x2c, 0x20, 0x25, 0x66, 0x00
  }; // clang-format on

  private static final String CSV_DATABASE = // clang-format off
      "00000001,          ,\"\",\"This is message 1\"\n" +
      "00000002,1998-09-04,\"\",\"This is message 2: %s\"\n" +
      "00000003,          ,\"\",\"This is message 3: %d, %f\"\n";
  // clang-format on

  private static final String DATABASE_2 = // clang-format off
      "00000000,          ,\"\",\"Hello ${nested}#%08x\"\n" +
      "00000001,          ,\"nested\",\"world\"\n";
  // clang-format on

  private static final byte[] NESTED_HELLO_WORLD = new byte[] {0, 0, 0, 0, 2};

  private final Detokenizer detokenizerBinary = Detokenizer.fromBinary(BINARY_DATABASE);

  private final Detokenizer detokenizerCsv = Detokenizer.fromCsv(CSV_DATABASE);

  @Test
  public void lookup_oneResult() {
    assertThat(detokenizerBinary.lookup(1)).containsExactly("This is message 1");
    assertThat(detokenizerCsv.lookup(2, Detokenizer.DEFAULT_DOMAIN))
        .containsExactly("This is message 2: %s");
  }

  @Test
  public void lookup_noResults() {
    assertThat(detokenizerBinary.lookup(1, "wrong_domain")).isEmpty();
    assertThat(detokenizerCsv.lookup(0)).isEmpty();
  }

  @Test
  public void lookup_multipleMatches() {
    Detokenizer detokenizer = Detokenizer.fromCsv( // clang-format off
        "00000100,          ,\"\",\"Uh oh!\"\n" +
        "00000100,          ,\"\",\"Oh NO!\"\n" +
        "00000100,          ,\"other\",\"This is fine\"\n");
    // clang-format on
    assertThat(detokenizer.lookup(0x100, "")).containsExactly("Uh oh!", "Oh NO!");
    assertThat(detokenizer.lookup(0x100, "other")).containsExactly("This is fine");
  }

  @Test
  public void lookupByteArray_padsToken() {
    assertThat(detokenizerBinary.lookup(new byte[] {1})).containsExactly("This is message 1");
    assertThat(detokenizerCsv.lookup(new byte[] {1, 0}, Detokenizer.DEFAULT_DOMAIN))
        .containsExactly("This is message 1");
  }

  @Test
  public void lookupByteArray_noResults() {
    byte[] message = new byte[] {0, 0, 0, 0};
    assertThat(detokenizerBinary.lookup(message, "wrong_domain")).isEmpty();
    assertThat(detokenizerCsv.lookup(message)).isEmpty();
  }

  @Test
  public void lookupByteArray_emptyMessage_throwsException() {
    assertThrows(IllegalArgumentException.class, () -> detokenizerBinary.lookup(new byte[0]));
  }

  @Test
  public void lookupByteArray_multipleMatches() {
    Detokenizer detokenizer = Detokenizer.fromCsv( // clang-format off
        "0A0B0C0D,          ,\"\",\"Uh oh!\"\n" +
        "0A0B0C0D,          ,\"\",\"Oh NO!\"\n" +
        "0A0B0C0D,          ,\"other\",\"This is fine\"\n");
    // clang-format on
    byte[] message = new byte[] {0x0D, 0x0C, 0x0B, 0x0A};
    assertThat(detokenizer.lookup(message, "")).containsExactly("Uh oh!", "Oh NO!");
    assertThat(detokenizer.lookup(message, "other")).containsExactly("This is fine");
  }

  @Test
  public void detokenize_emptyMessage() {
    assertThat(detokenizerBinary.detokenize(new byte[0])).isNull();
    assertThat(detokenizerCsv.detokenize(new byte[0])).isNull();
  }

  @Test
  public void detokenizeText_emptyMessage() {
    assertThat(detokenizerBinary.detokenizeText("")).isEmpty();
    assertThat(detokenizerCsv.detokenizeText("")).isEmpty();
  }

  @Test
  public void detokenize_noArgs_returnsString() {
    byte[] message = new byte[] {1, 0, 0, 0};
    assertThat(detokenizerBinary.detokenize(message)).isEqualTo("This is message 1");
    assertThat(detokenizerCsv.detokenize(message)).isEqualTo("This is message 1");
  }

  @Test
  public void detokenize_unknownToken_returnsNull() {
    byte[] message = new byte[] {4, 0, 0, 0};
    assertThat(detokenizerBinary.detokenize(message)).isNull();
    assertThat(detokenizerCsv.detokenize(message)).isNull();
  }

  @Test
  public void detokenize_collision_returnsNull() {
    Detokenizer detokenizer = Detokenizer.fromCsv( // clang-format off
        "00000001,          ,\"\",\"Uh oh!\"\n" +
        "00000001,          ,\"\",\"Oh NO!\"\n" +
        "00000002,          ,\"\",\"Hey %d\"\n" +
        "00000002,          ,\"\",\"Hi %d!\"\n");
    // clang-format on
    assertThat(detokenizer.detokenize(new byte[] {1, 0, 0, 0})).isNull();
    assertThat(detokenizer.detokenize(new byte[] {2, 0, 0, 0})).isNull();
  }

  @Test
  public void detokenize_missingArgument_returnsNull() {
    Detokenizer detokenizer = Detokenizer.fromCsv("00000001,          ,\"\",\"The %d\"\n");
    assertThat(detokenizer.detokenize(new byte[] {1, 0, 0, 0})).isNull();
  }

  @Test
  public void detokenize_stringArg_returnsFormattedString() {
    byte[] message = new byte[] {2, 0, 0, 0, 5, 'w', 'o', 'r', 'l', 'd'};
    assertThat(detokenizerBinary.detokenize(message)).isEqualTo("This is message 2: world");
    assertThat(detokenizerCsv.detokenize(message)).isEqualTo("This is message 2: world");
  }

  @Test
  public void detokenize_ignoresNestedMessage() {
    Detokenizer detokenizer = Detokenizer.fromCsv(DATABASE_2);
    assertThat(detokenizer.detokenize(NESTED_HELLO_WORLD)).isEqualTo("Hello ${nested}#00000001");
  }

  @Test
  public void recurisveDetokenize_decodesNestedMessage() {
    Detokenizer detokenizer = Detokenizer.fromCsv(DATABASE_2);
    assertThat(detokenizer.recursiveDetokenize(NESTED_HELLO_WORLD)).isEqualTo("Hello world");
  }

  @Test
  public void detokenizeText_noTokens_returnsOriginalString() {
    assertThat(detokenizerBinary.detokenizeText("this is a test")).isEqualTo("this is a test");
    assertThat(detokenizerCsv.detokenizeText("this is a test")).isEqualTo("this is a test");
  }

  @Test
  public void detokenizeText_withToken_returnsDetokenizedString() {
    // "$AQAAAA==" is the Base64 encoding of the token 0x00000001.
    String message = "Message! $AQAAAA== Yay!";
    assertThat(detokenizerBinary.detokenizeText(message))
        .isEqualTo("Message! This is message 1 Yay!");
    assertThat(detokenizerCsv.detokenizeText(message)).isEqualTo("Message! This is message 1 Yay!");
  }
}
