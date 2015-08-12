/*jslint es5: true, node: true, nomen: true */
/*global requireJsDefine: true, requirejs: true*/

/*
 * #%L
 * %%
 * Copyright (C) 2011 - 2015 BMW Car IT GmbH
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

var requirejsNode;

function populateJoynrApi(joynr, api) {
    var key;
    for (key in api) {
        if (api.hasOwnProperty(key)) {
            joynr[key] = api[key];
        }
    }
}

function recursiveFreeze(object) {
    var property;
    var propertyKey = null;
    Object.freeze(object); // First freeze the object.
    for (propertyKey in object) {
        if (object.hasOwnProperty(propertyKey)) {
            property = object[propertyKey];
            if ((typeof property === 'object') && !Object.isFrozen(property)) {
                recursiveFreeze(property);
            }
        }
    }
}

function freeze(joynr, capabilitiesWritable) {
    Object.defineProperties(joynr, {
        "messaging" : {
            writable : false
        },
        "types" : {
            writable : false
        },
        "start" : {
            writable : false
        },
        "shutdown" : {
            writable : true
        },
        "typeRegistry" : {
            writable : false
        },
        "capabilities" : {
            writable : capabilitiesWritable === true
        },
        "proxy" : {
            writable : false
        },
        "proxyBuilder" : {
            writable : true
        },
        "providerBuilder" : {
            writable : false
        }
    });
}

/**
 * @name joynr
 * @class
 */
var joynr = {
    /**
     * @name joynr#load
     * @function
     * @param provisioning
     * @param callback
     * @param capabilitiesWriteable
     */
    load : function load(provisioning, callback, capabilitiesWritable) {
        requirejs = requirejsNode || requirejs;
        requirejs([ 'libjoynr-deps'
        ], function(joynrapi) {
            var runtime;
            runtime = new joynrapi.Runtime(provisioning);
            runtime.start().then(function() {
                populateJoynrApi(joynr, joynrapi);
                //remove Runtime, as it is not required for the end user
                delete joynr.Runtime;
                populateJoynrApi(joynr, runtime);
                freeze(joynr, capabilitiesWritable);
                callback(undefined, joynr);
            }).catch(function(error) {
                callback(error);
            });
        });
    },
    /**
     * Adds a typeName to constructor entry in the type registry.
     *
     * @name joynr#addType
     * @function
     * @see TypeRegistry#addType
     *
     * @param {String}
     *            joynrTypeName - the joynr type name that is sent on the wire.
     * @param {Function}
     *            typeConstructor - the corresponding JavaScript constructor for this type.
     */
    addType : function registerType(name, type) {
        requirejs([ "joynr/types/TypeRegistrySingleton"
        ], function(TypeRegistrySingleton) {
            TypeRegistrySingleton.getInstance().addType(name, type);
        });
    },
    JoynrObject : function JoynrObject() {}
};

var exports, module;
if (typeof define === 'function' && define.amd) {
    // expose joynr to requirejs in the event that requirejs is being
    // used natively, or by almond in the event that it has overwritten
    // define
    define([], function() {
        return joynr;
    });
}

// using optimized joynr, never used with nodejs
if (typeof requireJsDefine === 'function' && requireJsDefine.amd) {
    // define has been mapped to almond's define
    requireJsDefine([], function() {
        return joynr;
    });

    // using joynr with native nodejs require
} else if (exports !== undefined) {
    // configuring requirejs
    var requirejsConfig = require("./require.config.node.js");
    requirejsNode = require("requirejs");
    requirejsNode.config(requirejsConfig);
    if ((module !== undefined) && module.exports) {
        exports.joynr = module.exports = joynr;
    } else {
        // support CommonJS module 1.1.1 spec (`exports` cannot be a function)
        exports.joynr = joynr;
    }

    // not using AMD
} else if (typeof window === "object") {
    // export namespace fragment or module read-only to the parent namespace
    Object.defineProperty(window, "joynr", {
        readable : true,
        enumerable : true,
        configurable : false,
        writable : false,
        value : joynr
    });
}

/* jslint nomen: false */