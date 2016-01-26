/*
 * #%L
 * %%
 * Copyright (C) 2011 - 2016 BMW Car IT GmbH
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
#include "HttpMessagingSkeleton.h"

#include "joynr/JsonSerializer.h"
#include "joynr/MessageRouter.h"
#include "joynr/system/RoutingTypes/MqttAddress.h"

namespace joynr
{

INIT_LOGGER(HttpMessagingSkeleton);

HttpMessagingSkeleton::HttpMessagingSkeleton(MessageRouter& messageRouter)
        : messageRouter(messageRouter)
{
}

void HttpMessagingSkeleton::transmit(JoynrMessage& message)
{
    if (message.getType() == JoynrMessage::VALUE_MESSAGE_TYPE_REQUEST ||
        message.getType() == JoynrMessage::VALUE_MESSAGE_TYPE_SUBSCRIPTION_REQUEST ||
        message.getType() == JoynrMessage::VALUE_MESSAGE_TYPE_BROADCAST_SUBSCRIPTION_REQUEST) {
        // TODO ca: check if replyTo header info is available?
        std::string replyChannelId = message.getHeaderReplyChannelId();
        std::shared_ptr<system::RoutingTypes::ChannelAddress> address(
                new system::RoutingTypes::ChannelAddress(replyChannelId));
        messageRouter.addNextHop(message.getHeaderFrom(), address);
    }

    messageRouter.route(message);
}

void HttpMessagingSkeleton::onTextMessageReceived(const std::string& message)
{
    JoynrMessage* joynrMsg = JsonSerializer::deserialize<JoynrMessage>(message);
    if (joynrMsg == nullptr) {
        JOYNR_LOG_ERROR(logger, "Unable to deserialize message. Raw message: {}", message);
        return;
    }
    if (joynrMsg->getType().empty()) {
        JOYNR_LOG_ERROR(logger, "received empty message - dropping Messages");
        return;
    }
    if (!joynrMsg->containsHeaderExpiryDate()) {
        JOYNR_LOG_ERROR(logger,
                        "received message [msgId=[{}] without decay time - dropping message",
                        joynrMsg->getHeaderMessageId());
    }

    JOYNR_LOG_TRACE(logger, "<<< INCOMING <<< {}", message);
    // message router copies joynr message when scheduling thread that handles
    // message delivery
    transmit(*joynrMsg);
    delete joynrMsg;
}

} // namespace joynr