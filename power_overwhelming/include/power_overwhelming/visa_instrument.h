﻿// <copyright file="visa_instrument.h" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2021 - 2023 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// </copyright>
// <author>Christoph Müller</author>

#pragma once

#include <string>

#if defined(POWER_OVERWHELMING_WITH_VISA)
#include <visa.h>
#endif /*defined(POWER_OVERWHELMING_WITH_VISA) */

#include "power_overwhelming/blob.h"


namespace visus {
namespace power_overwhelming {

    /* Forward declarations. */
    namespace detail { struct visa_instrument_impl; }

    /// <summary>
    /// Implementation of a VISA instrument, which can be used to implement a
    /// sensor or control an instrument like an oscilloscope manually.
    /// </summary>
    /// <remarks>
    /// <para>The <see cref="visa_instrument" /> manages a
    /// <see cref="ViSession" /> on a single device, which might be used by
    /// multiple <see cref="sensor" />s.
    /// </remarks>
    class POWER_OVERWHELMING_API visa_instrument {

    public:

        /// <summary>
        /// The type representing a single byte.
        /// </summary>
        typedef blob::byte_type byte_type;

        /// <summary>
        /// The type used to express device timeouts in milliseconds.
        /// </summary>
        typedef std::uint32_t timeout_type;

        /// <summary>
        /// Find all resources matching the given VISA resource query.
        /// </summary>
        /// <param name="query">The query to issue on the resource manager.
        /// </param>
        /// <remarks>
        /// <para>The return value is a multi-sz string which can be procesed
        /// as follows:</para>
        /// <code>
        /// for (auto d = devices.as&lt;wchar_t&gt;();
        ///         (d != nullptr) && (*d != 0);
        ///         d += ::wcslen(d) + 1) {
        ///     visa_instrument instrument(d, 5000);
        ///     // Do something with the instrument.
        /// }
        /// </code>
        /// </remarks>
        /// <returns>A <c>wchar_t</c> <see cref="blob" /> holding a a multi-sz
        /// string (registry type) of all device paths matching the query.
        /// </returns>
        /// <exception cref="std::invalid_argument">If <paramref name="query" />
        /// is <c>nullptr</c>.</exception>
        /// <exception cref="visa_exception">If the query failed.</exception>
        /// <exception cref="std::bad_alloc">If the memory for the output could
        /// not be allocated.</exception>
        static blob find_resources(_In_z_ const wchar_t *query);

        /// <summary>
        /// Find all resources matching the given VISA resource query.
        /// </summary>
        /// <param name="query">The query to issue on the resource manager.
        /// </param>
        /// <remarks>
        /// <para>The return value is a multi-sz string which can be procesed
        /// as follows:</para>
        /// <code>
        /// for (auto d = devices.as&lt;cjar&gt;();
        ///         (d != nullptr) && (*d != 0);
        ///         d += ::strlen(d) + 1) {
        ///     visa_instrument instrument(d, 5000);
        ///     // Do something with the instrument.
        /// }
        /// </code>
        /// </remarks>
        /// <returns>A <c>char</c> <see cref="blob" /> holding a a multi-sz
        /// string (registry type) of all device paths matching the query.
        /// </returns>
        /// <exception cref="std::invalid_argument">If <paramref name="query" />
        /// is <c>nullptr</c>.</exception>
        /// <exception cref="visa_exception">If the query failed.</exception>
        /// <exception cref="std::bad_alloc">If the memory for the output could
        /// not be allocated.</exception>
        static blob find_resources(_In_z_ const char *query);

        /// <summary>
        /// Find all instruments of the specified type connected to the machine
        /// the code is running on.
        /// </summary>
        /// <param name="vendor_id">The ID of the vendor to search for.</param>
        /// <param name="instrument_id">The ID of the instrument to search for.
        /// </param>
        /// <returns>A <c>char</c> <see cref="blob" /> holding a a multi-sz
        /// string (registry type) of all device paths matching the query.
        /// </returns>
        /// <exception cref="std::invalid_argument">If
        /// <paramref name="vendor_id" /> is <c>nullptr</c> or if
        ///  <paramref name="instrument_id" /> is <c>nullptr</c>.</exception>
        /// <exception cref="visa_exception">If the query failed.</exception>
        /// <exception cref="std::bad_alloc">If the memory for the output could
        /// not be allocated.</exception>
        static blob find_resources(_In_z_ const wchar_t *vendor_id,
            _In_z_ const wchar_t *instrument_id);

