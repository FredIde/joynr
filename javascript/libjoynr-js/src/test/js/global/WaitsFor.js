/*
 * #%L
 * %%
 * Copyright (C) 2011 - 2017 BMW Car IT GmbH
 * %%
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
 * #L%
 */

/**
 * waitsFor helper function for jasmine 2.x
 */
define([ "global/Promise"
], function(Promise) {
    var originalSetInterval = setInterval;
    var originalClearInterval = clearInterval;
    var originalSetTimeout = setTimeout;
    function waitsFor(checker, message, delayMs, checkIntervalMs) {
        return new Promise(function(resolve, reject) {
            delayMs = delayMs || 5000;
            checkIntervalMs = checkIntervalMs || 10;
            var intervalId = originalSetInterval(function() {
                if (checker() === true) {
                    originalClearInterval(intervalId);
                    resolve();
                }
                delayMs -= checkIntervalMs;
                if (delayMs <= 0) {
                    originalClearInterval(intervalId);
                    reject(new Error(message));
                }
            }, checkIntervalMs);
        });
    }
    return waitsFor;
});
