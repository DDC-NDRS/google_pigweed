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

#pragma once

#include <algorithm>
#include <cstddef>
#include <limits>

#include "pb.h"
#include "pw_containers/vector.h"
#include "pw_metric/metric.h"
#include "pw_metric/metric_walker.h"
#include "pw_metric_proto/metric_service.pb.h"
#include "pw_span/span.h"
#include "pw_status/status.h"

namespace pw::metric {

// Writes all metrics from a MetricWalker into a nanopb C struct array.
//
// This utility class implements the pw::metric::internal::MetricWriter
// interface to bridge the MetricWalker to any nanopb message that contains a
// `repeated pw_metric_proto_Metric` field (e.g., WalkResponse or a
// custom snapshot proto).
//
// This class handles all sizing logic for fixed-size C struct arrays
// and gracefully stops the walk by returning ResourceExhausted if
// either the array runs out of space or an application-defined
// metric count limit is reached.
class NanopbMetricWriter : public internal::MetricWriter {
 public:
  // Constructs a new nanopb metric writer.
  //
  // @param metrics_array A span covering the nanopb `repeated Metric`
  //    array (e.g., `response.metrics`).
  // @param metrics_count A reference to the parent struct's `metrics_count`
  //    field, which will be incremented by this class.
  // @param metric_limit A reference to an external counter for an
  //    application-level limit. The walk stops when this hits 0.
  //    To specify no limit, pass a size_t initialized to
  //    `std::numeric_limits<size_t>::max()`.
  NanopbMetricWriter(span<pw_metric_proto_Metric> metrics_array,
                     pb_size_t& metrics_count,
                     size_t& metric_limit)
      : metrics_array_(metrics_array),
        metrics_count_(metrics_count),  // Bind the reference
        metric_limit_(metric_limit) {
    // Assigns a value through the reference to zero-out the caller's
    // variable, ensuring the writer appends to an empty array.
    metrics_count_ = 0;
  }

  pw::Status Write(const Metric& metric, const Vector<Token>& path) override {
    // Check application-defined limit first.
    if (metric_limit_ == 0) {
      return pw::Status::ResourceExhausted();
    }

    // Check if the nanopb C struct array is full.
    if (metrics_count_ >= metrics_array_.size()) {
      return pw::Status::ResourceExhausted();
    }

    // Get the next available slot in the array.
    pw_metric_proto_Metric& proto_metric = metrics_array_[metrics_count_];

    // Copy the token path.
    // The MetricWalker's ScopedName checks that the path depth does not
    // exceed its internal capacity, which is sized to match the proto.
    PW_DCHECK_INT_LE(path.size(), std::size(proto_metric.token_path));
    proto_metric.token_path_count = path.size();
    std::copy(path.begin(), path.end(), proto_metric.token_path);

    // Copy the metric value.
    if (metric.is_float()) {
      proto_metric.value.as_float = metric.as_float();
      proto_metric.which_value = pw_metric_proto_Metric_as_float_tag;
    } else {
      proto_metric.value.as_int = metric.as_int();
      proto_metric.which_value = pw_metric_proto_Metric_as_int_tag;
    }

    // Commit the write.
    --metric_limit_;
    ++metrics_count_;

    return OkStatus();
  }

 private:
  span<pw_metric_proto_Metric> metrics_array_;
  pb_size_t& metrics_count_;
  size_t& metric_limit_;
};

}  // namespace pw::metric
