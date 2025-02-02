﻿// <copyright file="hmc8015_sensor.h" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2021 - 2023 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// </copyright>
// <author>Christoph Müller</author>

#pragma once

#include <climits>
#include <cstddef>
#include <cstdint>
#include <cinttypes>

#include "power_overwhelming/sensor.h"
#include "power_overwhelming/visa_instrument.h"

#include "instrument_range.h"
#include "log_mode.h"


namespace visus {
namespace power_overwhelming {

    /// <summary>
    /// Allows for controlling a Rohde &amp; Schwarz HMC8015 power analyser.
    /// </summary>
    class POWER_OVERWHELMING_API hmc8015_sensor final : public sensor {

    public:

        /// <summary>
        /// Create sensor objects for all Rohde &amp; Schwarz HMC8015
        /// instruments that can be enumerated via VISA.
        /// </summary>
        /// <param name="out_sensors">An array receiving the sensors. If this is
        /// <c>nullptr</c>, nothing is returned.</param>
        /// <param name="cnt_sensors">The number of sensors that can be stored in
        /// <paramref name="out_sensors" />.</param>
        /// <param name="timeout">The timeout in milliseconds for establishing a
        /// connection to each instrument that was found. This parameter defaults
        /// to 3000.</param>
        /// <returns>The number of HMC8015 instruments found, regardless of how
        /// many have been returned to <paramref name="out_sensors" />.</returns>
        static std::size_t for_all(
            _Out_writes_opt_(cnt_sensors) hmc8015_sensor *out_sensors,
            _In_ std::size_t cnt_sensors,
            _In_ const std::int32_t timeout = 3000);

        /// <summary>
        /// The product ID of the HMC8015.
        /// </summary>
        /// <remarks>
        /// This is the ID reported by the device we use at VISUS. If this does
        /// not work, use the RsVisaTester tool and discover the device via the
        /// &quot;Find Resource&quot; functionality.
        /// </remarks>
        static constexpr const char *product_id = "0x0135";

        /// <summary>
        /// Initialises a new instance.
        /// </summary>
        inline hmc8015_sensor(void) : _name(nullptr) { }

        /// <summary>
        /// Initialises a new instance.
        /// </summary>
        /// <param name="path">The path to the sensor</param>
        /// <param name="timeout">The timeout in milliseconds for connecting to
        /// the sensor.</param>
        /// <exception cref="std::invalid_argument">If <paramref name="path" />
        /// is <c>nullptr</c>.</exception>
        /// <exception cref="std::bad_alloc">If the memory for the sensor state
        /// could not be allocated.</exception>
        /// <exception cref="std::system_error">If the VISA library could not be
        /// loaded.</exception>
        /// <exception cref="visa_exception">If the sensor could not be
        /// initialised.</exception>
        hmc8015_sensor(_In_z_ const wchar_t *path,
            _In_ const visa_instrument::timeout_type timeout);

        /// <summary>
        /// Initialises a new instance.
        /// </summary>
        /// <param name="path">The path to the sensor</param>
        /// <param name="timeout">The timeout in milliseconds for connecting to
        /// the sensor.</param>
        /// <exception cref="std::invalid_argument">If <paramref name="path" />
        /// is <c>nullptr</c>.</exception>
        /// <exception cref="std::bad_alloc">If the memory for the sensor state
        /// could not be allocated.</exception>
        /// <exception cref="std::system_error">If the VISA library could not be
        /// loaded.</exception>
        /// <exception cref="visa_exception">If the sensor could not be
        /// initialised.</exception>
        hmc8015_sensor(_In_z_ const char *path,
            _In_ const visa_instrument::timeout_type timeout);

        /// <summary>
        /// Move <paramref name="rhs" /> into a new instance.
        /// </summary>
        /// <param name="rhs">The object to be moved.</param>
        hmc8015_sensor(_Inout_ hmc8015_sensor&& rhs) noexcept;

        /// <summary>
        /// Finalise the instance.
        /// </summary>
        virtual ~hmc8015_sensor(void);

