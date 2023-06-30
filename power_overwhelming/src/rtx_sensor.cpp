﻿// <copyright file="rtx_sensor.cpp" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2021 - 2023 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// </copyright>
// <author>Christoph Müller</author>

#include "power_overwhelming/rtx_sensor.h"

#include <algorithm>
#include <cassert>
#include <limits>
#include <stdexcept>
#include <utility>

#include "power_overwhelming/convert_string.h"

#include "timestamp.h"
#include "tokenise.h"
#include "visa_library.h"
#include "visa_sensor_impl.h"


/*
 * visus::power_overwhelming::rtx_sensor::for_all
 */
std::size_t visus::power_overwhelming::rtx_sensor::for_all(
        _Out_writes_opt_(cnt_sensors) rtx_sensor *out_sensors,
        _In_ std::size_t cnt_sensors,
        _In_ const std::int32_t timeout) {
    // Build the query for all R&S RTB2004 instruments.
    std::string query("?*::");      // Any protocol
    query += rohde_und_schwarz;     // Only R&S
    query += "::";
    query += rtb2004_id;            // Only RTB2004
    query += "::?*::INSTR";         // All serial numbers.

    // Search the instruments using VISA.
    auto devices = detail::visa_library::instance().find_resource(
        query.c_str());

    // Guard against misuse.
    if (out_sensors == nullptr) {
        cnt_sensors = 0;
    }

    // Create a sensor for each instrument we found.
    for (std::size_t i = 0; (i < cnt_sensors) && (i < devices.size()); ++i) {
        out_sensors[i] = rtx_sensor(devices[i].c_str(), timeout);
    }

    return devices.size();
}



/*
 * visus::power_overwhelming::rtx_sensor::rtx_sensor
 */
visus::power_overwhelming::rtx_sensor::rtx_sensor(
        _In_z_ const char *path, _In_ const std::int32_t timeout)
        : detail::visa_sensor(path, timeout) {
#if defined(POWER_OVERWHELMING_WITH_VISA)
#endif /*defined(POWER_OVERWHELMING_WITH_VISA) */
}


/*
 * visus::power_overwhelming::rtx_sensor::rtx_sensor
 */
visus::power_overwhelming::rtx_sensor::rtx_sensor(
        _In_z_ const wchar_t *path, _In_ const std::int32_t timeout)
        : detail::visa_sensor(path, timeout) {
#if defined(POWER_OVERWHELMING_WITH_VISA)
#endif /*defined(POWER_OVERWHELMING_WITH_VISA) */
}


/*
 * visus::power_overwhelming::rtx_sensor::~rtx_sensor
 */
visus::power_overwhelming::rtx_sensor::~rtx_sensor(void) { }


/*
 * visus::power_overwhelming::rtx_sensor::configure
 */
void visus::power_overwhelming::rtx_sensor::configure(
        _In_reads_(cnt) const oscilloscope_sensor_definition *sensors,
        _In_ const std::size_t cnt) {
#if defined(POWER_OVERWHELMING_WITH_VISA)
    if ((cnt > 0) && (sensors == nullptr)) {
        throw std::invalid_argument("The sensor definitions must be valid.");
    }

    auto impl = static_cast<detail::visa_sensor_impl&>(*this);

    for (std::uint32_t i = 1; impl.system_error() == VI_SUCCESS; ++i) {
        impl.printf("CHAN%u:STAT OFF\n", i);
    }

    for (std::size_t i = 0; i < cnt; ++i) {
        {
            auto c = sensors[i].channel_current();
            impl.printf("PROB%u:SET:ATT:UNIT %s\n", c, "A");
            this->throw_on_system_error();

            if (!sensors[i].auto_attenuation_current()) {
                impl.printf("PROB%u:SET:ATT:MAN %f\n",
                    c, sensors[i].attenuation_current());
                this->throw_on_system_error();
            }

            impl.printf("CHAN%u:STAT ON\n", c);
            this->throw_on_system_error();
        }

        {
            auto c = sensors[i].channel_voltage();
            impl.printf("PROB%u:SET:ATT:UNIT %s\n", c, "V");
            this->throw_on_system_error();

            if (!sensors[i].auto_attenuation_voltage()) {
                impl.printf("PROB%u:SET:ATT:MAN %f\n",
                    c, sensors[i].attenuation_voltage());
                this->throw_on_system_error();
            }

            impl.printf("CHAN%u:STAT ON\n", c);
            this->throw_on_system_error();
        }
    }
#else /*defined(POWER_OVERWHELMING_WITH_VISA) */
    throw std::logic_error("This function is unavailable unless compiled with "
        "support for VISA.");
#endif /*defined(POWER_OVERWHELMING_WITH_VISA) */
}


/*
 * visus::power_overwhelming::rtx_sensor::configure
 */
void visus::power_overwhelming::rtx_sensor::configure(
        _In_ const oscilloscope_channel& channel) {
#if defined(POWER_OVERWHELMING_WITH_VISA)
    auto impl = static_cast<detail::visa_sensor_impl &>(*this);

    // Note: Attenuation should be set first, because changing the attenuation
    // will also scale other values like the range.
    impl.printf("PROB%d:SET:ATT:UNIT %s\n", channel.channel(),
        channel.gain().unit());
    this->throw_on_system_error();
    impl.printf("PROB%d:SET:ATT:MAN %f\n", channel.channel(),
        channel.gain().value());
    this->throw_on_system_error();

    switch (channel.bandwidth()) {
        case oscilloscope_channel_bandwidth::limit_to_20_mhz:
            impl.printf("CHAN%d:BAND B20\n", channel.channel());
            break;

        default:
            impl.printf("CHAN%d:BAND FULL\n", channel.channel());
            break;
    }
    this->throw_on_system_error();

    switch (channel.coupling()) {
        case oscilloscope_channel_coupling::alternating_current_limit:
            impl.printf("CHAN%d:COUP ACL\n", channel.channel());
            break;

        case oscilloscope_channel_coupling::ground:
            impl.printf("CHAN%d:COUP GND\n", channel.channel());
            break;

        default:
            impl.printf("CHAN%d:COUP DCL\n", channel.channel());
            break;
    }
    this->throw_on_system_error();

    switch (channel.decimation_mode()) {
        case oscilloscope_decimation_mode::high_resolution:
            impl.printf("CHAN%d:TYPE HRES\n", channel.channel());
            break;

        case oscilloscope_decimation_mode::peak_detect:
            impl.printf("CHAN%d:TYPE PDET\n", channel.channel());
            break;

        default:
            impl.printf("CHAN%d:TYPE SAMP\n", channel.channel());
            break;
    }
    this->throw_on_system_error();

    impl.printf("CHAN%d:LAB \"%s\"\n", channel.channel(),
        channel.label().text());
    this->throw_on_system_error();
    impl.printf("CHAN%d:LAB:STAT %s\n", channel.channel(),
        channel.label().visible() ? "ON" : "OFF");
    this->throw_on_system_error();

    impl.printf("CHAN%d:OFFS %f%s\n", channel.channel(),
        channel.offset().value(), channel.offset().unit());
    this->throw_on_system_error();

    switch (channel.polarity()) {
        case oscilloscope_channel_polarity::inverted:
            impl.printf("CHAN%d:POL INV\n", channel.channel());
            break;

        default:
            impl.printf("CHAN%d:POL NORM\n", channel.channel());
            break;
    }
    this->throw_on_system_error();

    impl.printf("CHAN%d:RANG %f%s\n", channel.channel(),
        channel.range().value(), channel.range().unit());
    this->throw_on_system_error();

    impl.printf("CHAN%d:SKEW %f%s\n", channel.channel(),
        channel.skew().value(), channel.skew().unit());
    this->throw_on_system_error();

    impl.printf("CHAN%d:STAT %s\n", channel.channel(),
        channel.state() ? "ON" : "OFF");
    this->throw_on_system_error();

    impl.printf("CHAN%d:ZOFF %f%s\n", channel.channel(),
        channel.zero_offset().value(), channel.zero_offset().unit());
    this->throw_on_system_error();
#endif /*defined(POWER_OVERWHELMING_WITH_VISA) */
}


/*
 * visus::power_overwhelming::rtx_sensor::configure
 */
void visus::power_overwhelming::rtx_sensor::configure(
        _In_ const oscilloscope_single_acquisition& acquisition) {
#if defined(POWER_OVERWHELMING_WITH_VISA)
    auto impl = static_cast<detail::visa_sensor_impl &>(*this);

    if (acquisition.automatic_points()) {
        impl.printf("ACQ:POIN:AUT ON\n", acquisition.points());
    } else {
        impl.printf("ACQ:POIN %u\n", acquisition.points());
    }
    this->throw_on_system_error();

    impl.printf("ACQ:NSIN:COUN %u\n", acquisition.count());
    this->throw_on_system_error();

    impl.printf("SING\n");
    this->throw_on_system_error();

    impl.printf("ACQ:STAT RUN\n");
    this->throw_on_system_error();

    impl.write("*TRG\n");
    this->throw_on_system_error();
#endif /*defined(POWER_OVERWHELMING_WITH_VISA) */
}


/*
 * visus::power_overwhelming::rtx_sensor::data
 */
visus::power_overwhelming::blob visus::power_overwhelming::rtx_sensor::data(
        _In_ const std::uint32_t channel) {
#if defined(POWER_OVERWHELMING_WITH_VISA)
    auto impl = static_cast<detail::visa_sensor_impl &>(*this);

    impl.write("FORM REAL,32\n");
    this->throw_on_system_error();
    impl.query("*OPC?\n");
    this->throw_on_system_error();

    auto query = std::string("CHAN") + std::to_string(channel) + ":DATA?\n";
    impl.write(query);
    std::vector<std::uint8_t> hack(2);
    impl.read(hack.data(), hack.size());
    hack.resize(std::atoi((char *) hack.data() + 1));
    impl.read(hack.data(), hack.size());
    auto data = impl.read(std::atoi((char *)hack.data()));

    blob retval(data.size());
    std::copy(data.begin(), data.end(), retval.as<std::uint8_t>());
    return retval;

#else /*defined(POWER_OVERWHELMING_WITH_VISA) */
    throw std::logic_error("This function is unavailable unless compiled with "
        "support for VISA.");
#endif /*defined(POWER_OVERWHELMING_WITH_VISA) */
}


/*
 * visus::power_overwhelming::rtx_sensor::expression
 */
void visus::power_overwhelming::rtx_sensor::expression(
        _In_ const std::uint32_t channel,
        _In_opt_z_  const char *expression,
        _In_opt_z_ const char *unit) {
    auto impl = static_cast<detail::visa_sensor_impl &>(*this);

    if (expression != nullptr) {
        if (unit != nullptr) {
            impl.printf("CALC:MATH%u:EXPR:DEF \"%s in %s\"\n", channel,
                expression, unit);
        } else {
            impl.printf("CALC:MATH%u:EXPR:DEF \"%s\"\n", channel, expression);
        }

        impl.printf("CALC:MATH%u:STAT ON\n", channel);

    } else {
        impl.printf("CALC:MATH%u:STAT OFF\n", channel);
    }

    this->throw_on_system_error();
}


/*
 * visus::power_overwhelming::rtx_sensor::reference_position
 */
void visus::power_overwhelming::rtx_sensor::reference_position(
        _In_ const oscilloscope_reference_point position) {
    auto impl = static_cast<detail::visa_sensor_impl &>(*this);
    impl.printf("TIM:REF %f\n", static_cast<float>(position) / 100.0f);
    this->throw_on_system_error();
}


/*
 * visus::power_overwhelming::rtx_sensor::unit
 */
void visus::power_overwhelming::rtx_sensor::unit(
        _In_ const std::uint32_t channel, _In_z_ const char *unit) {
    if (unit == nullptr) {
        throw std::invalid_argument("The unit for a probe cannot be null.");
    }

    auto impl = static_cast<detail::visa_sensor_impl&>(*this);
    impl.printf("PROB%u:SET:ATT:UNIT %s\n", channel, unit);
    this->throw_on_system_error();
}


/*
 * visus::power_overwhelming::rtx_sensor::time_range
 */
void visus::power_overwhelming::rtx_sensor::time_range(
        _In_ const oscilloscope_quantity& scale) {
    auto impl = static_cast<detail::visa_sensor_impl &>(*this);
    impl.printf("TIM:RANG %f %s\n", scale.value(), scale.unit());
    this->throw_on_system_error();
}


/*
 * visus::power_overwhelming::rtx_sensor::time_scale
 */
void visus::power_overwhelming::rtx_sensor::time_scale(
        _In_ const oscilloscope_quantity &scale) {
    auto impl = static_cast<detail::visa_sensor_impl&>(*this);
    impl.printf("TIM:SCAL %f %s\n", scale.value(), scale.unit());
    this->throw_on_system_error();
}


/*
 * visus::power_overwhelming::rtx_sensor::trigger
 */
void visus::power_overwhelming::rtx_sensor::trigger(
        const oscilloscope_trigger& trigger) {
    auto impl = static_cast<detail::visa_sensor_impl &>(*this);

    // Apply configuration that is valid for all triggers.
    switch (trigger.mode()) {
        case oscilloscope_trigger_mode::automatic:
            impl.printf("TRIG:A:MODE AUTO\n");
            break;

        default:
            impl.printf("TRIG:A:MODE NORM\n");
            break;
    }
    this->throw_on_system_error();

    impl.printf("TRIG:A:SOUR %s\n", trigger.source());
    this->throw_on_system_error();

    impl.printf("TRIG:A:TYPE %s\n", trigger.type());
    this->throw_on_system_error();

    if (trigger.hold_off() == nullptr) {
        impl.printf("TRIG:A:HOLD:MODE OFF\n");
        this->throw_on_system_error();

    } else {
        impl.printf("TRIG:A:HOLD:MODE TIME\n");
        this->throw_on_system_error();

        impl.printf("TRIG:A:HOLD:TIME %s\n", trigger.hold_off());
        this->throw_on_system_error();
    }

    // Apply special configuration if the trigger is an edge trigger.
    auto et = dynamic_cast<const oscilloscope_edge_trigger *>(&trigger);
    if (et != nullptr) {
        switch (et->slope()) {
            case oscilloscope_trigger_slope::both:
                impl.printf("TRIG:A:EDGE:SLOP EITH\n");
                break;

            case oscilloscope_trigger_slope::rising:
                impl.printf("TRIG:A:EDGE:SLOP POS\n");
                break;

            case oscilloscope_trigger_slope::falling:
                impl.printf("TRIG:A:EDGE:SLOP NEG\n");
                break;
        }
        this->throw_on_system_error();

        impl.printf("TRIG:A:LEV%d:VAL %f %s\n", et->input(),
            et->level().value(), et->level().unit());
        this->throw_on_system_error();

        switch (et->coupling()) {
            case oscilloscope_trigger_coupling::alternating_current:
                impl.printf("TRIG:A:EDGE:COUP AC\n");
                break;

            case oscilloscope_trigger_coupling::direct_current:
                impl.printf("TRIG:A:EDGE:COUP DC\n");
                break;

            case oscilloscope_trigger_coupling::low_frequency_reject:
                impl.printf("TRIG:A:EDGE:COUP LFR\n");
                break;
        }
        this->throw_on_system_error();

#if 0
        // TODO: Only RTA
        switch (et->hysteresis()) {
            case oscilloscope_trigger_hysteresis::automatic:
                impl.printf("TRIG:A:HYST AUTO\n");
                break;

            case oscilloscope_trigger_hysteresis::high:
                impl.printf("TRIG:A:HYST LARGE\n");
                break;

            case oscilloscope_trigger_hysteresis::low:
                impl.printf("TRIG:A:HYST SMAL\n");
                break;

            case oscilloscope_trigger_hysteresis::medium:
                impl.printf("TRIG:A:HYST MED\n");
                break;
        }
        this->throw_on_system_error();
#endif
    }
}


/*
 * visus::power_overwhelming::rtx_sensor::trigger_position
 */
