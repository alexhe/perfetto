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
#include "tools/ftrace_proto_gen/format_parser.h"

namespace {

TEST(FormatParser, HappyPath) {
  const std::string input = R"(name: the_name
ID: 42
format:
	field:unsigned short common_type;	offset:0;	size:2;	signed:0;
	field:unsigned char common_flags;	offset:2;	size:1;	signed:0;
	field:unsigned char common_preempt_count;	offset:3;	size:1;	signed:0;
	field:int common_pid;	offset:4;	size:4;	signed:1;

	field:char client_name[64];	offset:8;	size:64;	signed:0;
	field:const char * heap_name;	offset:72;	size:4;	signed:0;
	field:size_t len;	offset:76;	size:4;	signed:0;
	field:unsigned int mask;	offset:80;	size:4;	signed:0;
	field:unsigned int flags;	offset:84;	size:4;	signed:0;

print fmt: "client_name=%s heap_name=%s len=%zu mask=0x%x flags=0x%x", REC->client_name, REC->heap_name, REC->len, REC->mask, REC->flags
)";

  Format output;
  ASSERT_TRUE(ParseFormat(input));
  ASSERT_TRUE(ParseFormat(input.c_str(), input.length(), &output));
  EXPECT_EQ(output.name, "the_name");
  EXPECT_EQ(output.id, 42);
}

TEST(FormatParser, HalfAssedFuzzing) {
  const std::string input = R"(name: the_name
ID: 42
format:
	field:unsigned short common_type;	offset:0;	size:2;	signed:0;
	field:unsigned char common_flags;	offset:2;	size:1;	signed:0;
	field:unsigned char common_preempt_count;	offset:3;	size:1;	signed:0;
	field:int common_pid;	offset:4;	size:4;	signed:1;

	field:char client_name[64];	offset:8;	size:64;	signed:0;
	field:const char * heap_name;	offset:72;	size:4;	signed:0;
	field:size_t len;	offset:76;	size:4;	signed:0;
	field:unsigned int mask;	offset:80;	size:4;	signed:0;
	field:unsigned int flags;	offset:84;	size:4;	signed:0;

print fmt: "client_name=%s heap_name=%s len=%zu mask=0x%x flags=0x%x", REC->client_name, REC->heap_name, REC->len, REC->mask, REC->flags
)";

  for (size_t i=0; i<input.length(); i++) {
    for (size_t j=1; j<10 && i+j < input.length(); j++) {
      std::string copy = input;
      copy.erase(i, j);
      ParseFormat(copy);
    }
  }
}

}  // namespace
