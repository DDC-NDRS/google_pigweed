// Copyright 2023 The Pigweed Authors
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
#include <lib/fit/function.h>

#include <cstddef>
#include <cstdint>

#include "pw_bluetooth_sapphire/internal/host/common/byte_buffer.h"
#include "pw_bluetooth_sapphire/internal/host/common/uuid.h"

namespace bt {

// EIR Data Type, Advertising Data Type (AD Type), OOB Data Type definitions.
// TODO(fxbug.dev/337934217): Switch to using the Emboss defined versions
// of this enum class
// clang-format off
enum class DataType : uint8_t {
  kFlags                        = 0x01,
  kIncomplete16BitServiceUuids  = 0x02,
  kComplete16BitServiceUuids    = 0x03,
  kIncomplete32BitServiceUuids  = 0x04,
  kComplete32BitServiceUuids    = 0x05,
  kIncomplete128BitServiceUuids = 0x06,
  kComplete128BitServiceUuids   = 0x07,
  kShortenedLocalName           = 0x08,
  kCompleteLocalName            = 0x09,
  kTxPowerLevel                 = 0x0A,
  kClassOfDevice                = 0x0D,
  kSSPOOBHash                   = 0x0E,
  kSSPOOBRandomizer             = 0x0F,
  kSolicitationUuid16Bit        = 0x14,
  kSolicitationUuid128Bit       = 0x15,
  kServiceData16Bit             = 0x16,
  kAppearance                   = 0x19,
  kSolicitationUuid32Bit        = 0x1F,
  kServiceData32Bit             = 0x20,
  kServiceData128Bit            = 0x21,
  kURI                          = 0x24,
  kResolvableSetIdentifier      = 0x2E,
  kBroadcastName                = 0x30,
  kManufacturerSpecificData     = 0xFF,
  // TODO(armansito): Complete this list.
};
// clang-format on

UUIDElemSize SizeForType(DataType type);

using UuidFunction = fit::function<bool(const UUID&)>;

// Parses `data` into `data.size()` / `uuid_size` UUIDs, calling `func` with
// each parsed UUID. Returns false without further parsing if `uuid_size` does
// not evenly divide `data.size()` or `func` returns false for any UUID,
// otherwise returns true.
bool ParseUuids(const BufferView& data,
                UUIDElemSize uuid_size,
                UuidFunction func);

// Convenience classes for reading and writing the contents
// of Advertising Data, Scan Response Data, or Extended Inquiry Response Data
// payloads. The format in which data is stored looks like the following:
//
//    [1-octet LENGTH][1-octet TYPE][LENGTH-1 octets DATA]
//
// Used for parsing data in TLV-format as described at the beginning of the file
// above.
class SupplementDataReader {
 public:
  // |data| must point to a valid piece of memory for the duration in which this
  // object is to remain alive.
  explicit SupplementDataReader(const ByteBuffer& data);

  // Returns false if the fields of |data| have been formatted incorrectly. For
  // example, this could happen if the length of an advertising data structure
  // would exceed the bounds of the buffer.
  inline bool is_valid() const { return is_valid_; }

  // Returns the data and type fields of the next advertising data structure in
  // |out_data| and |out_type|. Returns false if there is no more data to read
  // or if the data is formatted incorrectly.
  bool GetNextField(DataType* out_type, BufferView* out_data);

  // Returns true if there is more data to read. Returns false if the end of
  // data has been reached or if the current segment is malformed in a way that
  // would exceed the bounds of the data this reader was initialized with.
  bool HasMoreData() const;

 private:
  bool is_valid_;
  BufferView remaining_;
};

// Used for writing data in TLV-format as described at the beginning of the file
// above.
class SupplementDataWriter {
 public:
  // |buffer| is the piece of memory on which this SupplementDataWriter should
  // operate. The buffer must out-live this instance and must point to a valid
  // piece of memory.
  explicit SupplementDataWriter(MutableByteBuffer* buffer);

  // Writes the given piece of type/tag and data into the next available segment
  // in the underlying buffer. Returns false if there isn't enough space left in
  // the buffer for writing. Returns true on success.
  bool WriteField(DataType type, const ByteBuffer& data);

  // The total number of bytes that have been written into the buffer.
  size_t bytes_written() const { return bytes_written_; }

 private:
  MutableByteBuffer* buffer_;
  size_t bytes_written_;
};

}  // namespace bt
