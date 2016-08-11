/*
 * #%L
 * %%
 * Copyright (C) 2016 BMW Car IT GmbH
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

#include "ShortCircuitTest.h"
#include <boost/program_options.hpp>
#ifdef JOYNR_ENABLE_DLT_LOGGING
#include <dlt/dlt.h>
#endif // JOYNR_ENABLE_DLT_LOGGING

int main(int argc, char* argv[])
{
#ifdef JOYNR_ENABLE_DLT_LOGGING
    // Register app at the dlt-daemon for logging
    DLT_REGISTER_APP("JOYT", argv[0]);
#endif // JOYNR_ENABLE_DLT_LOGGING

    namespace po = boost::program_options;

    std::size_t runs;

    auto validateRuns = [](std::size_t value) {
        if (value == 0) {
            throw po::validation_error(
                    po::validation_error::invalid_option_value, "runs", std::to_string(value));
        }
    };

    po::options_description desc("Available options");
    desc.add_options()("help,h", "produce help message")(
            "runs,r", po::value(&runs)->required()->notifier(validateRuns), "number of runs");

    try {
        po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);
        if (vm.count("help")) {
            std::cout << desc << std::endl;
            return EXIT_FAILURE;
        }

        ShortCircuitTest test(runs);

        test.roundTripByteArray(10000);
        test.roundTripByteArray(100000);
        test.roundTripString(36);
        test.roundTripStruct(29);

    } catch (const std::exception& e) {
        std::cerr << e.what();
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
