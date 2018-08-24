/*
 * Copyright (C) 2018 The Android Open Source Project
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

#include "src/trace_processor/proto_trace_parser.h"

#include <string>

#include "perfetto/base/string_view.h"
#include "perfetto/base/utils.h"
#include "perfetto/protozero/proto_decoder.h"
#include "perfetto/protozero/proto_utils.h"
#include "src/trace_processor/process_tracker.h"
#include "src/trace_processor/sched_tracker.h"
#include "src/trace_processor/trace_processor_context.h"

#include "perfetto/trace/trace.pb.h"
#include "perfetto/trace/trace_packet.pb.h"

namespace perfetto {
namespace trace_processor {

using protozero::ProtoDecoder;
using protozero::proto_utils::kFieldTypeLengthDelimited;
using protozero::proto_utils::ParseVarInt;
using protozero::proto_utils::MakeTagVarInt;
using protozero::proto_utils::MakeTagLengthDelimited;

namespace {

template <int field_id>
inline bool FindIntField(ProtoDecoder* decoder, uint64_t* field_value) {
  bool res = false;
  for (auto f = decoder->ReadField(); f.id != 0; f = decoder->ReadField()) {
    if (f.id == field_id) {
      *field_value = f.int_value;
      res = true;
      break;
    }
  }
  decoder->Reset();
  return res;
}

}  // namespace

ProtoTraceParser::ProtoTraceParser(TraceProcessorContext* context)
    : context_(context) {}

ProtoTraceParser::~ProtoTraceParser() = default;

bool ProtoTraceParser::Parse(std::unique_ptr<uint8_t[]> data, size_t size) {
  size_t data_off = 0;
  if (!partial_buf_.empty()) {
    // It takes ~5 bytes for a proto preamble + the varint size.
    const size_t kHeaderBytes = 5;
    if (PERFETTO_UNLIKELY(partial_buf_.size() < kHeaderBytes)) {
      data_off = std::min(kHeaderBytes - partial_buf_.size(), size);
      partial_buf_.insert(partial_buf_.end(), &data[0], &data[0] + data_off);
      if (partial_buf_.size() < kHeaderBytes)
        return true;
    }

    // At this point we have enough data in partial_buf_ to read at least the
    // field header and know the size of the next TracePacket.
    constexpr uint8_t kTracePacketTag =
        MakeTagLengthDelimited(protos::Trace::kPacketFieldNumber);
    const uint8_t* pos = &*partial_buf_.begin();
    uint8_t proto_field_tag = *pos;
    uint64_t field_size = 0;
    const uint8_t* next = ParseVarInt(++pos, &*partial_buf_.end(), &field_size);
    bool parse_failed = next == pos;
    pos = next;
    if (proto_field_tag != kTracePacketTag || field_size == 0 || parse_failed) {
      PERFETTO_ELOG("Failed parsing a TracePacket from the partial buffer");
      return false;  // Unrecoverable error, stop parsing.
    }

    // At this point we know how big the TracePacket is.
    size_t hdr_size = static_cast<size_t>(pos - &*partial_buf_.begin());
    size_t size_incl_header = static_cast<size_t>(field_size + hdr_size);
    PERFETTO_DCHECK(size_incl_header > partial_buf_.size());

    // There is a good chance that between the |partial_buf_| and the new |data|
    // of the current call we have enough bytes to parse a TracePacket.
    if (partial_buf_.size() + size - data_off >= size_incl_header) {
      // Create a new buffer for the whole TracePacket and copy into that:
      // 1) The beginning of the TracePacket (including the proto header) from
      //    the partial buffer.
      // 2) The rest of the TracePacket from the current |data| buffer (note
      //    that we might have consumed already a few bytes form |data| earlier
      //    in this function, hence we need to keep |off| into account).
      std::unique_ptr<uint8_t[]> buf(new uint8_t[size_incl_header]);
      memcpy(&buf[0], partial_buf_.data(), partial_buf_.size());
      // |size_missing| is the number of bytes for the rest of the TracePacket
      // in |data|.
      size_t size_missing = size_incl_header - partial_buf_.size();
      memcpy(&buf[partial_buf_.size()], &data[data_off], size_missing);
      data_off += size_missing;
      partial_buf_.clear();
      ParseInternal(std::move(buf), 0, size_incl_header);
    } else {
      partial_buf_.insert(partial_buf_.end(), &data[data_off],
                          &data[size - data_off]);
      return true;
    }
  }
  ParseInternal(std::move(data), data_off, size - data_off);
  return true;
}

void ProtoTraceParser::ParseInternal(std::unique_ptr<uint8_t[]> data,
                                     size_t off,
                                     size_t size) {
  ProtoDecoder decoder(data.get() + off, size);
  for (auto fld = decoder.ReadField(); fld.id != 0; fld = decoder.ReadField()) {
    if (fld.id != protos::Trace::kPacketFieldNumber) {
      PERFETTO_ELOG("Non-trace packet field found in root Trace proto");
      continue;
    }
    ParsePacket(fld.data(), fld.size());
  }

  const size_t leftover = static_cast<size_t>(size - decoder.offset());
  if (leftover > 0) {
    PERFETTO_DCHECK(partial_buf_.empty());
    partial_buf_.clear();
    partial_buf_.insert(partial_buf_.end(), data.get() + off + decoder.offset(),
                        data.get() + off + decoder.offset() + leftover);
  }
}

void ProtoTraceParser::ParsePacket(const uint8_t* data, size_t length) {
  ProtoDecoder decoder(data, length);
  for (auto fld = decoder.ReadField(); fld.id != 0; fld = decoder.ReadField()) {
    switch (fld.id) {
      case protos::TracePacket::kFtraceEventsFieldNumber:
        ParseFtraceEventBundle(fld.data(), fld.size());
        break;
      case protos::TracePacket::kProcessTreeFieldNumber:
        ParseProcessTree(fld.data(), fld.size());
        break;
      default:
        break;
    }
  }
  PERFETTO_DCHECK(decoder.IsEndOfBuffer());
}

void ProtoTraceParser::ParseProcessTree(const uint8_t* data, size_t length) {
  ProtoDecoder decoder(data, length);

  for (auto fld = decoder.ReadField(); fld.id != 0; fld = decoder.ReadField()) {
    switch (fld.id) {
      case protos::ProcessTree::kProcessesFieldNumber:
        ParseProcess(fld.data(), fld.size());
        break;
      case protos::ProcessTree::kThreadsFieldNumber:
        ParseThread(fld.data(), fld.size());
        break;
      default:
        break;
    }
  }
  PERFETTO_DCHECK(decoder.IsEndOfBuffer());
}

void ProtoTraceParser::ParseThread(const uint8_t* data, size_t length) {
  ProtoDecoder decoder(data, length);
  uint32_t tid = 0;
  uint32_t tgid = 0;
  for (auto fld = decoder.ReadField(); fld.id != 0; fld = decoder.ReadField()) {
    switch (fld.id) {
      case protos::ProcessTree::Thread::kTidFieldNumber:
        tid = fld.as_uint32();
        break;
      case protos::ProcessTree::Thread::kTgidFieldNumber:
        tgid = fld.as_uint32();
        break;
      default:
        break;
    }
  }
  context_->process_tracker->UpdateThread(tid, tgid);

  PERFETTO_DCHECK(decoder.IsEndOfBuffer());
}

void ProtoTraceParser::ParseProcess(const uint8_t* data, size_t length) {
  ProtoDecoder decoder(data, length);
  uint32_t pid = 0;
  base::StringView process_name;
  for (auto fld = decoder.ReadField(); fld.id != 0; fld = decoder.ReadField()) {
    switch (fld.id) {
      case protos::ProcessTree::Process::kPidFieldNumber:
        pid = fld.as_uint32();
        break;
      case protos::ProcessTree::Process::kCmdlineFieldNumber:
        if (process_name.empty())
          process_name = fld.as_string();
        break;
      default:
        break;
    }
  }
  context_->process_tracker->UpdateProcess(pid, process_name);

  PERFETTO_DCHECK(decoder.IsEndOfBuffer());
}

void ProtoTraceParser::ParseFtraceEventBundle(const uint8_t* data,
                                              size_t length) {
  ProtoDecoder decoder(data, length);
  uint64_t cpu = 0;
  constexpr auto kCpuFieldNumber = protos::FtraceEventBundle::kCpuFieldNumber;
  constexpr auto kCpuFieldTag = MakeTagVarInt(kCpuFieldNumber);

  // For speed we speculate on the location and size (<128) of the cpu field.
  // In P+ cpu is pushed as the first field.
  // In P cpu is pushed as the 2nd last field.
  if (length > 2 && data[0] == kCpuFieldTag && data[1] < 0x80) {
    cpu = data[1];
  } else if (PERFETTO_LIKELY(length > 4 && data[length - 4] == kCpuFieldTag) &&
             data[length - 3] < 0x80) {
    cpu = data[length - 3];
  } else {
    if (!PERFETTO_LIKELY((FindIntField<kCpuFieldNumber>(&decoder, &cpu)))) {
      PERFETTO_ELOG("CPU field not found in FtraceEventBundle");
      return;
    }
  }

  for (auto fld = decoder.ReadField(); fld.id != 0; fld = decoder.ReadField()) {
    switch (fld.id) {
      case protos::FtraceEventBundle::kEventFieldNumber:
        ParseFtraceEvent(static_cast<uint32_t>(cpu), fld.data(), fld.size());
        break;
      default:
        break;
    }
  }
  PERFETTO_DCHECK(decoder.IsEndOfBuffer());
}

PERFETTO_ALWAYS_INLINE
void ProtoTraceParser::ParseFtraceEvent(uint32_t cpu,
                                        const uint8_t* data,
                                        size_t length) {
  constexpr auto kTimestampFieldNumber =
      protos::FtraceEvent::kTimestampFieldNumber;
  ProtoDecoder decoder(data, length);
  uint64_t timestamp;
  bool timestamp_found = false;

  // Speculate on the fact that the timestamp is often the 1st field of the
  // event.
  constexpr auto timestampFieldTag = MakeTagVarInt(kTimestampFieldNumber);
  if (PERFETTO_LIKELY(length > 10 && data[0] == timestampFieldTag)) {
    // Fastpath.
    const uint8_t* next = ParseVarInt(data + 1, data + 10, &timestamp);
    timestamp_found = next != data + 1;
    decoder.Reset(next);
  } else {
    // Slowpath.
    timestamp_found = FindIntField<kTimestampFieldNumber>(&decoder, &timestamp);
  }

  if (PERFETTO_UNLIKELY(!timestamp_found)) {
    PERFETTO_ELOG("Timestamp field not found in FtraceEvent");
    return;
  }

  for (auto fld = decoder.ReadField(); fld.id != 0; fld = decoder.ReadField()) {
    switch (fld.id) {
      case protos::FtraceEvent::kSchedSwitchFieldNumber:
        PERFETTO_DCHECK(timestamp > 0);
        ParseSchedSwitch(cpu, timestamp, fld.data(), fld.size());
        break;
      default:
        break;
    }
  }
  PERFETTO_DCHECK(decoder.IsEndOfBuffer());
}

PERFETTO_ALWAYS_INLINE
void ProtoTraceParser::ParseSchedSwitch(uint32_t cpu,
                                        uint64_t timestamp,
                                        const uint8_t* data,
                                        size_t length) {
  ProtoDecoder decoder(data, length);
  uint32_t prev_pid = 0;
  uint32_t prev_state = 0;
  base::StringView prev_comm;
  uint32_t next_pid = 0;
  for (auto fld = decoder.ReadField(); fld.id != 0; fld = decoder.ReadField()) {
    switch (fld.id) {
      case protos::SchedSwitchFtraceEvent::kPrevPidFieldNumber:
        prev_pid = fld.as_uint32();
        break;
      case protos::SchedSwitchFtraceEvent::kPrevStateFieldNumber:
        prev_state = fld.as_uint32();
        break;
      case protos::SchedSwitchFtraceEvent::kPrevCommFieldNumber:
        prev_comm = fld.as_string();
        break;
      case protos::SchedSwitchFtraceEvent::kNextPidFieldNumber:
        next_pid = fld.as_uint32();
        break;
      default:
        break;
    }
  }
  context_->sched_tracker->PushSchedSwitch(cpu, timestamp, prev_pid, prev_state,
                                           prev_comm, next_pid);

  PERFETTO_DCHECK(decoder.IsEndOfBuffer());
}

}  // namespace trace_processor
}  // namespace perfetto
