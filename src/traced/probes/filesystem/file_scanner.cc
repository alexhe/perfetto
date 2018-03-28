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

#include "src/traced/probes/filesystem/file_scanner.h"

#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "perfetto/base/scoped_file.h"
#include "src/traced/probes/filesystem/inode_file_data_source.h"

namespace perfetto {
namespace {

std::string JoinPaths(const std::string& one, const std::string& other) {
  std::string result;
  result.reserve(one.size() + other.size() + 1);
  result += one;
  if (!result.empty() && result[result.size() - 1] != '/') {
    result += '/';
  }
  result += other;
  return result;
}
}  // namespace

FileScanner::FileScanner(
    std::string root_directory,
    std::function<bool(BlockDeviceID block_device_id,
                       Inode inode_number,
                       const std::string& path,
                       protos::pbzero::InodeFileMap_Entry_Type type)> callback,
    std::function<void()> done_callback,
    uint64_t scan_interval,
    uint64_t scan_steps)
    : callback_(std::move(callback)),
      done_callback_(done_callback),
      scan_interval_ms_(scan_interval),
      scan_steps_(scan_steps),
      queue_({std::move(root_directory)}) {}

void FileScanner::Scan(base::TaskRunner* task_runner) {
  Steps(scan_steps_);
  if (!done()) {
    task_runner->PostDelayedTask([this, task_runner] { Scan(task_runner); },
                                 scan_interval_ms_);
  }
}

void FileScanner::NextDirectory() {
  std::string directory = queue_.back();
  queue_.pop_back();
  current_directory_fd_.reset(opendir(directory.c_str()));

  struct stat buf;
  if (fstat(dirfd(current_directory_fd_.get()), &buf) != 0)
    return;

  if (S_ISLNK(buf.st_mode)) {
    current_directory_fd_.reset();
    return;
  }

  current_directory_ = std::move(directory);
}

void FileScanner::Step() {
  if (!current_directory_fd_) {
    if (queue_.empty())
      return;
    NextDirectory();
  }

  if (!current_directory_fd_)
    return;

  struct dirent* entry = readdir(current_directory_fd_.get());
  if (entry == nullptr) {
    current_directory_fd_.reset();
    return;
  }

  std::string filename = entry->d_name;
  if (filename == "." || filename == "..")
    return;

  std::string filepath = JoinPaths(current_directory_, filename);

  protos::pbzero::InodeFileMap_Entry_Type type =
      protos::pbzero::InodeFileMap_Entry_Type_UNKNOWN;
  // Readdir and stat not guaranteed to have directory info for all systems
  if (entry->d_type == DT_DIR) {
    // Continue iterating through files if current entry is a directory
    queue_.emplace_back(filepath);
    type = protos::pbzero::InodeFileMap_Entry_Type_DIRECTORY;
  } else if (entry->d_type == DT_REG) {
    type = protos::pbzero::InodeFileMap_Entry_Type_FILE;
  }

  if (!callback_(current_block_device_id_, entry->d_ino, filepath, type)) {
    queue_.clear();
    current_directory_fd_.reset();
  }
}

void FileScanner::Steps(uint64_t n) {
  for (uint64_t i = 0; i < n && !done(); ++i)
    Step();
}

bool FileScanner::done() {
  return !current_directory_fd_ && queue_.empty();
}
}  // namespace perfetto
