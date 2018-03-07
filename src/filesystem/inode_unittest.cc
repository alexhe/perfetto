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

#include "src/filesystem/inode.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace perfetto {
namespace {

using testing::ElementsAre;

TEST(InodeTest, ParseMounts) {
  auto mounts = ParseMounts();
  struct stat buf;
  ASSERT_NE(stat("/proc", &buf), -1);
  EXPECT_THAT(mounts[buf.st_dev], ElementsAre("/proc"));
}

}  // namespace
}  // namespace perfetto
