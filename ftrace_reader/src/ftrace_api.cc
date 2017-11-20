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

#include "ftrace_api.h"

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "base/logging.h"
#include "base/utils.h"

namespace perfetto {
namespace {

// Reading this file produces human readable trace output.
// Writing to this file clears all trace buffers for all CPUS.
// const char kTracePath[] = "/sys/kernel/debug/tracing/trace";

// Writing to this file injects an event into the trace buffer.
// const char kTraceMarkerPath[] = "/sys/kernel/debug/tracing/trace_marker";

// Reading this file returns 1/0 if tracing is enabled/disabled.
// Writing 1/0 to this file enables/disables tracing.
// Disabling tracing with this file prevents further writes but
// does not clear the buffer.
// const char kTracingOnPath[] = "/sys/kernel/debug/tracing/tracing_on";

char ReadOneCharFromFile(const std::string& path) {
  base::ScopedFile fd(open(path.c_str(), O_RDONLY));
  if (!fd)
    return '\0';
  char result = '\0';
  ssize_t bytes = PERFETTO_EINTR(read(fd.get(), &result, 1));
  PERFETTO_DCHECK(bytes == 1 || bytes == -1);
  return result;
}

}  // namespace

FtraceApi::FtraceApi(const std::string& root) : root_(root) {}
FtraceApi::~FtraceApi() = default;

bool FtraceApi::EnableEvent(const std::string& group, const std::string& name) {
  std::string path = root_ + "events/" + group + "/" + name + "/enable";
  return WriteToFile(path, "1");
}

bool FtraceApi::DisableEvent(const std::string& group,
                             const std::string& name) {
  std::string path = root_ + "events/" + group + "/" + name + "/enable";
  return WriteToFile(path, "0");
}

bool FtraceApi::WriteToFile(const std::string& path, const std::string& str) {
  base::ScopedFile fd(open(path.c_str(), O_WRONLY));
  if (!fd)
    return false;
  ssize_t written = PERFETTO_EINTR(write(fd.get(), str.c_str(), str.length()));
  ssize_t length = static_cast<ssize_t>(str.length());
  // This should either fail or write fully.
  PERFETTO_DCHECK(written == length || written == -1);
  return written == length;
}

base::ScopedFile FtraceApi::OpenFile(const std::string& path) {
  return base::ScopedFile(open(path.c_str(), O_RDONLY));
}

size_t FtraceApi::NumberOfCpus() const {
  static size_t num_cpus = sysconf(_SC_NPROCESSORS_CONF);
  return num_cpus;
}

std::string FtraceApi::GetTracePipeRawPath(size_t cpu) {
  return root_ + "per_cpu/" + std::to_string(cpu) + "/trace_pipe_raw";
}

void FtraceApi::ClearTrace() {
  std::string path = root_ + "trace";
  base::ScopedFile fd(open(path.c_str(), O_WRONLY | O_TRUNC));
  PERFETTO_CHECK(fd);  // Could not clear.
}

bool FtraceApi::WriteTraceMarker(const std::string& str) {
  std::string path = root_ + "trace_marker";
  return WriteToFile(path, str);
}

bool FtraceApi::EnableTracing() {
  std::string path = root_ + "tracing_on";
  return WriteToFile(path, "1");
}

bool FtraceApi::DisableTracing() {
  std::string path = root_ + "tracing_on";
  return WriteToFile(path, "0");
}

bool FtraceApi::IsTracingEnabled() {
  std::string path = root_ + "tracing_on";
  return ReadOneCharFromFile(path) == '1';
}

}  // namespace perfetto
