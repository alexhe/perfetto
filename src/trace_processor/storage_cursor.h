/*
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

#ifndef SRC_TRACE_PROCESSOR_STORAGE_CURSOR_H_
#define SRC_TRACE_PROCESSOR_STORAGE_CURSOR_H_

#include <deque>

#include "src/trace_processor/sqlite_utils.h"
#include "src/trace_processor/table.h"
#include "src/trace_processor/trace_storage.h"

namespace perfetto {
namespace trace_processor {

// A Cursor implementation which is has backing storage (i.e. can access a value
// at a row and column in constant time).
// Users can pass in a row iteration strategy and a column retriever; this class
// will use these to respond to cursor calls from SQLite.
class StorageCursor final : public Table::Cursor {
 public:
  class RowIterator {
   public:
    virtual ~RowIterator();

    virtual void NextRow() = 0;
    virtual uint32_t Row() = 0;
    virtual bool IsEnd() = 0;
  };

  class ColumnDefn {
   public:
    struct Bounds {
      uint32_t min_idx = 0;
      uint32_t max_idx = std::numeric_limits<uint32_t>::max();
      bool consumed = false;
    };
    using Predicate = std::function<bool(uint32_t)>;
    using Comparator = std::function<int(uint32_t, uint32_t)>;

    ColumnDefn(std::string col_name, bool hidden);
    virtual ~ColumnDefn();

    virtual Bounds BoundFilter(int op, sqlite3_value* value) = 0;
    virtual Predicate Filter(int op, sqlite3_value* value) = 0;
    virtual Comparator Sort(QueryConstraints::OrderBy ob) = 0;
    virtual void ReportResult(sqlite3_context*, uint32_t row) = 0;
    virtual Table::ColumnType GetType() = 0;
    virtual bool IsNaturallyOrdered() = 0;

    const std::string& name() { return col_name_; }
    bool hidden() { return hidden_; }

   private:
    std::string col_name_;
    bool hidden_ = false;
  };

  template <typename T>
  class NumericColumn final : public StorageCursor::ColumnDefn {
   public:
    using value_type = T;

    NumericColumn(std::string col_name,
                  const std::deque<T>* deque,
                  bool hidden,
                  bool is_naturally_ordered)
        : ColumnDefn(col_name, hidden),
          deque_(deque),
          is_naturally_ordered_(is_naturally_ordered) {}

    Bounds BoundFilter(int op, sqlite3_value* sqlite_val) override {
      Bounds bounds;
      bounds.max_idx = static_cast<uint32_t>(deque_->size());

      if (!is_naturally_ordered_)
        return bounds;

      auto min = std::numeric_limits<T>::min();
      auto max = std::numeric_limits<T>::max();

      // Makes the below code much more readable.
      using namespace sqlite_utils;

      // Try and bound the min and max value based on the constraints.
      auto value = sqlite_utils::ExtractSqliteValue<T>(sqlite_val);
      if (IsOpGe(op) || IsOpGt(op)) {
        min = IsOpGe(op) ? value : value + 1;
      } else if (IsOpLe(op) || IsOpLt(op)) {
        max = IsOpLe(op) ? value : value - 1;
      } else if (IsOpEq(op)) {
        min = value;
        max = value;
      } else {
        // We cannot bound on this constraint.
        return bounds;
      }

      // Convert the values into indices into the deque.
      auto min_it = std::lower_bound(deque_->begin(), deque_->end(), min);
      bounds.min_idx =
          static_cast<uint32_t>(std::distance(deque_->begin(), min_it));
      auto max_it = std::upper_bound(min_it, deque_->end(), max);
      bounds.max_idx =
          static_cast<uint32_t>(std::distance(deque_->begin(), max_it));

      return bounds;
    }

    Predicate Filter(int op, sqlite3_value* value) override {
      auto bipredicate = sqlite_utils::GetPredicateForOp<T>(op);
      T extracted = sqlite_utils::ExtractSqliteValue<T>(value);
      return [this, bipredicate, extracted](uint32_t idx) {
        return bipredicate(deque_->operator[](idx), extracted);
      };
    }

    Comparator Sort(QueryConstraints::OrderBy ob) override {
      if (ob.desc) {
        return [this](uint32_t f, uint32_t s) {
          auto a = deque_[f];
          auto b = deque_[s];
          if (a < b)
            return -1;
          else if (a > b)
            return 1;
          return 0;
        };
      }
      return [this](uint32_t f, uint32_t s) {
        auto a = deque_[f];
        auto b = deque_[s];
        if (a > b)
          return -1;
        else if (a < b)
          return 1;
        return 0;
      };
    }

    void ReportResult(sqlite3_context* ctx, uint32_t row) override {
      sqlite_utils::ReportSqliteResult(ctx, deque_->operator[](row));
    }

    bool IsNaturallyOrdered() override { return is_naturally_ordered_; }

    Table::ColumnType GetType() override {
      if (std::is_same<T, int32_t>::value) {
        return Table::ColumnType::kInt;
      } else if (std::is_same<T, uint32_t>::value) {
        return Table::ColumnType::kUint;
      } else if (std::is_same<T, int64_t>::value) {
        return Table::ColumnType::kLong;
      } else if (std::is_same<T, uint64_t>::value) {
        return Table::ColumnType::kUlong;
      } else if (std::is_same<T, double>::value) {
        return Table::ColumnType::kDouble;
      }
      PERFETTO_CHECK(false);
    }

   private:
    const std::deque<T>* deque_ = nullptr;
    bool is_naturally_ordered_ = false;
  };

  template <typename Id>
  class StringColumn final : public StorageCursor::ColumnDefn {
   public:
    StringColumn(std::string col_name,
                 const std::deque<Id>* deque,
                 const std::deque<std::string>* string_map,
                 bool hidden = false)
        : ColumnDefn(col_name, hidden),
          deque_(deque),
          string_map_(string_map) {}

    Bounds BoundFilter(int, sqlite3_value*) override {
      Bounds bounds;
      bounds.max_idx = static_cast<uint32_t>(deque_->size());
      return bounds;
    }

    Predicate Filter(int, sqlite3_value*) override {
      return [](uint32_t) { return true; };
    }

    Comparator Sort(QueryConstraints::OrderBy ob) override {
      if (ob.desc) {
        return [this](uint32_t f, uint32_t s) {
          const auto& a = string_map_->operator[](deque_->operator[](f));
          const auto& b = string_map_->operator[](deque_->operator[](s));
          if (a < b)
            return -1;
          else if (a > b)
            return 1;
          return 0;
        };
      }
      return [this](uint32_t f, uint32_t s) {
        const auto& a = string_map_->operator[](deque_->operator[](f));
        const auto& b = string_map_->operator[](deque_->operator[](s));
        if (a > b)
          return -1;
        else if (a < b)
          return 1;
        return 0;
      };
    }

    void ReportResult(sqlite3_context* ctx, uint32_t row) override {
      const auto& str = string_map_->operator[](deque_->operator[](row));
      if (str.empty()) {
        sqlite3_result_null(ctx);
      } else {
        auto kStatic = static_cast<sqlite3_destructor_type>(0);
        sqlite3_result_text(ctx, str.c_str(), -1, kStatic);
      }
    }

    Table::ColumnType GetType() override { return Table::ColumnType::kString; }

    bool IsNaturallyOrdered() override { return false; }

   private:
    const std::deque<Id>* deque_ = nullptr;
    const std::deque<std::string>* string_map_ = nullptr;
  };

  StorageCursor(std::unique_ptr<RowIterator>, std::vector<ColumnDefn*> cols);

  // Implementation of Table::Cursor.
  int Next() override;
  int Eof() override;
  int Column(sqlite3_context*, int N) override;

  template <typename T>
  static std::unique_ptr<NumericColumn<T>> NumericColumnPtr(
      std::string column_name,
      const std::deque<T>* deque,
      bool hidden = false,
      bool is_naturally_ordered = false) {
    return base::make_unique<NumericColumn<T>>(column_name, deque, hidden,
                                               is_naturally_ordered);
  }

  template <typename Id>
  static std::unique_ptr<StringColumn<Id>> StringColumnPtr(
      std::string column_name,
      const std::deque<Id>* deque,
      const std::deque<std::string>* lookup_map,
      bool hidden = false) {
    return base::make_unique<StringColumn<Id>>(column_name, deque, lookup_map,
                                               hidden);
  }

 private:
  std::unique_ptr<RowIterator> iterator_;
  std::vector<ColumnDefn*> columns_;
};

}  // namespace trace_processor
}  // namespace perfetto

#endif  // SRC_TRACE_PROCESSOR_STORAGE_CURSOR_H_