        /// <summary>
        /// Find all instruments of the specified type connected to the machine
        /// the code is running on.
        /// </summary>
        /// <param name="vendor_id">The ID of the vendor to search for.</param>
        /// <param name="instrument_id">The ID of the instrument to search for.
        /// </param>
        /// <returns>A <c>char</c> <see cref="blob" /> holding a a multi-sz
        /// string (registry type) of all device paths matching the query.
        /// </returns>
        /// <exception cref="std::invalid_argument">If
        /// <paramref name="vendor_id" /> is <c>nullptr</c> or if
        ///  <paramref name="instrument_id" /> is <c>nullptr</c>.</exception>
        /// <exception cref="visa_exception">If the query failed.</exception>
        /// <exception cref="std::bad_alloc">If the memory for the output could
        /// not be allocated.</exception>
        static blob find_resources(_In_z_ const char *vendor_id,
            _In_z_ const char *instrument_id);

        /// <summary>
        /// The vendor ID of Rohde &amp; Schwarz.
        /// </summary>
        static constexpr const char *rohde_und_schwarz = "0x0AAD";

        /// <summary>
        /// Initialises a new, but invalid instance.
        /// </summary>
        visa_instrument(void);

        /// <summary>
        /// Initialises a new instance.
        /// </summary>
        /// <remarks>
        /// This constructor will set the name of the sensor to the identity
        /// string of the instrument, reset the instrument and clear any error
        /// state in the instrument.
        /// </remarks>
        /// <param name="path"></param>
        /// <param name="timeout"></param>
        /// <exception cref="std::invalid_argument">If <paramref name="path" />
        /// is <c>nullptr</c>.</exception>
        /// <exception cref="std::bad_alloc">If the memory for the sensor state
        /// could not be allocated.</exception>
        /// <exception cref="std::system_error">If the VISA library could not be
        /// loaded.</exception>
        /// <exception cref="visa_exception">If the sensor could not be
        /// initialised.</exception>
        visa_instrument(_In_z_ const wchar_t *path,
            _In_ const timeout_type timeout = 2000);

        /// <summary>
        /// Initialises a new instance.
        /// </summary>
        /// <remarks>
        /// This constructor will set the name of the sensor to the identity
        /// string of the instrument, reset the instrument and clear any error
        /// state in the instrument.
        /// </remarks>
        /// <param name="path"></param>
        /// <param name="timeout"></param>
        /// <exception cref="std::invalid_argument">If <paramref name="path" />
        /// is <c>nullptr</c>.</exception>
        /// <exception cref="std::bad_alloc">If the memory for the sensor state
        /// could not be allocated.</exception>
        /// <exception cref="std::system_error">If the VISA library could not be
        /// loaded.</exception>
        /// <exception cref="visa_exception">If the sensor could not be
        /// initialised.</exception>
        visa_instrument(_In_z_ const char *path,
            _In_ const timeout_type timeout = 2000);

        visa_instrument(const visa_instrument&) = delete;

        /// <summary>
        /// Move <paramref name="rhs" /> into a new instance.
        /// </summary>
        /// <param name="rhs">The object to be moved.</param>
        inline visa_instrument(_Inout_ visa_instrument&& rhs) noexcept
                : _impl(rhs._impl) {
            rhs._impl = nullptr;
        }

