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

#include "gtest/gtest.h"
#include "tools/ftrace_proto_gen/ftrace_gen.h"

namespace {

TEST(FormatParser, NameFromTypeAndName) {
  EXPECT_EQ(NameFromTypeAndName("int foo"), "foo");
  EXPECT_EQ(NameFromTypeAndName("int foo_bar"), "foo_bar");
  EXPECT_EQ(NameFromTypeAndName("const char * foo"), "foo");
  EXPECT_EQ(NameFromTypeAndName("const char foo[64]"), "foo");
  EXPECT_EQ(NameFromTypeAndName("u8 foo[(int)sizeof(struct blah)]"), "foo");

  EXPECT_EQ(NameFromTypeAndName(""), "");
  EXPECT_EQ(NameFromTypeAndName("]"), "");
  EXPECT_EQ(NameFromTypeAndName("["), "");
  EXPECT_EQ(NameFromTypeAndName(" "), "");
  EXPECT_EQ(NameFromTypeAndName(" []"), "");
  EXPECT_EQ(NameFromTypeAndName(" ]["), "");
  EXPECT_EQ(NameFromTypeAndName("char"), "");
  EXPECT_EQ(NameFromTypeAndName("char *"), "");
  EXPECT_EQ(NameFromTypeAndName("char 42"), "");
}

TEST(FormatParser, InferProtoType) {
  EXPECT_EQ(InferProtoType(FormatField{"char * foo", 2, 0, false}), "string");
}

}  // namespace
