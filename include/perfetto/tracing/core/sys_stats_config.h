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

/*******************************************************************************
 * AUTOGENERATED - DO NOT EDIT
 *******************************************************************************
 * This file has been generated from the protobuf message
 * perfetto/config/sys_stats/sys_stats_config.proto
 * by
 * ../../tools/proto_to_cpp/proto_to_cpp.cc.
 * If you need to make changes here, change the .proto file and then run
 * ./tools/gen_tracing_cpp_headers_from_protos.py
 */

#ifndef INCLUDE_PERFETTO_TRACING_CORE_SYS_STATS_CONFIG_H_
#define INCLUDE_PERFETTO_TRACING_CORE_SYS_STATS_CONFIG_H_

#include <stdint.h>
#include <string>
#include <type_traits>
#include <vector>

#include "perfetto/base/export.h"

#include "perfetto/tracing/core/sys_stats_counters.h"

// Forward declarations for protobuf types.
namespace perfetto {
namespace protos {
class SysStatsConfig;
}
}  // namespace perfetto

namespace perfetto {

class PERFETTO_EXPORT SysStatsConfig {
 public:
  enum MeminfoCounters {
    MEM_TOTAL = 1,
    MEM_FREE = 2,
    MEM_AVAILABLE = 3,
    BUFFERS = 4,
    CACHED = 5,
    SWAP_CACHED = 6,
    ACTIVE = 7,
    INACTIVE = 8,
    ACTIVE_ANON = 9,
    INACTIVE_ANON = 10,
    ACTIVE_FILE = 11,
    INACTIVE_FILE = 12,
    UNEVICTABLE = 13,
    MLOCKED = 14,
    SWAP_TOTAL = 15,
    SWAP_FREE = 16,
    DIRTY = 17,
    WRITEBACK = 18,
    ANON_PAGES = 19,
    MAPPED = 20,
    SHMEM = 21,
    SLAB = 22,
    SLAB_RECLAIMABLE = 23,
    SLAB_UNRECLAIMABLE = 24,
    KERNEL_STACK = 25,
    PAGE_TABLES = 26,
    COMMIT_LIMIT = 27,
    COMMITED_AS = 28,
    VMALLOC_TOTAL = 29,
    VMALLOC_USED = 30,
    VMALLOC_CHUNK = 31,
    CMA_TOTAL = 32,
    CMA_FREE = 33,
  };
  enum VmstatCounters {
    NR_FREE_PAGES = 1,
    NR_ALLOC_BATCH = 2,
    NR_INACTIVE_ANON = 3,
    NR_ACTIVE_ANON = 4,
    NR_INACTIVE_FILE = 5,
    NR_ACTIVE_FILE = 6,
    NR_UNEVICTABLE = 7,
    NR_MLOCK = 8,
    NR_ANON_PAGES = 9,
    NR_MAPPED = 10,
    NR_FILE_PAGES = 11,
    NR_DIRTY = 12,
    NR_WRITEBACK = 13,
    NR_SLAB_RECLAIMABLE = 14,
    NR_SLAB_UNRECLAIMABLE = 15,
    NR_PAGE_TABLE_PAGES = 16,
    NR_KERNEL_STACK = 17,
    NR_OVERHEAD = 18,
    NR_UNSTABLE = 19,
    NR_BOUNCE = 20,
    NR_VMSCAN_WRITE = 21,
    NR_VMSCAN_IMMEDIATE_RECLAIM = 22,
    NR_WRITEBACK_TEMP = 23,
    NR_ISOLATED_ANON = 24,
    NR_ISOLATED_FILE = 25,
    NR_SHMEM = 26,
    NR_DIRTIED = 27,
    NR_WRITTEN = 28,
    NR_PAGES_SCANNED = 29,
    WORKINGSET_REFAULT = 30,
    WORKINGSET_ACTIVATE = 31,
    WORKINGSET_NODERECLAIM = 32,
    NR_ANON_TRANSPARENT_HUGEPAGES = 33,
    NR_FREE_CMA = 34,
    NR_SWAPCACHE = 35,
    NR_DIRTY_THRESHOLD = 36,
    NR_DIRTY_BACKGROUND_THRESHOLD = 37,
    PGPGIN = 38,
    PGPGOUT = 39,
    PGPGOUTCLEAN = 40,
    PSWPIN = 41,
    PSWPOUT = 42,
    PGALLOC_DMA = 43,
    PGALLOC_NORMAL = 44,
    PGALLOC_MOVABLE = 45,
    PGFREE = 46,
    PGACTIVATE = 47,
    PGDEACTIVATE = 48,
    PGFAULT = 49,
    PGMAJFAULT = 50,
    PGREFILL_DMA = 51,
    PGREFILL_NORMAL = 52,
    PGREFILL_MOVABLE = 53,
    PGSTEAL_KSWAPD_DMA = 54,
    PGSTEAL_KSWAPD_NORMAL = 55,
    PGSTEAL_KSWAPD_MOVABLE = 56,
    PGSTEAL_DIRECT_DMA = 57,
    PGSTEAL_DIRECT_NORMAL = 58,
    PGSTEAL_DIRECT_MOVABLE = 59,
    PGSCAN_KSWAPD_DMA = 60,
    PGSCAN_KSWAPD_NORMAL = 61,
    PGSCAN_KSWAPD_MOVABLE = 62,
    PGSCAN_DIRECT_DMA = 63,
    PGSCAN_DIRECT_NORMAL = 64,
    PGSCAN_DIRECT_MOVABLE = 65,
    PGSCAN_DIRECT_THROTTLE = 66,
    PGINODESTEAL = 67,
    SLABS_SCANNED = 68,
    KSWAPD_INODESTEAL = 69,
    KSWAPD_LOW_WMARK_HIT_QUICKLY = 70,
    KSWAPD_HIGH_WMARK_HIT_QUICKLY = 71,
    PAGEOUTRUN = 72,
    ALLOCSTALL = 73,
    PGROTATED = 74,
    DROP_PAGECACHE = 75,
    DROP_SLAB = 76,
    PGMIGRATE_SUCCESS = 77,
    PGMIGRATE_FAIL = 78,
    COMPACT_MIGRATE_SCANNED = 79,
    COMPACT_FREE_SCANNED = 80,
    COMPACT_ISOLATED = 81,
    COMPACT_STALL = 82,
    COMPACT_FAIL = 83,
    COMPACT_SUCCESS = 84,
    COMPACT_DAEMON_WAKE = 85,
    UNEVICTABLE_PGS_CULLED = 86,
    UNEVICTABLE_PGS_SCANNED = 87,
    UNEVICTABLE_PGS_RESCUED = 88,
    UNEVICTABLE_PGS_MLOCKED = 89,
    UNEVICTABLE_PGS_MUNLOCKED = 90,
    UNEVICTABLE_PGS_CLEARED = 91,
    UNEVICTABLE_PGS_STRANDED = 92,
  };
  enum StatCounters {
    CPU_TIMES = 1,
    IRQ_COUNTS = 2,
    SOFTIRQ_COUNTS = 3,
    FORK_COUNT = 4,
  };
  SysStatsConfig();
  ~SysStatsConfig();
  SysStatsConfig(SysStatsConfig&&) noexcept;
  SysStatsConfig& operator=(SysStatsConfig&&);
  SysStatsConfig(const SysStatsConfig&);
  SysStatsConfig& operator=(const SysStatsConfig&);