        /// <summary>
        /// Finalise the instance.
        /// </summary>
        virtual ~visa_instrument(void);

#if defined(POWER_OVERWHELMING_WITH_VISA)
        /// <summary>
        /// Read an attribute value from the instrument.
        /// </summary>
        /// <param name="dst">Receives the current value of the attribute. The
        /// size of this buffer must match the type of the attribute.</param>
        /// <param name="name">The name of the attribute to be retrieve.</param>
        /// <returns><c>*this</c>.</returns>
        /// <exception cref="std::runtime_error">If the method is called on an
        /// object that has been disposed by moving it.</exception>
        /// <exception cref="visa_exception">If the VISA command was not
        /// processed successfully.</exception>
        const visa_instrument& attribute(_Out_ void *dst,
            _In_ ViAttr name) const;
#endif /*defined(POWER_OVERWHELMING_WITH_VISA) */

#if defined(POWER_OVERWHELMING_WITH_VISA)
        /// <summary>
        /// Set an attribute of the instrument.
        /// </summary>
        /// <param name="name">The name of the attribute to be set.</param>
        /// <param name="value">The new value of teh attribute.</param>
        /// <returns><c>*this</c>.</returns>
        /// <exception cref="std::runtime_error">If the method is called on an
        /// object that has been disposed by moving it.</exception>
        /// <exception cref="visa_exception">If the VISA command was not
        /// processed successfully.</exception>
        visa_instrument& attribute(_In_ ViAttr name,
            _In_ ViAttrState value);
#endif /*defined(POWER_OVERWHELMING_WITH_VISA) */

        /// <summary>
        /// Shortcut to <see cref="viSetBuf" /> on the device session
        /// represented by <see cref="scope" />.
        /// </summary>
        /// <param name="mask"></param>
        /// <param name="size"></param>
        /// <returns><c>*this</c>.</returns>
        /// <exception cref="std::runtime_error">If the method is called on an
        /// object that has been disposed by moving it.</exception>
        /// <exception cref="visa_exception">If the operation failed. Note that
        /// a failure here only refers to the use of the API, ie the instrument
        /// can be in a failed state even if the call succeeded. Use
        /// <see cref="throw_on_system_error" /> to check the internal state of
        /// the instrument after the call.</exception>
        visa_instrument& buffer(_In_ const std::uint16_t mask,
            _In_ const std::uint32_t size);

        /// <summary>
        /// Call <see cref="viClear" /> on the instrument, which will flush all
        /// buffers.
        /// </summary>
        /// <remarks>
        /// This method does nothing if the library was compiled without support
        /// for VISA.
        /// </remarks>
        /// <returns><c>*this</c>.</returns>
        /// <exception cref="std::runtime_error">If the method is called on an
        /// object that has been disposed by moving it.</exception>
        /// <exception cref="visa_exception">If the operation failed.
        /// </exception>
        visa_instrument& clear(void);

        /// <summary>
        /// Clear the error queue of the instrument.
        /// </summary>
        /// <remarks>
        /// This method does nothing if the library was compiled without support
        /// for VISA.
        /// </remarks>
        /// <returns><c>*this</c>.</returns>
        /// <exception cref="std::runtime_error">If the method is called on an
        /// object that has been disposed by moving it.</exception>
        /// <exception cref="visa_exception">If the operation failed.
        /// </exception>
        visa_instrument& clear_status(void);

        /// <summary>
        /// Send the &quot;identify&quot; SCPI command to the instrument and
        /// return its response.
        /// </summary>
        /// <param name="dst">If not <c>nullptr</c>, receives at most
        /// <paramref name="cnt" /> characters of the identification string. It
        /// is safe to pass <c>nulltpr</c> in which case the string will only
        /// be measured, but not returned. It is guaranteed that this string is
        /// null-terminated even if it was truncated.</param>
        /// <param name="cnt">The size of <paramref name="dst" /> in characters.
        /// </param>
        /// <returns>The number of characters, including the terminating null,
        /// required to store the identity of the instrument.</returns>
        /// <exception cref="std::runtime_error">If the method is called on an
        /// object that has been disposed by moving it.</exception>
        /// <exception cref="visa_exception">If the operation failed.
        /// </exception>
        std::size_t identify(_Out_writes_opt_z_(cnt) wchar_t *dst,
            _In_ const std::size_t cnt) const;

