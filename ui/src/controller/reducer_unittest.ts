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

import {createEmptyState} from '../common/state';
import {rootReducer} from './reducer';

test('navigate', async () => {
  const before = createEmptyState();
  const after = rootReducer(before, {type: 'NAVIGATE', route: '/foo'});
  expect(after.route).toBe('/foo');
});

test('open trace', async () => {
  const before = createEmptyState();
  const after = rootReducer(before, {
    type: 'OPEN_TRACE',
    url: 'https://example.com/bar',
  });
  expect(after.engines[0].source).toBe('https://example.com/bar');
  expect(after.nextId).toBe(1);
  expect(after.route).toBe('/viewer');
});
