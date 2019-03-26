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

#ifndef SRC_PERFETTO_CMD_PLATFORM_TASK_RUNNER_H_
#define SRC_PERFETTO_CMD_PLATFORM_TASK_RUNNER_H_

#include "perfetto/base/build_config.h"
#include "perfetto/base/unix_task_runner.h"

#if PERFETTO_BUILDFLAG(PERFETTO_ANDROID_BUILD)
#include "perfetto/base/android_task_runner.h"
#endif  // PERFETTO_BUILDFLAG(PERFETTO_ANDROID_BUILD)

namespace perfetto {
#if PERFETTO_BUILDFLAG(PERFETTO_ANDROID_BUILD)
using PlatformTaskRunner = base::AndroidTaskRunner;
#else
using PlatformTaskRunner = base::UnixTaskRunner;
#endif
}  // namespace perfetto

#endif  // SRC_PERFETTO_CMD_PLATFORM_TASK_RUNNER_H_