        /// <summary>
        /// Send the &quot;identify&quot; SCPI command to the instrument and
        /// return its response.
        /// </summary>
        /// <param name="dst">If not <c>nullptr</c>, receives at most
        /// <paramref name="cnt" /> characters of the identification string. It
        /// is safe to pass <c>nulltpr</c> in which case the string will only
        /// be measured, but not returned. It is guaranteed that this string is
        /// null-terminated even if it was truncated.</param>
        /// <param name="cnt">The size of <paramref name="dst" /> in characters.
        /// </param>
        /// <returns>The number of characters, including the terminating null,
        /// required to store the identity of the instrument.</returns>
        /// <exception cref="std::runtime_error">If the method is called on an
        /// object that has been disposed by moving it.</exception>
        /// <exception cref="visa_exception">If the operation failed.
        /// </exception>
        std::size_t identify(_Out_writes_opt_z_(cnt) char *dst,
            _In_ const std::size_t cnt) const;

        /// <summary>
        /// Gets the interface type of the underlying session in the form of one
        /// of the enumerated values <c>VI_INTF_GPIB</c>, <c>VI_INTF_VXI</c>,
        /// <c>VI_INTF_GPIB_VXI</c>, <c>VI_INTF_ASRL</c>, <c>VI_INTF_PXI</c>,
        /// <c>VI_INTF_TCPIP</c> or <c>VI_INTF_USB</c>.
        /// </summary>
        /// <returns>The interface type.</returns>
        /// <exception cref="std::runtime_error">If the method is called on an
        /// object that has been disposed by moving it.</exception>
        /// <exception cref="std::logic_error">If the library was compiled
        /// without support for VISA.</exception>
        std::uint16_t interface_type(void) const;

        /// <summary>
        /// Gets the VISA path of the device.
        /// </summary>
        /// <returns>The VISA path used to open the device.</returns>
        _Ret_maybenull_z_ const char *path(void) const noexcept;

        /// <summary>
        /// Write the given data to the instrument and directly read the
        /// response using a buffer size of <paramref name="buffer_size" />
        /// bytes.
        /// </summary>
        /// <param name="query">The query to send to the instrument.</param>
        /// <param name="cnt">The size of <paramref name="query" /> in bytes.
        /// </param>
        /// <param name="buffer_size">The buffer size used to read the response.
        /// This parameter defaults to 1024.</param>
        /// <returns>The data received from the instrument.</returns>
        /// <exception cref="std::invalid_argument">If <paramref name="query" />
        /// is <c>nullptr</c>.</exception>
        /// <exception cref="std::runtime_error">If the method is called on an
        /// object that has been disposed by moving it.</exception>
        /// <exception cref="visa_exception">If the operation failed. Note that
        /// a failure here only refers to the use of the API, ie the instrument
        /// can be in a failed state even if the call succeeded. Use
        /// <see cref="throw_on_system_error" /> to check the internal state of
        /// the instrument after the call.</exception>
        blob query(_In_reads_bytes_(cnt) const byte_type *query,
            _In_ const std::size_t cnt,
            _In_ const std::size_t buffer_size = 1024) const;

        /// <summary>
        /// Write the given null-terminated query to the instrument and directly
        /// read the response using a buffer size of
        /// <paramref name="buffer_size" /> bytes.
        /// </summary>
        /// <param name="query">The query to be sent to the instrument.</param>
        /// <param name="buffer_size">The buffer size used to read the response.
        /// This parameter defaults to 1024.</param>
        /// <returns>The data received from the instrument.</returns>
        /// <exception cref="std::invalid_argument">If <paramref name="query" />
        /// is <c>nullptr</c>.</exception>
        /// <exception cref="std::runtime_error">If the method is called on an
        /// object that has been disposed by moving it.</exception>
        /// <exception cref="visa_exception">If the operation failed. Note that
        /// a failure here only refers to the use of the API, ie the instrument
        /// can be in a failed state even if the call succeeded. Use
        /// <see cref="throw_on_system_error" /> to check the internal state of
        /// the instrument after the call.</exception>
        blob query(_In_z_ const char *query,
            _In_ const std::size_t buffer_size = 1024) const;

