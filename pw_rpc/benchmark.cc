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

#include "pw_rpc/benchmark.h"

#include <algorithm>

#include "pw_rpc/internal/config.h"

namespace pw::rpc {

void BenchmarkService::UnaryEcho(ConstByteSpan request,
                                 RawUnaryResponder& responder) {
  responder.Finish(span(request).first(request.size()), PW_STATUS_OK)
      .IgnoreError();
}

BenchmarkService::ReaderWriterId BenchmarkService::AllocateReaderWriterId() {
  return next_reader_writer_id_++;
}

void BenchmarkService::BidirectionalEcho(
    RawServerReaderWriter& new_reader_writer) {
  auto id = AllocateReaderWriterId();

  struct Captures {
    BenchmarkService* self;
    ReaderWriterId id;
  };

  auto captures = std::make_unique<Captures>(Captures{.self = this, .id = id});
  new_reader_writer.set_on_next(
      [context = std::move(captures)](ConstByteSpan request) {
        auto& reader_writers = context->self->reader_writers_;
        auto rw_id = context->id;
        auto reader_writer = reader_writers.find(rw_id);
        if (reader_writer == reader_writers.end()) {
          return;
        }
        Status status = reader_writer->second.Write(request);
        if (!status.ok()) {
          reader_writer->second.Finish(status)
              .IgnoreError();  // TODO: b/242598609 - Handle Status properly
          reader_writers.erase(rw_id);
        }
      });
  reader_writers_.insert({id, std::move(new_reader_writer)});
}

}  // namespace pw::rpc
