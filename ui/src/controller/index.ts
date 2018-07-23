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

import {forwardRemoteCalls, Remote} from '../base/remote';
import {Action} from '../common/actions';
import {createEmptyState, State} from '../common/state';
import {rootReducer} from './reducer';
import {WasmEngineProxy} from './wasm_engine_proxy';

class Controller {
  private state: State;
  private _frontend?: FrontendProxy;

  constructor() {
    this.state = createEmptyState();
  }

  get frontend(): FrontendProxy {
    if (!this._frontend) throw new Error('No FrontendProxy');
    return this._frontend;
  }

  init(frontendProxyPort: MessagePort): State {
    this._frontend = new FrontendProxy(new Remote(frontendProxyPort));
    return this.state;
  }

  dispatch(action: Action): void {
    this.doAction(action);
    this.frontend.updateState(this.state);
  }

  /**
   * Special case handling of loading a trace from a blob.
   * This can't be a pure action since we don't want to store
   * the Blob in the state.
   */
  loadTraceFromBlob(blob: Blob): void {
    this.createEngine(blob);
    this.frontend.updateState(this.state);
  }

  private async createEngine(blob: Blob) {
    const port = await this.frontend.createWasmEnginePort();
    const engine = WasmEngineProxy.create(port, blob);
    engine.rawQuery({sqlQuery: 'select * from sched;'}).then((result) => {
      console.log(result);
    });
  }

  private doAction(action: Action): void {
    this.state = rootReducer(this.state, action);
  }
}

/**
 * Proxy for talking to the main thread.
 * TODO(hjd): Reduce the boilerplate.
 */
class FrontendProxy {
  private readonly remote: Remote;

  constructor(remote: Remote) {
    this.remote = remote;
  }

  updateState(state: State) {
    return this.remote.send<void>('updateState', [state]);
  }

  createWasmEnginePort() {
    return this.remote.send<MessagePort>('createWasmEnginePort', []);
  }
}

function main() {
  const controller = new Controller();
  forwardRemoteCalls(self as {} as MessagePort, controller);
}

main();
