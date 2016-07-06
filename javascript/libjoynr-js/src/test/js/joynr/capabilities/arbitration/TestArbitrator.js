/*global joynrTestRequire: true */
/*jslint es5: true */

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

joynrTestRequire(
        "joynr/capabilities/arbitration/TestArbitrator",
        [
            "joynr/capabilities/arbitration/Arbitrator",
            "joynr/types/DiscoveryEntry",
            "joynr/types/ProviderQos",
            "joynr/types/CustomParameter",
            "joynr/proxy/DiscoveryQos",
            "joynr/types/ArbitrationStrategyCollection",
            "joynr/types/DiscoveryQos",
            "joynr/types/DiscoveryScope",
            "joynr/exceptions/DiscoveryException",
            "joynr/exceptions/NoCompatibleProviderFoundException",
            "joynr/types/Version",
            "global/Promise",
            "Date"
        ],
        function(
                Arbitrator,
                DiscoveryEntry,
                ProviderQos,
                CustomParameter,
                DiscoveryQos,
                ArbitrationStrategyCollection,
                DiscoveryQosGen,
                DiscoveryScope,
                DiscoveryException,
                NoCompatibleProviderFoundException,
                Version,
                Promise,
                Date) {
            var capabilities, fakeTime, staticArbitrationSettings, staticArbitrationSpy, domain;
            var interfaceName, discoveryQos, capDiscoverySpy, arbitrator, discoveryEntries, nrTimes;
            var safetyTimeoutDelta = 100;
            var discoveryEntryWithMajor47AndMinor0, discoveryEntryWithMajor47AndMinor1;
            var discoveryEntryWithMajor47AndMinor2, discoveryEntryWithMajor47AndMinor3;
            var discoveryEntryWithMajor48AndMinor2;

            function increaseFakeTime(time_ms) {
                fakeTime = fakeTime + time_ms;
                jasmine.Clock.tick(time_ms);
            }

            function getDiscoveryEntry(domain, interfaceName, discoveryStrategy, providerVersion) {
                return new DiscoveryEntry({
                    providerVersion : providerVersion,
                    domain : domain,
                    interfaceName : interfaceName,
                    qos : new ProviderQos({
                        customParameter : [new CustomParameter( { name : "theName", value : "theValue"})],
                        priority : 123,
                        scope : discoveryQos.discoveryScope === DiscoveryScope.LOCAL_ONLY ? true : false,
                        supportsOnChangeSubscriptions : true}),
                    participandId : "700",
                    lastSeenDateMs : Date.now(),
                    publicKeyId : ""
                });
            }

            describe(
                    "libjoynr-js.joynr.capabilities.arbitration.Arbitrator",
                    function() {
                        capabilities = [
                            {
                                domain : "myDomain",
                                interfaceName : "myInterface",
                                participantId : 1,
                                providerVersion : new Version({ majorVersion: 47, minorVersion: 11})
                            },
                            {
                                domain : "myDomain",
                                interfaceName : "myInterface",
                                participantId : 2,
                                providerVersion : new Version({ majorVersion: 47, minorVersion: 11})
                            },
                            {
                                domain : "otherDomain",
                                interfaceName : "otherInterface",
                                participantId : 3,
                                providerVersion : new Version({ majorVersion: 47, minorVersion: 11})
                            },
                            {
                                domain : "thirdDomain",
                                interfaceName : "otherInterface",
                                participantId : 4,
                                providerVersion : new Version({ majorVersion: 47, minorVersion: 11})
                            }
                        ];

                        beforeEach(function() {
                            var i;
                            var providerQos;
                            domain = "myDomain";
                            interfaceName = "myInterface";
                            discoveryQos = new DiscoveryQos({
                                discoveryTimeoutMs : 5000,
                                discoveryRetryDelayMs : 900,
                                arbitrationStrategy : ArbitrationStrategyCollection.Nothing,
                                cacheMaxAgeMs : 0,
                                discoveryScope : DiscoveryScope.LOCAL_THEN_GLOBAL,
                                additionalParameters : {}
                            });

                            staticArbitrationSettings = {
                                domains : [domain],
                                interfaceName : interfaceName,
                                discoveryQos : discoveryQos,
                                staticArbitration : true,
                                proxyVersion : new Version({ majorVersion: 47, minorVersion: 11})
                            };

                            staticArbitrationSpy = jasmine.createSpyObj("staticArbitrationSpy", [
                                "resolve",
                                "reject"
                            ]);

                            capDiscoverySpy =
                                    jasmine.createSpyObj("capabilityDiscovery", [ "lookup"
                                    ]);
                            capDiscoverySpy.lookup.andReturn(Promise.resolve([]));

                            arbitrator = new Arbitrator(capDiscoverySpy, capabilities);

                            discoveryEntries = [];
                            for (i = 0; i < 12; ++i) {
                                discoveryEntries.push(getDiscoveryEntry(
                                        domain + i.toString(),
                                        interfaceName + i.toString(),
                                        discoveryQos.discoveryStrategy,
                                        new Version({ majorVersion: 47, minorVersion: 11})
                                ));
                            }

                            // prepare a number of similar discovery entries with different
                            // provider versions

                            providerQos = new ProviderQos({
                                    customParameters : [],
                                    priority: 123,
                                    scope: discoveryQos.discoveryScope === DiscoveryScope.LOCAL_ONLY
                                            ? true : false,
                                    supportsOnChangeSubscriptions : true
                            });

                            discoveryEntryWithMajor47AndMinor0 = new DiscoveryEntry({
                                providerVersion : new Version({ majorVersion: 47, minorVersion: 0 }),
                                domain : domain,
                                interfaceName : interfaceName,
                                qos : providerQos,
                                participantId : "700",
                                lastSeenDateMs : Date.now(),
                                publicKeyId : ""
                            });

                            discoveryEntryWithMajor47AndMinor1 = new DiscoveryEntry({
                                providerVersion : new Version({ majorVersion: 47, minorVersion: 1 }),
                                domain : domain,
                                interfaceName : interfaceName,
                                qos : providerQos,
                                participantId : "700",
                                lastSeenDateMs : Date.now(),
                                publicKeyId : ""
                            });

                            discoveryEntryWithMajor47AndMinor2 = new DiscoveryEntry({
                                providerVersion : new Version({ majorVersion: 47, minorVersion: 2 }),
                                domain : domain,
                                interfaceName : interfaceName,
                                qos : providerQos,
                                participantId : "700",
                                lastSeenDateMs : Date.now(),
                                publicKeyId : ""
                            });

                            discoveryEntryWithMajor47AndMinor3 = new DiscoveryEntry({
                                providerVersion : new Version({ majorVersion: 47, minorVersion: 3 }),
                                domain : domain,
                                interfaceName : interfaceName,
                                qos : providerQos,
                                participantId : "700",
                                lastSeenDateMs : Date.now(),
                                publicKeyId : ""
                            });

                            discoveryEntryWithMajor48AndMinor2 = new DiscoveryEntry({
                                providerVersion : new Version({ majorVersion: 48, minorVersion: 2 }),
                                domain : domain,
                                interfaceName : interfaceName,
                                qos : providerQos,
                                participantId : "700",
                                lastSeenDateMs : Date.now(),
                                publicKeyId : ""
                            });

                            //discoveryQos.arbitrationStrategy.andReturn([]);

                            nrTimes = 5;
                            fakeTime = 0;

                            jasmine.Clock.useMock();
                            jasmine.Clock.reset();
                            spyOn(Date, "now").andCallFake(function() {
                                return fakeTime;
                            });
                        });

                        it("is instantiable", function() {
                            expect(new Arbitrator({})).toBeDefined();
                        });

                        it(
                                "is of correct type and has all members",
                                function() {
                                    expect(arbitrator).toBeDefined();
                                    expect(arbitrator).not.toBeNull();
                                    expect(typeof arbitrator === "object").toBeTruthy();
                                    expect(arbitrator instanceof Arbitrator).toEqual(true);
                                    expect(arbitrator.startArbitration).toBeDefined();
                                    expect(typeof arbitrator.startArbitration === "function")
                                            .toEqual(true);
                                });

                        it("calls capabilityDiscovery upon arbitration", function() {

                            // return some discoveryEntries so that arbitration is faster
                            // (instantly instead of discoveryTimeoutMs)
                            capDiscoverySpy.lookup.andReturn(Promise.resolve(discoveryEntries));
                            spyOn(discoveryQos, "arbitrationStrategy").andReturn(discoveryEntries);
                            arbitrator = new Arbitrator(capDiscoverySpy);

                            var resolved = false;

                            runs(function() {
                                // start arbitration
                                arbitrator.startArbitration({
                                    domains : [domain],
                                    interfaceName : interfaceName,
                                    discoveryQos : discoveryQos,
                                    proxyVersion : new Version({ majorVersion: 47, minorVersion: 11})
                                }).then(function() {
                                    resolved = true;
                                });
                                increaseFakeTime(1);
                            });

                            waitsFor(function() {
                                return resolved;
                            }, "successful arbitration", 100);

                            runs(function() {
                                expect(resolved).toBeTruthy();
                                expect(capDiscoverySpy.lookup).toHaveBeenCalled();
                                /* The arbitrator.startArbitration does a deep copy of its arguments.
                                 * Thus, two discoveryScope objects cannot be compared, as during deep copy
                                 * complex types are created as pure objects
                                 */
                                expect(capDiscoverySpy.lookup.mostRecentCall.args[0]).toEqual([domain]);
                                expect(capDiscoverySpy.lookup.mostRecentCall.args[1]).toBe(interfaceName);
                                expect(capDiscoverySpy.lookup.mostRecentCall.args[2].cacheMaxAge).toBe(discoveryQos.cacheMaxAgeMs);
                                expect(capDiscoverySpy.lookup.mostRecentCall.args[2].discoveryScope.name).toBe(discoveryQos.discoveryScope.name);
                            });
                        });

                        it("returns capabilities from discovery", function() {
                            var onFulfilledSpy, onRejectedSpy;

                            // return discoveryEntries to check whether these are eventually
                            // returned by the arbitrator
                            capDiscoverySpy.lookup.andReturn(Promise.resolve(discoveryEntries));
                            arbitrator = new Arbitrator(capDiscoverySpy);

                            // spy on and instrument arbitrationStrategy
                            spyOn(discoveryQos, "arbitrationStrategy").andCallThrough();

                            // call arbitrator
                            onFulfilledSpy = jasmine.createSpy("onFulfilledSpy");
                            onRejectedSpy = jasmine.createSpy("onRejectedSpy");

                            runs(function() {
                                arbitrator.startArbitration({
                                    domains : [domain],
                                    interfaceName : interfaceName,
                                    discoveryQos : discoveryQos,
                                    proxyVersion : new Version({ majorVersion: 47, minorVersion: 11})
                                }).then(onFulfilledSpy).catch(onRejectedSpy);
                                increaseFakeTime(1);
                            });

                            waitsFor(function() {
                                return onFulfilledSpy.callCount > 0;
                            }, "until onresolve has been invoked", 300);

                            runs(function() {

                                // arbitrator finally returned the discoveryEntries (unfiltered
                                // because of ArbitrationStrategyCollection.Nothing)
                                expect(onRejectedSpy).not.toHaveBeenCalled();
                                expect(onFulfilledSpy).toHaveBeenCalled();
                                expect(onFulfilledSpy).toHaveBeenCalledWith(discoveryEntries);
                            });
                        });

                        it("returns capabilities with matching provider version", function() {
                            var onFulfilledSpy, onRejectedSpy;
                            var discoveryEntriesWithDifferentProviderVersions = [
                                discoveryEntryWithMajor47AndMinor0,
                                discoveryEntryWithMajor47AndMinor1,
                                discoveryEntryWithMajor47AndMinor2,
                                discoveryEntryWithMajor47AndMinor3,
                                discoveryEntryWithMajor48AndMinor2
                            ];

                            // return discoveryEntries to check whether a subset is eventually
                            // returned by the arbitrator
                            capDiscoverySpy.lookup.andReturn(Promise.resolve(discoveryEntriesWithDifferentProviderVersions));
                            arbitrator = new Arbitrator(capDiscoverySpy);

                            // spy on and instrument arbitrationStrategy
                            spyOn(discoveryQos, "arbitrationStrategy").andCallThrough();

                            // call arbitrator
                            onFulfilledSpy = jasmine.createSpy("onFulfilledSpy");
                            onRejectedSpy = jasmine.createSpy("onRejectedSpy");

                            runs(function() {
                                arbitrator.startArbitration({
                                    domains : [domain],
                                    interfaceName : interfaceName,
                                    discoveryQos : discoveryQos,
                                    proxyVersion : new Version({ majorVersion: 47, minorVersion: 2 })
                                }).then(onFulfilledSpy).catch(onRejectedSpy);
                                increaseFakeTime(1);
                            });

                            waitsFor(function() {
                                return onFulfilledSpy.callCount > 0;
                            }, "until onresolve has been invoked", 300);

                            runs(function() {
                                // arbitrator finally returned the discoveryEntries (filtered only
                                // by providerVersion because of ArbitrationStrategyCollection.Nothing)
                                expect(onRejectedSpy).not.toHaveBeenCalled();
                                expect(onFulfilledSpy).toHaveBeenCalled();
                                var returnedDiscoveryEntries = onFulfilledSpy.calls[0].args[0];
                                expect(returnedDiscoveryEntries).not.toContain(discoveryEntryWithMajor47AndMinor0);
                                expect(returnedDiscoveryEntries).not.toContain(discoveryEntryWithMajor47AndMinor1);
                                expect(returnedDiscoveryEntries).toContain(discoveryEntryWithMajor47AndMinor2);
                                expect(returnedDiscoveryEntries).toContain(discoveryEntryWithMajor47AndMinor3);
                                expect(returnedDiscoveryEntries).not.toContain(discoveryEntryWithMajor48AndMinor2);
                            });
                        });

                        it("rejects with NoCompatibleProviderFoundException including a list of incompatible provider version of latest lookup", function() {
                            var onFulfilledSpy, onRejectedSpy;
                            var expectedMinimumMinorVersion = 2;
                            var firstLookupResult = [
                                discoveryEntryWithMajor47AndMinor0,
                                discoveryEntryWithMajor47AndMinor1,
                                discoveryEntryWithMajor47AndMinor2,
                                discoveryEntryWithMajor47AndMinor3,
                                discoveryEntryWithMajor48AndMinor2
                            ];
                            var secondLookupResult = [
                                discoveryEntryWithMajor47AndMinor0,
                                discoveryEntryWithMajor48AndMinor2
                            ];

                            // return discoveryEntries to check whether these are eventually
                            // returned by the arbitrator
                            capDiscoverySpy.lookup.andReturn(Promise.resolve(firstLookupResult));
                            arbitrator = new Arbitrator(capDiscoverySpy);

                            var discoveryQosWithShortTimers = new DiscoveryQos({
                                discoveryTimeoutMs : 1000,
                                discoveryRetryDelayMs : 600,
                                arbitrationStrategy : ArbitrationStrategyCollection.Nothing,
                                cacheMaxAgeMs : 0,
                                discoveryScope : DiscoveryScope.LOCAL_THEN_GLOBAL,
                                additionalParameters : {}
                            });

                            // spy on and instrument arbitrationStrategy
                            spyOn(discoveryQosWithShortTimers, "arbitrationStrategy").andCallThrough();

                            // call arbitrator
                            onFulfilledSpy = jasmine.createSpy("onFulfilledSpy");
                            onRejectedSpy = jasmine.createSpy("onRejectedSpy");

                            runs(function() {
                                arbitrator.startArbitration({
                                    domains : [domain],
                                    interfaceName : interfaceName,
                                    discoveryQos : discoveryQosWithShortTimers,
                                    proxyVersion : new Version({ majorVersion: 49, minorVersion: expectedMinimumMinorVersion})
                                }).then(onFulfilledSpy).catch(onRejectedSpy);

                                increaseFakeTime(1);
                            });

                            // wait until immediate lookup is finished
                            waitsFor(function() {
                                return discoveryQosWithShortTimers.arbitrationStrategy.callCount === 1;
                            }, "capDiscoverySpy.lookup call", 100);

                            runs(function() {
                                expect(capDiscoverySpy.lookup.callCount).toBe(1);
                            });

                            runs(function() {
                                increaseFakeTime(discoveryQosWithShortTimers.discoveryRetryDelayMs - 2);
                            });

                            waitsFor(function() {
                                return discoveryQosWithShortTimers.arbitrationStrategy.callCount === 1;
                            }, "discoveryQosWithShortTimers.arbitrationStrategy.callCount call", 1000);

                            runs(function() {
                                capDiscoverySpy.lookup.andReturn(Promise.resolve(secondLookupResult));
                                expect(capDiscoverySpy.lookup.callCount).toBe(1);
                                increaseFakeTime(2);
                            });

                            // wait until 1st retry is finished, which returns different
                            // discovery entries
                            waitsFor(function() {
                                return discoveryQosWithShortTimers.arbitrationStrategy.callCount === 2;
                            }, "capDiscoverySpy.lookup call", 1000);

                            runs(function() {
                                expect(capDiscoverySpy.lookup.callCount).toBe(2);
                                increaseFakeTime(1000);
                            });

                            // wait until arbitration expires, the incompatible versions found
                            // during the 2nd lookup (= 1st retry) should be returned inside
                            // a NoCompatibleProviderFoundException
                            waitsFor(function() {
                                return onRejectedSpy.callCount > 0;
                            }, "until onRejected has been invoked", 5000);

                            runs(function() {
                                expect(onRejectedSpy).toHaveBeenCalled();
                                expect(onFulfilledSpy).not.toHaveBeenCalled();
                                expect(onRejectedSpy.calls[0].args[0] instanceof NoCompatibleProviderFoundException).toBeTruthy();
                                // discoverVersion should contain all not matching entries of only the last(!) lookup
                                var discoveredVersions = onRejectedSpy.calls[0].args[0].discoveredVersions;
                                expect(discoveredVersions).toContain(discoveryEntryWithMajor47AndMinor0.providerVersion);
                                expect(discoveredVersions).not.toContain(discoveryEntryWithMajor47AndMinor1.providerVersion);
                                expect(discoveredVersions).not.toContain(discoveryEntryWithMajor47AndMinor2.providerVersion);
                                expect(discoveredVersions).not.toContain(discoveryEntryWithMajor47AndMinor3.providerVersion);
                                expect(discoveredVersions).toContain(discoveryEntryWithMajor48AndMinor2.providerVersion);
                            });
                        });

                        it(
                                "timeouts after the given discoveryTimeoutMs on empty results",
                                function() {
                                    var onFulfilledSpy, onRejectedSpy;

                                    onFulfilledSpy = jasmine.createSpy("onFulfilledSpy");
                                    onRejectedSpy = jasmine.createSpy("onRejectedSpy");

                                    runs(function() {
                                        arbitrator.startArbitration({
                                            domains : [domain],
                                            interfaceName : interfaceName,
                                            discoveryQos : discoveryQos,
                                            proxyVersion : new Version({ majorVersion: 47, minorVersion: 11})
                                        }).then(onFulfilledSpy).catch(onRejectedSpy);
                                        // let discoveryTimeoutMs - 1 pass
                                        increaseFakeTime(discoveryQos.discoveryTimeoutMs - 1);

                                        expect(onFulfilledSpy).not.toHaveBeenCalled();
                                        expect(onRejectedSpy).not.toHaveBeenCalled();

                                        // let discoveryTimeoutMs pass
                                        increaseFakeTime(1);
                                    });

                                    waitsFor(function() {
                                        return onRejectedSpy.callCount > 0;
                                    }, "until onReject has been invoked", 300);

                                    runs(function() {
                                        expect(onFulfilledSpy).not.toHaveBeenCalled();
                                        expect(onRejectedSpy).toHaveBeenCalled();
                                        expect(onRejectedSpy.calls[0].args[0] instanceof DiscoveryException).toBeTruthy();
                                    });
                                });

                        it(
                                "reruns discovery for empty discovery results according to discoveryTimeoutMs and discoveryRetryDelayMs",
                                function() {
                                    expect(capDiscoverySpy.lookup).not.toHaveBeenCalled();
                                    spyOn(discoveryQos, "arbitrationStrategy").andReturn([]);

                                    runs(function() {
                                        arbitrator.startArbitration({
                                            domains : [domain],
                                            interfaceName : interfaceName,
                                            discoveryQos : discoveryQos,
                                            proxyVersion : new Version({ majorVersion: 47, minorVersion: 11})
                                        });
                                        increaseFakeTime(1);
                                    });

                                    waitsFor(function() {
                                        return discoveryQos.arbitrationStrategy.callCount === 1;
                                    }, "capDiscoverySpy.lookup call", 10);

                                    runs(function() {
                                        expect(capDiscoverySpy.lookup.callCount).toBe(1);
                                    });

                                    var internalCheck =
                                            function(i) {
                                                runs(function() {
                                                    increaseFakeTime(discoveryQos.discoveryRetryDelayMs - 2);
                                                });

                                                waitsFor(
                                                        function() {
                                                            return discoveryQos.arbitrationStrategy.callCount === i;
                                                        },
                                                        "discoveryQos.arbitrationStrategy.callCount call i="
                                                            + i,
                                                        1000);

                                                runs(function() {
                                                    expect(capDiscoverySpy.lookup.callCount)
                                                            .toBe(i);
                                                    increaseFakeTime(2);
                                                });

                                                waitsFor(
                                                        function() {
                                                            return discoveryQos.arbitrationStrategy.callCount === (i + 1);
                                                        },
                                                        "discoveryQos.arbitrationStrategy.callCount call i+1="
                                                            + (i + 1),
                                                        1000);

                                                runs(function() {
                                                    expect(capDiscoverySpy.lookup.callCount).toBe(
                                                            i + 1);
                                                });
                                            };

                                    var i;
                                    for (i = 1; i < nrTimes + 1; ++i) {
                                        internalCheck(i);
                                    }

                                });

                        it("uses arbitration strategy and returns its results", function() {
                            var onFulfilledSpy, onRejectedSpy, fakeDiscoveredCaps = [
                                {},
                                {},
                                {},
                                {},
                                {}
                            ];

                            // just return some object so that arbitration is successful and
                            // arbitration strategy is called
                            capDiscoverySpy.lookup.andReturn(Promise.resolve(fakeDiscoveredCaps));
                            arbitrator = new Arbitrator(capDiscoverySpy);

                            // spy on and instrument arbitrationStrategy to return discoveryEntries
                            spyOn(discoveryQos, "arbitrationStrategy").andReturn(discoveryEntries);
                            onFulfilledSpy = jasmine.createSpy("onFulfilledSpy");

                            runs(function() {
                                // call arbitrator
                                arbitrator.startArbitration({
                                    domain : [domain],
                                    interfaceName : interfaceName,
                                    discoveryQos : discoveryQos,
                                    proxyVersion : new Version({ majorVersion: 47, minorVersion: 11})
                                }).then(onFulfilledSpy);
                                // increaseFakeTime: is required for test purpose to ensure the
                                // resolve/reject callbacks are called
                                increaseFakeTime(1);
                            });

                            waitsFor(function() {
                                return onFulfilledSpy.callCount > 0;
                            }, "until onresolve has been invoked", 300);

                            runs(function() {
                                // the arbitrationStrategy was called with the fakeDiscoveredCaps
                                // returned by the discovery spy
                                expect(discoveryQos.arbitrationStrategy).toHaveBeenCalled();
                                expect(discoveryQos.arbitrationStrategy).toHaveBeenCalledWith(
                                        fakeDiscoveredCaps);

                                // arbitrator returns discoveryEntries that were returned by the
                                // arbitrationStrategy spy
                                expect(onFulfilledSpy).toHaveBeenCalled();
                                expect(onFulfilledSpy).toHaveBeenCalledWith(discoveryEntries);
                            });
                        });

                        it("is instantiable, of correct type and has all members", function() {
                            expect(Arbitrator).toBeDefined();
                            expect(typeof Arbitrator === "function").toBeTruthy();
                            expect(arbitrator).toBeDefined();
                            expect(arbitrator instanceof Arbitrator).toBeTruthy();
                            expect(arbitrator.startArbitration).toBeDefined();
                            expect(typeof arbitrator.startArbitration === "function").toBeTruthy();
                        });

                        function arbitratesCorrectly(settings) {
                            runs(function() {
                                staticArbitrationSpy.resolve.reset();
                                staticArbitrationSpy.reject.reset();
                                staticArbitrationSettings.domains = settings.domains;
                                staticArbitrationSettings.interfaceName = settings.interfaceName;
                                arbitrator.startArbitration(staticArbitrationSettings).then(
                                        staticArbitrationSpy.resolve).catch(staticArbitrationSpy.reject);
                                // resolve/reject callbacks are called
                                increaseFakeTime(1);
                            });

                            waitsFor(function() {
                                return staticArbitrationSpy.resolve.callCount > 0;
                            }, "successfull call of startArbitration", 300);

                            runs(function() {
                                expect(staticArbitrationSpy.resolve).toHaveBeenCalledWith(settings.expected);
                                expect(staticArbitrationSpy.reject).not.toHaveBeenCalled();
                            });
                        }

                        it("arbitrates correctly", function() {
                            spyOn(discoveryQos, "arbitrationStrategy").andCallFake(
                                    function(discoveredCaps) {
                                        return discoveredCaps;
                                    });

                            arbitratesCorrectly({
                                domains: ["myDomain"],
                                interfaceName: "noneExistingInterface",
                                expected: []
                            });
                            arbitratesCorrectly({
                                domains: ["noneExistingDomain"],
                                interfaceName: "myInterface",
                                expected: []
                            });
                            arbitratesCorrectly({
                                domains: ["myDomain"],
                                interfaceName: "myInterface",
                                expected: [capabilities[0],
                                           capabilities[1]]
                            });
                            arbitratesCorrectly({
                                domains: ["otherDomain"],
                                interfaceName: "otherInterface",
                                expected: [capabilities[2]]
                            });
                            arbitratesCorrectly({
                                domains: ["thirdDomain"],
                                interfaceName: "otherInterface",
                                expected: [capabilities[3]]
                            });
                        });

                        it("uses the provided arbitrationStrategy", function() {
                            spyOn(discoveryQos, "arbitrationStrategy").andReturn(discoveryEntries);

                            runs(function() {
                                arbitrator.startArbitration(staticArbitrationSettings).then(
                                        staticArbitrationSpy.resolve).catch(staticArbitrationSpy.reject);
                                // resolve/reject callbacks are called
                                increaseFakeTime(1);
                            });

                            waitsFor(function() {
                                return staticArbitrationSpy.resolve.callCount > 0;
                            }, "until arbitration has been performed successfully", 100);

                            runs(function() {
                                expect(discoveryQos.arbitrationStrategy).toHaveBeenCalledWith([
                                    capabilities[0],
                                    capabilities[1]
                                ]);
                                expect(staticArbitrationSpy.resolve).toHaveBeenCalledWith(
                                        discoveryEntries);
                                expect(staticArbitrationSpy.reject).not.toHaveBeenCalled();
                            });
                        });

                        it(
                                "fails if arbitrationStrategy throws an exception",
                                function() {
                                    var rejectCalled = false;
                                    spyOn(discoveryQos, "arbitrationStrategy").andCallFake(
                                            function(discoveredCaps) {
                                                throw new Error("myError");
                                            });

                                    expect(function() {
                                            arbitrator.startArbitration(staticArbitrationSettings).then(function() { return null; }).catch(function() { return null; });
                                    }).not.toThrow();

                                    runs(function() {
                                        arbitrator.startArbitration(staticArbitrationSettings)
                                                .then(staticArbitrationSpy.resolve).catch(
                                                        function(error) {
                                                            rejectCalled = true;
                                                            staticArbitrationSpy.reject(error);
                                                        });
                                        // increaseFakeTime: is required for test purpose to ensure the
                                        // resolve/reject callbacks are called
                                        increaseFakeTime(1);
                                    });

                                    waitsFor(function() {
                                        return rejectCalled;
                                    }, "until onReject has been invoked", 300);

                                    runs(function() {
                                        expect(staticArbitrationSpy.resolve).not.toHaveBeenCalled();
                                        expect(
                                                Object.prototype.toString
                                                        .call(staticArbitrationSpy.reject.mostRecentCall.args[0]) === "[object Error]")
                                                .toBeTruthy();
                                    });
                                });
                    });

        }); // require
