// Copyright (C) 2019 The Android Open Source Project
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

import {Controller} from './controller';
import {globals} from './globals';
import {SliceSelection} from '../common/state';
import {Engine} from '../common/engine';
import {assertExists} from '../base/logging';

export interface SelectionControllerArgs {
  engine: Engine;
}

// This class queries the TP for the details on a specific slice that has
// been clicked.
export class SelectionController extends Controller<'main'> {
  private lastSelectedSlice?: SliceSelection;
  constructor(private args: SelectionControllerArgs) {
    super('main');
  }

  run() {
    if (globals.state.selectedSlice === undefined ||
        globals.state.selectedSlice === this.lastSelectedSlice) {
      return;
    }
    const selectedSlice = assertExists(globals.state.selectedSlice);
    this.lastSelectedSlice = selectedSlice;

    const ts = selectedSlice.timestamp;
    if (ts !== undefined) {
      const sqlQuery = `SELECT ts, dur, end_state, priority FROM sched 
                        WHERE ts = ${ts}`;
      this.args.engine.query(sqlQuery).then(result => {
        if (result.numRecords === 1) {
          const ts = result.columns[0].longValues![0] as number;
          const dur = result.columns[1].longValues![0] as number;
          const endState = result.columns[2].stringValues![0];
          const priority = result.columns[3].longValues![0] as number;
          const selected = {ts, dur, priority, endState};
          if (globals.state.selectedSlice === selectedSlice) {
            globals.publish('ClickSelection', selected);
          } 
        }
      });
    }
    
  }
}