        /// <summary>
        /// Write the given null-terminated query to the instrument and directly
        /// read the response using a buffer size of
        /// <paramref name="buffer_size" /> bytes.
        /// </summary>
        /// <param name="query">The query to be sent to the instrument.</param>
        /// <param name="buffer_size">The buffer size used to read the response.
        /// This parameter defaults to 1024.</param>
        /// <returns>The data received from the instrument.</returns>
        /// <exception cref="std::invalid_argument">If <paramref name="query" />
        /// is <c>nullptr</c>.</exception>
        /// <exception cref="std::runtime_error">If the method is called on an
        /// object that has been disposed by moving it.</exception>
        /// <exception cref="visa_exception">If the operation failed. Note that
        /// a failure here only refers to the use of the API, ie the instrument
        /// can be in a failed state even if the call succeeded. Use
        /// <see cref="throw_on_system_error" /> to check the internal state of
        /// the instrument after the call.</exception>
        blob query(_In_z_ const wchar_t *query,
            _In_ const std::size_t buffer_size = 1024) const;

        /// <summary>
        /// Read from the instrument into the given buffer.
        /// </summary>
        /// <remarks>
        /// <para>This method has no effect if the library has been compiled
        /// without support for VISA.</para>
        /// </remarks>
        /// <param name="buffer">The buffer to write the data to.</param>
        /// <param name="cnt">The size of the buffer in bytes.</param>
        /// <returns>The number of bytes actually read. If this is equal to
        /// <see cref="cnt" />, the response has most likely not been read in
        /// total and you should call the method again.</returns>
        /// <exception cref="std::runtime_error">If the method is called on an
        /// object that has been disposed by moving it.</exception>
        /// <exception cref="std::invalid_argument">If
        /// <paramref name="buffer" /> is <c>nullptr</c>.</exception>
        /// <exception cref="visa_exception">If the operation failed. Note that
        /// a failure here only refers to the use of the API, ie the instrument
        /// can be in a failed state even if the call succeeded. Use
        /// <see cref="throw_on_system_error" /> to check the internal state of
        /// the instrument after the call.</exception>
        std::size_t read(_Out_writes_bytes_(cnt) byte_type *buffer,
            _In_ const std::size_t cnt) const;

        /// <summary>
        /// Read a full response.
        /// </summary>
        /// <remarks>
        /// This method has no effect if the library has been compiled without
        /// support for VISA.
        /// </remarks>
        /// <param name="buffer_size">The size of the read buffer being used. If
        /// this is less than the response size, the buffer will be resized
        /// until everything was read.
        /// </param>
        /// <returns>The full output from the device. The buffer will have
        /// exactly the size of the response, all padding will be removed before
        /// the buffer is returned.</returns>
        /// <exception cref="std::runtime_error">If the method is called on an
        /// object that has been disposed by moving it.</exception>
        /// <exception cref="visa_exception">If the operation failed. Note that
        /// a failure here only refers to the use of the API, ie the instrument
        /// can be in a failed state even if the call succeeded. Use
        /// <see cref="throw_on_system_error" /> to check the internal state of
        /// the instrument after the call.</exception>
        blob read_all(_In_ const std::size_t buffer_size = 1024) const;

        /// <summary>
        /// Resets the instrument to its default state by issuing the *RST
        /// command.
        /// </summary>
        /// <returns><c>*this</c>.</returns>
        /// <exception cref="std::runtime_error">If the method is called on an
        /// object that has been disposed by moving it.</exception>
        /// <exception cref="visa_exception">If the VISA command was not
        /// processed successfully.</exception>
        visa_instrument& reset(void);

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
        visa_instrument& synchronise_clock(_In_ const bool utc = false);

        /// <summary>
        /// Reads the status bytes.
        /// </summary>
        /// <returns>The status bytes.</returns>
        /// <exception cref="std::runtime_error">If the method is called on an
        /// object that has been disposed by moving it.</exception>
        /// <exception cref="std::logic_error">If the library was compiled
        /// without support for VISA.</exception>
        /// <exception cref="visa_exception">If the operation failed.
        /// </exception>
        std::int32_t status(void);

        /// <summary>
        /// Query the oldest error in the queue.
        /// </summary>
        /// <remarks>
        /// This method always returns zero if the library was compiled without
        /// support for VISA.
        /// </remarks>
        /// <returns>The current system error, or zero if the system has no
        /// previous error.</returns>
        /// <exception cref="std::runtime_error">If the method is called on an
        /// object that has been disposed by moving it.</exception>
        /// <exception cref="visa_exception">If the current system state could
        /// not be retrieved.</exception>
        int system_error(void) const;

