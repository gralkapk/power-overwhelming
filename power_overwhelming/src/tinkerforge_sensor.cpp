// <copyright file="tinkerforge_sensor.cpp" company="Visualisierungsinstitut der Universit�t Stuttgart">
// Copyright � 2021 Visualisierungsinstitut der Universit�t Stuttgart. Alle Rechte vorbehalten.
// </copyright>
// <author>Christoph M�ller</author>

#include "power_overwhelming/tinkerforge_sensor.h"

#include <cassert>
#include <chrono>
#include <thread>
#include <vector>

#include <bricklet_voltage_current_v2.h>

#include "timestamp.h"
#include "tinkerforge_exception.h"
#include "tinkerforge_sensor_impl.h"
#include "zero_memory.h"


/*
 * visus::power_overwhelming::tinkerforge_sensor::for_all
 */
std::size_t visus::power_overwhelming::tinkerforge_sensor::for_all(
        tinkerforge_sensor *out_sensors, const std::size_t timeout,
        const std::size_t cnt_sensors, const char *host,
        const std::uint16_t port) {
    if ((out_sensors == nullptr) || (cnt_sensors == 0)) {
        return tinkerforge_sensor::get_definitions(nullptr, 0, timeout, host,
            port);

    } else {
        std::vector<tinkerforge_sensor_definiton> descs(cnt_sensors);
        auto retval = tinkerforge_sensor::get_definitions(descs.data(),
            descs.size(), timeout, host, port);

        // Handle potential attachment of sensors between measurment call and
        // actual instantiation call.
        if (retval < cnt_sensors) {
            retval = cnt_sensors;
        }

        for (std::size_t i = 0; i < retval; ++i) {
            out_sensors[i] = tinkerforge_sensor(descs[i]);
        }

        return retval;
    }
}


/*
 * visus::power_overwhelming::tinkerforge_sensor::get_definitions
 */
std::size_t visus::power_overwhelming::tinkerforge_sensor::get_definitions(
        tinkerforge_sensor_definiton *out_definitions,
        const std::size_t cnt_definitions, const std::size_t timeout,
        const char *host, const std::uint16_t port) {
    std::vector<detail::tinkerforge_bricklet> bricklets;
    detail::tinkerforge_scope scope(host, port);

    auto retval = scope.copy_bricklets(std::back_inserter(bricklets),
        [](const detail::tinkerforge_bricklet& b) {
            return (b.device_type() == VOLTAGE_CURRENT_V2_DEVICE_IDENTIFIER);
        }, std::chrono::milliseconds(timeout), cnt_definitions);

    if (out_definitions != nullptr) {
        for (std::size_t i = 0; (i < cnt_definitions) && (i < retval); ++i) {
            out_definitions[i] = tinkerforge_sensor_definiton(
                bricklets[i].uid().c_str());
        }
    }

    return retval;
}


/*
 * visus::power_overwhelming::tinkerforge_sensor::tinkerforge_sensor
 */
visus::power_overwhelming::tinkerforge_sensor::tinkerforge_sensor(
        const char *uid, const char *host, const std::uint16_t port)
        : _impl(nullptr) {
    // The implementation will (i) obtain and manage the scope with the
    // connection to the master brick, (ii) allocate the voltage/current
    // bricklet and manage its life time.
    this->_impl = new detail::tinkerforge_sensor_impl(
        (host != nullptr) ? host : default_host,
        port,
        uid);
}


/*
 * visus::power_overwhelming::tinkerforge_sensor::tinkerforge_sensor
 */
visus::power_overwhelming::tinkerforge_sensor::tinkerforge_sensor(
        const char *uid, const wchar_t *description, const char *host,
        const std::uint16_t port) : _impl(nullptr) {
    this->_impl = new detail::tinkerforge_sensor_impl(
        (host != nullptr) ? host : default_host,
        port,
        uid);

    if (description != nullptr) {
        this->_impl->description = description;
    }
}


/*
 * visus::power_overwhelming::tinkerforge_sensor::tinkerforge_sensor
 */
visus::power_overwhelming::tinkerforge_sensor::tinkerforge_sensor(
        const tinkerforge_sensor_definiton& definition,
        const char *host, const std::uint16_t port) : _impl(nullptr) {
    this->_impl = new detail::tinkerforge_sensor_impl(
        (host != nullptr) ? host : default_host,
        port,
        definition.uid());

    if (definition.description() != nullptr) {
        this->_impl->description = definition.description();
    }
}


