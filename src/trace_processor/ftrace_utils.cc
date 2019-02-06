/*
 * Copyright (C) 2019 The Android Open Source Project
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

#include "src/trace_processor/ftrace_utils.h"

#include <inttypes.h>
#include <algorithm>

#include "perfetto/base/logging.h"

namespace perfetto {
namespace trace_processor {
namespace ftrace_utils {

namespace {
int64_t TimestampToSeconds(int64_t timestamp) {
  return timestamp / 1000000000l;
}

int64_t TimestampToMicroseconds(int64_t timestamp) {
  return (timestamp / 1000) % 1000000l;
}
}  // namespace

TaskState::TaskState(const char* state_str) {
  bool invalid_char = false;
  bool is_runnable = false;
  for (size_t i = 0; state_str[i] != '\0'; i++) {
    char c = state_str[i];
    if (is_kernel_preempt()) {
      // No other character should be encountered after '+'.
      invalid_char = true;
      break;
    } else if (c == '+') {
      state_ |= kMaxState;
      continue;
    }

    if (is_runnable) {
      // We should not encounter any character apart from '+' if runnable.
      invalid_char = true;
      break;
    }

    if (c == 'R') {
      if (state_ != 0) {
        // We should not encounter R if we already have set other atoms.
        invalid_char = true;
        break;
      } else {
        is_runnable = true;
        continue;
      }
    }

    if (c == 'S')
      state_ |= Atom::kInterruptibleSleep;
    else if (c == 'D')
      state_ |= Atom::kUninterruptibleSleep;
    else if (c == 'T')
      state_ |= Atom::kStopped;
    else if (c == 't')
      state_ |= Atom::kTraced;
    else if (c == 'X')
      state_ |= Atom::kExitDead;
    else if (c == 'Z')
      state_ |= Atom::kExitZombie;
    else if (c == 'x')
      state_ |= Atom::kTaskDead;
    else if (c == 'K')
      state_ |= Atom::kWakeKill;
    else if (c == 'W')
      state_ |= Atom::kWaking;
    else if (c == 'P')
      state_ |= Atom::kParked;
    else if (c == 'N')
      state_ |= Atom::kNoLoad;
    else if (c == 'n')
      state_ |= Atom::kNewTask;
    else {
      invalid_char = true;
      break;
    }
  }

  bool no_state = !is_runnable && state_ == 0;
  if (invalid_char || no_state) {
    state_ = 0;
  } else {
    state_ |= kValid;
  }
}

TaskState::TaskStateStr TaskState::ToString() const {
  PERFETTO_CHECK(is_valid());

  char buffer[32];
  size_t pos = 0;

  // This mapping is given by the file
  // https://android.googlesource.com/kernel/msm.git/+/android-msm-wahoo-4.4-pie-qpr1/include/trace/events/sched.h#155
  if (is_runnable()) {
    buffer[pos++] = 'R';
  } else {
    if (state_ & Atom::kInterruptibleSleep)
      buffer[pos++] = 'S';
    if (state_ & Atom::kUninterruptibleSleep)
      buffer[pos++] = 'D';  // D for (D)isk sleep
    if (state_ & Atom::kStopped)
      buffer[pos++] = 'T';
    if (state_ & Atom::kTraced)
      buffer[pos++] = 't';
    if (state_ & Atom::kExitDead)
      buffer[pos++] = 'X';
    if (state_ & Atom::kExitZombie)
      buffer[pos++] = 'Z';
    if (state_ & Atom::kTaskDead)
      buffer[pos++] = 'x';
    if (state_ & Atom::kWakeKill)
      buffer[pos++] = 'K';
    if (state_ & Atom::kWaking)
      buffer[pos++] = 'W';
    if (state_ & Atom::kParked)
      buffer[pos++] = 'P';
    if (state_ & Atom::kNoLoad)
      buffer[pos++] = 'N';
    if (state_ & Atom::kNewTask)
      buffer[pos++] = 'n';
  }

  if (is_kernel_preempt())
    buffer[pos++] = '+';

  TaskStateStr output{};
  memcpy(output.data(), buffer, std::min(pos, output.size() - 1));
  return output;
}

void FormatSystracePrefix(int64_t timestamp,
                          uint32_t cpu,
                          uint32_t pid,
                          uint32_t tgid,
                          base::StringView name,
                          base::StringWriter* writer) {
  int64_t seconds = TimestampToSeconds(timestamp);
  int64_t useconds = TimestampToMicroseconds(timestamp);
  if (pid == 0) {
    name = "<idle>";
  }

  writer->WriteString(name);
  writer->WriteChar('-');
  writer->WriteInt(pid);

  constexpr char kTgidPrefix[] = "     (";
  writer->WriteString(kTgidPrefix, sizeof(kTgidPrefix) - 1);
  if (tgid == 0) {
    constexpr char kNoTgid[] = "-----";
    writer->WriteString(kNoTgid, sizeof(kNoTgid) - 1);
  } else {
    writer->WritePaddedInt<' ', 5>(tgid);
  }
  constexpr char kTgidSuffix[] = ") [";
  writer->WriteString(kTgidSuffix, sizeof(kTgidSuffix) - 1);

  writer->WritePaddedInt<'0', 3>(cpu);

  constexpr char kCpuSuffix[] = "] d..3 ";
  writer->WriteString(kCpuSuffix, sizeof(kCpuSuffix) - 1);

  writer->WriteInt(seconds);
  writer->WriteChar('.');
  writer->WritePaddedInt<'0', 6>(useconds);
  writer->WriteChar(':');
}

}  // namespace ftrace_utils
}  // namespace trace_processor
}  // namespace perfetto
