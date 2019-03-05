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

#include "perfetto/base/unix_task_runner.h"
#include "perfetto/base/watchdog.h"
#include "perfetto/traced/traced.h"
#include "perfetto/tracing/ipc/service_ipc_host.h"
#include "src/traced/service/lazy_producers.h"
#include "src/tracing/core/tracing_service_impl.h"
#include "src/tracing/ipc/default_socket.h"
#include "src/tracing/ipc/posix_shared_memory.h"

namespace perfetto {

int __attribute__((visibility("default"))) ServiceMain(int, char**) {
  base::UnixTaskRunner task_runner;
  LazyProducers lazy_producers;
  std::unique_ptr<ServiceIPCHost> svc;
  auto shm_factory =
      std::unique_ptr<SharedMemory::Factory>(new PosixSharedMemory::Factory());
  std::unique_ptr<TracingServiceImpl> tracing_svc(
      new TracingServiceImpl(std::move(shm_factory), &task_runner));
  tracing_svc->SetTracingSessionObserver(&lazy_producers);
  svc = ServiceIPCHost::CreateInstance(&task_runner, std::move(tracing_svc));

  // When built as part of the Android tree, the two socket are created and
  // bonund by init and their fd number is passed in two env variables.
  // See libcutils' android_get_control_socket().
  const char* env_prod = getenv("ANDROID_SOCKET_traced_producer");
  const char* env_cons = getenv("ANDROID_SOCKET_traced_consumer");
  PERFETTO_CHECK((!env_prod && !env_cons) || (env_prod && env_cons));
  bool started;
  if (env_prod) {
    base::ScopedFile producer_fd(atoi(env_prod));
    base::ScopedFile consumer_fd(atoi(env_cons));
    started = svc->Start(std::move(producer_fd), std::move(consumer_fd));
  } else {
    unlink(GetProducerSocket());
    unlink(GetConsumerSocket());
    started = svc->Start(GetProducerSocket(), GetConsumerSocket());
  }

  if (!started) {
    PERFETTO_ELOG("Failed to start the traced service");
    return 1;
  }

  // Set the CPU limit and start the watchdog running. The memory limit will
  // be set inside the service code as it relies on the size of buffers.
  // The CPU limit is 75% over a 30 second interval.
  base::Watchdog* watchdog = base::Watchdog::GetInstance();
  watchdog->SetCpuLimit(75, 30 * 1000);
  watchdog->Start();

  PERFETTO_ILOG("Started traced, listening on %s %s", GetProducerSocket(),
                GetConsumerSocket());
  task_runner.Run();
  return 0;
}

}  // namespace perfetto
