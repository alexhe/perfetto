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

#ifndef SRC_PROFILING_MEMORY_STRING_INTERNER_H_
#define SRC_PROFILING_MEMORY_STRING_INTERNER_H_

#include <stddef.h>
#include <set>
#include <string>

namespace perfetto {

class StringInterner {
 private:
  class Entry {
   public:
    Entry(std::string string, StringInterner* interner);
    bool operator<(const Entry& other) const;
    const std::string& str();
    int& ref_count();
    StringInterner& interner();

   private:
    const std::string string_;
    int ref_count_ = 0;
    StringInterner* interner_;
  };

 public:
  class InternedString {
   public:
    friend class StringInterner;
    InternedString(StringInterner::Entry* str);
    InternedString(const InternedString& other);
    InternedString(InternedString&& other);
    InternedString& operator=(InternedString other);

    const std::string& str() const;
    ~InternedString();

   private:
    StringInterner::Entry* entry_;
  };

  InternedString Intern(std::string str);
  size_t entries_for_testing();

 private:
  void Return(Entry* entry);
  std::set<Entry> entries_;
};

static_assert(sizeof(StringInterner::InternedString) == sizeof(void*),
              "interned strings should be small");

}  // namespace perfetto

#endif  // SRC_PROFILING_MEMORY_STRING_INTERNER_H_
