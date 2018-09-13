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

#include <stdlib.h>
#include <memory>

#include "src/ipc/unix_socket.h"
#include "src/profiling/memory/bounded_queue.h"
#include "src/profiling/memory/socket_listener.h"

#include "perfetto/base/unix_task_runner.h"

namespace perfetto {
namespace {

constexpr size_t kUnwinderQueueSize = 1000;
constexpr size_t kBookkeepingQueueSize = 1000;
constexpr size_t kUnwinderThreads = 5;

int HeapprofdMain(int argc, char** argv) {
  std::unique_ptr<ipc::UnixSocket> sock;
  BoundedQueue<UnwindingRecord> unwinder_queue(kUnwinderQueueSize, true);
  BoundedQueue<UnwoundRecord> bookkeeping_queue(kBookkeepingQueueSize, true);

  std::vector<std::thread> unwinding_threads;
  unwinding_threads.reserve(kUnwinderThreads);
  for (size_t i = 0; i < kUnwinderThreads; ++i) {
    unwinding_threads.emplace_back([&unwinder_queue, &bookkeeping_queue] {
      UnwindingMainLoop(&unwinder_queue, &bookkeeping_queue);
    });
  }

  SocketListener listener([&unwinder_queue](UnwindingRecord r) {
    unwinder_queue.Add(std::move(r));
  });

  base::UnixTaskRunner read_task_runner;
  if (argc == 2) {
    // Allow to be able to manually specify the socket to listen on
    // for testing and sideloading purposes.
    sock = ipc::UnixSocket::Listen(argv[1], &listener, &read_task_runner);
  } else if (argc == 1) {
    // When running as a service launched by init on Android, the socket
    // is created by init and passed to the application using an environment
    // variable.
    const char* sock_fd = getenv("ANDROID_SOCKET_heapprofd");
    if (sock_fd == nullptr)
      PERFETTO_FATAL(
          "No argument given and environment variable ANDROID_SOCKET_heapprof "
          "is unset.");
    char* end;
    int raw_fd = static_cast<int>(strtol(sock_fd, &end, 10));
    if (*end != '\0')
      PERFETTO_FATAL(
          "Invalid ANDROID_SOCKET_heapprofd. Expected decimal integer.");
    sock = ipc::UnixSocket::Listen(base::ScopedFile(raw_fd), &listener,
                                   &read_task_runner);
  } else {
    PERFETTO_FATAL("Invalid number of arguments. %s [SOCKET]", argv[0]);
  }

  if (sock->last_error() != 0)
    PERFETTO_FATAL("Failed to initialize socket: %s",
                   strerror(sock->last_error()));

  read_task_runner.Run();
  return 0;
}
}  // namespace
}  // namespace perfetto

int main(int argc, char** argv) {
  return perfetto::HeapprofdMain(argc, argv);
}
