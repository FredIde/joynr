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
#include "joynr/ThreadPoolDelayedScheduler.h"

#include <cassert>
#include <functional>

#include <boost/asio/io_service.hpp>

namespace joynr
{

ThreadPoolDelayedScheduler::ThreadPoolDelayedScheduler(std::uint8_t numberOfThreads,
                                                       const std::string& name,
                                                       boost::asio::io_service& ioService,
                                                       std::chrono::milliseconds defaultDelayMs)
        : DelayedScheduler(std::bind(&ThreadPool::execute, &threadPool, std::placeholders::_1),
                           ioService,
                           defaultDelayMs),
          threadPool(name, numberOfThreads)
{
}

ThreadPoolDelayedScheduler::~ThreadPoolDelayedScheduler()
{
    assert(!threadPool.isRunning());
}

void ThreadPoolDelayedScheduler::shutdown()
{
    DelayedScheduler::shutdown();
    threadPool.shutdown();
}

} // namespace joynr
