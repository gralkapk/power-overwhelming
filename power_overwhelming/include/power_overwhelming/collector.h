// <copyright file="collector.h" company="Visualisierungsinstitut der Universit�t Stuttgart">
// Copyright � 2022 Visualisierungsinstitut der Universit�t Stuttgart. Alle Rechte vorbehalten.
// </copyright>
// <author>Christoph M�ller</author>

#pragma once

#include "power_overwhelming/adl_sensor.h"
#include "power_overwhelming/hmc8015_sensor.h"
#include "power_overwhelming/nvml_sensor.h"
#include "power_overwhelming/rtb_sensor.h"
#include "power_overwhelming/tinkerforge_sensor.h"


namespace visus {
namespace power_overwhelming {

    /* Forward declarations */
    namespace detail { struct collector_impl; }

    /// <summary>
    /// A collector is a set of sensors that asynchronously collect data and
    /// write them to a file.
    /// </summary>
    class POWER_OVERWHELMING_API collector final {

    public:

        /// <summary>
        /// Creates a configuration file for all sensors currently attached to
        /// the system.
        /// </summary>
        /// <remarks>
        /// <para>This method might run for a significant amount of time as it
        /// tries probing all known sensor types.</para>
        /// <para><see cref="tinkerforge_sensor" />s are only enumerated on
        /// <see cref="tinkerforge_sensor::default_host" /> and must listen on
        /// <see cref="tinkerforge_sensor::default_port" />. For remote machines
        /// or differently configured brickds, you must build the configuration
        /// manually.</para>
        /// </remarks>
        /// <param name="path">The path where the configuration file should be
        /// stored.</param>
        static void make_configuration_template(const wchar_t *path);

        /// <summary>
        /// Initialise a new instance from the given JSON configuration file.
        /// </summary>
        /// <param name="path">The path to the JSON configuration file.</param>
        collector(const wchar_t *path);

        /// <summary>
        /// Move <paramref name="rhs" /> into a new instance.
        /// </summary>
        /// <param name="rhs"></param>
        /// <returns></returns>
        inline collector(collector&& rhs) noexcept : _impl(rhs._impl) {
            rhs._impl = nullptr;
        }

        /// <summary>
        /// Finalises the instance.
        /// </summary>
        ~collector(void);

        /// <summary>
        /// Answer the number of sensors in the collector.
        /// </summary>
        /// <returns>The number of sensors. For disposed collectors, this is
        /// always zero.</returns>
        std::size_t size(void) const noexcept;

        /// <summary>
        /// Start collecting from all sensors.
        /// </summary>
        void start(void);

        /// <summary>
        /// Stop collecting.
        /// </summary>
        /// <remarks>
        /// It is safe to call this method on disposed instances, in which case it
        /// will have no effect.
        /// </remarks>
        void stop(void);

        /// <summary>
        /// Move assignment.
        /// </summary>
        /// <param name="rhs">The right-hand side operand</param>
        /// <returns><c>*this</c></returns>
        collector& operator =(collector&& rhs) noexcept;

        /// <summary>
        /// Determines whether the object is valid.
        /// </summary>
        /// <remarks>
        /// The collector is considered valid until it has been disposed by a
        /// move operation.
        /// </remarks>
        /// <returns><c>true</c> if the object is valid, <c>false</c>
        /// otherwise.</returns>
        operator bool(void) const noexcept;

    private:

        detail::collector_impl *_impl;

    };

} /* namespace power_overwhelming */
} /* namespace visus */
