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
 * perfetto/common/commit_data_request.proto
 * by
 * ../../tools/proto_to_cpp/proto_to_cpp.cc.
 * If you need to make changes here, change the .proto file and then run
 * ./tools/gen_tracing_cpp_headers_from_protos
 */

#ifndef INCLUDE_PERFETTO_TRACING_CORE_COMMIT_DATA_REQUEST_H_
#define INCLUDE_PERFETTO_TRACING_CORE_COMMIT_DATA_REQUEST_H_

#include <stdint.h>
#include <string>
#include <type_traits>
#include <vector>

#include "perfetto/base/export.h"

// Forward declarations for protobuf types.
namespace perfetto {
namespace protos {
class CommitDataRequest;
class CommitDataRequest_ChunksToMove;
class CommitDataRequest_ChunkToPatch;
class CommitDataRequest_ChunkToPatch_Patch;
}  // namespace protos
}  // namespace perfetto

namespace perfetto {

class PERFETTO_EXPORT CommitDataRequest {
 public:
  class PERFETTO_EXPORT ChunksToMove {
   public:
    ChunksToMove();
    ~ChunksToMove();
    ChunksToMove(ChunksToMove&&) noexcept;
    ChunksToMove& operator=(ChunksToMove&&);
    ChunksToMove(const ChunksToMove&);
    ChunksToMove& operator=(const ChunksToMove&);
    bool operator==(const ChunksToMove&) const;

    // Conversion methods from/to the corresponding protobuf types.
    void FromProto(const perfetto::protos::CommitDataRequest_ChunksToMove&);
    void ToProto(perfetto::protos::CommitDataRequest_ChunksToMove*) const;

    uint32_t page() const { return page_; }
    void set_page(uint32_t value) { page_ = value; }

    uint32_t chunk() const { return chunk_; }
    void set_chunk(uint32_t value) { chunk_ = value; }

    uint32_t target_buffer() const { return target_buffer_; }
    void set_target_buffer(uint32_t value) { target_buffer_ = value; }

   private:
    uint32_t page_ = {};
    uint32_t chunk_ = {};
    uint32_t target_buffer_ = {};

    // Allows to preserve unknown protobuf fields for compatibility
    // with future versions of .proto files.
    std::string unknown_fields_;
  };

  class PERFETTO_EXPORT ChunkToPatch {
   public:
    class PERFETTO_EXPORT Patch {
     public:
      Patch();
      ~Patch();
      Patch(Patch&&) noexcept;
      Patch& operator=(Patch&&);
      Patch(const Patch&);
      Patch& operator=(const Patch&);
      bool operator==(const Patch&) const;

      // Conversion methods from/to the corresponding protobuf types.
      void FromProto(
          const perfetto::protos::CommitDataRequest_ChunkToPatch_Patch&);
      void ToProto(
          perfetto::protos::CommitDataRequest_ChunkToPatch_Patch*) const;

      uint32_t offset() const { return offset_; }
      void set_offset(uint32_t value) { offset_ = value; }

      const std::string& data() const { return data_; }
      void set_data(const std::string& value) { data_ = value; }
      void set_data(const void* p, size_t s) {
        data_.assign(reinterpret_cast<const char*>(p), s);
      }

     private:
      uint32_t offset_ = {};
      std::string data_ = {};

      // Allows to preserve unknown protobuf fields for compatibility
      // with future versions of .proto files.
      std::string unknown_fields_;
    };

    ChunkToPatch();
    ~ChunkToPatch();
    ChunkToPatch(ChunkToPatch&&) noexcept;
    ChunkToPatch& operator=(ChunkToPatch&&);
    ChunkToPatch(const ChunkToPatch&);
    ChunkToPatch& operator=(const ChunkToPatch&);
    bool operator==(const ChunkToPatch&) const;

    // Conversion methods from/to the corresponding protobuf types.
    void FromProto(const perfetto::protos::CommitDataRequest_ChunkToPatch&);
    void ToProto(perfetto::protos::CommitDataRequest_ChunkToPatch*) const;

    uint32_t target_buffer() const { return target_buffer_; }
    void set_target_buffer(uint32_t value) { target_buffer_ = value; }

    uint32_t writer_id() const { return writer_id_; }
    void set_writer_id(uint32_t value) { writer_id_ = value; }

    uint32_t chunk_id() const { return chunk_id_; }
    void set_chunk_id(uint32_t value) { chunk_id_ = value; }

    int patches_size() const { return static_cast<int>(patches_.size()); }
    const std::vector<Patch>& patches() const { return patches_; }
    void clear_patches() { patches_.clear(); }
    Patch* add_patches() {
      patches_.emplace_back();
      return &patches_.back();
    }

    bool has_more_patches() const { return has_more_patches_; }
    void set_has_more_patches(bool value) { has_more_patches_ = value; }

   private:
    uint32_t target_buffer_ = {};
    uint32_t writer_id_ = {};
    uint32_t chunk_id_ = {};
    std::vector<Patch> patches_;
    bool has_more_patches_ = {};

    // Allows to preserve unknown protobuf fields for compatibility
    // with future versions of .proto files.
    std::string unknown_fields_;
  };

  CommitDataRequest();
  ~CommitDataRequest();
  CommitDataRequest(CommitDataRequest&&) noexcept;
  CommitDataRequest& operator=(CommitDataRequest&&);
  CommitDataRequest(const CommitDataRequest&);
  CommitDataRequest& operator=(const CommitDataRequest&);
  bool operator==(const CommitDataRequest&) const;

  // Conversion methods from/to the corresponding protobuf types.
  void FromProto(const perfetto::protos::CommitDataRequest&);
  void ToProto(perfetto::protos::CommitDataRequest*) const;

  int chunks_to_move_size() const {
    return static_cast<int>(chunks_to_move_.size());
  }
  const std::vector<ChunksToMove>& chunks_to_move() const {
    return chunks_to_move_;
  }
  void clear_chunks_to_move() { chunks_to_move_.clear(); }
  ChunksToMove* add_chunks_to_move() {
    chunks_to_move_.emplace_back();
    return &chunks_to_move_.back();
  }

  int chunks_to_patch_size() const {
    return static_cast<int>(chunks_to_patch_.size());
  }
  const std::vector<ChunkToPatch>& chunks_to_patch() const {
    return chunks_to_patch_;
  }
  void clear_chunks_to_patch() { chunks_to_patch_.clear(); }
  ChunkToPatch* add_chunks_to_patch() {
    chunks_to_patch_.emplace_back();
    return &chunks_to_patch_.back();
  }

  uint64_t flush_request_id() const { return flush_request_id_; }
  void set_flush_request_id(uint64_t value) { flush_request_id_ = value; }

 private:
  std::vector<ChunksToMove> chunks_to_move_;
  std::vector<ChunkToPatch> chunks_to_patch_;
  uint64_t flush_request_id_ = {};

  // Allows to preserve unknown protobuf fields for compatibility
  // with future versions of .proto files.
  std::string unknown_fields_;
};

}  // namespace perfetto

#endif  // INCLUDE_PERFETTO_TRACING_CORE_COMMIT_DATA_REQUEST_H_
