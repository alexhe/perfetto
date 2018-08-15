// Copyright (C) 2018 The Android Open Source Project
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

import {IRawQueryArgs, RawQueryResult, TraceProcessor} from '../common/protos';
import {TimeSpan} from '../common/time';

/**
 * Abstract interface of a trace proccessor.
 * This class is wrapper for multiple proto services defined in:
 * //protos/perfetto/trace_processor/*
 * For each service ("FooService") Engine will have abstract getter
 * ("fooService") which returns a protobufjs rpc.Service object for
 * the given service.
 *
 * Engine also defines helpers for the most common service methods
 * (e.g. rawQuery).
 */
export abstract class Engine {
  abstract get traceProcessor(): TraceProcessor;

  /**
   * Send a raw SQL query to the engine.
   */
  rawQuery(args: IRawQueryArgs): Promise<RawQueryResult> {
    return this.traceProcessor.rawQuery(args);
  }

  async rawQueryOneRow(sqlQuery: string): Promise<number[]> {
    const result = await this.rawQuery({sqlQuery});
    const res: number[] = [];
    result.columns.map(c => res.push(+c.longValues![0]));
    return res;
  }

  // TODO(hjd): Maybe we should cache result? But then Engine must be
  // streaming aware.
  async getNumberOfCpus(): Promise<number> {
    const result = await this.rawQuery({
      sqlQuery: 'select count(distinct(cpu)) as cpuCount from sched;',
    });
    return +result.columns[0].longValues![0];
  }

  // TODO: This should live in code that's more specific to chrome, instead of
  // in engine.
  async getNumberOfProcesses(): Promise<number> {
    const result = await this.rawQuery({
      sqlQuery: 'select count(distinct(upid)) from thread;',
    });
    return +result.columns[0].longValues![0];
  }

  async getTraceTimeBounds(): Promise<TimeSpan> {
    const numSlices =
        (await this.rawQueryOneRow('select count(ts) from slices limit 1'))[0];
    const numSched =
        (await this.rawQueryOneRow('select count(ts) from sched limit 1'))[0];
    let start = Infinity;
    let end = -Infinity;
    if (numSlices > 0) {
      start = (await this.rawQueryOneRow(
          'select ts from slices order by ts limit 1'))[0];
      end = (await this.rawQueryOneRow(
          'select ts from slices order by ts desc limit 1'))[0];
    }
    if (numSched > 0) {
      const start2 = (await this.rawQueryOneRow(
          'select ts from sched order by ts limit 1'))[0];
      const end2 = (await this.rawQueryOneRow(
          'select ts from sched order by ts desc limit 1'))[0];
      start = Math.min(start, start2);
      end = Math.max(end, end2);
    }
    return new TimeSpan(start / 1e9, end / 1e9);
  }
}