
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
 * protos/tracing_service/data_source_config.proto
 * by
 * ../../tools/proto_to_cpp/proto_to_cpp.cc.
 * If you need to make changes here, change the .proto file and then run
 * ./tools/generate_cpp_headers.py
 */

#include "include/perfetto/tracing/core/data_source_config.h"

#include "protos/tracing_service/data_source_config.pb.h"

namespace perfetto {

DataSourceConfig::DataSourceConfig() = default;
DataSourceConfig::~DataSourceConfig() = default;
DataSourceConfig::DataSourceConfig(DataSourceConfig&&) noexcept = default;
DataSourceConfig& DataSourceConfig::operator=(DataSourceConfig&&) = default;

DataSourceConfig& DataSourceConfig::operator=(
    const perfetto::protos::DataSourceConfig& proto) {
  name_ = static_cast<decltype(name_)>(proto.name());
  target_buffer_ = static_cast<decltype(target_buffer_)>(proto.target_buffer());
  trace_category_filters_ = static_cast<decltype(trace_category_filters_)>(
      proto.trace_category_filters());
  unknown_fields_ = proto.unknown_fields();
  return *this;
}

void DataSourceConfig::ToProto(
    perfetto::protos::DataSourceConfig* proto) const {
  proto->Clear();
  proto->set_name(static_cast<decltype(proto->name())>(name_));
  proto->set_target_buffer(
      static_cast<decltype(proto->target_buffer())>(target_buffer_));
  proto->set_trace_category_filters(
      static_cast<decltype(proto->trace_category_filters())>(
          trace_category_filters_));
  *(proto->mutable_unknown_fields()) = unknown_fields_;
}

}  // namespace perfetto
