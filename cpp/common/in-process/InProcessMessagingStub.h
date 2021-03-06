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
#ifndef INPROCESSMESSAGINGSTUB_H
#define INPROCESSMESSAGINGSTUB_H
#include "joynr/PrivateCopyAssign.h"

#include "joynr/IMessaging.h"
#include "joynr/JoynrCommonExport.h"

#include <memory>

namespace joynr
{

class JoynrMessage;
class InProcessMessagingSkeleton;

class JOYNRCOMMON_EXPORT InProcessMessagingStub : public IMessaging
{
public:
    explicit InProcessMessagingStub(std::shared_ptr<InProcessMessagingSkeleton> skeleton);
    ~InProcessMessagingStub() override = default;
    void transmit(JoynrMessage& message,
                  const std::function<void(const exceptions::JoynrRuntimeException&)>& onFailure)
            override;

private:
    DISALLOW_COPY_AND_ASSIGN(InProcessMessagingStub);
    std::shared_ptr<InProcessMessagingSkeleton> skeleton;
};

} // namespace joynr
#endif // INPROCESSMESSAGINGSTUB_H
