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
#pragma once

#include "pw_rpc/benchmark.raw_rpc.pb.h"

namespace pw::rpc {

// RPC service with low-level RPCs for transmitting data. Used for benchmarking
// and testing.
//
// NOTE: the implementation of `BidirectionalEcho` is *not* thread-safe.
class BenchmarkService
    : public pw_rpc::raw::Benchmark::Service<BenchmarkService> {
 public:
  static void UnaryEcho(ConstByteSpan request, RawUnaryResponder& responder);

  void BidirectionalEcho(RawServerReaderWriter& reader_writer);

 private:
  using ReaderWriterId = uint64_t;
  ReaderWriterId AllocateReaderWriterId();

  ReaderWriterId next_reader_writer_id_ = 0;
  std::unordered_map<ReaderWriterId, RawServerReaderWriter> reader_writers_;
};

}  // namespace pw::rpc
