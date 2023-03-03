// <copyright file="emi_sensor.cpp" company="Visualisierungsinstitut der Universit�t Stuttgart">
// Copyright � 2023 Visualisierungsinstitut der Universit�t Stuttgart. Alle Rechte vorbehalten.
// </copyright>
// <author>Christoph M�ller</author>

#include "pch.h"
#include "emi_sensor.h"

//#include "../power_overwhelming/src/timestamp.h"


/*
 * ::sample_emi_sensor
 */
void sample_emi_sensor(void) {
    using namespace visus::power_overwhelming;

#if defined(_WIN32)
    try {
        std::vector<emi_sensor> sensors;
        sensors.resize(emi_sensor::for_all(nullptr, 0));
        emi_sensor::for_all(sensors.data(), sensors.size());

        for (auto& s : sensors) {
            //FILETIME ft;
            //::GetSystemTimePreciseAsFileTime(&ft);
            //std::cout << ">>>" << detail::convert(ft, timestamp_resolution::milliseconds) << std::endl;
            std::wcout << s.name() << L":" << std::endl;
            auto m = s.sample();
            std::wcout << m.timestamp() << L" (" << m.sensor() << L"): "
                << m.power() << L" W" << std::endl;
        }
    } catch (std::exception& ex) {
        std::cerr << ex.what() << std::endl;
    }
#endif /* defined(_WIN32) */
}


/*
 * ::sample_emi_sensor_async
 */
void sample_emi_sensor_async(const unsigned int dt) {
    using namespace visus::power_overwhelming;

#if defined(_WIN32)
    try {
        std::vector<emi_sensor> sensors;
        sensors.resize(emi_sensor::for_all(nullptr, 0));
        emi_sensor::for_all(sensors.data(), sensors.size());

        // Enable asynchronous sampling.
        for (auto &s : sensors) {
            s.sample([](const measurement &m, void *) {
                std::wcout << m.timestamp() << L" "
                    << "(" << m.sensor() << "): "
                    << m.power() << L" W"
                    << std::endl;
            });
        }

        // Wait for the requested number of seconds.
        std::this_thread::sleep_for(std::chrono::seconds(dt));

        // Disable asynchronous sampling.
        for (auto &s : sensors) {
            s.sample(nullptr);
        }

    } catch (std::exception& ex) {
        std::cerr << ex.what() << std::endl;
    }

#endif /* defined(_WIN32) */
}