        /// <summary>
        /// Sets the current range.
        /// </summary>
        /// <remarks>
        /// <para>The instrument can be instructed to select a reasonable range
        /// by specifying <see cref="instrument_range::automatically" />.
        /// However, this will prevent a correct integration of measurements if
        /// the range changes during the measurement period. Therefore, it is
        /// reasonable to specify a safe range explictly in such an application
        /// case.</para>
        /// </remarks>
        /// <param name="range">Defines how <paramref name="value" /> is
        /// interpreted.</param>
        /// <param name="value">The current range in Amperes. This parameter
        /// is only used if <paramref name="range" /> is set to
        /// <see cref="instrument_range::explicitly" />. Valid values are
        /// 0.005, 0.01, 0.02, 0.05, 0.1, 0.2, 0.5, 1, 2, 5, 10 an 20.</param>
        /// <returns><c>*this</c>.</returns>
        /// <exception cref="std::runtime_error">If the method is called on an
        /// object that has been disposed by moving it.</exception>
        /// <exception cref="visa_exception">If the VISA command was not
        /// processed successfully.</exception>
        inline hmc8015_sensor& current_range(_In_ const instrument_range range,
                _In_ const float value = 0.0f) {
            // Note: HMC8015 supports only one channel, so this is hard coded.
            this->set_range(1, "CURR", range, value);
            return *this;
        }

        /// <summary>
        /// Displays the given text or clears the display.
        /// </summary>
        /// <param name="text">The text to be displayed or <c>nullptr</c> to
        /// clear the display.</param>
        /// <returns><c>*this</c>.</returns>
        /// <exception cref="std::runtime_error">If the method is called on an
        /// object that has been disposed by moving it.</exception>
        /// <exception cref="visa_exception">If the VISA command was not
        /// processed successfully.</exception>
        hmc8015_sensor& display(_In_opt_z_ const char *text);

        /// <summary>
        /// Displays the given text or clears the display.
        /// </summary>
        /// <param name="text">The text to be displayed or <c>nullptr</c> to
        /// clear the display.</param>
        /// <returns><c>*this</c>.</returns>
        /// <exception cref="std::runtime_error">If the method is called on an
        /// object that has been disposed by moving it.</exception>
        /// <exception cref="visa_exception">If the VISA command was not
        /// processed successfully.</exception>
        hmc8015_sensor& display(_In_opt_z_ const wchar_t *text);

        /// <summary>
        /// Gets whether logging is enabled or not.
        /// </summary>
        /// <returns><c>true</c> if the instrument is currently logging,
        /// <c>false</c> otherwise.</returns>
        /// <exception cref="std::runtime_error">If the method is called on an
        /// object that has been disposed by moving it.</exception>
        /// <exception cref="visa_exception">If the VISA command was not
        /// processed successfully.</exception>
        bool is_log(void);

        /// <summary>
        /// Enables or disables logging.
        /// </summary>
        /// <param name="enable">Start logging if <c>true</c>, otherwise, stop
        /// it.</param>
        /// <returns><c>*this</c>.</returns>
        /// <exception cref="std::runtime_error">If the method is called on an
        /// object that has been disposed by moving it.</exception>
        /// <exception cref="visa_exception">If the VISA command was not
        /// processed successfully.</exception>
        hmc8015_sensor& log(_In_ const bool enable);

