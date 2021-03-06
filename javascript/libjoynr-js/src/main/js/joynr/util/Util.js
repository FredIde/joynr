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

define("joynr/util/Util", [
    "joynr/util/Typing",
    "joynr/types/TypeRegistrySingleton"
], function(Typing, TypeRegistrySingleton) {

    /**
     * @name Util
     * @class
     */
    var Util = {};

    /**
     * @function Util#ensureTypedValues
     * @param {Object} value
     * @param {Object} typeRegistry
     */
    Util.ensureTypedValues = function(value, typeRegistry) {
        var i;

        typeRegistry = typeRegistry || TypeRegistrySingleton.getInstance();

        if (value !== undefined && value !== null) {
            if (value.constructor.name === "Array") {
                for (i = 0; i < value.length; i++) {
                    value[i] = Util.ensureTypedValues(value[i]);
                }
            } else if (typeof value === "object" && !Typing.isComplexJoynrObject(value)) {
                value = Typing.augmentTypes(value, typeRegistry);
                /*jslint nomen: true */
                var Constructor = typeRegistry.getConstructor(value._typeName);
                /*jslint nomen: false */
                if (Constructor.checkMembers) {
                    Constructor.checkMembers(value, Typing.checkPropertyIfDefined);
                }
            }
        }

        return value;
    };

    return Util;

});
