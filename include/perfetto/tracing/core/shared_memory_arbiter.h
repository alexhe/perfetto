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

#ifndef INCLUDE_PERFETTO_TRACING_CORE_SHARED_MEMORY_ARBITER_H_
#define INCLUDE_PERFETTO_TRACING_CORE_SHARED_MEMORY_ARBITER_H_

#include <stddef.h>

#include <functional>
#include <memory>
#include <vector>

#include "perfetto/base/export.h"
#include "perfetto/tracing/core/basic_types.h"
#include "perfetto/tracing/core/tracing_service.h"

namespace perfetto {

namespace base {
class TaskRunner;
}

class CommitDataRequest;
class StartupTraceWriter;
class StartupTraceWriterRegistry;
class SharedMemory;
class TraceWriter;

// Used by the Producer-side of the transport layer to vend TraceWriters
// from the SharedMemory it receives from the Service-side.
class PERFETTO_EXPORT SharedMemoryArbiter {
 public:
  virtual ~SharedMemoryArbiter();

  // Creates a new TraceWriter and assigns it a new WriterID. The WriterID is
  // written in each chunk header owned by a given TraceWriter and is used by
  // the Service to reconstruct TracePackets written by the same TraceWriter.
  // Returns null impl of TraceWriter if all WriterID slots are exhausted.
  virtual std::unique_ptr<TraceWriter> CreateTraceWriter(
      BufferID target_buffer) = 0;

  // Binds the provided unbound StartupTraceWriterRegistry to the arbiter's SMB.
  // Normally this happens when the perfetto service has been initialized and we
  // want to rebind all the writers created in the early startup phase.
  //
  // All StartupTraceWriters created by the registry are bound to the arbiter
  // and the given target buffer. The writers may not be bound immediately if
  // they are concurrently being written to. The registry will retry on the
  // arbiter's TaskRunner until all writers were bound successfully.
  //
  // Should only be called on the passed TaskRunner's sequence. By calling this
  // method, the registry's ownership is transferred to the arbiter. The arbiter
  // will delete the registry once all writers were bound.
  //
  // TODO(eseckler): Make target buffer assignment more flexible (i.e. per
  // writer). For now, embedders can use multiple registries instead.
  virtual void BindStartupTraceWriterRegistry(
      std::unique_ptr<StartupTraceWriterRegistry>,
      BufferID target_buffer) = 0;

  // Notifies the service that all data for the given FlushRequestID has been
  // committed in the shared memory buffer.
  virtual void NotifyFlushComplete(FlushRequestID) = 0;

  // Implemented in src/core/shared_memory_arbiter_impl.cc .
  static std::unique_ptr<SharedMemoryArbiter> CreateInstance(
      SharedMemory*,
      size_t page_size,
      TracingService::ProducerEndpoint*,
      base::TaskRunner*);
};

}  // namespace perfetto

#endif  // INCLUDE_PERFETTO_TRACING_CORE_SHARED_MEMORY_ARBITER_H_
