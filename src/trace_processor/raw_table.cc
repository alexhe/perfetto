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

#include "src/trace_processor/raw_table.h"

#include "src/trace_processor/sqlite_utils.h"

namespace perfetto {
namespace trace_processor {

RawTable::RawTable(sqlite3*, const TraceStorage* storage) : storage_(storage) {}

void RawTable::RegisterTable(sqlite3* db, const TraceStorage* storage) {
  Table::Register<RawTable>(db, storage, "raw");
}

base::Optional<Table::Schema> RawTable::Init(int, const char* const*) {
  const auto& raw = storage_->raw_events();
  schema_ =
      StorageSchema::Builder()
          .AddColumn<IdColumn>("id", TableId::kRawEvents)
          .AddOrderedNumericColumn("ts", &raw.timestamps())
          .AddStringColumn("name", &raw.name_ids(), &storage_->string_pool())
          .AddNumericColumn("utid", &raw.utids())
          .Build({"name", "ts"});
  return schema_.ToTableSchema();
}

std::unique_ptr<Table::Cursor> RawTable::CreateCursor(
    const QueryConstraints& qc,
    sqlite3_value** argv) {
  uint32_t count =
      static_cast<uint32_t>(storage_->raw_events().raw_event_count());
  auto it = CreateBestRowIteratorForGenericSchema(count, qc, argv);
  return std::unique_ptr<Table::Cursor>(
      new Cursor(std::move(it), schema_.mutable_columns()));
}

int RawTable::BestIndex(const QueryConstraints& qc, BestIndexInfo* info) {
  info->estimated_cost =
      static_cast<uint32_t>(storage_->raw_events().raw_event_count());

  // Only the string columns are handled by SQLite
  info->order_by_consumed = true;
  size_t name_index = schema_.ColumnIndexFromName("name");
  for (size_t i = 0; i < qc.constraints().size(); i++) {
    info->omit[i] = qc.constraints()[i].iColumn != static_cast<int>(name_index);
  }

  return SQLITE_OK;
}

}  // namespace trace_processor
}  // namespace perfetto