  // Conversion methods from/to the corresponding protobuf types.
  void FromProto(const perfetto::protos::SysStatsConfig&);
  void ToProto(perfetto::protos::SysStatsConfig*) const;

  uint32_t meminfo_period_ms() const { return meminfo_period_ms_; }
  void set_meminfo_period_ms(uint32_t value) { meminfo_period_ms_ = value; }

  int meminfo_counters_size() const {
    return static_cast<int>(meminfo_counters_.size());
  }
  const std::vector<MeminfoCounters>& meminfo_counters() const {
    return meminfo_counters_;
  }
  MeminfoCounters* add_meminfo_counters() {
    meminfo_counters_.emplace_back();
    return &meminfo_counters_.back();
  }

  uint32_t vmstat_period_ms() const { return vmstat_period_ms_; }
  void set_vmstat_period_ms(uint32_t value) { vmstat_period_ms_ = value; }

  int vmstat_counters_size() const {
    return static_cast<int>(vmstat_counters_.size());
  }
  const std::vector<VmstatCounters>& vmstat_counters() const {
    return vmstat_counters_;
  }
  VmstatCounters* add_vmstat_counters() {
    vmstat_counters_.emplace_back();
    return &vmstat_counters_.back();
  }

  uint32_t stat_period_ms() const { return stat_period_ms_; }
  void set_stat_period_ms(uint32_t value) { stat_period_ms_ = value; }

  int stat_counters_size() const {
    return static_cast<int>(stat_counters_.size());
  }
  const std::vector<StatCounters>& stat_counters() const {
    return stat_counters_;
  }
  StatCounters* add_stat_counters() {
    stat_counters_.emplace_back();
    return &stat_counters_.back();
  }

 private:
  uint32_t meminfo_period_ms_ = {};
  std::vector<MeminfoCounters> meminfo_counters_;
  uint32_t vmstat_period_ms_ = {};
  std::vector<VmstatCounters> vmstat_counters_;
  uint32_t stat_period_ms_ = {};
  std::vector<StatCounters> stat_counters_;

  // Allows to preserve unknown protobuf fields for compatibility
  // with future versions of .proto files.
  std::string unknown_fields_;
};

}  // namespace perfetto
#endif  // INCLUDE_PERFETTO_TRACING_CORE_SYS_STATS_CONFIG_H_