/*
 * visus::power_overwhelming::tinkerforge_sensor::~tinkerforge_sensor
 */
visus::power_overwhelming::tinkerforge_sensor::~tinkerforge_sensor(
        void) {
    delete this->_impl;
}


/*
 * visus::power_overwhelming::tinkerforge_sensor::configuration
 */
void visus::power_overwhelming::tinkerforge_sensor::configuration(
        sample_averaging& averaging, conversion_time& voltage_conversion_time,
        conversion_time& current_conversion_time) {
    typedef std::underlying_type<conversion_time>::type native_adc_type;
    typedef std::underlying_type<conversion_time>::type native_avg_type;

    if (!*this) {
        throw std::runtime_error("The configuration of a disposed "
            "tinkerforge_sensor cannot be retrieved.");
    }

    auto status = ::voltage_current_v2_get_configuration(
        &this->_impl->bricklet,
        reinterpret_cast<native_avg_type *>(&averaging),
        reinterpret_cast<native_adc_type *>(&voltage_conversion_time),
        reinterpret_cast<native_adc_type *>(&current_conversion_time));
    if (status < 0) {
        throw tinkerforge_exception(status);
    }
}


/*
 * visus::power_overwhelming::tinkerforge_sensor::configure
 */
void visus::power_overwhelming::tinkerforge_sensor::configure(
        const sample_averaging averaging,
        const conversion_time voltage_conversion_time,
        const conversion_time current_conversion_time) {
    typedef std::underlying_type<conversion_time>::type native_adc_type;
    typedef std::underlying_type<conversion_time>::type native_avg_type;

    if (!*this) {
        throw std::runtime_error("A disposed instance of tinkerforge_sensor "
            "cannot be configured.");
    }

    auto status = ::voltage_current_v2_set_configuration(
        &this->_impl->bricklet,
        static_cast<native_avg_type>(averaging),
        static_cast<native_adc_type>(voltage_conversion_time),
        static_cast<native_adc_type>(current_conversion_time));
    if (status < 0) {
        throw tinkerforge_exception(status);
    }
}


/*
 * visus::power_overwhelming::tinkerforge_sensor::description
 */
const wchar_t *visus::power_overwhelming::tinkerforge_sensor::description(
        void) const noexcept {
    if (this->_impl != nullptr) {
        return this->_impl->description.c_str();
    } else {
        return nullptr;
    }
}


/*
 * visus::power_overwhelming::tinkerforge_sensor::identify
 */
void visus::power_overwhelming::tinkerforge_sensor::identify(char uid[8],
        char connected_to_uid[8], char& position,
        std::uint8_t hardware_version[3], std::uint8_t firmware_version[3],
        std::uint16_t& device_id) const {
    if (!*this) {
        throw std::runtime_error("A disposed instance of tinkerforge_sensor "
            "cannot be identified.");
    }

    auto status = ::voltage_current_v2_get_identity(&this->_impl->bricklet, uid,
        connected_to_uid, &position, hardware_version, firmware_version,
        &device_id);
    if (status < 0) {
        throw tinkerforge_exception(status);
    }
}


/*
 * visus::power_overwhelming::tinkerforge_sensor::identify
 */
void visus::power_overwhelming::tinkerforge_sensor::identify(
        char uid[8]) const {
    char dummy0[8];
    char dummy1;
    std::uint8_t dummy2[3];
    std::uint8_t dummy3[3];
    std::uint16_t dummy4;
    this->identify(uid, dummy0, dummy1, dummy2, dummy3, dummy4);
}


/*
 * visus::power_overwhelming::tinkerforge_sensor::name
 */
const wchar_t *visus::power_overwhelming::tinkerforge_sensor::name(
        void) const noexcept {
    if (this->_impl != nullptr) {
        return this->_impl->sensor_name.c_str();
    } else {
        return nullptr;
    }
}


/*
 * visus::power_overwhelming::tinkerforge_sensor::reset
 */
void visus::power_overwhelming::tinkerforge_sensor::reset(void) {
    if (!*this) {
        throw std::runtime_error("A disposed instance of tinkerforge_sensor "
            "cannot be reset.");
    }

    auto status = ::voltage_current_v2_reset(&this->_impl->bricklet);
    if (status < 0) {
        throw tinkerforge_exception(status);
    }
}