        /// <summary>
        /// Checks the return value of <see cref="system_error" /> and throws a
        /// <see cref="std::runtime_error" /> if it does not return zero.
        /// </summary>
        /// <remarks>
        /// It is safe to call this method on a disposed instrument, in which
        /// case nothing will happen.
        /// </remarks>
        /// <exception cref="visa_exception">If the current system state could
        /// not be retrieved.</exception>
        /// <exception cref="std::runtime_error">If the current system state was
        /// retrieved and is not zero.</exception>
        void throw_on_system_error(void);

        /// <summary>
        /// Sets the timeout of the underlying VISA session.
        /// </summary>
        /// <remarks>
        /// This method has no effect if the library was compiled without
        /// support for VISA.
        /// </remarks>
        /// <param name="timeout">The timeout in milliseconds.</param>
        /// <returns><c>*this</c>.</returns>
        /// <exception cref="std::runtime_error">If the method is called on an
        /// object that has been disposed by moving it.</exception>
        /// <exception cref="visa_exception">If the current system state could
        /// not be retrieved.</exception>
        visa_instrument& timeout(_In_ const timeout_type timeout);

        /// <summary>
        /// Issue and wait for an OPC query.
        /// </summary>
        /// <remarks>
        /// This method does nothing if the library was compiled without support
        /// for VISA.
        /// </remarks>
        /// <returns><c>*this</c>.</returns>
        /// <exception cref="std::runtime_error">If the method is called on an
        /// object that has been disposed by moving it.</exception>
        /// <exception cref="visa_exception">If the operation failed.
        /// </exception>
        visa_instrument& wait(void);

        /// <summary>
        /// Write at most <paramref name="cnt" /> bytes of the given data to the
        /// instrument.
        /// </summary>
        /// <remarks>
        /// This method does nothing if the library was compiled without support
        /// for VISA.
        /// </remarks>
        /// <param name="buffer">The buffer holding the data to write.</param>
        /// <param name="cnt">The size of <paramref name="buffer" /> in bytes.
        /// </param>
        /// <returns>The number of bytes actually written.</returns>
        /// <exception cref="std::runtime_error">If the method is called on an
        /// object that has been disposed by moving it.</exception>
        /// <exception cref="std::invalid_argument">If
        /// <paramref name="buffer" /> is <c>nullptr</c>.</exception>
        /// <exception cref="visa_exception">If the operation failed. Note that
        /// a failure here only refers to the use of the API, ie the instrument
        /// can be in a failed state even if the call succeeded. Use
        /// <see cref="throw_on_system_error" /> to check the internal state of
        /// the instrument after the call.</exception>
        std::size_t write(_In_reads_bytes_(cnt) const byte_type *buffer,
            _In_ const std::size_t cnt) const;

        /// <summary>
        /// Write the given data to the instrument.
        /// </summary>
        /// <remarks>
        /// This method does nothing if the library was compiled without support
        /// for VISA.
        /// </remarks>
        /// <param name="buffer">The buffer holding the data to write.</param>
        /// <param name="cnt">The size of <paramref name="buffer" /> in bytes.
        /// </param>
        /// <returns><c>*this</c>.</returns>
        /// <exception cref="std::runtime_error">If the method is called on an
        /// object that has been disposed by moving it.</exception>
        /// <exception cref="std::invalid_argument">If
        /// <paramref name="buffer" /> is <c>nullptr</c>.</exception>
        /// <exception cref="visa_exception">If the operation failed. Note that
        /// a failure here only refers to the use of the API, ie the instrument
        /// can be in a failed state even if the call succeeded. Use
        /// <see cref="throw_on_system_error" /> to check the internal state of
        /// the instrument after the call.</exception>
        const visa_instrument& write_all(
            _In_reads_bytes_(cnt) const byte_type *buffer,
            _In_ const std::size_t cnt) const;

