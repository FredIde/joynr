package io.joynr.runtime;

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

import io.joynr.messaging.routing.MessageRouter;
import io.joynr.messaging.routing.MessagingStubFactory;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import com.fasterxml.jackson.databind.ObjectMapper;
import com.google.inject.Inject;
import com.google.inject.name.Named;
import io.joynr.capabilities.CapabilitiesRegistrar;
import io.joynr.capabilities.LocalCapabilitiesDirectory;
import io.joynr.discovery.LocalDiscoveryAggregator;
import io.joynr.dispatching.Dispatcher;
import io.joynr.dispatching.RequestCallerDirectory;
import io.joynr.dispatching.rpc.ReplyCallerDirectory;
import io.joynr.messaging.ConfigurableMessagingSettings;
import io.joynr.messaging.MessagingSkeletonFactory;
import io.joynr.proxy.ProxyBuilderFactory;
import joynr.system.RoutingTypes.Address;
import joynr.types.ProviderQos;

public class ClusterControllerRuntime extends JoynrRuntimeImpl {

    public static final Logger logger = LoggerFactory.getLogger(ClusterControllerRuntime.class);

    // CHECKSTYLE:OFF
    @Inject
    public ClusterControllerRuntime(ObjectMapper objectMapper,
                                    ProxyBuilderFactory proxyBuilderFactory,
                                    RequestCallerDirectory requestCallerDirectory,
                                    ReplyCallerDirectory replyCallerDirectory,
                                    Dispatcher dispatcher,
                                    MessagingStubFactory messagingStubFactory,
                                    MessagingSkeletonFactory messagingSkeletonFactory,
                                    LocalDiscoveryAggregator localDiscoveryAggregator,
                                    @Named(SystemServicesSettings.PROPERTY_SYSTEM_SERVICES_DOMAIN) String systemServicesDomain,
                                    @Named(SystemServicesSettings.PROPERTY_DISPATCHER_ADDRESS) Address dispatcherAddress,
                                    @Named(ConfigurableMessagingSettings.PROPERTY_CAPABILITIES_DIRECTORY_ADDRESS) Address capabilitiesDirectoryAddress,
                                    @Named(ConfigurableMessagingSettings.PROPERTY_CHANNEL_URL_DIRECTORY_ADDRESS) Address channelUrlDirectoryAddress,
                                    @Named(ConfigurableMessagingSettings.PROPERTY_DOMAIN_ACCESS_CONTROLLER_ADDRESS) Address domainAccessControllerAddress,
                                    @Named(SystemServicesSettings.PROPERTY_CC_MESSAGING_ADDRESS) Address discoveryProviderAddress,
                                    CapabilitiesRegistrar capabilitiesRegistrar,
                                    LocalCapabilitiesDirectory localCapabilitiesDirectory,
                                    MessageRouter messageRouter) {
        super(objectMapper,
              proxyBuilderFactory,
              requestCallerDirectory,
              replyCallerDirectory,
              dispatcher,
              messagingStubFactory,
              messagingSkeletonFactory,
              localDiscoveryAggregator,
              systemServicesDomain,
              dispatcherAddress,
              capabilitiesDirectoryAddress,
              channelUrlDirectoryAddress,
              domainAccessControllerAddress,
              discoveryProviderAddress);
        // CHECKSTYLE:ON

        ProviderQos providerQos = new ProviderQos();
        capabilitiesRegistrar.registerProvider(systemServicesDomain, localCapabilitiesDirectory, providerQos);
        capabilitiesRegistrar.registerProvider(systemServicesDomain, messageRouter, providerQos);
    }

    @Override
    public void shutdown(boolean clear) {
        super.shutdown(clear);
    }
}