/*
 * visus::power_overwhelming::tinkerforge_sensor::sample
 */
visus::power_overwhelming::measurement
visus::power_overwhelming::tinkerforge_sensor::sample(
        const timestamp_resolution resolution) const {
    if (!*this) {
        throw std::runtime_error("A disposed instance of tinkerforge_sensor "
            "cannot be sampled.");
    }

    static const auto thousand = static_cast<measurement::value_type>(1000);
    std::int32_t current = 0;   // Current in mA.
    std::int32_t power = 0;     // Power in mW.
    std::int32_t voltage = 0;   // Voltage in mV.

    {
        auto status = ::voltage_current_v2_get_voltage(&this->_impl->bricklet,
            &voltage);
        if (status < 0) {
            throw tinkerforge_exception(status);
        }
    }

    {
        auto status = ::voltage_current_v2_get_current(&this->_impl->bricklet,
            &voltage);
        if (status < 0) {
            throw tinkerforge_exception(status);
        }
    }

    {
        auto status = ::voltage_current_v2_get_power(&this->_impl->bricklet,
            &power);
        if (status < 0) {
            throw tinkerforge_exception(status);
        }
    }

    return measurement(this->_impl->sensor_name.c_str(),
        create_timestamp(resolution),
        static_cast<measurement::value_type>(voltage) / thousand,
        static_cast<measurement::value_type>(current) / thousand,
        static_cast<measurement::value_type>(power) / thousand);
}


/*
 * visus::power_overwhelming::tinkerforge_sensor::sample
 */
void visus::power_overwhelming::tinkerforge_sensor::sample(
        const measurement_callback on_measurement,
        const tinkerforge_sensor_source source,
        const microseconds_type sampling_period,
        void *context) {
    static constexpr auto one = static_cast<microseconds_type>(1);
    static constexpr auto thousand = static_cast<microseconds_type>(1000);

    if (!*this) {
        throw std::runtime_error("A disposed instance of tinkerforge_sensor "
            "cannot be sampled.");
    }

    if (on_measurement != nullptr) {
        // Callback is non-null, so user wants to enable asynchronous sampling.
        measurement_callback expected = nullptr;

        if (!this->_impl->on_measurement.compare_exchange_strong(expected,
                on_measurement)) {
            throw std::logic_error("Asynchronous sampling cannot be started "
                "while it is already running.");
        }

        try {
            auto millis = static_cast<std::int32_t>((std::max)(one,
                sampling_period / thousand));

            this->_impl->on_measurement_context = context;

            if (source == tinkerforge_sensor_source::all) {
                // Enable all sensor readings.
                this->_impl->enable_callbacks(millis);

            } else {
                // Enable individual sensor readings.
                if ((source & tinkerforge_sensor_source::current)
                        == tinkerforge_sensor_source::current) {
                    this->_impl->enable_current_callback(millis);
                }
                if ((source & tinkerforge_sensor_source::power)
                    == tinkerforge_sensor_source::power) {
                    this->_impl->enable_power_callback(millis);
                }
                if ((source & tinkerforge_sensor_source::voltage)
                    == tinkerforge_sensor_source::voltage) {
                    this->_impl->enable_voltage_callback(millis);
                }
            }
        } catch (...) {
            // Clear the guard in case the operation failed.
            this->_impl->on_measurement.exchange(nullptr);
            throw;
        }

    } else {
        // If the callback is null, disable asynchronous sampling.
        if (this->_impl->on_measurement != nullptr) {
            this->_impl->disable_callbacks();
        }

        this->_impl->on_measurement.exchange(on_measurement);
    }
}


/*
 * visus::power_overwhelming::tinkerforge_sensor::operator =
 */
visus::power_overwhelming::tinkerforge_sensor&
visus::power_overwhelming::tinkerforge_sensor::operator =(
        tinkerforge_sensor&& rhs) noexcept {
    if (this != std::addressof(rhs)) {
        this->_impl = rhs._impl;
        rhs._impl = nullptr;
    }

    return *this;
}


/*
 * visus::power_overwhelming::tinkerforge_sensor::operator bool
 */
visus::power_overwhelming::tinkerforge_sensor::operator bool(
        void) const noexcept {
    return (this->_impl != nullptr);
}
