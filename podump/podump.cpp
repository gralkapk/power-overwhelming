// <copyright file="podump.cpp" company="Visualisierungsinstitut der Universit�t Stuttgart">
// Copyright � 2021 Visualisierungsinstitut der Universit�t Stuttgart. Alle Rechte vorbehalten.
// </copyright>
// <author>Christoph M�ller</author>

#include <iostream>
#include <thread>
#include <vector>

#include <adl_sensor.h>
#include <hmc8015_sensor.h>
#include <measurement.h>
#include <nvml_sensor.h>
#include <tinkerforge_display.h>
#include <tinkerforge_sensor.h>
#include <tchar.h>


/// <summary>
/// Entry point of the podump application, which dumps all available sensor data
/// once to the console.
/// </summary>
/// <param name="argc"></param>
/// <param name="argv"></param>
/// <returns></returns>
int _tmain(const int argc, const TCHAR **argv) {
    using namespace visus::power_overwhelming;

    // Print data for all supported AMD cards.
    try {
        std::vector<adl_sensor> sensors;
        sensors.resize(adl_sensor::for_all(nullptr, 0));
        adl_sensor::for_all(sensors.data(), sensors.size());

        for (auto& s : sensors) {
            std::wcout << s.name() << L":" << std::endl;
            auto m = s.sample();
            std::wcout << m.timestamp() << L" (" << m.sensor() << L"): "
                << m.power() << L" W" << std::endl;
        }
    } catch (std::exception &ex) {
        std::cerr << ex.what() << std::endl;
    }

    // Print data for all supported NVIDIA cards.
    try {
        std::vector<nvml_sensor> sensors;
        sensors.resize(nvml_sensor::for_all(nullptr, 0));
        nvml_sensor::for_all(sensors.data(), sensors.size());

        for (auto& s : sensors) {
            std::wcout << s.name() << L":" << std::endl;
            auto m = s.sample();
            std::wcout << m.timestamp() << L": " << m.power() << L" W"
                << std::endl;
        }
    } catch (std::exception &ex) {
        std::cerr << ex.what() << std::endl;
    }

    // Print some useful messsage to all Tinkerforge LCDs attached.
    try {
        std::vector<tinkerforge_display> displays;
        displays.resize(tinkerforge_display::for_all(nullptr, 0));
        auto cnt = tinkerforge_display::for_all(displays.data(),
            displays.size());

        if (cnt < displays.size()) {
            displays.resize(cnt);
        }

        for (auto& d : displays) {
            d.clear();
            d.print("Power overwhelming!");
        }

    } catch (std::exception& ex) {
        std::cerr << ex.what() << std::endl;
    }

    // Print data for all connected Tinkerforge sensors.
    try {
        std::vector<tinkerforge_sensor_definiton> descs;
        descs.resize(tinkerforge_sensor::get_definitions(nullptr, 0));
        auto cnt = tinkerforge_sensor::get_definitions(descs.data(),
            descs.size());

        if (cnt < descs.size()) {
            descs.resize(cnt);
        }

        for (auto &d : descs) {
            tinkerforge_sensor s(d);
            std::wcout << s.name() << L":" << std::endl;
            auto m = s.sample();
            std::wcout << m.timestamp() << L": " << m.voltage() << " V * "
                << m.current() << " A = " << m.power() << L" W"
                << std::endl;
        }

    } catch (std::exception &ex) {
        std::cerr << ex.what() << std::endl;
    }

    // Asynchronously sample the Tinkerforge sensors for five seconds.
    try {
        std::vector<tinkerforge_sensor_definiton> descs;
        std::vector<tinkerforge_sensor> sensors;
        descs.resize(tinkerforge_sensor::get_definitions(nullptr, 0));
        auto cnt = tinkerforge_sensor::get_definitions(descs.data(),
            descs.size());

        if (cnt < descs.size()) {
            descs.resize(cnt);
        }

        for (auto& d : descs) {
            sensors.emplace_back(d);
            sensors.back().sample([](const measurement& m) {
                std::wcout << m.sensor() << L":" << m.timestamp() << L": "
                    << m.voltage() << " V * " << m.current() << " A = "
                    << m.power() << L" W"
                    << std::endl;
                });
        }

        std::this_thread::sleep_for(std::chrono::seconds(5));

        for (auto& s : sensors) {
            s.sample(nullptr);
        }

    } catch (std::exception &ex) {
        std::cerr << ex.what() << std::endl;
    }

    // Query HMC8015
    try {
        std::vector<hmc8015_sensor> sensors;
        sensors.resize(hmc8015_sensor::for_all(nullptr, 0));
        hmc8015_sensor::for_all(sensors.data(), sensors.size());

        for (auto &s : sensors) {
            std::wcout << s.name() << L":" << std::endl;
            //auto m = s.sample();
            //std::wcout << m.timestamp() << L": " << m.power() << L" W"
            //    << std::endl;
        }

    } catch (std::exception &ex) {
        std::cerr << ex.what() << std::endl;
    }

     return 0;
}
