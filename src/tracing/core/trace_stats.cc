/*
 * Copyright (C) 2017 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*******************************************************************************
 * AUTOGENERATED - DO NOT EDIT
 *******************************************************************************
 * This file has been generated from the protobuf message
 * perfetto/common/trace_stats.proto
 * by
 * ../../tools/proto_to_cpp/proto_to_cpp.cc.
 * If you need to make changes here, change the .proto file and then run
 * ./tools/gen_tracing_cpp_headers_from_protos
 */

#include "perfetto/tracing/core/trace_stats.h"

#include "perfetto/common/trace_stats.pb.h"

namespace perfetto {

TraceStats::TraceStats() = default;
TraceStats::~TraceStats() = default;
TraceStats::TraceStats(const TraceStats&) = default;
TraceStats& TraceStats::operator=(const TraceStats&) = default;
TraceStats::TraceStats(TraceStats&&) noexcept = default;
TraceStats& TraceStats::operator=(TraceStats&&) = default;

void TraceStats::FromProto(const perfetto::protos::TraceStats& proto) {
  buffer_stats_.clear();
  for (const auto& field : proto.buffer_stats()) {
    buffer_stats_.emplace_back();
    buffer_stats_.back().FromProto(field);
  }

  static_assert(
      sizeof(producers_connected_) == sizeof(proto.producers_connected()),
      "size mismatch");
  producers_connected_ =
      static_cast<decltype(producers_connected_)>(proto.producers_connected());

  static_assert(sizeof(producers_seen_) == sizeof(proto.producers_seen()),
                "size mismatch");
  producers_seen_ =
      static_cast<decltype(producers_seen_)>(proto.producers_seen());

  static_assert(sizeof(data_sources_registered_) ==
                    sizeof(proto.data_sources_registered()),
                "size mismatch");
  data_sources_registered_ = static_cast<decltype(data_sources_registered_)>(
      proto.data_sources_registered());

  static_assert(sizeof(data_sources_seen_) == sizeof(proto.data_sources_seen()),
                "size mismatch");
  data_sources_seen_ =
      static_cast<decltype(data_sources_seen_)>(proto.data_sources_seen());

  static_assert(sizeof(tracing_sessions_) == sizeof(proto.tracing_sessions()),
                "size mismatch");
  tracing_sessions_ =
      static_cast<decltype(tracing_sessions_)>(proto.tracing_sessions());

  static_assert(sizeof(total_buffers_) == sizeof(proto.total_buffers()),
                "size mismatch");
  total_buffers_ = static_cast<decltype(total_buffers_)>(proto.total_buffers());
  unknown_fields_ = proto.unknown_fields();
}

void TraceStats::ToProto(perfetto::protos::TraceStats* proto) const {
  proto->Clear();

  for (const auto& it : buffer_stats_) {
    auto* entry = proto->add_buffer_stats();
    it.ToProto(entry);
  }

  static_assert(
      sizeof(producers_connected_) == sizeof(proto->producers_connected()),
      "size mismatch");
  proto->set_producers_connected(
      static_cast<decltype(proto->producers_connected())>(
          producers_connected_));

  static_assert(sizeof(producers_seen_) == sizeof(proto->producers_seen()),
                "size mismatch");
  proto->set_producers_seen(
      static_cast<decltype(proto->producers_seen())>(producers_seen_));

  static_assert(sizeof(data_sources_registered_) ==
                    sizeof(proto->data_sources_registered()),
                "size mismatch");
  proto->set_data_sources_registered(
      static_cast<decltype(proto->data_sources_registered())>(
          data_sources_registered_));

  static_assert(
      sizeof(data_sources_seen_) == sizeof(proto->data_sources_seen()),
      "size mismatch");
  proto->set_data_sources_seen(
      static_cast<decltype(proto->data_sources_seen())>(data_sources_seen_));

  static_assert(sizeof(tracing_sessions_) == sizeof(proto->tracing_sessions()),
                "size mismatch");
  proto->set_tracing_sessions(
      static_cast<decltype(proto->tracing_sessions())>(tracing_sessions_));

  static_assert(sizeof(total_buffers_) == sizeof(proto->total_buffers()),
                "size mismatch");
  proto->set_total_buffers(
      static_cast<decltype(proto->total_buffers())>(total_buffers_));
  *(proto->mutable_unknown_fields()) = unknown_fields_;
}

TraceStats::BufferStats::BufferStats() = default;
TraceStats::BufferStats::~BufferStats() = default;
TraceStats::BufferStats::BufferStats(const TraceStats::BufferStats&) = default;
TraceStats::BufferStats& TraceStats::BufferStats::operator=(
    const TraceStats::BufferStats&) = default;
TraceStats::BufferStats::BufferStats(TraceStats::BufferStats&&) noexcept =
    default;
TraceStats::BufferStats& TraceStats::BufferStats::operator=(
    TraceStats::BufferStats&&) = default;

void TraceStats::BufferStats::FromProto(
    const perfetto::protos::TraceStats_BufferStats& proto) {
  static_assert(sizeof(buffer_size_) == sizeof(proto.buffer_size()),
                "size mismatch");
  buffer_size_ = static_cast<decltype(buffer_size_)>(proto.buffer_size());

  static_assert(sizeof(bytes_written_) == sizeof(proto.bytes_written()),
                "size mismatch");
  bytes_written_ = static_cast<decltype(bytes_written_)>(proto.bytes_written());

  static_assert(sizeof(bytes_overwritten_) == sizeof(proto.bytes_overwritten()),
                "size mismatch");
  bytes_overwritten_ =
      static_cast<decltype(bytes_overwritten_)>(proto.bytes_overwritten());

  static_assert(sizeof(bytes_read_) == sizeof(proto.bytes_read()),
                "size mismatch");
  bytes_read_ = static_cast<decltype(bytes_read_)>(proto.bytes_read());

  static_assert(
      sizeof(padding_bytes_written_) == sizeof(proto.padding_bytes_written()),
      "size mismatch");
  padding_bytes_written_ = static_cast<decltype(padding_bytes_written_)>(
      proto.padding_bytes_written());

  static_assert(
      sizeof(padding_bytes_cleared_) == sizeof(proto.padding_bytes_cleared()),
      "size mismatch");
  padding_bytes_cleared_ = static_cast<decltype(padding_bytes_cleared_)>(
      proto.padding_bytes_cleared());

  static_assert(sizeof(chunks_written_) == sizeof(proto.chunks_written()),
                "size mismatch");
  chunks_written_ =
      static_cast<decltype(chunks_written_)>(proto.chunks_written());

  static_assert(sizeof(chunks_rewritten_) == sizeof(proto.chunks_rewritten()),
                "size mismatch");
  chunks_rewritten_ =
      static_cast<decltype(chunks_rewritten_)>(proto.chunks_rewritten());

  static_assert(
      sizeof(chunks_overwritten_) == sizeof(proto.chunks_overwritten()),
      "size mismatch");
  chunks_overwritten_ =
      static_cast<decltype(chunks_overwritten_)>(proto.chunks_overwritten());

  static_assert(sizeof(chunks_discarded_) == sizeof(proto.chunks_discarded()),
                "size mismatch");
  chunks_discarded_ =
      static_cast<decltype(chunks_discarded_)>(proto.chunks_discarded());

  static_assert(sizeof(chunks_read_) == sizeof(proto.chunks_read()),
                "size mismatch");
  chunks_read_ = static_cast<decltype(chunks_read_)>(proto.chunks_read());

  static_assert(sizeof(chunks_committed_out_of_order_) ==
                    sizeof(proto.chunks_committed_out_of_order()),
                "size mismatch");
  chunks_committed_out_of_order_ =
      static_cast<decltype(chunks_committed_out_of_order_)>(
          proto.chunks_committed_out_of_order());

  static_assert(sizeof(write_wrap_count_) == sizeof(proto.write_wrap_count()),
                "size mismatch");
  write_wrap_count_ =
      static_cast<decltype(write_wrap_count_)>(proto.write_wrap_count());

  static_assert(sizeof(patches_succeeded_) == sizeof(proto.patches_succeeded()),
                "size mismatch");
  patches_succeeded_ =
      static_cast<decltype(patches_succeeded_)>(proto.patches_succeeded());

  static_assert(sizeof(patches_failed_) == sizeof(proto.patches_failed()),
                "size mismatch");
  patches_failed_ =
      static_cast<decltype(patches_failed_)>(proto.patches_failed());

  static_assert(
      sizeof(readaheads_succeeded_) == sizeof(proto.readaheads_succeeded()),
      "size mismatch");
  readaheads_succeeded_ = static_cast<decltype(readaheads_succeeded_)>(
      proto.readaheads_succeeded());

  static_assert(sizeof(readaheads_failed_) == sizeof(proto.readaheads_failed()),
                "size mismatch");
  readaheads_failed_ =
      static_cast<decltype(readaheads_failed_)>(proto.readaheads_failed());

  static_assert(sizeof(abi_violations_) == sizeof(proto.abi_violations()),
                "size mismatch");
  abi_violations_ =
      static_cast<decltype(abi_violations_)>(proto.abi_violations());
  unknown_fields_ = proto.unknown_fields();
}

void TraceStats::BufferStats::ToProto(
    perfetto::protos::TraceStats_BufferStats* proto) const {
  proto->Clear();

  static_assert(sizeof(buffer_size_) == sizeof(proto->buffer_size()),
                "size mismatch");
  proto->set_buffer_size(
      static_cast<decltype(proto->buffer_size())>(buffer_size_));

  static_assert(sizeof(bytes_written_) == sizeof(proto->bytes_written()),
                "size mismatch");
  proto->set_bytes_written(
      static_cast<decltype(proto->bytes_written())>(bytes_written_));

  static_assert(
      sizeof(bytes_overwritten_) == sizeof(proto->bytes_overwritten()),
      "size mismatch");
  proto->set_bytes_overwritten(
      static_cast<decltype(proto->bytes_overwritten())>(bytes_overwritten_));

  static_assert(sizeof(bytes_read_) == sizeof(proto->bytes_read()),
                "size mismatch");
  proto->set_bytes_read(
      static_cast<decltype(proto->bytes_read())>(bytes_read_));

  static_assert(
      sizeof(padding_bytes_written_) == sizeof(proto->padding_bytes_written()),
      "size mismatch");
  proto->set_padding_bytes_written(
      static_cast<decltype(proto->padding_bytes_written())>(
          padding_bytes_written_));

  static_assert(
      sizeof(padding_bytes_cleared_) == sizeof(proto->padding_bytes_cleared()),
      "size mismatch");
  proto->set_padding_bytes_cleared(
      static_cast<decltype(proto->padding_bytes_cleared())>(
          padding_bytes_cleared_));

  static_assert(sizeof(chunks_written_) == sizeof(proto->chunks_written()),
                "size mismatch");
  proto->set_chunks_written(
      static_cast<decltype(proto->chunks_written())>(chunks_written_));

  static_assert(sizeof(chunks_rewritten_) == sizeof(proto->chunks_rewritten()),
                "size mismatch");
  proto->set_chunks_rewritten(
      static_cast<decltype(proto->chunks_rewritten())>(chunks_rewritten_));

  static_assert(
      sizeof(chunks_overwritten_) == sizeof(proto->chunks_overwritten()),
      "size mismatch");
  proto->set_chunks_overwritten(
      static_cast<decltype(proto->chunks_overwritten())>(chunks_overwritten_));

  static_assert(sizeof(chunks_discarded_) == sizeof(proto->chunks_discarded()),
                "size mismatch");
  proto->set_chunks_discarded(
      static_cast<decltype(proto->chunks_discarded())>(chunks_discarded_));

  static_assert(sizeof(chunks_read_) == sizeof(proto->chunks_read()),
                "size mismatch");
  proto->set_chunks_read(
      static_cast<decltype(proto->chunks_read())>(chunks_read_));

  static_assert(sizeof(chunks_committed_out_of_order_) ==
                    sizeof(proto->chunks_committed_out_of_order()),
                "size mismatch");
  proto->set_chunks_committed_out_of_order(
      static_cast<decltype(proto->chunks_committed_out_of_order())>(
          chunks_committed_out_of_order_));

  static_assert(sizeof(write_wrap_count_) == sizeof(proto->write_wrap_count()),
                "size mismatch");
  proto->set_write_wrap_count(
      static_cast<decltype(proto->write_wrap_count())>(write_wrap_count_));

  static_assert(
      sizeof(patches_succeeded_) == sizeof(proto->patches_succeeded()),
      "size mismatch");
  proto->set_patches_succeeded(
      static_cast<decltype(proto->patches_succeeded())>(patches_succeeded_));

  static_assert(sizeof(patches_failed_) == sizeof(proto->patches_failed()),
                "size mismatch");
  proto->set_patches_failed(
      static_cast<decltype(proto->patches_failed())>(patches_failed_));

  static_assert(
      sizeof(readaheads_succeeded_) == sizeof(proto->readaheads_succeeded()),
      "size mismatch");
  proto->set_readaheads_succeeded(
      static_cast<decltype(proto->readaheads_succeeded())>(
          readaheads_succeeded_));

  static_assert(
      sizeof(readaheads_failed_) == sizeof(proto->readaheads_failed()),
      "size mismatch");
  proto->set_readaheads_failed(
      static_cast<decltype(proto->readaheads_failed())>(readaheads_failed_));

  static_assert(sizeof(abi_violations_) == sizeof(proto->abi_violations()),
                "size mismatch");
  proto->set_abi_violations(
      static_cast<decltype(proto->abi_violations())>(abi_violations_));
  *(proto->mutable_unknown_fields()) = unknown_fields_;
}

}  // namespace perfetto