void visus::power_overwhelming::rtx_sensor::trigger_position(
        _In_ const float offset, _In_z_ const char *unit) {
    auto impl = static_cast<detail::visa_sensor_impl&>(*this);
    impl.printf("TIM:POS %f%s\n", offset, (unit != nullptr) ? unit : "");
    this->throw_on_system_error();
}


/*
 * visus::power_overwhelming::rtx_sensor::sample_sync
 */
visus::power_overwhelming::measurement_data
visus::power_overwhelming::rtx_sensor::sample_sync(
        _In_ const timestamp_resolution resolution) const {
    throw "TODO";

}


// TIM:SCAL 5
//CHAN2:STAT OFF
// CHAN1:BAND?
// PROB1:SET:ATT:MAN 10
// TRIG:A:SOUR?
// TRIG:A:SOUR "CH1"

/*
*LRN
*CLS
*ESE
*ESR
*IDN
*OPC
*RST
*SRE
*STB
*WAI
*PSC
*TRG
:SYST:NAME
:SYST:ERR
:SYST:ERR:NEXT
:SYST:ERR:ALL
:SYST:ELIS
:SYST:SET
:SYST:CIE
:SYST:SNUM
:SYST:TREE
:SYST:SOFT
:SYST:HARD
:SYST:DEV
:SYST:DATE
:SYST:TIME
:STAT:PRES
&DCL
&GTL
&GTR
&GET
&LLO
&NREN
&POL
:CHAN:COUP
:CHAN2:COUP
:CHAN3:COUP
:CHAN4:COUP
:CHAN:POL
:CHAN2:POL
:CHAN3:POL
:CHAN4:POL
:CHAN:POS
:CHAN2:POS
:CHAN3:POS
:CHAN4:POS
:CHAN:SCAL
:CHAN2:SCAL
:CHAN3:SCAL
:CHAN4:SCAL
:CHAN:RANG
:CHAN2:RANG
:CHAN3:RANG
:CHAN4:RANG
:CHAN:STAT
:CHAN2:STAT
:CHAN3:STAT
:CHAN4:STAT
:CHAN:BAND
:CHAN2:BAND
:CHAN3:BAND
:CHAN4:BAND
:CHAN:AOFF
:CHAN2:AOFF
:CHAN3:AOFF
:CHAN4:AOFF
:CHAN:AON
:CHAN2:AON
:CHAN3:AON
:CHAN4:AON
:CHAN:OFFS
:CHAN2:OFFS
:CHAN3:OFFS
:CHAN4:OFFS
:CHAN:SKEW
:CHAN2:SKEW
:CHAN3:SKEW
:CHAN4:SKEW
:CHAN:THR
:CHAN2:THR
:CHAN3:THR
:CHAN4:THR
:CHAN:THR:FIND
:CHAN2:THR:FIND
:CHAN3:THR:FIND
:CHAN4:THR:FIND
:CHAN:THR:HYST
:CHAN2:THR:HYST
:CHAN3:THR:HYST
:CHAN4:THR:HYST
:AUT
:CHAN:TYPE
:CHAN2:TYPE
:CHAN3:TYPE
:CHAN4:TYPE
:CHAN:ARIT
:CHAN2:ARIT
:CHAN3:ARIT
:CHAN4:ARIT
:CHAN:OVER
:CHAN2:OVER
:CHAN3:OVER
:CHAN4:OVER
:CHAN:LAB
:CHAN2:LAB
:CHAN3:LAB
:CHAN4:LAB
:CHAN:LAB:STAT
:CHAN2:LAB:STAT
:CHAN3:LAB:STAT
:CHAN4:LAB:STAT
:CHAN:ZOFF
:CHAN2:ZOFF
:CHAN3:ZOFF
:CHAN4:ZOFF
:CHAN:ZOFF:VAL
:CHAN2:ZOFF:VAL
:CHAN3:ZOFF:VAL
:CHAN4:ZOFF:VAL
:CHAN:DATA
:CHAN2:DATA
:CHAN3:DATA
:CHAN4:DATA
:CHAN:DATA:POIN
:CHAN2:DATA:POIN
:CHAN3:DATA:POIN
:CHAN4:DATA:POIN
:CHAN:DATA:HEAD
:CHAN2:DATA:HEAD
:CHAN3:DATA:HEAD
:CHAN4:DATA:HEAD
:CHAN:DATA:XINC
:CHAN2:DATA:XINC
:CHAN3:DATA:XINC
:CHAN4:DATA:XINC
:CHAN:DATA:XOR
:CHAN2:DATA:XOR
:CHAN3:DATA:XOR
:CHAN4:DATA:XOR
:CHAN:DATA:YINC
:CHAN2:DATA:YINC
:CHAN3:DATA:YINC
:CHAN4:DATA:YINC
:CHAN:DATA:YOR
:CHAN2:DATA:YOR
:CHAN3:DATA:YOR
:CHAN4:DATA:YOR
:CHAN:DATA:YRES
:CHAN2:DATA:YRES
:CHAN3:DATA:YRES
:CHAN4:DATA:YRES
:CHAN:DATA:ENV
:CHAN2:DATA:ENV
:CHAN3:DATA:ENV
:CHAN4:DATA:ENV
:CHAN:DATA:ENV:HEAD
:CHAN2:DATA:ENV:HEAD
:CHAN3:DATA:ENV:HEAD
:CHAN4:DATA:ENV:HEAD
:CHAN:DATA:ENV:XINC
:CHAN2:DATA:ENV:XINC
:CHAN3:DATA:ENV:XINC
:CHAN4:DATA:ENV:XINC
:CHAN:DATA:ENV:XOR
:CHAN2:DATA:ENV:XOR
:CHAN3:DATA:ENV:XOR
:CHAN4:DATA:ENV:XOR
:CHAN:DATA:ENV:YINC
:CHAN2:DATA:ENV:YINC
:CHAN3:DATA:ENV:YINC
:CHAN4:DATA:ENV:YINC
:CHAN:DATA:ENV:YOR
:CHAN2:DATA:ENV:YOR
:CHAN3:DATA:ENV:YOR
:CHAN4:DATA:ENV:YOR
:CHAN:DATA:ENV:YRES
:CHAN2:DATA:ENV:YRES
:CHAN3:DATA:ENV:YRES
:CHAN4:DATA:ENV:YRES
:CHAN:WCOL
:CHAN2:WCOL
:CHAN3:WCOL
:CHAN4:WCOL
:CHAN:HIST:PLAY:STAT
:CHAN2:HIST:PLAY:STAT
:CHAN3:HIST:PLAY:STAT
:CHAN4:HIST:PLAY:STAT
:CHAN:HIST:PLAY:SPE
:CHAN2:HIST:PLAY:SPE
:CHAN3:HIST:PLAY:SPE
:CHAN4:HIST:PLAY:SPE
:CHAN:HIST:REPL
:CHAN2:HIST:REPL
:CHAN3:HIST:REPL
:CHAN4:HIST:REPL
:CHAN:HIST:PALL
:CHAN2:HIST:PALL
:CHAN3:HIST:PALL
:CHAN4:HIST:PALL
:CHAN:HIST:CURR
:CHAN2:HIST:CURR
:CHAN3:HIST:CURR
:CHAN4:HIST:CURR
:CHAN:HIST:STAR
:CHAN2:HIST:STAR
:CHAN3:HIST:STAR
:CHAN4:HIST:STAR
:CHAN:HIST:STOP
:CHAN2:HIST:STOP
:CHAN3:HIST:STOP
:CHAN4:HIST:STOP
:CHAN:HIST:TSR
:CHAN2:HIST:TSR
:CHAN3:HIST:TSR
:CHAN4:HIST:TSR
:CHAN:HIST:TSR:ALL
:CHAN2:HIST:TSR:ALL
:CHAN3:HIST:TSR:ALL
:CHAN4:HIST:TSR:ALL
:CHAN:HIST:TSAB
:CHAN2:HIST:TSAB
:CHAN3:HIST:TSAB
:CHAN4:HIST:TSAB
:CHAN:HIST:TSAB:ALL
:CHAN2:HIST:TSAB:ALL
:CHAN3:HIST:TSAB:ALL
:CHAN4:HIST:TSAB:ALL
:CHAN:HIST:TSD
:CHAN2:HIST:TSD
:CHAN3:HIST:TSD
:CHAN4:HIST:TSD
:CHAN:HIST:TSD:ALL
:CHAN2:HIST:TSD:ALL
:CHAN3:HIST:TSD:ALL
:CHAN4:HIST:TSD:ALL
:CHAN:HIST:ATIC
:CHAN2:HIST:ATIC
:CHAN3:HIST:ATIC
:CHAN4:HIST:ATIC
:CHAN:HIST:ATIC:ALL
:CHAN2:HIST:ATIC:ALL
:CHAN3:HIST:ATIC:ALL
:CHAN4:HIST:ATIC:ALL
:CHAN:HIST:DTIC
:CHAN2:HIST:DTIC
:CHAN3:HIST:DTIC
:CHAN4:HIST:DTIC
:CHAN:HIST:DTIC:ALL
:CHAN2:HIST:DTIC:ALL
:CHAN3:HIST:DTIC:ALL
:CHAN4:HIST:DTIC:ALL
:CHAN:HIST:TCL
:CHAN2:HIST:TCL
:CHAN3:HIST:TCL
:CHAN4:HIST:TCL
:CHAN:HIST:TINC
:CHAN2:HIST:TINC
:CHAN3:HIST:TINC
:CHAN4:HIST:TINC
:CHAN:HIST:TMOD
:CHAN2:HIST:TMOD
:CHAN3:HIST:TMOD
:CHAN4:HIST:TMOD
:CHAN:HIST:EXP:NAME
:CHAN2:HIST:EXP:NAME
:CHAN3:HIST:EXP:NAME
:CHAN4:HIST:EXP:NAME
:CHAN:HIST:EXP:SAVE
:CHAN2:HIST:EXP:SAVE
:CHAN3:HIST:EXP:SAVE
:CHAN4:HIST:EXP:SAVE
:CHAN:HIST:TTAB
:CHAN2:HIST:TTAB
:CHAN3:HIST:TTAB
:CHAN4:HIST:TTAB
:CHAN:HIST:TTAB:ENAB
:CHAN2:HIST:TTAB:ENAB
:CHAN3:HIST:TTAB:ENAB
:CHAN4:HIST:TTAB:ENAB
:CHAN:HIST:CONT
:CHAN2:HIST:CONT
:CHAN3:HIST:CONT
:CHAN4:HIST:CONT
:CHAN:HIST:CONT:ENAB
:CHAN2:HIST:CONT:ENAB
:CHAN3:HIST:CONT:ENAB
:CHAN4:HIST:CONT:ENAB
:MEAS
:MEAS2
:MEAS3
:MEAS4
:MEAS:ENAB
:MEAS2:ENAB
:MEAS3:ENAB
:MEAS4:ENAB
:MEAS:SOUR
:MEAS2:SOUR
:MEAS3:SOUR
:MEAS4:SOUR
:MEAS:CAT
:MEAS2:CAT
:MEAS3:CAT
:MEAS4:CAT
:MEAS:AON
:MEAS2:AON
:MEAS3:AON
:MEAS4:AON
:MEAS:AOFF
:MEAS2:AOFF
:MEAS3:AOFF
:MEAS4:AOFF
:MEAS:ARES
:MEAS2:ARES
:MEAS3:ARES
:MEAS4:ARES
:MEAS:MAIN
:MEAS2:MAIN
:MEAS3:MAIN
:MEAS4:MAIN
:MEAS:RES
:MEAS2:RES
:MEAS3:RES
:MEAS4:RES
:MEAS:RES:ACT
:MEAS2:RES:ACT
:MEAS3:RES:ACT
:MEAS4:RES:ACT
:MEAS:RES:NPE
:MEAS2:RES:NPE
:MEAS3:RES:NPE
:MEAS4:RES:NPE
:MEAS:RES:PPE
:MEAS2:RES:PPE
:MEAS3:RES:PPE
:MEAS4:RES:PPE
:MEAS:RES:AVG
:MEAS2:RES:AVG
:MEAS3:RES:AVG
:MEAS4:RES:AVG
:MEAS:RES:STDD
:MEAS2:RES:STDD
:MEAS3:RES:STDD
:MEAS4:RES:STDD
:MEAS:RES:WFMC
:MEAS2:RES:WFMC
:MEAS3:RES:WFMC
:MEAS4:RES:WFMC
:MEAS:RES:BUFF:NPE
:MEAS2:RES:BUFF:NPE
:MEAS3:RES:BUFF:NPE
:MEAS4:RES:BUFF:NPE
:MEAS:RES:BUFF:PPE
:MEAS2:RES:BUFF:PPE
:MEAS3:RES:BUFF:PPE
:MEAS4:RES:BUFF:PPE
:MEAS:RES:BUFF:AVG
:MEAS2:RES:BUFF:AVG
:MEAS3:RES:BUFF:AVG
:MEAS4:RES:BUFF:AVG
:MEAS:RES:BUFF:STDD
:MEAS2:RES:BUFF:STDD
:MEAS3:RES:BUFF:STDD
:MEAS4:RES:BUFF:STDD
:MEAS:STAT
:MEAS2:STAT
:MEAS3:STAT
:MEAS4:STAT
:MEAS:STAT:ENAB
:MEAS2:STAT:ENAB
:MEAS3:STAT:ENAB
:MEAS4:STAT:ENAB
:MEAS:STAT:RES
:MEAS2:STAT:RES
:MEAS3:STAT:RES
:MEAS4:STAT:RES
:MEAS:STAT:WEIG
:MEAS2:STAT:WEIG
:MEAS3:STAT:WEIG
:MEAS4:STAT:WEIG
:MEAS:DEL:SLOP
:MEAS2:DEL:SLOP
:MEAS3:DEL:SLOP
:MEAS4:DEL:SLOP
:MEAS:ALL
:MEAS2:ALL
:MEAS3:ALL
:MEAS4:ALL
:MEAS:ALL:STAT
:MEAS2:ALL:STAT
:MEAS3:ALL:STAT
:MEAS4:ALL:STAT
:DISP:INT:WAV
:DISP:INT:GRID
:DISP:GRID:STYL
:DISP:GRID:ANN
:DISP:GRID:ANN:ENAB
:DISP:GRID:ANN:TRAC
:DISP:DTIM
:DISP:STYL
:DISP:PAL
:DISP:PERS
:DISP:PERS:CLE
:DISP:PERS:TYPE
:DISP:PERS:STAT
:DISP:PERS:INF
:DISP:PERS:TIME
:DISP:DIAL:CLOS
:DISP:DIAL:MESS
:DISP:DIAL:HELP
:DISP:DIAL:TRAN
:DISP:XY:ZMOD
:DISP:XY:ZTHR
:DISP:XY:XSO
:DISP:XY:Y1S
:DISP:XY:Y2S
:DISP:XY:ZSO
:DISP:CBAR:RES
:DISP:CBAR:RES:POS
:DISP:CBAR:ZOOM
:DISP:CBAR:ZOOM:POS
:DISP:CBAR:FFT
:DISP:CBAR:FFT:POS
:DISP:CBAR:SPEC
:DISP:CBAR:SPEC:POS
:DISP:MODE
:DISP:PROG:TYPE
:DISP:PROG:VAL
:DISP:LANG
:DISP:LANG:CAT
:DISP:LANG:REM
:DISP:LANG:ADD
:DISP:CLE
:DISP:CLE:SCR
:DISP:TOOL:DEF
:DISP:RES
:DISP:RES:TAB
:DIG0:DISP
:DIG1:DISP
:DIG2:DISP
:DIG3:DISP
:DIG4:DISP
:DIG5:DISP
:DIG6:DISP
:DIG7:DISP
:DIG8:DISP
:DIG9:DISP
:DIG10:DISP
:DIG11:DISP
:DIG12:DISP
:DIG13:DISP
:DIG14:DISP
:DIG15:DISP
:DIG0:SIZE
:DIG1:SIZE
:DIG2:SIZE
:DIG3:SIZE
:DIG4:SIZE
:DIG5:SIZE
:DIG6:SIZE
:DIG7:SIZE
:DIG8:SIZE
:DIG9:SIZE
:DIG10:SIZE
:DIG11:SIZE
:DIG12:SIZE
:DIG13:SIZE
:DIG14:SIZE
:DIG15:SIZE
:DIG0:POS
:DIG1:POS
:DIG2:POS
:DIG3:POS
:DIG4:POS
:DIG5:POS
:DIG6:POS
:DIG7:POS
:DIG8:POS
:DIG9:POS
:DIG10:POS
:DIG11:POS
:DIG12:POS
:DIG13:POS
:DIG14:POS
:DIG15:POS
:DIG0:LAB
:DIG1:LAB
:DIG2:LAB
:DIG3:LAB
:DIG4:LAB
:DIG5:LAB
:DIG6:LAB
:DIG7:LAB
:DIG8:LAB
:DIG9:LAB
:DIG10:LAB
:DIG11:LAB
:DIG12:LAB
:DIG13:LAB
:DIG14:LAB
:DIG15:LAB
:DIG0:LAB:STAT
:DIG1:LAB:STAT
:DIG2:LAB:STAT
:DIG3:LAB:STAT
:DIG4:LAB:STAT
:DIG5:LAB:STAT
:DIG6:LAB:STAT
:DIG7:LAB:STAT
:DIG8:LAB:STAT
:DIG9:LAB:STAT
:DIG10:LAB:STAT
:DIG11:LAB:STAT
:DIG12:LAB:STAT
:DIG13:LAB:STAT
:DIG14:LAB:STAT
:DIG15:LAB:STAT
:DIG0:HYST
:DIG1:HYST
:DIG2:HYST
:DIG3:HYST
:DIG4:HYST
:DIG5:HYST
:DIG6:HYST
:DIG7:HYST
:DIG8:HYST
:DIG9:HYST
:DIG10:HYST
:DIG11:HYST
:DIG12:HYST
:DIG13:HYST
:DIG14:HYST
:DIG15:HYST
:DIG0:TECH
:DIG1:TECH
:DIG2:TECH
:DIG3:TECH
:DIG4:TECH
:DIG5:TECH
:DIG6:TECH
:DIG7:TECH
:DIG8:TECH
:DIG9:TECH
:DIG10:TECH
:DIG11:TECH
:DIG12:TECH
:DIG13:TECH
:DIG14:TECH
:DIG15:TECH
:DIG0:THR
:DIG1:THR
:DIG2:THR
:DIG3:THR
:DIG4:THR
:DIG5:THR
:DIG6:THR
:DIG7:THR
:DIG8:THR
:DIG9:THR
:DIG10:THR
:DIG11:THR
:DIG12:THR
:DIG13:THR
:DIG14:THR
:DIG15:THR
:DIG0:TYPE
:DIG1:TYPE
:DIG2:TYPE
:DIG3:TYPE
:DIG4:TYPE
:DIG5:TYPE
:DIG6:TYPE
:DIG7:TYPE
:DIG8:TYPE
:DIG9:TYPE
:DIG10:TYPE
:DIG11:TYPE
:DIG12:TYPE
:DIG13:TYPE
:DIG14:TYPE
:DIG15:TYPE
:DIG0:DATA
:DIG1:DATA
:DIG2:DATA
:DIG3:DATA
:DIG4:DATA
:DIG5:DATA
:DIG6:DATA
:DIG7:DATA
:DIG8:DATA
:DIG9:DATA
:DIG10:DATA
:DIG11:DATA
:DIG12:DATA
:DIG13:DATA
:DIG14:DATA
:DIG15:DATA
:DIG0:DATA:POIN
:DIG1:DATA:POIN
:DIG2:DATA:POIN
:DIG3:DATA:POIN
:DIG4:DATA:POIN
:DIG5:DATA:POIN
:DIG6:DATA:POIN
:DIG7:DATA:POIN
:DIG8:DATA:POIN
:DIG9:DATA:POIN
:DIG10:DATA:POIN
:DIG11:DATA:POIN
:DIG12:DATA:POIN
:DIG13:DATA:POIN
:DIG14:DATA:POIN
:DIG15:DATA:POIN
:DIG0:DATA:HEAD
:DIG1:DATA:HEAD
:DIG2:DATA:HEAD
:DIG3:DATA:HEAD
:DIG4:DATA:HEAD
:DIG5:DATA:HEAD
:DIG6:DATA:HEAD
:DIG7:DATA:HEAD
:DIG8:DATA:HEAD
:DIG9:DATA:HEAD
:DIG10:DATA:HEAD
:DIG11:DATA:HEAD
:DIG12:DATA:HEAD
:DIG13:DATA:HEAD
:DIG14:DATA:HEAD
:DIG15:DATA:HEAD
:DIG0:DATA:XINC
:DIG1:DATA:XINC
:DIG2:DATA:XINC
:DIG3:DATA:XINC
:DIG4:DATA:XINC
:DIG5:DATA:XINC
:DIG6:DATA:XINC
:DIG7:DATA:XINC
:DIG8:DATA:XINC
:DIG9:DATA:XINC
:DIG10:DATA:XINC
:DIG11:DATA:XINC
:DIG12:DATA:XINC
:DIG13:DATA:XINC
:DIG14:DATA:XINC
:DIG15:DATA:XINC
:DIG0:DATA:XOR
:DIG1:DATA:XOR
:DIG2:DATA:XOR
:DIG3:DATA:XOR
:DIG4:DATA:XOR
:DIG5:DATA:XOR
:DIG6:DATA:XOR
:DIG7:DATA:XOR
:DIG8:DATA:XOR
:DIG9:DATA:XOR
:DIG10:DATA:XOR
:DIG11:DATA:XOR
:DIG12:DATA:XOR
:DIG13:DATA:XOR
:DIG14:DATA:XOR
:DIG15:DATA:XOR
:DIG0:DATA:YINC
:DIG1:DATA:YINC
:DIG2:DATA:YINC
:DIG3:DATA:YINC
:DIG4:DATA:YINC
:DIG5:DATA:YINC
:DIG6:DATA:YINC
:DIG7:DATA:YINC
:DIG8:DATA:YINC
:DIG9:DATA:YINC
:DIG10:DATA:YINC
:DIG11:DATA:YINC
:DIG12:DATA:YINC
:DIG13:DATA:YINC
:DIG14:DATA:YINC
:DIG15:DATA:YINC
:DIG0:DATA:YOR
:DIG1:DATA:YOR
:DIG2:DATA:YOR
:DIG3:DATA:YOR
:DIG4:DATA:YOR
:DIG5:DATA:YOR
:DIG6:DATA:YOR
:DIG7:DATA:YOR
:DIG8:DATA:YOR
:DIG9:DATA:YOR
:DIG10:DATA:YOR
:DIG11:DATA:YOR
:DIG12:DATA:YOR
:DIG13:DATA:YOR
:DIG14:DATA:YOR
:DIG15:DATA:YOR
:DIG0:DATA:YRES
:DIG1:DATA:YRES
:DIG2:DATA:YRES
:DIG3:DATA:YRES
:DIG4:DATA:YRES
:DIG5:DATA:YRES
:DIG6:DATA:YRES
:DIG7:DATA:YRES
:DIG8:DATA:YRES
:DIG9:DATA:YRES
:DIG10:DATA:YRES
:DIG11:DATA:YRES
:DIG12:DATA:YRES
:DIG13:DATA:YRES
:DIG14:DATA:YRES
:DIG15:DATA:YRES
:DIG0:AON
:DIG1:AON
:DIG2:AON
:DIG3:AON
:DIG4:AON
:DIG5:AON
:DIG6:AON
:DIG7:AON
:DIG8:AON
:DIG9:AON
:DIG10:AON
:DIG11:AON
:DIG12:AON
:DIG13:AON
:DIG14:AON
:DIG15:AON
:DIG0:AOFF
:DIG1:AOFF
:DIG2:AOFF
:DIG3:AOFF
:DIG4:AOFF
:DIG5:AOFF
:DIG6:AOFF
:DIG7:AOFF
:DIG8:AOFF
:DIG9:AOFF
:DIG10:AOFF
:DIG11:AOFF
:DIG12:AOFF
:DIG13:AOFF
:DIG14:AOFF
:DIG15:AOFF
:DIG0:HIST:PLAY:STAT
:DIG1:HIST:PLAY:STAT
:DIG2:HIST:PLAY:STAT
:DIG3:HIST:PLAY:STAT
:DIG4:HIST:PLAY:STAT
:DIG5:HIST:PLAY:STAT
:DIG6:HIST:PLAY:STAT
:DIG7:HIST:PLAY:STAT
:DIG8:HIST:PLAY:STAT
:DIG9:HIST:PLAY:STAT
:DIG10:HIST:PLAY:STAT
:DIG11:HIST:PLAY:STAT
:DIG12:HIST:PLAY:STAT
:DIG13:HIST:PLAY:STAT
:DIG14:HIST:PLAY:STAT
:DIG15:HIST:PLAY:STAT
:DIG0:HIST:PLAY:SPE
:DIG1:HIST:PLAY:SPE
:DIG2:HIST:PLAY:SPE
:DIG3:HIST:PLAY:SPE
:DIG4:HIST:PLAY:SPE
:DIG5:HIST:PLAY:SPE
:DIG6:HIST:PLAY:SPE
:DIG7:HIST:PLAY:SPE
:DIG8:HIST:PLAY:SPE
:DIG9:HIST:PLAY:SPE
:DIG10:HIST:PLAY:SPE
:DIG11:HIST:PLAY:SPE
:DIG12:HIST:PLAY:SPE
:DIG13:HIST:PLAY:SPE
:DIG14:HIST:PLAY:SPE
:DIG15:HIST:PLAY:SPE
:DIG0:HIST:REPL
:DIG1:HIST:REPL
:DIG2:HIST:REPL
:DIG3:HIST:REPL
:DIG4:HIST:REPL
:DIG5:HIST:REPL
:DIG6:HIST:REPL
:DIG7:HIST:REPL
:DIG8:HIST:REPL
:DIG9:HIST:REPL
:DIG10:HIST:REPL
:DIG11:HIST:REPL
:DIG12:HIST:REPL
:DIG13:HIST:REPL
:DIG14:HIST:REPL
:DIG15:HIST:REPL
:DIG0:HIST:PALL
:DIG1:HIST:PALL
:DIG2:HIST:PALL
:DIG3:HIST:PALL
:DIG4:HIST:PALL
:DIG5:HIST:PALL
:DIG6:HIST:PALL
:DIG7:HIST:PALL
:DIG8:HIST:PALL
:DIG9:HIST:PALL
:DIG10:HIST:PALL
:DIG11:HIST:PALL
:DIG12:HIST:PALL
:DIG13:HIST:PALL
:DIG14:HIST:PALL
:DIG15:HIST:PALL
:DIG0:HIST:CURR
:DIG1:HIST:CURR
:DIG2:HIST:CURR
:DIG3:HIST:CURR
:DIG4:HIST:CURR
:DIG5:HIST:CURR
:DIG6:HIST:CURR
:DIG7:HIST:CURR
:DIG8:HIST:CURR
:DIG9:HIST:CURR
:DIG10:HIST:CURR
:DIG11:HIST:CURR
:DIG12:HIST:CURR
:DIG13:HIST:CURR
:DIG14:HIST:CURR
:DIG15:HIST:CURR
:DIG0:HIST:STAR
:DIG1:HIST:STAR
:DIG2:HIST:STAR
:DIG3:HIST:STAR
:DIG4:HIST:STAR
:DIG5:HIST:STAR
:DIG6:HIST:STAR
:DIG7:HIST:STAR
:DIG8:HIST:STAR
:DIG9:HIST:STAR
:DIG10:HIST:STAR
:DIG11:HIST:STAR
:DIG12:HIST:STAR
:DIG13:HIST:STAR
:DIG14:HIST:STAR
:DIG15:HIST:STAR
:DIG0:HIST:STOP
:DIG1:HIST:STOP
:DIG2:HIST:STOP
:DIG3:HIST:STOP
:DIG4:HIST:STOP
:DIG5:HIST:STOP
:DIG6:HIST:STOP
:DIG7:HIST:STOP
:DIG8:HIST:STOP
:DIG9:HIST:STOP
:DIG10:HIST:STOP
:DIG11:HIST:STOP
:DIG12:HIST:STOP
:DIG13:HIST:STOP
:DIG14:HIST:STOP
:DIG15:HIST:STOP
:DIG0:HIST:TSR
:DIG1:HIST:TSR
:DIG2:HIST:TSR
:DIG3:HIST:TSR
:DIG4:HIST:TSR
:DIG5:HIST:TSR
:DIG6:HIST:TSR
:DIG7:HIST:TSR
:DIG8:HIST:TSR
:DIG9:HIST:TSR
:DIG10:HIST:TSR
:DIG11:HIST:TSR
:DIG12:HIST:TSR
:DIG13:HIST:TSR
:DIG14:HIST:TSR
:DIG15:HIST:TSR
:DIG0:HIST:TSR:ALL
:DIG1:HIST:TSR:ALL
:DIG2:HIST:TSR:ALL
:DIG3:HIST:TSR:ALL
:DIG4:HIST:TSR:ALL
:DIG5:HIST:TSR:ALL
:DIG6:HIST:TSR:ALL
:DIG7:HIST:TSR:ALL
:DIG8:HIST:TSR:ALL
:DIG9:HIST:TSR:ALL
:DIG10:HIST:TSR:ALL
:DIG11:HIST:TSR:ALL
:DIG12:HIST:TSR:ALL
:DIG13:HIST:TSR:ALL
:DIG14:HIST:TSR:ALL
:DIG15:HIST:TSR:ALL
:DIG0:HIST:TSAB
:DIG1:HIST:TSAB
:DIG2:HIST:TSAB
:DIG3:HIST:TSAB
:DIG4:HIST:TSAB
:DIG5:HIST:TSAB
:DIG6:HIST:TSAB
:DIG7:HIST:TSAB
:DIG8:HIST:TSAB
:DIG9:HIST:TSAB
:DIG10:HIST:TSAB
:DIG11:HIST:TSAB
:DIG12:HIST:TSAB
:DIG13:HIST:TSAB
:DIG14:HIST:TSAB
:DIG15:HIST:TSAB
:DIG0:HIST:TSAB:ALL
:DIG1:HIST:TSAB:ALL
:DIG2:HIST:TSAB:ALL
:DIG3:HIST:TSAB:ALL
:DIG4:HIST:TSAB:ALL
:DIG5:HIST:TSAB:ALL
:DIG6:HIST:TSAB:ALL
:DIG7:HIST:TSAB:ALL
:DIG8:HIST:TSAB:ALL
:DIG9:HIST:TSAB:ALL
:DIG10:HIST:TSAB:ALL
:DIG11:HIST:TSAB:ALL
:DIG12:HIST:TSAB:ALL
:DIG13:HIST:TSAB:ALL
:DIG14:HIST:TSAB:ALL
:DIG15:HIST:TSAB:ALL
:DIG0:HIST:TSD
:DIG1:HIST:TSD
:DIG2:HIST:TSD
:DIG3:HIST:TSD
:DIG4:HIST:TSD
:DIG5:HIST:TSD
:DIG6:HIST:TSD
:DIG7:HIST:TSD
:DIG8:HIST:TSD
:DIG9:HIST:TSD
:DIG10:HIST:TSD
:DIG11:HIST:TSD
:DIG12:HIST:TSD
:DIG13:HIST:TSD
:DIG14:HIST:TSD
:DIG15:HIST:TSD
:DIG0:HIST:TSD:ALL
:DIG1:HIST:TSD:ALL
:DIG2:HIST:TSD:ALL
:DIG3:HIST:TSD:ALL
:DIG4:HIST:TSD:ALL
:DIG5:HIST:TSD:ALL
:DIG6:HIST:TSD:ALL
:DIG7:HIST:TSD:ALL
:DIG8:HIST:TSD:ALL
:DIG9:HIST:TSD:ALL
:DIG10:HIST:TSD:ALL
:DIG11:HIST:TSD:ALL
:DIG12:HIST:TSD:ALL
:DIG13:HIST:TSD:ALL
:DIG14:HIST:TSD:ALL
:DIG15:HIST:TSD:ALL
:DIG0:HIST:ATIC
:DIG1:HIST:ATIC
:DIG2:HIST:ATIC
:DIG3:HIST:ATIC
:DIG4:HIST:ATIC
:DIG5:HIST:ATIC
:DIG6:HIST:ATIC
:DIG7:HIST:ATIC
:DIG8:HIST:ATIC
:DIG9:HIST:ATIC
:DIG10:HIST:ATIC
:DIG11:HIST:ATIC
:DIG12:HIST:ATIC
:DIG13:HIST:ATIC
:DIG14:HIST:ATIC
:DIG15:HIST:ATIC
:DIG0:HIST:ATIC:ALL
:DIG1:HIST:ATIC:ALL
:DIG2:HIST:ATIC:ALL
:DIG3:HIST:ATIC:ALL
:DIG4:HIST:ATIC:ALL
:DIG5:HIST:ATIC:ALL
:DIG6:HIST:ATIC:ALL
:DIG7:HIST:ATIC:ALL
:DIG8:HIST:ATIC:ALL
:DIG9:HIST:ATIC:ALL
:DIG10:HIST:ATIC:ALL
:DIG11:HIST:ATIC:ALL
:DIG12:HIST:ATIC:ALL
:DIG13:HIST:ATIC:ALL
:DIG14:HIST:ATIC:ALL
:DIG15:HIST:ATIC:ALL
:DIG0:HIST:DTIC
:DIG1:HIST:DTIC
:DIG2:HIST:DTIC
:DIG3:HIST:DTIC
:DIG4:HIST:DTIC
:DIG5:HIST:DTIC
:DIG6:HIST:DTIC
:DIG7:HIST:DTIC
:DIG8:HIST:DTIC
:DIG9:HIST:DTIC
:DIG10:HIST:DTIC
:DIG11:HIST:DTIC
:DIG12:HIST:DTIC
:DIG13:HIST:DTIC
:DIG14:HIST:DTIC
:DIG15:HIST:DTIC
:DIG0:HIST:DTIC:ALL
:DIG1:HIST:DTIC:ALL
:DIG2:HIST:DTIC:ALL
:DIG3:HIST:DTIC:ALL
:DIG4:HIST:DTIC:ALL
:DIG5:HIST:DTIC:ALL
:DIG6:HIST:DTIC:ALL
:DIG7:HIST:DTIC:ALL
:DIG8:HIST:DTIC:ALL
:DIG9:HIST:DTIC:ALL
:DIG10:HIST:DTIC:ALL
:DIG11:HIST:DTIC:ALL
:DIG12:HIST:DTIC:ALL
:DIG13:HIST:DTIC:ALL
:DIG14:HIST:DTIC:ALL
:DIG15:HIST:DTIC:ALL
:DIG0:HIST:TCL
:DIG1:HIST:TCL
:DIG2:HIST:TCL
:DIG3:HIST:TCL
:DIG4:HIST:TCL
:DIG5:HIST:TCL
:DIG6:HIST:TCL
:DIG7:HIST:TCL
:DIG8:HIST:TCL
:DIG9:HIST:TCL
:DIG10:HIST:TCL
:DIG11:HIST:TCL
:DIG12:HIST:TCL
:DIG13:HIST:TCL
:DIG14:HIST:TCL
:DIG15:HIST:TCL
:DIG0:HIST:TINC
:DIG1:HIST:TINC
:DIG2:HIST:TINC
:DIG3:HIST:TINC
:DIG4:HIST:TINC
:DIG5:HIST:TINC
:DIG6:HIST:TINC
:DIG7:HIST:TINC
:DIG8:HIST:TINC
:DIG9:HIST:TINC
:DIG10:HIST:TINC
:DIG11:HIST:TINC
:DIG12:HIST:TINC
:DIG13:HIST:TINC
:DIG14:HIST:TINC
:DIG15:HIST:TINC
:DIG0:HIST:TMOD
:DIG1:HIST:TMOD
:DIG2:HIST:TMOD
:DIG3:HIST:TMOD
:DIG4:HIST:TMOD
:DIG5:HIST:TMOD
:DIG6:HIST:TMOD
:DIG7:HIST:TMOD
:DIG8:HIST:TMOD
:DIG9:HIST:TMOD
:DIG10:HIST:TMOD
:DIG11:HIST:TMOD
:DIG12:HIST:TMOD
:DIG13:HIST:TMOD
:DIG14:HIST:TMOD
:DIG15:HIST:TMOD
:DIG0:HIST:EXP:NAME
:DIG1:HIST:EXP:NAME
:DIG2:HIST:EXP:NAME
:DIG3:HIST:EXP:NAME
:DIG4:HIST:EXP:NAME
:DIG5:HIST:EXP:NAME
:DIG6:HIST:EXP:NAME
:DIG7:HIST:EXP:NAME
:DIG8:HIST:EXP:NAME
:DIG9:HIST:EXP:NAME
:DIG10:HIST:EXP:NAME
:DIG11:HIST:EXP:NAME
:DIG12:HIST:EXP:NAME
:DIG13:HIST:EXP:NAME
:DIG14:HIST:EXP:NAME
:DIG15:HIST:EXP:NAME
:DIG0:HIST:EXP:SAVE
:DIG1:HIST:EXP:SAVE
:DIG2:HIST:EXP:SAVE
:DIG3:HIST:EXP:SAVE
:DIG4:HIST:EXP:SAVE
:DIG5:HIST:EXP:SAVE
:DIG6:HIST:EXP:SAVE
:DIG7:HIST:EXP:SAVE
:DIG8:HIST:EXP:SAVE
:DIG9:HIST:EXP:SAVE
:DIG10:HIST:EXP:SAVE
:DIG11:HIST:EXP:SAVE
:DIG12:HIST:EXP:SAVE
:DIG13:HIST:EXP:SAVE
:DIG14:HIST:EXP:SAVE
:DIG15:HIST:EXP:SAVE
:DIG0:HIST:TTAB
:DIG1:HIST:TTAB
:DIG2:HIST:TTAB
:DIG3:HIST:TTAB
:DIG4:HIST:TTAB
:DIG5:HIST:TTAB
:DIG6:HIST:TTAB
:DIG7:HIST:TTAB
:DIG8:HIST:TTAB
:DIG9:HIST:TTAB
:DIG10:HIST:TTAB
:DIG11:HIST:TTAB
:DIG12:HIST:TTAB
:DIG13:HIST:TTAB
:DIG14:HIST:TTAB
:DIG15:HIST:TTAB
:DIG0:HIST:TTAB:ENAB
:DIG1:HIST:TTAB:ENAB
:DIG2:HIST:TTAB:ENAB
:DIG3:HIST:TTAB:ENAB
:DIG4:HIST:TTAB:ENAB
:DIG5:HIST:TTAB:ENAB
:DIG6:HIST:TTAB:ENAB
:DIG7:HIST:TTAB:ENAB
:DIG8:HIST:TTAB:ENAB
:DIG9:HIST:TTAB:ENAB
:DIG10:HIST:TTAB:ENAB
:DIG11:HIST:TTAB:ENAB
:DIG12:HIST:TTAB:ENAB
:DIG13:HIST:TTAB:ENAB
:DIG14:HIST:TTAB:ENAB
:DIG15:HIST:TTAB:ENAB
:DIG0:HIST:CONT
:DIG1:HIST:CONT
:DIG2:HIST:CONT
:DIG3:HIST:CONT
:DIG4:HIST:CONT
:DIG5:HIST:CONT
:DIG6:HIST:CONT
:DIG7:HIST:CONT
:DIG8:HIST:CONT
:DIG9:HIST:CONT
:DIG10:HIST:CONT
:DIG11:HIST:CONT
:DIG12:HIST:CONT
:DIG13:HIST:CONT
:DIG14:HIST:CONT
:DIG15:HIST:CONT
:DIG0:HIST:CONT:ENAB
:DIG1:HIST:CONT:ENAB
:DIG2:HIST:CONT:ENAB
:DIG3:HIST:CONT:ENAB
:DIG4:HIST:CONT:ENAB
:DIG5:HIST:CONT:ENAB
:DIG6:HIST:CONT:ENAB
:DIG7:HIST:CONT:ENAB
:DIG8:HIST:CONT:ENAB
:DIG9:HIST:CONT:ENAB
:DIG10:HIST:CONT:ENAB
:DIG11:HIST:CONT:ENAB
:DIG12:HIST:CONT:ENAB
:DIG13:HIST:CONT:ENAB
:DIG14:HIST:CONT:ENAB
:DIG15:HIST:CONT:ENAB
:DIG0:CURR:STAT:MIN
:DIG1:CURR:STAT:MIN
:DIG2:CURR:STAT:MIN
:DIG3:CURR:STAT:MIN
:DIG4:CURR:STAT:MIN
:DIG5:CURR:STAT:MIN
:DIG6:CURR:STAT:MIN
:DIG7:CURR:STAT:MIN
:DIG8:CURR:STAT:MIN
:DIG9:CURR:STAT:MIN
:DIG10:CURR:STAT:MIN
:DIG11:CURR:STAT:MIN
:DIG12:CURR:STAT:MIN
:DIG13:CURR:STAT:MIN
:DIG14:CURR:STAT:MIN
:DIG15:CURR:STAT:MIN
:DIG0:CURR:STAT:MAX
:DIG1:CURR:STAT:MAX
:DIG2:CURR:STAT:MAX
:DIG3:CURR:STAT:MAX
:DIG4:CURR:STAT:MAX
:DIG5:CURR:STAT:MAX
:DIG6:CURR:STAT:MAX
:DIG7:CURR:STAT:MAX
:DIG8:CURR:STAT:MAX
:DIG9:CURR:STAT:MAX
:DIG10:CURR:STAT:MAX
:DIG11:CURR:STAT:MAX
:DIG12:CURR:STAT:MAX
:DIG13:CURR:STAT:MAX
:DIG14:CURR:STAT:MAX
:DIG15:CURR:STAT:MAX
:DIG0:PROB:ENAB
:DIG0:PROB2:ENAB
:DIG1:PROB:ENAB
:DIG1:PROB2:ENAB
:DIG2:PROB:ENAB
:DIG2:PROB2:ENAB
:DIG3:PROB:ENAB
:DIG3:PROB2:ENAB
:DIG4:PROB:ENAB
:DIG4:PROB2:ENAB
:DIG5:PROB:ENAB
:DIG5:PROB2:ENAB
:DIG6:PROB:ENAB
:DIG6:PROB2:ENAB
:DIG7:PROB:ENAB
:DIG7:PROB2:ENAB
:DIG8:PROB:ENAB
:DIG8:PROB2:ENAB
:DIG9:PROB:ENAB
:DIG9:PROB2:ENAB
:DIG10:PROB:ENAB
:DIG10:PROB2:ENAB
:DIG11:PROB:ENAB
:DIG11:PROB2:ENAB
:DIG12:PROB:ENAB
:DIG12:PROB2:ENAB
:DIG13:PROB:ENAB
:DIG13:PROB2:ENAB
:DIG14:PROB:ENAB
:DIG14:PROB2:ENAB
:DIG15:PROB:ENAB
:DIG15:PROB2:ENAB
:LOG:THR
:LOG2:THR
:LOG:THR:UDL
:LOG:THR:UDL2
:LOG2:THR:UDL
:LOG2:THR:UDL2
:LOG:HYST
:LOG2:HYST
:LOG:STAT
:LOG2:STAT
:LOG:TYPE
:LOG2:TYPE
:LOG:ARIT
:LOG2:ARIT
:LOG:DATA
:LOG2:DATA
:LOG:DATA:HEAD
:LOG2:DATA:HEAD
:LOG:DATA:POIN
:LOG2:DATA:POIN
:LOG:DATA:XINC
:LOG2:DATA:XINC
:LOG:DATA:XOR
:LOG2:DATA:XOR
:LOG:DATA:YINC
:LOG2:DATA:YINC
:LOG:DATA:YOR
:LOG2:DATA:YOR
:LOG:DATA:YRES
:LOG2:DATA:YRES
:LOG:HIST:PLAY:STAT
:LOG2:HIST:PLAY:STAT
:LOG:HIST:PLAY:SPE
:LOG2:HIST:PLAY:SPE
:LOG:HIST:REPL
:LOG2:HIST:REPL
:LOG:HIST:PALL
:LOG2:HIST:PALL
:LOG:HIST:CURR
:LOG2:HIST:CURR
:LOG:HIST:STAR
:LOG2:HIST:STAR
:LOG:HIST:STOP
:LOG2:HIST:STOP
:LOG:HIST:TSR
:LOG2:HIST:TSR
:LOG:HIST:TSR:ALL
:LOG2:HIST:TSR:ALL
:LOG:HIST:TSAB
:LOG2:HIST:TSAB
:LOG:HIST:TSAB:ALL
:LOG2:HIST:TSAB:ALL
:LOG:HIST:TSD
:LOG2:HIST:TSD
:LOG:HIST:TSD:ALL
:LOG2:HIST:TSD:ALL
:LOG:HIST:ATIC
:LOG2:HIST:ATIC
:LOG:HIST:ATIC:ALL
:LOG2:HIST:ATIC:ALL
:LOG:HIST:DTIC
:LOG2:HIST:DTIC
:LOG:HIST:DTIC:ALL
:LOG2:HIST:DTIC:ALL
:LOG:HIST:TCL
:LOG2:HIST:TCL
:LOG:HIST:TINC
:LOG2:HIST:TINC
:LOG:HIST:TMOD
:LOG2:HIST:TMOD
:LOG:HIST:EXP:NAME
:LOG2:HIST:EXP:NAME
:LOG:HIST:EXP:SAVE
:LOG2:HIST:EXP:SAVE
:LOG:HIST:TTAB
:LOG2:HIST:TTAB
:LOG:HIST:TTAB:ENAB
:LOG2:HIST:TTAB:ENAB
:LOG:HIST:CONT
:LOG2:HIST:CONT
:LOG:HIST:CONT:ENAB
:LOG2:HIST:CONT:ENAB
:LOG:CURR:STAT:MIN
:LOG2:CURR:STAT:MIN
:LOG:CURR:STAT:MAX
:LOG2:CURR:STAT:MAX
:LOG:AON
:LOG2:AON
:LOG:AOFF
:LOG2:AOFF
:LOG:PROB
:LOG2:PROB
:LOG:PROB:ENAB
:LOG2:PROB:ENAB
:ACQ:STAT
:ACQ:TYPE
:ACQ:AVER:COUN
:ACQ:AVER:COMP
:ACQ:AVER:RES
:ACQ:AVER:CURR
:ACQ:NSIN:COUN
:ACQ:REAL
:ACQ:PEAK
:ACQ:HRES
:ACQ:INT
:ACQ:MODE
:ACQ:SRAT
:ACQ:ZRAT
:ACQ:SRAT:ZOOM
:ACQ:POIN
:ACQ:POIN:ARAT
:ACQ:POIN:VAL
:ACQ:POIN:AUT
:ACQ:MEM
:ACQ:MEM:MODE
:RUN
:RUNC
:STOP
:SING
:RUNS
:ACQ:SEGM:STAT
:ACQ:SEGM:MAX
:ACQ:COUN
:ACQ:AVA
:ACQ:WRAT:MAX
:ACQ:ADC:RES
:TRIG:OUT:MODE
:TRIG:OUT:POL
:TRIG:OUT:PLEN
:TRIG:OUT:FREQ
:TRIG:A:TYPE
:TRIG:A:SOUR
:TRIG:A:FIND
:TRIG:A:MODE
:TRIG:A:LEV
:TRIG:A:LEV2
:TRIG:A:LEV3
:TRIG:A:LEV4
:TRIG:A:LEV5
:TRIG:A:LEV:VAL
:TRIG:A:LEV2:VAL
:TRIG:A:LEV3:VAL
:TRIG:A:LEV4:VAL
:TRIG:A:LEV5:VAL
:TRIG:A:HOLD:TIME
:TRIG:A:HOLD:MODE
:TRIG:A:EDGE:SLOP
:TRIG:A:EDGE:COUP
:TRIG:A:EDGE:FILT:HFR
:TRIG:A:EDGE:FILT:NREJ
:TRIG:A:TV:FIEL
:TRIG:A:TV:LINE
:TRIG:A:TV:POL
:TRIG:A:TV:STAN
:TRIG:A:WIDT:RANG
:TRIG:A:WIDT:POL
:TRIG:A:WIDT:WIDT
:TRIG:A:WIDT:DELT
:TRIG:A:I2C:AMOD
:TRIG:A:I2C:PLEN
:TRIG:A:I2C:POFF
:TRIG:A:I2C:ACC
:TRIG:A:I2C:MODE
:TRIG:A:I2C:PATT
:TRIG:A:I2C:ADDR
:TRIG:A:SPI:MODE
:TRIG:A:SPI:PLEN
:TRIG:A:SPI:POFF
:TRIG:A:SPI:PATT
:TRIG:A:UART:MODE
:TRIG:A:UART:PLEN
:TRIG:A:UART:POFF
:TRIG:A:UART:PATT
:TRIG:A:PATT:COND
:TRIG:A:PATT:FUNC
:TRIG:A:PATT:SOUR
:TRIG:A:PATT:MODE
:TRIG:A:PATT:WIDT
:TRIG:A:PATT:WIDT:WIDT
:TRIG:A:PATT:WIDT:RANG
:TRIG:A:PATT:WIDT:DELT
:TRIG:A:CAN:TYPE
:TRIG:A:CAN:FTYP
:TRIG:A:CAN:ITYP
:TRIG:A:CAN:ICON
:TRIG:A:CAN:IDEN
:TRIG:A:CAN:DCON
:TRIG:A:CAN:DATA
:TRIG:A:CAN:DLEN
:TRIG:A:CAN:DLC
:TRIG:A:CAN:ACK
:TRIG:A:CAN:CRC
:TRIG:A:CAN:FORM
:TRIG:A:CAN:BITS
:TRIG:A:LIN:TYPE
:TRIG:A:LIN:ICON
:TRIG:A:LIN:IDEN
:TRIG:A:LIN:DCON
:TRIG:A:LIN:DATA
:TRIG:A:LIN:DLEN
:TRIG:A:LIN:CHKS
:TRIG:A:LIN:IPER
:TRIG:A:LIN:SYER
:TRIG:A:TIM:RANG
:TRIG:A:TIM:TIME
:TRIG:EVEN
:TRIG:EVEN:ENAB
:TRIG:EVEN:SCRS
:TRIG:EVEN:SCRS:DEST
:TRIG:EVEN:WFMS
:TRIG:EVEN:WFMS:DEST
:TRIG:EVEN:REFS
:TRIG:EVEN:SOUN
:TRIG:EVEN:TRIG
:TRAC
:TRAC:SOUR
:TRAC:SOUR:CAT
:TRAC:DATA
:TRAC:POIN
:TRAC:TYPE
:TRAC:FORM
:TRAC:BORD
:TRAC:EXP:NAME
:TRAC:EXP:SAVE
:TRAC:YINC
:TRAC:YOR
:TRAC:YREF
:TRAC:XOR
:TRAC:XINC
:TRAC:XREF
:SYST:TEMP:ALL
:SYST:TEMP:SENS
:SYST:TEMP:SENS2
:SYST:TEMP:SENS3
:SYST:TEMP:SENS4
:SYST:TEMP:VAL
:SYST:TEMP:VAL2
:SYST:TEMP:VAL3
:SYST:TEMP:VAL4
:SYST:TEMP:HIS
:SYST:TEMP:HIS2
:SYST:TEMP:HIS3
:SYST:TEMP:HIS4
:SYST:PRES
*OPT
*TST
*CAL
:SYST:HIDK
:SYST:HIDM
:SYST:HIDT
:SYST:COMM:INT
:SYST:COMM:INT:SEL
:SYST:COMM:INT:USB:CLAS
:SYST:COMM:INT:ETH:DHCP
:SYST:COMM:INT:ETH:IPAD
:SYST:COMM:INT:ETH:SUB
:SYST:COMM:INT:ETH:GAT
:SYST:COMM:INT:ETH:IPP
:SYST:COMM:INT:ETH:HTTP
:SYST:COMM:INT:ETH:VXIP
:SYST:COMM:INT:ETH:TRAN
:SYST:COMM:INT:ETH:MAC
:SYST:COMM:INT:ETH:SAVE
:SYST:BEEP
:SYST:BEEP:CONT:STAT
:SYST:BEEP:ERR:STAT
:SYST:BEEP:TRIG:STAT
:SYST:BEEP:IMM
:SYST:EDUC:PRES
:SYST:DFPR
:SYST:CAL
:SYST:CAL:STAT
:SYST:CAL:ERR
:MMEM:CDIR
:MMEM:CAT
:MMEM:CAT:LENG
:MMEM:DCAT
:MMEM:DCAT:LENG
:MMEM:DRIV
:MMEM:COPY
:MMEM:MOVE
:MMEM:DEL
:MMEM:DATA
:MMEM:MDIR
:MMEM:RDIR
:MMEM:COMP
:MMEM:LOAD:STAT
:MMEM:STOR:STAT
:MMEM:MSIS
:MMEM:NAME
:MMEM:EXIS
:DIAG:SERV:SNUM
:DIAG:SERV:HWID
:DIAG:SERV:SWID
:DIAG:UPD:TRAN:OPEN
:DIAG:UPD:TRAN:CLOS
:DIAG:UPD:TRAN:DATA
:DIAG:UPD:INST
:DIAG:SERV:HCLT
:STAT:OPER
:STAT:OPER:COND
:STAT:OPER:PTR
:STAT:OPER:NTR
:STAT:OPER:EVEN
:STAT:OPER:ENAB
:STAT:QUES
:STAT:QUES:COND
:STAT:QUES:PTR
:STAT:QUES:NTR
:STAT:QUES:EVEN
:STAT:QUES:ENAB
:STAT:QUES:COV
:STAT:QUES:COV:COND
:STAT:QUES:COV:PTR
:STAT:QUES:COV:NTR
:STAT:QUES:COV:EVEN
:STAT:QUES:COV:ENAB
:STAT:QUES:TEMP
:STAT:QUES:TEMP:COND
:STAT:QUES:TEMP:PTR
:STAT:QUES:TEMP:NTR
:STAT:QUES:TEMP:EVEN
:STAT:QUES:TEMP:ENAB
:STAT:QUES:LIM
:STAT:QUES:LIM:COND
:STAT:QUES:LIM:PTR
:STAT:QUES:LIM:NTR
:STAT:QUES:LIM:EVEN
:STAT:QUES:LIM:ENAB
:STAT:QUES:MARG
:STAT:QUES:MARG:COND
:STAT:QUES:MARG:PTR
:STAT:QUES:MARG:NTR
:STAT:QUES:MARG:EVEN
:STAT:QUES:MARG:ENAB
:STAT:QUES:MASK
:STAT:QUES:MASK:COND
:STAT:QUES:MASK:PTR
:STAT:QUES:MASK:NTR
:STAT:QUES:MASK:EVEN
:STAT:QUES:MASK:ENAB
:STAT:QUES:ADCS
:STAT:QUES:ADCS:COND
:STAT:QUES:ADCS:PTR
:STAT:QUES:ADCS:NTR
:STAT:QUES:ADCS:EVEN
:STAT:QUES:ADCS:ENAB
:TST:SET
:TST:CLE
:TST:NEXT
:TST:PREV
:TST:ACL
:TIM:SCAL
:TIM:POS
:TIM:DIV
:TIM:REF
:TIM:ACQT
:TIM:RANG
:TIM:RAT
:TIM:ROLL:ENAB
:TIM:ROLL:AUT
:TIM:ROLL:MTIM
:TIM:ZOOM:SCAL
:TIM:ZOOM:TIME
:TIM:ZOOM:STAT
:TIM:ZOOM:POS
:CALC:MATH
:CALC:MATH2
:CALC:MATH3
:CALC:MATH4
:CALC:MATH5
:CALC:MATH:AVER:COUN
:CALC:MATH2:AVER:COUN
:CALC:MATH3:AVER:COUN
:CALC:MATH4:AVER:COUN
:CALC:MATH5:AVER:COUN
:CALC:MATH:AVER:COMP
:CALC:MATH2:AVER:COMP
:CALC:MATH3:AVER:COMP
:CALC:MATH4:AVER:COMP
:CALC:MATH5:AVER:COMP
:CALC:MATH:AVER:RES
:CALC:MATH2:AVER:RES
:CALC:MATH3:AVER:RES
:CALC:MATH4:AVER:RES
:CALC:MATH5:AVER:RES
:CALC:MATH:SCAL
:CALC:MATH2:SCAL
:CALC:MATH3:SCAL
:CALC:MATH4:SCAL
:CALC:MATH5:SCAL
:CALC:MATH:POS
:CALC:MATH2:POS
:CALC:MATH3:POS
:CALC:MATH4:POS
:CALC:MATH5:POS
:CALC:MATH:ARIT
:CALC:MATH2:ARIT
:CALC:MATH3:ARIT
:CALC:MATH4:ARIT
:CALC:MATH5:ARIT
:CALC:MATH:WCOL
:CALC:MATH2:WCOL
:CALC:MATH3:WCOL
:CALC:MATH4:WCOL
:CALC:MATH5:WCOL
:CALC:MATH:DATA
:CALC:MATH2:DATA
:CALC:MATH3:DATA
:CALC:MATH4:DATA
:CALC:MATH5:DATA
:CALC:MATH:DATA:POIN
:CALC:MATH2:DATA:POIN
:CALC:MATH3:DATA:POIN
:CALC:MATH4:DATA:POIN
:CALC:MATH5:DATA:POIN
:CALC:MATH:DATA:HEAD
:CALC:MATH2:DATA:HEAD
:CALC:MATH3:DATA:HEAD
:CALC:MATH4:DATA:HEAD
:CALC:MATH5:DATA:HEAD
:CALC:MATH:DATA:XINC
:CALC:MATH2:DATA:XINC
:CALC:MATH3:DATA:XINC
:CALC:MATH4:DATA:XINC
:CALC:MATH5:DATA:XINC
:CALC:MATH:DATA:XOR
:CALC:MATH2:DATA:XOR
:CALC:MATH3:DATA:XOR
:CALC:MATH4:DATA:XOR
:CALC:MATH5:DATA:XOR
:CALC:MATH:DATA:YINC
:CALC:MATH2:DATA:YINC
:CALC:MATH3:DATA:YINC
:CALC:MATH4:DATA:YINC
:CALC:MATH5:DATA:YINC
:CALC:MATH:DATA:YOR
:CALC:MATH2:DATA:YOR
:CALC:MATH3:DATA:YOR
:CALC:MATH4:DATA:YOR
:CALC:MATH5:DATA:YOR
:CALC:MATH:DATA:YRES
:CALC:MATH2:DATA:YRES
:CALC:MATH3:DATA:YRES
:CALC:MATH4:DATA:YRES
:CALC:MATH5:DATA:YRES
:CALC:MATH:DATA:ENV
:CALC:MATH2:DATA:ENV
:CALC:MATH3:DATA:ENV
:CALC:MATH4:DATA:ENV
:CALC:MATH5:DATA:ENV
:CALC:MATH:DATA:ENV:POIN
:CALC:MATH2:DATA:ENV:POIN
:CALC:MATH3:DATA:ENV:POIN
:CALC:MATH4:DATA:ENV:POIN
:CALC:MATH5:DATA:ENV:POIN
:CALC:MATH:DATA:ENV:HEAD
:CALC:MATH2:DATA:ENV:HEAD
:CALC:MATH3:DATA:ENV:HEAD
:CALC:MATH4:DATA:ENV:HEAD
:CALC:MATH5:DATA:ENV:HEAD
:CALC:MATH:DATA:ENV:XINC
:CALC:MATH2:DATA:ENV:XINC
:CALC:MATH3:DATA:ENV:XINC
:CALC:MATH4:DATA:ENV:XINC
:CALC:MATH5:DATA:ENV:XINC
:CALC:MATH:DATA:ENV:XOR
:CALC:MATH2:DATA:ENV:XOR
:CALC:MATH3:DATA:ENV:XOR
:CALC:MATH4:DATA:ENV:XOR
:CALC:MATH5:DATA:ENV:XOR
:CALC:MATH:DATA:ENV:YINC
:CALC:MATH2:DATA:ENV:YINC
:CALC:MATH3:DATA:ENV:YINC
:CALC:MATH4:DATA:ENV:YINC
:CALC:MATH5:DATA:ENV:YINC
:CALC:MATH:DATA:ENV:YOR
:CALC:MATH2:DATA:ENV:YOR
:CALC:MATH3:DATA:ENV:YOR
:CALC:MATH4:DATA:ENV:YOR
:CALC:MATH5:DATA:ENV:YOR
:CALC:MATH:DATA:ENV:YRES
:CALC:MATH2:DATA:ENV:YRES
:CALC:MATH3:DATA:ENV:YRES
:CALC:MATH4:DATA:ENV:YRES
:CALC:MATH5:DATA:ENV:YRES
:CALC:MATH:STAT
:CALC:MATH2:STAT
:CALC:MATH3:STAT
:CALC:MATH4:STAT
:CALC:MATH5:STAT
:CALC:MATH:EXPR
:CALC:MATH2:EXPR
:CALC:MATH3:EXPR
:CALC:MATH4:EXPR
:CALC:MATH5:EXPR
:CALC:MATH:DEF
:CALC:MATH2:DEF
:CALC:MATH3:DEF
:CALC:MATH4:DEF
:CALC:MATH5:DEF
:CALC:MATH:LAB
:CALC:MATH2:LAB
:CALC:MATH3:LAB
:CALC:MATH4:LAB
:CALC:MATH5:LAB
:CALC:MATH:LAB:STAT
:CALC:MATH2:LAB:STAT
:CALC:MATH3:LAB:STAT
:CALC:MATH4:LAB:STAT
:CALC:MATH5:LAB:STAT
:CALC:MATH:HIST:PLAY:STAT
:CALC:MATH2:HIST:PLAY:STAT
:CALC:MATH3:HIST:PLAY:STAT
:CALC:MATH4:HIST:PLAY:STAT
:CALC:MATH5:HIST:PLAY:STAT
:CALC:MATH:HIST:PLAY:SPE
:CALC:MATH2:HIST:PLAY:SPE
:CALC:MATH3:HIST:PLAY:SPE
:CALC:MATH4:HIST:PLAY:SPE
:CALC:MATH5:HIST:PLAY:SPE
:CALC:MATH:HIST:REPL
:CALC:MATH2:HIST:REPL
:CALC:MATH3:HIST:REPL
:CALC:MATH4:HIST:REPL
:CALC:MATH5:HIST:REPL
:CALC:MATH:HIST:PALL
:CALC:MATH2:HIST:PALL
:CALC:MATH3:HIST:PALL
:CALC:MATH4:HIST:PALL
:CALC:MATH5:HIST:PALL
:CALC:MATH:HIST:CURR
:CALC:MATH2:HIST:CURR
:CALC:MATH3:HIST:CURR
:CALC:MATH4:HIST:CURR
:CALC:MATH5:HIST:CURR
:CALC:MATH:HIST:STAR
:CALC:MATH2:HIST:STAR
:CALC:MATH3:HIST:STAR
:CALC:MATH4:HIST:STAR
:CALC:MATH5:HIST:STAR
:CALC:MATH:HIST:STOP
:CALC:MATH2:HIST:STOP
:CALC:MATH3:HIST:STOP
:CALC:MATH4:HIST:STOP
:CALC:MATH5:HIST:STOP
:CALC:MATH:HIST:TSR
:CALC:MATH2:HIST:TSR
:CALC:MATH3:HIST:TSR
:CALC:MATH4:HIST:TSR
:CALC:MATH5:HIST:TSR
:CALC:MATH:HIST:TSR:ALL
:CALC:MATH2:HIST:TSR:ALL
:CALC:MATH3:HIST:TSR:ALL
:CALC:MATH4:HIST:TSR:ALL
:CALC:MATH5:HIST:TSR:ALL
:CALC:MATH:HIST:TSAB
:CALC:MATH2:HIST:TSAB
:CALC:MATH3:HIST:TSAB
:CALC:MATH4:HIST:TSAB
:CALC:MATH5:HIST:TSAB
:CALC:MATH:HIST:TSAB:ALL
:CALC:MATH2:HIST:TSAB:ALL
:CALC:MATH3:HIST:TSAB:ALL
:CALC:MATH4:HIST:TSAB:ALL
:CALC:MATH5:HIST:TSAB:ALL
:CALC:MATH:HIST:TSD
:CALC:MATH2:HIST:TSD
:CALC:MATH3:HIST:TSD
:CALC:MATH4:HIST:TSD
:CALC:MATH5:HIST:TSD
:CALC:MATH:HIST:TSD:ALL
:CALC:MATH2:HIST:TSD:ALL
:CALC:MATH3:HIST:TSD:ALL
:CALC:MATH4:HIST:TSD:ALL
:CALC:MATH5:HIST:TSD:ALL
:CALC:MATH:HIST:ATIC
:CALC:MATH2:HIST:ATIC
:CALC:MATH3:HIST:ATIC
:CALC:MATH4:HIST:ATIC
:CALC:MATH5:HIST:ATIC
:CALC:MATH:HIST:ATIC:ALL
:CALC:MATH2:HIST:ATIC:ALL
:CALC:MATH3:HIST:ATIC:ALL
:CALC:MATH4:HIST:ATIC:ALL
:CALC:MATH5:HIST:ATIC:ALL
:CALC:MATH:HIST:DTIC
:CALC:MATH2:HIST:DTIC
:CALC:MATH3:HIST:DTIC
:CALC:MATH4:HIST:DTIC
:CALC:MATH5:HIST:DTIC
:CALC:MATH:HIST:DTIC:ALL
:CALC:MATH2:HIST:DTIC:ALL
:CALC:MATH3:HIST:DTIC:ALL
:CALC:MATH4:HIST:DTIC:ALL
:CALC:MATH5:HIST:DTIC:ALL
:CALC:MATH:HIST:TCL
:CALC:MATH2:HIST:TCL
:CALC:MATH3:HIST:TCL
:CALC:MATH4:HIST:TCL
:CALC:MATH5:HIST:TCL
:CALC:MATH:HIST:TINC
:CALC:MATH2:HIST:TINC
:CALC:MATH3:HIST:TINC
:CALC:MATH4:HIST:TINC
:CALC:MATH5:HIST:TINC
:CALC:MATH:HIST:TMOD
:CALC:MATH2:HIST:TMOD
:CALC:MATH3:HIST:TMOD
:CALC:MATH4:HIST:TMOD
:CALC:MATH5:HIST:TMOD
:CALC:MATH:HIST:EXP:NAME
:CALC:MATH2:HIST:EXP:NAME
:CALC:MATH3:HIST:EXP:NAME
:CALC:MATH4:HIST:EXP:NAME
:CALC:MATH5:HIST:EXP:NAME
:CALC:MATH:HIST:EXP:SAVE
:CALC:MATH2:HIST:EXP:SAVE
:CALC:MATH3:HIST:EXP:SAVE
:CALC:MATH4:HIST:EXP:SAVE
:CALC:MATH5:HIST:EXP:SAVE
:CALC:MATH:HIST:TTAB
:CALC:MATH2:HIST:TTAB
:CALC:MATH3:HIST:TTAB
:CALC:MATH4:HIST:TTAB
:CALC:MATH5:HIST:TTAB
:CALC:MATH:HIST:TTAB:ENAB
:CALC:MATH2:HIST:TTAB:ENAB
:CALC:MATH3:HIST:TTAB:ENAB
:CALC:MATH4:HIST:TTAB:ENAB
:CALC:MATH5:HIST:TTAB:ENAB
:CALC:MATH:HIST:CONT
:CALC:MATH2:HIST:CONT
:CALC:MATH3:HIST:CONT
:CALC:MATH4:HIST:CONT
:CALC:MATH5:HIST:CONT
:CALC:MATH:HIST:CONT:ENAB
:CALC:MATH2:HIST:CONT:ENAB
:CALC:MATH3:HIST:CONT:ENAB
:CALC:MATH4:HIST:CONT:ENAB
:CALC:MATH5:HIST:CONT:ENAB
:CALC:MATH:FFT:STAR
:CALC:MATH2:FFT:STAR
:CALC:MATH3:FFT:STAR
:CALC:MATH4:FFT:STAR
:CALC:MATH5:FFT:STAR
:CALC:MATH:FFT:STOP
:CALC:MATH2:FFT:STOP
:CALC:MATH3:FFT:STOP
:CALC:MATH4:FFT:STOP
:CALC:MATH5:FFT:STOP
:CALC:MATH:FFT:CFR
:CALC:MATH2:FFT:CFR
:CALC:MATH3:FFT:CFR
:CALC:MATH4:FFT:CFR
:CALC:MATH5:FFT:CFR
:CALC:MATH:FFT:SPAN
:CALC:MATH2:FFT:SPAN
:CALC:MATH3:FFT:SPAN
:CALC:MATH4:FFT:SPAN
:CALC:MATH5:FFT:SPAN
:CALC:MATH:FFT:FULL
:CALC:MATH2:FFT:FULL
:CALC:MATH3:FFT:FULL
:CALC:MATH4:FFT:FULL
:CALC:MATH5:FFT:FULL
:CALC:MATH:FFT:WIND:TYPE
:CALC:MATH2:FFT:WIND:TYPE
:CALC:MATH3:FFT:WIND:TYPE
:CALC:MATH4:FFT:WIND:TYPE
:CALC:MATH5:FFT:WIND:TYPE
:CALC:MATH:FFT:SRAT
:CALC:MATH2:FFT:SRAT
:CALC:MATH3:FFT:SRAT
:CALC:MATH4:FFT:SRAT
:CALC:MATH5:FFT:SRAT
:CALC:MATH:FFT:BAND
:CALC:MATH2:FFT:BAND
:CALC:MATH3:FFT:BAND
:CALC:MATH4:FFT:BAND
:CALC:MATH5:FFT:BAND
:CALC:MATH:FFT:BAND:RES
:CALC:MATH2:FFT:BAND:RES
:CALC:MATH3:FFT:BAND:RES
:CALC:MATH4:FFT:BAND:RES
:CALC:MATH5:FFT:BAND:RES
:CALC:MATH:FFT:BAND:ADJ
:CALC:MATH2:FFT:BAND:ADJ
:CALC:MATH3:FFT:BAND:ADJ
:CALC:MATH4:FFT:BAND:ADJ
:CALC:MATH5:FFT:BAND:ADJ
:CALC:MATH:FFT:BAND:AUTO
:CALC:MATH2:FFT:BAND:AUTO
:CALC:MATH3:FFT:BAND:AUTO
:CALC:MATH4:FFT:BAND:AUTO
:CALC:MATH5:FFT:BAND:AUTO
:CALC:MATH:FFT:BAND:RAT
:CALC:MATH2:FFT:BAND:RAT
:CALC:MATH3:FFT:BAND:RAT
:CALC:MATH4:FFT:BAND:RAT
:CALC:MATH5:FFT:BAND:RAT
:CALC:MATH:FFT:BAND:VAL
:CALC:MATH2:FFT:BAND:VAL
:CALC:MATH3:FFT:BAND:VAL
:CALC:MATH4:FFT:BAND:VAL
:CALC:MATH5:FFT:BAND:VAL
:CALC:MATH:FFT:MAGN:SCAL
:CALC:MATH2:FFT:MAGN:SCAL
:CALC:MATH3:FFT:MAGN:SCAL
:CALC:MATH4:FFT:MAGN:SCAL
:CALC:MATH5:FFT:MAGN:SCAL
:CALC:MATH:FFT:AVER:COUN
:CALC:MATH2:FFT:AVER:COUN
:CALC:MATH3:FFT:AVER:COUN
:CALC:MATH4:FFT:AVER:COUN
:CALC:MATH5:FFT:AVER:COUN
:CALC:MATH:FFT:AVER:COMP
:CALC:MATH2:FFT:AVER:COMP
:CALC:MATH3:FFT:AVER:COMP
:CALC:MATH4:FFT:AVER:COMP
:CALC:MATH5:FFT:AVER:COMP
:CALC:MATH:FFT:AVER:RES
:CALC:MATH2:FFT:AVER:RES
:CALC:MATH3:FFT:AVER:RES
:CALC:MATH4:FFT:AVER:RES
:CALC:MATH5:FFT:AVER:RES
:CALC:MATH:FFT:TIME:RANG
:CALC:MATH2:FFT:TIME:RANG
:CALC:MATH3:FFT:TIME:RANG
:CALC:MATH4:FFT:TIME:RANG
:CALC:MATH5:FFT:TIME:RANG
:CALC:MATH:FFT:TIME:POS
:CALC:MATH2:FFT:TIME:POS
:CALC:MATH3:FFT:TIME:POS
:CALC:MATH4:FFT:TIME:POS
:CALC:MATH5:FFT:TIME:POS
:MASK:STAT
:MASK:COUN
:MASK:VCO
:MASK:TEST
:MASK:SOUR
:MASK:SAVE
:MASK:LOAD
:MASK:CHC
:MASK:YPOS
:MASK:YSC
:MASK:XWID
:MASK:YWID
:MASK:RES:COUN
:MASK:TTIM
:MASK:CAPT
:MASK:CAPT:MODE
:MASK:DATA
:MASK:DATA:HEAD
:MASK:DATA:XINC
:MASK:DATA:XOR
:MASK:DATA:YINC
:MASK:DATA:YOR
:MASK:DATA:YRES
:MASK:ACT:YOUT:ENAB
:MASK:ACT:SOUN:EVEN:MODE
:MASK:ACT:SOUN:EVEN:COUN
:MASK:ACT:STOP:EVEN:MODE
:MASK:ACT:STOP:EVEN:COUN
:MASK:ACT:SCRS:EVEN:MODE
:MASK:ACT:SCRS:EVEN:COUN
:MASK:ACT:SCRS:DEST
:MASK:ACT:WFMS:EVEN:MODE
:MASK:ACT:WFMS:EVEN:COUN
:MASK:ACT:WFMS:DEST
:MASK:ACT:PULS:EVEN:MODE
:MASK:ACT:PULS:EVEN:COUN
:MASK:ACT:PULS:POL
:MASK:ACT:PULS:PLEN
:BUS:STAT
:BUS2:STAT
:BUS:TYPE
:BUS2:TYPE
:BUS:POS
:BUS2:POS
:BUS:DSIG
:BUS2:DSIG
:BUS:FORM
:BUS2:FORM
:BUS:DSIZ
:BUS2:DSIZ
:BUS:RES
:BUS2:RES
:BUS:LAB
:BUS2:LAB
:BUS:LAB:STAT
:BUS2:LAB:STAT
:BUS:HIST:PLAY:STAT
:BUS2:HIST:PLAY:STAT
:BUS:HIST:PLAY:SPE
:BUS2:HIST:PLAY:SPE
:BUS:HIST:REPL
:BUS2:HIST:REPL
:BUS:HIST:PALL
:BUS2:HIST:PALL
:BUS:HIST:CURR
:BUS2:HIST:CURR
:BUS:HIST:STAR
:BUS2:HIST:STAR
:BUS:HIST:STOP
:BUS2:HIST:STOP
:BUS:HIST:TSR
:BUS2:HIST:TSR
:BUS:HIST:TSR:ALL
:BUS2:HIST:TSR:ALL
:BUS:HIST:TSAB
:BUS2:HIST:TSAB
:BUS:HIST:TSAB:ALL
:BUS2:HIST:TSAB:ALL
:BUS:HIST:TSD
:BUS2:HIST:TSD
:BUS:HIST:TSD:ALL
:BUS2:HIST:TSD:ALL
:BUS:HIST:ATIC
:BUS2:HIST:ATIC
:BUS:HIST:ATIC:ALL
:BUS2:HIST:ATIC:ALL
:BUS:HIST:DTIC
:BUS2:HIST:DTIC
:BUS:HIST:DTIC:ALL
:BUS2:HIST:DTIC:ALL
:BUS:HIST:TCL
:BUS2:HIST:TCL
:BUS:HIST:TINC
:BUS2:HIST:TINC
:BUS:HIST:TMOD
:BUS2:HIST:TMOD
:BUS:HIST:EXP:NAME
:BUS2:HIST:EXP:NAME
:BUS:HIST:EXP:SAVE
:BUS2:HIST:EXP:SAVE
:BUS:HIST:TTAB
:BUS2:HIST:TTAB
:BUS:HIST:TTAB:ENAB
:BUS2:HIST:TTAB:ENAB
:BUS:HIST:CONT
:BUS2:HIST:CONT
:BUS:HIST:CONT:ENAB
:BUS2:HIST:CONT:ENAB
:BUS:I2C:CLOC:SOUR
:BUS2:I2C:CLOC:SOUR
:BUS:I2C:DATA:SOUR
:BUS2:I2C:DATA:SOUR
:BUS:I2C:AMOD
:BUS2:I2C:AMOD
:BUS:I2C:FCO
:BUS2:I2C:FCO
:BUS:SPI:CLOC:SOUR
:BUS2:SPI:CLOC:SOUR
:BUS:SPI:CLOC:POL
:BUS2:SPI:CLOC:POL
:BUS:SPI:DATA:SOUR
:BUS2:SPI:DATA:SOUR
:BUS:SPI:DATA:POL
:BUS2:SPI:DATA:POL
:BUS:SPI:MOSI:SOUR
:BUS2:SPI:MOSI:SOUR
:BUS:SPI:MOSI:POL
:BUS2:SPI:MOSI:POL
:BUS:SPI:MISO:SOUR
:BUS2:SPI:MISO:SOUR
:BUS:SPI:MISO:POL
:BUS2:SPI:MISO:POL
:BUS:SPI:CS:SOUR
:BUS2:SPI:CS:SOUR
:BUS:SPI:CS:POL
:BUS2:SPI:CS:POL
:BUS:SPI:BORD
:BUS2:SPI:BORD
:BUS:SPI:SSIZ
:BUS2:SPI:SSIZ
:BUS:SPI:FCO
:BUS2:SPI:FCO
:BUS:SSPI:CLOC:SOUR
:BUS2:SSPI:CLOC:SOUR
:BUS:SSPI:CLOC:POL
:BUS2:SSPI:CLOC:POL
:BUS:SSPI:DATA:SOUR
:BUS2:SSPI:DATA:SOUR
:BUS:SSPI:DATA:POL
:BUS2:SSPI:DATA:POL
:BUS:SSPI:MOSI:SOUR
:BUS2:SSPI:MOSI:SOUR
:BUS:SSPI:MOSI:POL
:BUS2:SSPI:MOSI:POL
:BUS:SSPI:MISO:SOUR
:BUS2:SSPI:MISO:SOUR
:BUS:SSPI:MISO:POL
:BUS2:SSPI:MISO:POL
:BUS:SSPI:BORD
:BUS2:SSPI:BORD
:BUS:SSPI:SSIZ
:BUS2:SSPI:SSIZ
:BUS:SSPI:BIT
:BUS2:SSPI:BIT
:BUS:UART:SSIZ
:BUS2:UART:SSIZ
:BUS:UART:PAR
:BUS2:UART:PAR
:BUS:UART:BIT
:BUS2:UART:BIT
:BUS:UART:BAUD
:BUS2:UART:BAUD
:BUS:UART:SBIT
:BUS2:UART:SBIT
:BUS:UART:DATA:POL
:BUS2:UART:DATA:POL
:BUS:UART:DATA:SOUR
:BUS2:UART:DATA:SOUR
:BUS:UART:RX:SOUR
:BUS2:UART:RX:SOUR
:BUS:UART:TX:SOUR
:BUS2:UART:TX:SOUR
:BUS:UART:POL
:BUS2:UART:POL
:BUS:UART:RX:FCO
:BUS2:UART:RX:FCO
:BUS:UART:TX:FCO
:BUS2:UART:TX:FCO
:BUS:UART:FCO
:BUS2:UART:FCO
:BUS:PAR:DATA0:SOUR
:BUS:PAR:DATA1:SOUR
:BUS:PAR:DATA2:SOUR
:BUS:PAR:DATA3:SOUR
:BUS:PAR:DATA4:SOUR
:BUS:PAR:DATA5:SOUR
:BUS:PAR:DATA6:SOUR
:BUS:PAR:DATA7:SOUR
:BUS:PAR:DATA8:SOUR
:BUS:PAR:DATA9:SOUR
:BUS:PAR:DATA10:SOUR
:BUS:PAR:DATA11:SOUR
:BUS:PAR:DATA12:SOUR
:BUS:PAR:DATA13:SOUR
:BUS:PAR:DATA14:SOUR
:BUS:PAR:DATA15:SOUR
:BUS2:PAR:DATA0:SOUR
:BUS2:PAR:DATA1:SOUR
:BUS2:PAR:DATA2:SOUR
:BUS2:PAR:DATA3:SOUR
:BUS2:PAR:DATA4:SOUR
:BUS2:PAR:DATA5:SOUR
:BUS2:PAR:DATA6:SOUR
:BUS2:PAR:DATA7:SOUR
:BUS2:PAR:DATA8:SOUR
:BUS2:PAR:DATA9:SOUR
:BUS2:PAR:DATA10:SOUR
:BUS2:PAR:DATA11:SOUR
:BUS2:PAR:DATA12:SOUR
:BUS2:PAR:DATA13:SOUR
:BUS2:PAR:DATA14:SOUR
:BUS2:PAR:DATA15:SOUR
:BUS:PAR:WIDT
:BUS2:PAR:WIDT
:BUS:PAR:FCO
:BUS2:PAR:FCO
:BUS:CPAR:DATA0:SOUR
:BUS:CPAR:DATA1:SOUR
:BUS:CPAR:DATA2:SOUR
:BUS:CPAR:DATA3:SOUR
:BUS:CPAR:DATA4:SOUR
:BUS:CPAR:DATA5:SOUR
:BUS:CPAR:DATA6:SOUR
:BUS:CPAR:DATA7:SOUR
:BUS:CPAR:DATA8:SOUR
:BUS:CPAR:DATA9:SOUR
:BUS:CPAR:DATA10:SOUR
:BUS:CPAR:DATA11:SOUR
:BUS:CPAR:DATA12:SOUR
:BUS:CPAR:DATA13:SOUR
:BUS:CPAR:DATA14:SOUR
:BUS:CPAR:DATA15:SOUR
:BUS2:CPAR:DATA0:SOUR
:BUS2:CPAR:DATA1:SOUR
:BUS2:CPAR:DATA2:SOUR
:BUS2:CPAR:DATA3:SOUR
:BUS2:CPAR:DATA4:SOUR
:BUS2:CPAR:DATA5:SOUR
:BUS2:CPAR:DATA6:SOUR
:BUS2:CPAR:DATA7:SOUR
:BUS2:CPAR:DATA8:SOUR
:BUS2:CPAR:DATA9:SOUR
:BUS2:CPAR:DATA10:SOUR
:BUS2:CPAR:DATA11:SOUR
:BUS2:CPAR:DATA12:SOUR
:BUS2:CPAR:DATA13:SOUR
:BUS2:CPAR:DATA14:SOUR
:BUS2:CPAR:DATA15:SOUR
:BUS:CPAR:WIDT
:BUS2:CPAR:WIDT
:BUS:CPAR:CLOC:SOUR
:BUS2:CPAR:CLOC:SOUR
:BUS:CPAR:CLOC:SLOP
:BUS2:CPAR:CLOC:SLOP
:BUS:CPAR:CS:SOUR
:BUS2:CPAR:CS:SOUR
:BUS:CPAR:CS:POL
:BUS2:CPAR:CS:POL
:BUS:CPAR:FCO
:BUS2:CPAR:FCO
:BUS:CAN:TYPE
:BUS2:CAN:TYPE
:BUS:CAN:SAMP
:BUS2:CAN:SAMP
:BUS:CAN:BITR
:BUS2:CAN:BITR
:BUS:CAN:DATA:SOUR
:BUS2:CAN:DATA:SOUR
:BUS:CAN:FCO
:BUS2:CAN:FCO
:BUS:LIN:BITR
:BUS2:LIN:BITR
:BUS:LIN:POL
:BUS2:LIN:POL
:BUS:LIN:STAN
:BUS2:LIN:STAN
:BUS:LIN:DATA:SOUR
:BUS2:LIN:DATA:SOUR
:BUS:LIN:FCO
:BUS2:LIN:FCO
:REFC:STAT
:REFC2:STAT
:REFC3:STAT
:REFC4:STAT
:REFC:LOAD
:REFC2:LOAD
:REFC3:LOAD
:REFC4:LOAD
:REFC:LOAD:STAT
:REFC2:LOAD:STAT
:REFC3:LOAD:STAT
:REFC4:LOAD:STAT
:REFC:SAVE
:REFC2:SAVE
:REFC3:SAVE
:REFC4:SAVE
:REFC:SOUR
:REFC2:SOUR
:REFC3:SOUR
:REFC4:SOUR
:REFC:SOUR:CAT
:REFC2:SOUR:CAT
:REFC3:SOUR:CAT
:REFC4:SOUR:CAT
:REFC:UPD
:REFC2:UPD
:REFC3:UPD
:REFC4:UPD
:REFC:ASC
:REFC2:ASC
:REFC3:ASC
:REFC4:ASC
:REFC:VERT:SCAL
:REFC2:VERT:SCAL
:REFC3:VERT:SCAL
:REFC4:VERT:SCAL
:REFC:VERT:POS
:REFC2:VERT:POS
:REFC3:VERT:POS
:REFC4:VERT:POS
:REFC:HOR:SCAL
:REFC2:HOR:SCAL
:REFC3:HOR:SCAL
:REFC4:HOR:SCAL
:REFC:HOR:POS
:REFC2:HOR:POS
:REFC3:HOR:POS
:REFC4:HOR:POS
:REFC:DATA
:REFC2:DATA
:REFC3:DATA
:REFC4:DATA
:REFC:DATA:POIN
:REFC2:DATA:POIN
:REFC3:DATA:POIN
:REFC4:DATA:POIN
:REFC:DATA:HEAD
:REFC2:DATA:HEAD
:REFC3:DATA:HEAD
:REFC4:DATA:HEAD
:REFC:DATA:XINC
:REFC2:DATA:XINC
:REFC3:DATA:XINC
:REFC4:DATA:XINC
:REFC:DATA:XOR
:REFC2:DATA:XOR
:REFC3:DATA:XOR
:REFC4:DATA:XOR
:REFC:DATA:YINC
:REFC2:DATA:YINC
:REFC3:DATA:YINC
:REFC4:DATA:YINC
:REFC:DATA:YOR
:REFC2:DATA:YOR
:REFC3:DATA:YOR
:REFC4:DATA:YOR
:REFC:DATA:YRES
:REFC2:DATA:YRES
:REFC3:DATA:YRES
:REFC4:DATA:YRES
:REFC:WCOL
:REFC2:WCOL
:REFC3:WCOL
:REFC4:WCOL
:REFC:LAB
:REFC2:LAB
:REFC3:LAB
:REFC4:LAB
:CURS:STAT
:CURS:FUNC
:CURS:TRAC
:CURS:TRAC:STAT
:CURS:TRAC:SCAL
:CURS:TRAC:SCAL:STAT
:CURS:XCO
:CURS:YCO
:CURS:X1P
:CURS:X1P:INCR
:CURS:X2P
:CURS:X2P:INCR
:CURS:X3P
:CURS:X3P:INCR
:CURS:X4P
:CURS:X4P:INCR
:CURS:Y1P
:CURS:Y1P:INCR
:CURS:Y2P
:CURS:Y2P:INCR
:CURS:Y3P
:CURS:Y3P:INCR
:CURS:Y4P
:CURS:Y4P:INCR
:CURS:XDEL
:CURS:XDEL:VAL
:CURS:XDEL:INV
:CURS:YDEL
:CURS:YDEL:VAL
:CURS:YDEL:SLOP
:CURS:RES
:CURS:SOUR
:CURS:AOFF
:CURS:SSCR
:CURS:SWAV
:CURS:SNP
:CURS:SNP2
:CURS:SNP3
:CURS:SNP4
:CURS:SPP
:CURS:SPP2
:CURS:SPP3
:CURS:SPP4
:CURS:SPX
:SOUR:FUNC
:SOUR:FUNC:SHAP
:SOUR:FREQ
:REFL:REL:MODE
:REFL:REL:LOW
:REFL:REL:UPP
:REFL:REL:MIDD
:PROB:SET:MODE
:PROB2:SET:MODE
:PROB3:SET:MODE
:PROB4:SET:MODE
:PROB:SET:UOFF
:PROB2:SET:UOFF
:PROB3:SET:UOFF
:PROB4:SET:UOFF
:PROB:SET:CMOF
:PROB2:SET:CMOF
:PROB3:SET:CMOF
:PROB4:SET:CMOF
:PROB:SET:ATT
:PROB2:SET:ATT
:PROB3:SET:ATT
:PROB4:SET:ATT
:PROB:SET:ATT:AUTO
:PROB2:SET:ATT:AUTO
:PROB3:SET:ATT:AUTO
:PROB4:SET:ATT:AUTO
:PROB:SET:ATT:UNIT
:PROB2:SET:ATT:UNIT
:PROB3:SET:ATT:UNIT
:PROB4:SET:ATT:UNIT
:PROB:SET:ATT:MAN
:PROB2:SET:ATT:MAN
:PROB3:SET:ATT:MAN
:PROB4:SET:ATT:MAN
:PROB:SET:GAI
:PROB2:SET:GAI
:PROB3:SET:GAI
:PROB4:SET:GAI
:PROB:SET:GAI:AUTO
:PROB2:SET:GAI:AUTO
:PROB3:SET:GAI:AUTO
:PROB4:SET:GAI:AUTO
:PROB:SET:GAI:UNIT
:PROB2:SET:GAI:UNIT
:PROB3:SET:GAI:UNIT
:PROB4:SET:GAI:UNIT
:PROB:SET:GAI:MAN
:PROB2:SET:GAI:MAN
:PROB3:SET:GAI:MAN
:PROB4:SET:GAI:MAN
:PROB:SET:TYPE
:PROB2:SET:TYPE
:PROB3:SET:TYPE
:PROB4:SET:TYPE
:PROB:SET:NAME
:PROB2:SET:NAME
:PROB3:SET:NAME
:PROB4:SET:NAME
:PROB:SET:IMP
:PROB2:SET:IMP
:PROB3:SET:IMP
:PROB4:SET:IMP
:PROB:SET:CAP
:PROB2:SET:CAP
:PROB3:SET:CAP
:PROB4:SET:CAP
:PROB:SET:BAND
:PROB2:SET:BAND
:PROB3:SET:BAND
:PROB4:SET:BAND
:PROB:SET:OFFS
:PROB2:SET:OFFS
:PROB3:SET:OFFS
:PROB4:SET:OFFS
:PROB:SET:DCOF
:PROB2:SET:DCOF
:PROB3:SET:DCOF
:PROB4:SET:DCOF
:PROB:SET:ZAXV
:PROB2:SET:ZAXV
:PROB3:SET:ZAXV
:PROB4:SET:ZAXV
:PROB:SET:PRM
:PROB2:SET:PRM
:PROB3:SET:PRM
:PROB4:SET:PRM
:PROB:ID:BUIL
:PROB2:ID:BUIL
:PROB3:ID:BUIL
:PROB4:ID:BUIL
:PROB:ID:SWV
:PROB2:ID:SWV
:PROB3:ID:SWV
:PROB4:ID:SWV
:PROB:ID:PRD
:PROB2:ID:PRD
:PROB3:ID:PRD
:PROB4:ID:PRD
:PROB:ID:PART
:PROB2:ID:PART
:PROB3:ID:PART
:PROB4:ID:PART
:PROB:ID:SRN
:PROB2:ID:SRN
:PROB3:ID:SRN
:PROB4:ID:SRN
:PROB:SET:ADV:RANG
:PROB2:SET:ADV:RANG
:PROB3:SET:ADV:RANG
:PROB4:SET:ADV:RANG
:PROB:SET:ADV:FILT
:PROB2:SET:ADV:FILT
:PROB3:SET:ADV:FILT
:PROB4:SET:ADV:FILT
:PROB:SET:ADV:AUD
:PROB2:SET:ADV:AUD
:PROB3:SET:ADV:AUD
:PROB4:SET:ADV:AUD
:FORM
:FORM:DATA
:FORM:BORD
:SEAR:STAT
:SEAR:RCO
:SEAR:RESD:SHOW
:SEAR:COND
:SEAR:SOUR
:SEAR:DBG
:SEAR:MEAS:PEAK:POL
:SEAR:MEAS:LEV:PEAK:MAGN
:SEAR:TRIG:LEV:RUNT:UPP
:SEAR:TRIG:LEV:RUNT:LOW
:SEAR:TRIG:LEV:RIS:UPP
:SEAR:TRIG:LEV:RIS:LOW
:SEAR:TRIG:EDGE:SLOP
:SEAR:TRIG:EDGE:LEV
:SEAR:TRIG:EDGE:LEV:DELT
:SEAR:TRIG:WIDT:POL
:SEAR:TRIG:WIDT:RANG
:SEAR:TRIG:WIDT:WIDT
:SEAR:TRIG:WIDT:DELT
:SEAR:TRIG:WIDT:LEV
:SEAR:TRIG:WIDT:LEV:DELT
:SEAR:TRIG:RUNT:POL
:SEAR:TRIG:RUNT:RANG
:SEAR:TRIG:RUNT:WIDT
:SEAR:TRIG:RUNT:DELT
:SEAR:TRIG:RIS:SLOP
:SEAR:TRIG:RIS:RANG
:SEAR:TRIG:RIS:TIME
:SEAR:TRIG:RIS:DELT
:SEAR:TRIG:DAT:CSO
:SEAR:TRIG:DAT:CEDG
:SEAR:TRIG:DAT:CLEV
:SEAR:TRIG:DAT:CLEV:DELT
:SEAR:TRIG:DAT:DLEV
:SEAR:TRIG:DAT:DLEV:DELT
:SEAR:TRIG:DAT:STIM
:SEAR:TRIG:DAT:HTIM
:SEAR:TRIG:PATT:LEV
:SEAR:TRIG:PATT:LEV2
:SEAR:TRIG:PATT:LEV3
:SEAR:TRIG:PATT:LEV4
:SEAR:TRIG:PATT:LEV:DELT
:SEAR:TRIG:PATT:LEV2:DELT
:SEAR:TRIG:PATT:LEV3:DELT
:SEAR:TRIG:PATT:LEV4:DELT
:SEAR:TRIG:PATT:WIDT
:SEAR:TRIG:PATT:WIDT:WIDT
:SEAR:TRIG:PATT:WIDT:DELT
:SEAR:TRIG:PATT:WIDT:RANG
:SEAR:TRIG:PATT:FUNC
:SEAR:TRIG:PATT:SOUR
:SEAR:PROT:CAN:COND
:SEAR:PROT:CAN:FRAM
:SEAR:PROT:CAN:CRC
:SEAR:PROT:CAN:ACK
:SEAR:PROT:CAN:FORM
:SEAR:PROT:CAN:BITS
:SEAR:PROT:CAN:ITYP
:SEAR:PROT:CAN:FTYP
:SEAR:PROT:CAN:DLEN
:SEAR:PROT:CAN:ICON
:SEAR:PROT:CAN:DCON
:SEAR:PROT:CAN:IDEN
:SEAR:PROT:CAN:DATA
:SEAR:PROT:LIN:COND
:SEAR:PROT:LIN:FRAM
:SEAR:PROT:LIN:CHKS
:SEAR:PROT:LIN:IPER
:SEAR:PROT:LIN:SYER
:SEAR:PROT:LIN:DLEN
:SEAR:PROT:LIN:ICON
:SEAR:PROT:LIN:DCON
:SEAR:PROT:LIN:IDEN
:SEAR:PROT:LIN:DATA
:DEV:MODE
:EXP:WAV:SOUR
:EXP:WAV:NAME
:EXP:WAV:SAVE
:EXP:WAV:DATA
:EXP:WAV:HIST:STAR
:EXP:WAV:HIST:STOP
:EXP:WAV:HIST:CURR
:EXP:WAV:HIST:REM
:EXP:WAV:HIST:ALL
:EXP:WAV:HIST:STAT
:EXP:SEAR:NAME
:EXP:SEAR:SAVE
:EXP:MEAS:STAT:NAME
:EXP:MEAS2:STAT:NAME
:EXP:MEAS3:STAT:NAME
:EXP:MEAS4:STAT:NAME
:EXP:MEAS:STAT:SAVE
:EXP:MEAS2:STAT:SAVE
:EXP:MEAS3:STAT:SAVE
:EXP:MEAS4:STAT:SAVE
:EXP:MEAS:STAT:ALL:NAME
:EXP:MEAS2:STAT:ALL:NAME
:EXP:MEAS3:STAT:ALL:NAME
:EXP:MEAS4:STAT:ALL:NAME
:EXP:MEAS:STAT:ALL:SAVE
:EXP:MEAS2:STAT:ALL:SAVE
:EXP:MEAS3:STAT:ALL:SAVE
:EXP:MEAS4:STAT:ALL:SAVE
:EXP:ATAB:NAME
:EXP:ATAB:SAVE
:EXP:SCRS:DEST
:EXP:WFMS:DEST
:CAL
:CAL:STAT
:CAL:LOG
:WGEN:VOLT
:WGEN:VOLT:OFFS
:WGEN:FREQ
:WGEN:FUNC
:WGEN:FUNC:RAMP:POL
:WGEN:FUNC:EXP:POL
:WGEN:FUNC:PULS:DCYC
:WGEN:FUNC:PULS:ETIM
:WGEN:OUTP
:WGEN:OUTP:ENAB
:WGEN:OUTP:DEST
:WGEN:OUTP:LOAD
:WGEN:MOD
:WGEN:MOD:ENAB
:WGEN:MOD:TYPE
:WGEN:MOD:FUNC
:WGEN:MOD:AM:DEPT
:WGEN:MOD:AM:FREQ
:WGEN:MOD:FM:DEV
:WGEN:MOD:FM:FREQ
:WGEN:MOD:ASK:DEPT
:WGEN:MOD:ASK:FREQ
:WGEN:MOD:FSK:HFRE
:WGEN:MOD:FSK:RATE
:WGEN:MOD:RAMP:POL
:WGEN:NOIS:ABS
:WGEN:NOIS:REL
:WGEN:SWE
:WGEN:SWE:ENAB
:WGEN:SWE:FST
:WGEN:SWE:FEN
:WGEN:SWE:TIME
:WGEN:SWE:TYPE
:WGEN:ARB:SOUR
:WGEN:ARB:UPD
:WGEN:ARB:NAME
:WGEN:ARB:OPEN
:WGEN:ARB:FILE:NAME
:WGEN:ARB:FILE:OPEN
:WGEN:ARB:VIS
:WGEN:ARB:RANG:STAR
:WGEN:ARB:RANG:STOP
:WGEN:BURS
:WGEN:BURS:STAT
:WGEN:BURS:ITIM
:WGEN:BURS:NCYC
:WGEN:BURS:PHAS
:WGEN:BURS:TRIG
:WGEN:BURS:TRIG:MODE
:WGEN:BURS:TRIG:SING
:PGEN:FUNC
:PGEN:MAN:STAT0
:PGEN:MAN:STAT1
:PGEN:MAN:STAT2
:PGEN:MAN:STAT3
:PGEN:PATT:PER
:PGEN:PATT:STIM
:PGEN:PATT:ITIM
:PGEN:PATT:FREQ
:PGEN:PATT:STAT
:PGEN:PATT:BURS:STAT
:PGEN:PATT:BURS:NCYC
:PGEN:PATT:TRIG:MODE
:PGEN:PATT:TRIG:SING
:PGEN:PATT:TRIG:EXT:SLOP
:PGEN:PATT:ARB:DATA
:PGEN:PATT:ARB:DATA:SET
:PGEN:PATT:ARB:DATA:APP
:PGEN:PATT:ARB:DATA:APP:BOR
:PGEN:PATT:ARB:DATA:APP:BAND
:PGEN:PATT:ARB:DATA:APP:IND
:PGEN:PATT:ARB:DATA:LENG
:PGEN:PATT:SQU:DCYC
:PGEN:PATT:SQU:POL
:PGEN:PATT:COUN:FREQ
:PGEN:PATT:COUN:DIR
:DVM:TYPE
:DVM2:TYPE
:DVM3:TYPE
:DVM4:TYPE
:DVM:ENAB
:DVM2:ENAB
:DVM3:ENAB
:DVM4:ENAB
:DVM:SOUR
:DVM2:SOUR
:DVM3:SOUR
:DVM4:SOUR
:DVM:POS
:DVM2:POS
:DVM3:POS
:DVM4:POS
:DVM:FORM
:DVM2:FORM
:DVM3:FORM
:DVM4:FORM
:DVM:RES
:DVM2:RES
:DVM3:RES
:DVM4:RES
:DVM:RES:ACT
:DVM2:RES:ACT
:DVM3:RES:ACT
:DVM4:RES:ACT
:DVM:RES:ACT:STAT
:DVM2:RES:ACT:STAT
:DVM3:RES:ACT:STAT
:DVM4:RES:ACT:STAT
:DVM:RES:STAT
:DVM2:RES:STAT
:DVM3:RES:STAT
:DVM4:RES:STAT
:DVM:RES:RES
:DVM2:RES:RES
:DVM3:RES:RES
:DVM4:RES:RES
:DVM:RES:NPE
:DVM2:RES:NPE
:DVM3:RES:NPE
:DVM4:RES:NPE
:DVM:RES:PPE
:DVM2:RES:PPE
:DVM3:RES:PPE
:DVM4:RES:PPE
:DVM:RES:MAX
:DVM2:RES:MAX
:DVM3:RES:MAX
:DVM4:RES:MAX
:DVM:RES:MIN
:DVM2:RES:MIN
:DVM3:RES:MIN
:DVM4:RES:MIN
:DVM:RES:LTA
:DVM2:RES:LTA
:DVM3:RES:LTA
:DVM4:RES:LTA
:DVM:RES:LTS
:DVM2:RES:LTS
:DVM3:RES:LTS
:DVM4:RES:LTS
:DVM:RES:AVG
:DVM2:RES:AVG
:DVM3:RES:AVG
:DVM4:RES:AVG
:DVM:RES:STDD
:DVM2:RES:STDD
:DVM3:RES:STDD
:DVM4:RES:STDD
:DVM:RES:WFMC
:DVM2:RES:WFMC
:DVM3:RES:WFMC
:DVM4:RES:WFMC
:DVM:STAT:WEIG
:DVM2:STAT:WEIG
:DVM3:STAT:WEIG
:DVM4:STAT:WEIG
:DVM:STAT:RES
:DVM2:STAT:RES
:DVM3:STAT:RES
:DVM4:STAT:RES
:TCO:ENAB
:TCO:RES:FREQ
:TCO:RES:PER
:TCO:RES:ACT:FREQ
:TCO:RES:ACT:PER
:TCO:SOUR
:SPEC
:SPEC:STAT
:SPEC:SOUR
:SPEC:MODE
:SPEC:FREQ:MAGN:SCAL
:SPEC:FREQ:SCAL
:SPEC:FREQ:POS
:SPEC:FREQ:STAR
:SPEC:FREQ:STOP
:SPEC:FREQ:CENT
:SPEC:FREQ:SPAN
:SPEC:FREQ:BAND
:SPEC:FREQ:BAND:RES
:SPEC:FREQ:BAND:RES:VAL
:SPEC:FREQ:BAND:RES:RAT
:SPEC:FREQ:BAND:RES:AUTO
:SPEC:FREQ:BAND:RAT
:SPEC:FREQ:BAND:AUTO
:SPEC:FREQ:BAND:VAL
:SPEC:FREQ:WIND:TYPE
:SPEC:FREQ:AVER:COUN
:SPEC:FREQ:AVER:COMP
:SPEC:FREQ:RES
:SPEC:FREQ:FULL
:SPEC:SPEC:SCAL
:SPEC:SPEC:RES
:SPEC:DIAG:TDOM
:SPEC:DIAG:TDOM:ENAB
:SPEC:DIAG:SPEC
:SPEC:DIAG:SPEC:ENAB
:SPEC:DIAG:FDOM
:SPEC:DIAG:FDOM:ENAB
:SPEC:DIAG:COL:MAGN:MODE
:SPEC:DIAG:COL:SCH:FDOM
:SPEC:DIAG:COL:SCH:SPEC
:SPEC:DIAG:COL:MIN
:SPEC:DIAG:COL:MIN:LEV
:SPEC:DIAG:COL:MAX
:SPEC:DIAG:COL:MAX:LEV
:SPEC:TIME:RANG
:SPEC:TIME:POS
:SPEC:HIST:PLAY:STAT
:SPEC:HIST:PLAY:SPE
:SPEC:HIST:REPL
:SPEC:HIST:PALL
:SPEC:HIST:CURR
:SPEC:HIST:STAR
:SPEC:HIST:STOP
:SPEC:HIST:TSR
:SPEC:HIST:TSR:ALL
:SPEC:HIST:TSAB
:SPEC:HIST:TSAB:ALL
:SPEC:HIST:TSD
:SPEC:HIST:TSD:ALL
:SPEC:HIST:ATIC
:SPEC:HIST:ATIC:ALL
:SPEC:HIST:DTIC
:SPEC:HIST:DTIC:ALL
:SPEC:HIST:TCL
:SPEC:HIST:TINC
:SPEC:HIST:TMOD
:SPEC:HIST:EXP:NAME
:SPEC:HIST:EXP:SAVE
:SPEC:HIST:TTAB
:SPEC:HIST:TTAB:ENAB
:SPEC:HIST:CONT
:SPEC:HIST:CONT:ENAB
:SPEC:WAV:SPEC
:SPEC:WAV:SPEC:ENAB
:SPEC:WAV:SPEC:DATA
:SPEC:WAV:SPEC:DATA:POIN
:SPEC:WAV:SPEC:DATA:HEAD
:SPEC:WAV:SPEC:DATA:XINC
:SPEC:WAV:SPEC:DATA:XOR
:SPEC:WAV:SPEC:DATA:YINC
:SPEC:WAV:SPEC:DATA:YOR
:SPEC:WAV:SPEC:DATA:YRES
:SPEC:WAV:MIN
:SPEC:WAV:MIN:ENAB
:SPEC:WAV:MIN:DATA
:SPEC:WAV:MIN:DATA:POIN
:SPEC:WAV:MIN:DATA:HEAD
:SPEC:WAV:MIN:DATA:XINC
:SPEC:WAV:MIN:DATA:XOR
:SPEC:WAV:MIN:DATA:YINC
:SPEC:WAV:MIN:DATA:YOR
:SPEC:WAV:MIN:DATA:YRES
:SPEC:WAV:MAX
:SPEC:WAV:MAX:ENAB
:SPEC:WAV:MAX:DATA
:SPEC:WAV:MAX:DATA:POIN
:SPEC:WAV:MAX:DATA:HEAD
:SPEC:WAV:MAX:DATA:XINC
:SPEC:WAV:MAX:DATA:XOR
:SPEC:WAV:MAX:DATA:YINC
:SPEC:WAV:MAX:DATA:YOR
:SPEC:WAV:MAX:DATA:YRES
:SPEC:WAV:AVER
:SPEC:WAV:AVER:ENAB
:SPEC:WAV:AVER:DATA
:SPEC:WAV:AVER:DATA:POIN
:SPEC:WAV:AVER:DATA:HEAD
:SPEC:WAV:AVER:DATA:XINC
:SPEC:WAV:AVER:DATA:XOR
:SPEC:WAV:AVER:DATA:YINC
:SPEC:WAV:AVER:DATA:YOR
:SPEC:WAV:AVER:DATA:YRES
:BPL:ENAB
:BPL:STAT
:BPL:REP
:BPL:RES
:BPL:AUT
:BPL:STEP
:BPL:STEP:MODE
:BPL:GAIN:SCAL
:BPL:GAIN:POS
:BPL:GAIN:ENAB
:BPL:GAIN:DATA
:BPL:PHAS:SCAL
:BPL:PHAS:POS
:BPL:PHAS:ENAB
:BPL:PHAS:DATA
:BPL:POIN:LIN
:BPL:POIN:LOG
:BPL:INP
:BPL:INP:SOUR
:BPL:OUTP
:BPL:OUTP:SOUR
:BPL:MEAS:DEL
:BPL:MEAS:POIN
:BPL:MEAS:POIN:DISP
:BPL:FREQ:STAR
:BPL:FREQ:STOP
:BPL:FREQ:DATA
:BPL:AMPL:SCAL
:BPL:AMPL:POS
:BPL:AMPL:ENAB
:BPL:AMPL:DATA
:BPL:AMPL:MODE
:BPL:AMPL:PROF:COUN
:BPL:AMPL:PROF:POIN:FREQ
:BPL:AMPL:PROF:POIN2:FREQ
:BPL:AMPL:PROF:POIN3:FREQ
:BPL:AMPL:PROF:POIN4:FREQ
:BPL:AMPL:PROF:POIN5:FREQ
:BPL:AMPL:PROF:POIN6:FREQ
:BPL:AMPL:PROF:POIN7:FREQ
:BPL:AMPL:PROF:POIN8:FREQ
:BPL:AMPL:PROF:POIN9:FREQ
:BPL:AMPL:PROF:POIN10:FREQ
:BPL:AMPL:PROF:POIN11:FREQ
:BPL:AMPL:PROF:POIN12:FREQ
:BPL:AMPL:PROF:POIN13:FREQ
:BPL:AMPL:PROF:POIN14:FREQ
:BPL:AMPL:PROF:POIN15:FREQ
:BPL:AMPL:PROF:POIN16:FREQ
:BPL:AMPL:PROF:POIN:AMPL
:BPL:AMPL:PROF:POIN2:AMPL
:BPL:AMPL:PROF:POIN3:AMPL
:BPL:AMPL:PROF:POIN4:AMPL
:BPL:AMPL:PROF:POIN5:AMPL
:BPL:AMPL:PROF:POIN6:AMPL
:BPL:AMPL:PROF:POIN7:AMPL
:BPL:AMPL:PROF:POIN8:AMPL
:BPL:AMPL:PROF:POIN9:AMPL
:BPL:AMPL:PROF:POIN10:AMPL
:BPL:AMPL:PROF:POIN11:AMPL
:BPL:AMPL:PROF:POIN12:AMPL
:BPL:AMPL:PROF:POIN13:AMPL
:BPL:AMPL:PROF:POIN14:AMPL
:BPL:AMPL:PROF:POIN15:AMPL
:BPL:AMPL:PROF:POIN16:AMPL
:BPL:MARK:IND
:BPL:MARK2:IND
:BPL:MARK:FREQ
:BPL:MARK2:FREQ
:BPL:MARK:GAIN
:BPL:MARK2:GAIN
:BPL:MARK:PHAS
:BPL:MARK2:PHAS
:BPL:MARK:SSCR
:BPL:MARK2:SSCR
:BPL:MARK:DIFF:FREQ
:BPL:MARK2:DIFF:FREQ
:BPL:MARK:DIFF:GAIN
:BPL:MARK2:DIFF:GAIN
:BPL:MARK:DIFF:PHAS
:BPL:MARK2:DIFF:PHAS
:BPL:EXP:NAME
:BPL:EXP:SAVE
:BPL:EXP:DATA

*/