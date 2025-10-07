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

/**
 * This is a Java Native Interface (JNI) wrapper for the Detokenizer class.
 */
package dev.pigweed.pw_tokenizer;

import static java.nio.charset.StandardCharsets.UTF_8;

import com.github.fmeum.rules_jni.RulesJni;
import com.google.common.collect.ImmutableList;
import javax.annotation.Nullable;

/**
 * Detokenizer decodes tokenized messages encoded with pw_tokenizer.
 *
 * This class uses JNI to wrap the C++ Detokenizer class.
 *
 * The close() method MUST be called when the Detokenizer is no longer needed to prevent memory
 * leaks. Detokenizer is AutoCloseable, so it may used in a try-with-resources statement.
 * Otherwise, close() must be called manually.
 */
public final class Detokenizer implements AutoCloseable {
  static {
    RulesJni.loadLibrary("pw_tokenizer_jni", Detokenizer.class);
  }

  public static String DEFAULT_DOMAIN = "";

  // The handle (pointer) to the C++ detokenizer instance.
  private long handle;

  /** Creates a Detokenizer from a CSV token database in a string. */
  public static Detokenizer fromCsv(String csvTokenDatabase) {
    return new Detokenizer(newNativeDetokenizerCsv(csvTokenDatabase.getBytes(UTF_8)));
  }

  /** Creates a Detokenizer from a UTF-8 encoded CSV token database. */
  public static Detokenizer fromCsv(byte[] csvTokenDatabaseUtf8) {
    return new Detokenizer(newNativeDetokenizerCsv(csvTokenDatabaseUtf8));
  }

  /**
   * Creates a Detokenizer from a binary token database.
   *
   * Note that binary databases do NOT support domains, so nested detokenization may not succeeded.
   * Use a CSV database with domains for full nested detokenization support.
   */
  public static Detokenizer fromBinary(byte[] binaryTokenDatabase) {
    return new Detokenizer(newNativeDetokenizerBinary(binaryTokenDatabase));
  }

  private Detokenizer(long nativeHandle) {
    handle = nativeHandle;
  }

  /**
   * Looks up the token in the default domain, returning any matching strings in the database.
   */
  public ImmutableList<String> lookup(int token) {
    return lookup(token, DEFAULT_DOMAIN);
  }

  /**
   * Looks up the token in the specified domain, returning any matching strings in the database.
   */
  public ImmutableList<String> lookup(int token, String domain) {
    String[] result = lookupNative(handle, token, domain);
    return result == null ? ImmutableList.of() : ImmutableList.copyOf(result);
  }

  /**
   * Reads the token from the tokenized message and returns any matching strings in the database.
   */
  public ImmutableList<String> lookup(byte[] tokenizedMessage) {
    return lookup(tokenizedMessage, DEFAULT_DOMAIN);
  }

  /**
   * Reads the token from the tokenized message and returns any matching strings in the database.
   */
  public ImmutableList<String> lookup(byte[] tokenizedMessage, String domain) {
    if (tokenizedMessage.length == 0) {
      throw new IllegalArgumentException("The tokenizedMessage byte array cannot be empty");
    }
    int token = tokenizedMessage[0] & 0xFF;
    for (int i = 1; i < tokenizedMessage.length; ++i) {
      token |= (tokenizedMessage[i] & 0xFF) << (i * 8);
    }
    return lookup(token, DEFAULT_DOMAIN);
  }

  /**
   * Detokenizes the binary tokenized message without recursion, using the default domain.
   *
   * @return the detokenized string if there was one successful detokenization, otherwise null
   */
  @Nullable
  public String detokenize(byte[] binaryMessage) {
    return detokenize(binaryMessage, DEFAULT_DOMAIN);
  }

  /**
   * Detokenizes the binary tokenized message without recursion, using the specified domain.
   *
   * @return the detokenized string if there was one successful detokenization, otherwise null
   */
  @Nullable
  public String detokenize(byte[] binaryMessage, String domain) {
    checkIfOpen();
    byte[] bytes = detokenizeNative(handle, binaryMessage, domain);
    return bytes != null ? new String(bytes, UTF_8) : null;
  }

  /**
   * Recursively detokenizes the binary tokenized message.
   *
   * The first message uses the default domain.
   *
   * @return the detokenized string if the main message was successfully detokenized, otherwise null
   */
  @Nullable
  public String recursiveDetokenize(byte[] binaryMessage) {
    return recursiveDetokenize(binaryMessage, DEFAULT_DOMAIN);
  }

  /**
   * Recursively detokenizes the binary tokenized message.
   *
   * The first message uses the specified domain.
   *
   * @return the detokenized string if the main message was successfully detokenized, otherwise null
   */
  @Nullable
  public String recursiveDetokenize(byte[] binaryMessage, String domain) {
    checkIfOpen();
    byte[] bytes = recursiveDetokenizeNative(handle, binaryMessage, domain);
    return bytes != null ? new String(bytes, UTF_8) : null;
  }

  /**
   * Detokenizes and replaces all recognized nested tokenized messages ($) in the provided string.
   * Unrecognized tokenized strings are left unchanged.
   */
  public String detokenizeText(String message) {
    checkIfOpen();
    return new String(detokenizeTextNative(handle, message), UTF_8);
  }

  /**
   * Cleans up the Detokenizer, freeing resources allocated in C++; MUST be called to avoid leaks.
   *
   * Detokenizer is AutoCloseable, so it may be used in a try-with-resources block. Otherwise,
   * close() must be called manually.
   */
  @Override
  public void close() {
    if (handle != 0) {
      deleteNativeDetokenizer(handle);
      handle = 0;
    }
  }

  private void checkIfOpen() {
    if (handle == 0) {
      throw new IllegalStateException("Attemped to use a closed Detokenizer");
    }
  }

  /** Creates a new detokenizer using the provided binary token database. */
  private static native long newNativeDetokenizerBinary(byte[] data);

  /** Creates a new detokenizer using the provided CSV token database. */
  private static native long newNativeDetokenizerCsv(byte[] database);

  // Use non-static functions so this object has a reference held while the function is running,
  // which prevents finalize from running before the native functions finish.

  /** Deletes the detokenizer object with the provided handle, which MUST be valid. */
  private native void deleteNativeDetokenizer(long handle);

  /** Returns an array containing the Strings that map to this token; null if allocation fails. */
  @Nullable private native String[] lookupNative(long handle, int token, String java_domain);

  /** Returns the detokenized version of the provided data, or null if detokenization failed. */
  @Nullable private native byte[] detokenizeNative(long handle, byte[] data, String domain);

  /** Recursive form of detokenizeNative. */
  @Nullable
  private native byte[] recursiveDetokenizeNative(long handle, byte[] data, String domain);

  /** Returns the String with nested tokenized messages decoded within it. */
  private native byte[] detokenizeTextNative(long handle, String data);
}