        /// <summary>
        /// Configures how logging started by <see cref="log" /> behaves.
        /// </summary>
        /// <param name="interval">The interval between samples in seconds.
        /// Specifying the lowest/maximum value of <c>float</c> is interpreted
        /// as special values &quot;MIN&quot; and &quot;MAX&quot; respectively.
        /// </param>
        /// <param name="mode">The logging mode.</param>
        /// <param name="value">The count (number of samples) or duration (in
        /// seconds), depending on the given <paramref name="mode" />. This
        /// value is ignored for <see cref="log_mode::unlimited" />. Specifying
        /// the lowest/maximum value of <c>int</c> is interpreted as special
        /// values &quot;MIN&quot; and &quot;MAX&quot; respectively.</param>
        /// <param name="year">The year of the start time if
        /// <see cref="log_mode::time_span" /> is selected.</param>
        /// <param name="month">The month of the start time if
        /// <see cref="log_mode::time_span" /> is selected.</param>
        /// <param name="day">The day of the start time if
        /// <see cref="log_mode::time_span" /> is selected.</param>
        /// <param name="hour">The hour of the start time if
        /// <see cref="log_mode::time_span" /> is selected.</param>
        /// <param name="minute">The minute of the start time if
        /// <see cref="log_mode::time_span" /> is selected.</param>
        /// <param name="second">The second of the start time if
        /// <see cref="log_mode::time_span" /> is selected.</param>
        /// <returns><c>*this</c>.</returns>
        /// <exception cref="std::runtime_error">If the method is called on an
        /// object that has been disposed by moving it.</exception>
        /// <exception cref="visa_exception">If the VISA command was not
        /// processed successfully.</exception>
        hmc8015_sensor& log_behaviour(_In_ const float interval,
            _In_ const log_mode mode,
            _In_ const int value = INT_MAX,
            _In_ const std::int32_t year = 0,
            _In_ const std::int32_t month = 0,
            _In_ const std::int32_t day = 0,
            _In_ const std::int32_t hour = 0,
            _In_ const std::int32_t minute = 0,
            _In_ const std::int32_t second = 0);

        /// <summary>
        /// Gets the path to the log file.
        /// </summary>
        /// <param name="path">The buffer receiving the log file. Note that
        /// this is the path and a designator whether the internal or USB
        /// memory is used. If this parameter is <c>nullptr</c>, nothing will
        /// be written. The output string is guaranteed to be null-terminated
        /// if anything has been written.</param>
        /// <param name="cnt">The maximum number of bytes that can be written
        /// to <paramref name="path "/>.</param>
        /// <returns>The actual length of the path, including the terminating
        /// null character.</returns>
        /// <exception cref="std::runtime_error">If the method is called on an
        /// object that has been disposed by moving it.</exception>
        /// <exception cref="visa_exception">If the VISA command was not
        /// processed successfully.</exception>
        std::size_t log_file(_Out_writes_opt_z_(cnt) char *path,
            _In_ const std::size_t cnt);

        /// <summary>
        /// Sets the path to the log file.
        /// </summary>
        /// <param name="path">The path to the log file, usually just the
        /// file name.</param>
        /// <param name="overwrite">If <c>true</c>, the file will be cleared
        /// if it already exists. This parameter defaults to <c>false</c>.
        /// </param>
        /// <param name="use_usb">If <c>true</c>, the file will be written to
        /// the attached USB stick instead of internal memory. This parameter
        /// defaults to <c>false</c>.</param>
        /// <returns><c>*this</c>.</returns>
        /// <exception cref="std::runtime_error">If the method is called on an
        /// object that has been disposed by moving it.</exception>
        /// <exception cref="visa_exception">If the VISA command was not
        /// processed successfully.</exception>
        hmc8015_sensor& log_file(_In_z_ const char *path,
            _In_ const bool overwrite = false,
            _In_ const bool use_usb = false);

        /// <summary>
        /// Sets the path to the log file.
        /// </summary>
        /// <param name="path">The path to the log file, usually just the
        /// file name.</param>
        /// <param name="overwrite">If <c>true</c>, the file will be cleared
        /// if it already exists. This parameter defaults to <c>false</c>.
        /// </param>
        /// <param name="use_usb">If <c>true</c>, the file will be written to
        /// the attached USB stick instead of internal memory. This parameter
        /// defaults to <c>false</c>.</param>
        /// <returns><c>*this</c>.</returns>
        /// <exception cref="std::runtime_error">If the method is called on an
        /// object that has been disposed by moving it.</exception>
        /// <exception cref="visa_exception">If the VISA command was not
        /// processed successfully.</exception>
        hmc8015_sensor& log_file(_In_z_ const wchar_t *path,
            _In_ const bool overwrite = false,
            _In_ const bool use_usb = false);