        /// <summary>
        /// Writes the given null-terminated data to the instrument.
        /// </summary>
        /// <remarks>
        /// This method does nothing if the library was compiled without support
        /// for VISA.
        /// </remarks>
        /// <param name="str">A null-terminated string to write to the device.
        /// </param>
        /// <returns><c>*this</c>.</returns>
        /// <exception cref="std::runtime_error">If the method is called on an
        /// object that has been disposed by moving it.</exception>
        /// <exception cref="std::invalid_argument">If
        /// <paramref name="str" /> is <c>nullptr</c>.</exception>
        /// <exception cref="visa_exception">If the operation failed. Note that
        /// a failure here only refers to the use of the API, ie the instrument
        /// can be in a failed state even if the call succeeded. Use
        /// <see cref="throw_on_system_error" /> to check the internal state of
        /// the instrument after the call.</exception>
        const visa_instrument& write(_In_z_ const char *str) const;

        /// <summary>
        /// Writes the given null-terminated data to the instrument.
        /// </summary>
        /// <remarks>
        /// This method does nothing if the library was compiled without support
        /// for VISA.
        /// </remarks>
        /// <param name="str">A null-terminated string to write to the device.
        /// </param>
        /// <returns><c>*this</c>.</returns>
        /// <exception cref="std::runtime_error">If the method is called on an
        /// object that has been disposed by moving it.</exception>
        /// <exception cref="std::invalid_argument">If
        /// <paramref name="str" /> is <c>nullptr</c>.</exception>
        /// <exception cref="visa_exception">If the operation failed. Note that
        /// a failure here only refers to the use of the API, ie the instrument
        /// can be in a failed state even if the call succeeded. Use
        /// <see cref="throw_on_system_error" /> to check the internal state of
        /// the instrument after the call.</exception>
        const visa_instrument& write(_In_z_ const wchar_t *str) const;

        /// <summary>
        /// Writes the given string to the instrument.
        /// </summary>
        /// <remarks>
        /// This method does nothing if the library was compiled without support
        /// for VISA.
        /// </remarks>
        /// <typeparam name="TChar">The character type in a string. Only
        /// <c>wchar_t</c> and <c>char</c> are supported here.</typeparam>
        /// <param name="str">A string to write to the device.</param>
        /// <returns><c>*this</c>.</returns>
        /// <exception cref="std::runtime_error">If the method is called on an
        /// object that has been disposed by moving it.</exception>
        /// <exception cref="visa_exception">If the operation failed. Note that
        /// a failure here only refers to the use of the API, ie the instrument
        /// can be in a failed state even if the call succeeded. Use
        /// <see cref="throw_on_system_error" /> to check the internal state of
        /// the instrument after the call.</exception>
        template<class TChar>
        const visa_instrument& write(
            _In_ const std::basic_string<TChar>& str) const;

        visa_instrument& operator =(const visa_instrument&) = delete;

        /// <summary>
        /// Move assignment.
        /// </summary>
        /// <param name="rhs">The right-hand side operand</param>
        /// <returns><c>*this</c>.</returns>
        visa_instrument& operator =(_Inout_ visa_instrument&& rhs) noexcept;

        /// <summary>
        /// Determines whether the object is valid.
        /// </summary>
        /// <remarks>
        /// An instrument is considered valid until it has been disposed by a
        /// move operation.
        /// </remarks>
        /// <returns><c>true</c> if the instrument is valid, <c>false</c>
        /// otherwise.</returns>
        operator bool(void) const noexcept;

    protected:

        /// <summary>
        /// Checks whether the instance is valid and throws an
        /// <see cref="std::runtime_error" /> if this is not the case.
        /// </summary>
        /// <returns>The implementation object for the instrument.</returns>
        /// <exception cref="std::runtime_error">If the implementation has been
        /// released.</exception>
        detail::visa_instrument_impl& check_not_disposed(void);

        /// <summary>
        /// Checks whether the instance is valid and throws an
        /// <see cref="std::runtime_error" /> if this is not the case.
        /// </summary>
        /// <returns>The implementation object for the instrument.</returns>
        /// <exception cref="std::runtime_error">If the implementation has been
        /// released.</exception>
        const detail::visa_instrument_impl& check_not_disposed(void) const;

    private:

        detail::visa_instrument_impl *_impl;
    };

} /* namespace power_overwhelming */
} /* namespace visus */

#include "power_overwhelming/visa_instrument.inl"
