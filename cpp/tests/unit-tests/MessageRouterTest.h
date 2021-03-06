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

#include <cstdint>
#include <chrono>
#include <memory>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "JoynrTest.h"

#include "joynr/CcMessageRouter.h"
#include "joynr/ClusterControllerSettings.h"
#include "joynr/LibJoynrMessageRouter.h"

#include "joynr/Semaphore.h"
#include "tests/utils/MockObjects.h"
#include "joynr/system/RoutingTypes/MqttAddress.h"
#include "joynr/system/RoutingTypes/WebSocketAddress.h"
#include "joynr/MessagingStubFactory.h"
#include "joynr/MessageQueue.h"
#include "joynr/MulticastMessagingSkeletonDirectory.h"
#include "joynr/MqttMulticastAddressCalculator.h"
#include "joynr/SingleThreadedIOService.h"
#include "joynr/WebSocketMulticastAddressCalculator.h"

using namespace joynr;

template <typename T>
class MessageRouterTest : public ::testing::Test {
public:
    MessageRouterTest() :
        singleThreadedIOService(),
        settings(),
        messagingSettings(settings),
        messageQueue(nullptr),
        messagingStubFactory(nullptr),
        messageRouter(nullptr),
        joynrMessage(),
        multicastMessagingSkeletonDirectory(std::make_shared<MulticastMessagingSkeletonDirectory>()),
        brokerURL("mqtt://globalTransport.example.com"),
        mqttTopic(""),
        localTransport(std::make_shared<const joynr::system::RoutingTypes::WebSocketAddress>(
                         joynr::system::RoutingTypes::WebSocketProtocol::Enum::WS,
                         "host",
                         4242,
                         "path")
                       ),
        globalTransport(std::make_shared<const joynr::system::RoutingTypes::MqttAddress>(brokerURL, mqttTopic))
    {
        singleThreadedIOService.start();

        messagingStubFactory = std::make_shared<MockMessagingStubFactory>();
        messageRouter = createMessageRouter();

        JoynrTimePoint now = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
        joynrMessage.setHeaderExpiryDate(now + std::chrono::milliseconds(100));
        joynrMessage.setType(JoynrMessage::VALUE_MESSAGE_TYPE_ONE_WAY);
    }

    ~MessageRouterTest() override {
        std::remove(settingsFileName.c_str());
    }

protected:
    template<typename U = T,
             typename =  std::enable_if_t<std::is_same<U, LibJoynrMessageRouter>::value>>
    std::unique_ptr<LibJoynrMessageRouter> createMessageRouter()
    {
        auto messageQueueForMessageRouter = std::make_unique<MessageQueue>();
        messageQueue = messageQueueForMessageRouter.get();

        auto libJoynrMessageRouter = std::make_unique<LibJoynrMessageRouter>(
                    nullptr,
                    messagingStubFactory,
                    singleThreadedIOService.getIOService(),
                    std::make_unique<WebSocketMulticastAddressCalculator>(localTransport),
                    6,
                    std::move(messageQueueForMessageRouter)
                );

        return std::move(libJoynrMessageRouter);
    }

    template<typename U = T,
             typename =  std::enable_if_t<std::is_same<U, CcMessageRouter>::value>>
    std::unique_ptr<CcMessageRouter> createMessageRouter()
    {
        const std::string globalCcAddress("globalAddress");
        ClusterControllerSettings ccSettings(settings);
        auto messageQueueForMessageRouter = std::make_unique<MessageQueue>();
        messageQueue = messageQueueForMessageRouter.get();

        return std::make_unique<CcMessageRouter>(
                    messagingStubFactory,
                    multicastMessagingSkeletonDirectory,
                    std::unique_ptr<IPlatformSecurityManager>(),
                    singleThreadedIOService.getIOService(),
                    std::make_unique<MqttMulticastAddressCalculator>(globalTransport, ccSettings.getMqttMulticastTopicPrefix()),
                    globalCcAddress,
                    6,
                    std::move(messageQueueForMessageRouter)
                );
    }

    SingleThreadedIOService singleThreadedIOService;
    std::string settingsFileName;
    Settings settings;
    MessagingSettings messagingSettings;
    MessageQueue* messageQueue;
    std::shared_ptr<MockMessagingStubFactory> messagingStubFactory;

    std::unique_ptr<T> messageRouter;

    JoynrMessage joynrMessage;
    std::shared_ptr<MulticastMessagingSkeletonDirectory> multicastMessagingSkeletonDirectory;
    std::string brokerURL;
    std::string mqttTopic;

    std::shared_ptr<const joynr::system::RoutingTypes::WebSocketAddress> localTransport;
    std::shared_ptr<const joynr::system::RoutingTypes::MqttAddress> globalTransport;

    void routeMessageToAddress(){
        joynr::Semaphore semaphore(0);
        auto mockMessagingStub = std::make_shared<MockMessagingStub>();
        ON_CALL(*messagingStubFactory, create(_)).WillByDefault(Return(mockMessagingStub));
        ON_CALL(*mockMessagingStub, transmit(joynrMessage, A<const std::function<void(const joynr::exceptions::JoynrRuntimeException&)>&>()))
                .WillByDefault(ReleaseSemaphore(&semaphore));
        EXPECT_CALL(*mockMessagingStub, transmit(joynrMessage, A<const std::function<void(const joynr::exceptions::JoynrRuntimeException&)>&>()));
        messageRouter->route(joynrMessage);
        EXPECT_TRUE(semaphore.waitFor(std::chrono::seconds(2)));
    }
};

typedef ::testing::Types<
        LibJoynrMessageRouter,
        CcMessageRouter
    > MessageRouterTypes;

TYPED_TEST_CASE(MessageRouterTest, MessageRouterTypes);