        /// <summary>
        /// Gets the name of the sensor.
        /// </summary>
        /// <remarks>
        /// It is safe to call this method on a disposed object, in which case
        /// the name will be <c>nullptr</c>.
        /// </remarks>
        /// <returns>The implementation-defined, human-readable name of the
        /// sensor.</returns>
        _Ret_maybenull_z_ virtual const wchar_t *name(
            void) const noexcept override;

        /// <summary>
        /// Gets the VISA path of the instrument.
        /// </summary>
        /// <returns>The path of the instrument.</returns>
        inline _Ret_maybenull_z_ const char *path(void) const noexcept {
            return this->_instrument.path();
        }

        /// <summary>
        /// Resets the instrument to its default state.
        /// </summary>
        /// <returns><c>*this</c>.</returns>
        /// <exception cref="std::runtime_error">If the method is called on an
        /// object that has been disposed by moving it.</exception>
        /// <exception cref="visa_exception">If the VISA command was not
        /// processed successfully.</exception>
        hmc8015_sensor& reset(void);

        using sensor::sample;

        /// <summary>
        /// Synchonises the date and time on the instrument with the system
        /// clock of the computer calling this API.
        /// </summary>
        /// <param name="utc">If <c>true</c>, UTC will be used, the local time
        /// otherwise. This parameter defaults to <c>false</c>.</param>
        /// <returns><c>*this</c>.</returns>
        /// <exception cref="std::runtime_error">If the method is called on an
        /// object that has been disposed by moving it.</exception>
        /// <exception cref="visa_exception">If the VISA command was not
        /// processed successfully.</exception>
        inline hmc8015_sensor& synchronise_clock(_In_ const bool utc = false) {
            this->_instrument.synchronise_clock(utc);
            return *this;
        }

        /// <summary>
        /// Sets the voltage range.
        /// </summary>
        /// <remarks>
        /// <para>The instrument can be instructed to select a reasonable range
        /// by specifying <see cref="instrument_range::automatically" />.
        /// However, this will prevent a correct integration of measurements if
        /// the range changes during the measurement period. Therefore, it is
        /// reasonable to specify a safe range explictly in such an application
        /// case.</para>
        /// </remarks>
        /// <param name="range">Defines how <paramref name="value" /> is
        /// interpreted.</param>
        /// <param name="value">The current range in Volts. This parameter
        /// is only used if <paramref name="range" /> is set to
        /// <see cref="instrument_range::explicitly" />. Valid values are
        /// 5, 15, 30, 60, 150, 300 and 600.</param>
        /// <returns><c>*this</c>.</returns>
        /// <exception cref="std::runtime_error">If the method is called on an
        /// object that has been disposed by moving it.</exception>
        /// <exception cref="visa_exception">If the VISA command was not
        /// processed successfully.</exception>
        inline hmc8015_sensor& voltage_range(_In_ const instrument_range range,
                _In_ const std::int32_t value = 0) {
            // Note: HMC8015 supports only one channel, so this is hard coded.
            this->set_range(1, "VOLT", range, static_cast<float>(value));
            return *this;
        }

        /// <summary>
        /// Move assignment.
        /// </summary>
        /// <param name="rhs">The right-hand side operand</param>
        /// <returns><c>*this</c></returns>
        hmc8015_sensor& operator =(_Inout_ hmc8015_sensor&& rhs) noexcept;

        /// <summary>
        /// Determines whether the sensor is valid.
        /// </summary>
        /// <remarks>
        /// A sensor is considered valid until it has been disposed by a move
        /// operation.
        /// </remarks>
        /// <returns><c>true</c> if the sensor is valid, <c>false</c>
        /// otherwise.</returns>
        virtual operator bool(void) const noexcept override;

    protected:

        /// <inheritdoc />
        measurement_data sample_sync(
            _In_ const timestamp_resolution resolution) const override;

    private:

        void configure(void);

        void initialise(void);

        void set_range(_In_ const std::int32_t channel,
            _In_z_ const char *quantity,
            _In_ const instrument_range range,
            _In_ const float value);

        visa_instrument _instrument;
        wchar_t *_name;
    };

} /* namespace power_overwhelming */
} /* namespace visus */
