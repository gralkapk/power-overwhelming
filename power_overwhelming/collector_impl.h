// <copyright file="collector_impl.h" company="Visualisierungsinstitut der Universit�t Stuttgart">
// Copyright � 2022 Visualisierungsinstitut der Universit�t Stuttgart. Alle Rechte vorbehalten.
// </copyright>
// <author>Christoph M�ller</author>

#include "power_overwhelming/sensor.h"

#include <atomic>
#include <chrono>
#include <fstream>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include <Windows.h>


namespace visus {
namespace power_overwhelming {
namespace detail {

    /// <summary>
    /// Private data container for <see cref="collector" />.
    /// </summary>
    struct collector_impl final {

        /// <summary>
        /// The type of the buffer for incoming measurements that are kept until
        /// they can be written to disk.
        /// </summary>
        typedef std::vector<measurement> buffer_type;

        /// <summary>
        /// Represents a marker in the measurement buffer.
        /// </summary>
        typedef std::pair<std::wstring, std::size_t> marker_type;

        /// <summary>
        /// The type of a marker list.
        /// </summary>
        typedef std::vector<marker_type> marker_list_type;

        /// <summary>
        /// Processes asynchronously created measurements.
        /// </summary>
        /// <param name="m"></param>
        /// <param name="context"></param>
        static void on_measurement(const measurement& m, void *context);

        /// <summary>
        /// Buffers the measurements until a marker is reached.
        /// </summary>
        buffer_type buffer;

        /// <summary>
        /// The collector thread that polls all sensors that do not run
        /// asynchronouly by themselves (executes <see cref="collect" />).
        /// </summary>
        std::thread collector_thread;

        /// <summary>
        /// An event to wake the I/O thread.
        /// </summary>
        HANDLE evt_write;

        /// <summary>
        /// Indicates whether a have a valid marker.
        /// </summary>
        /// <remarks>
        /// This flag is used to bypass collection of
        /// <see cref="measurement" />s into <see cref="buffer" /> while we have
        /// no active marker.
        /// </remarks>
        std::atomic<bool> have_marker;

        /// <summary>
        /// The lock protecting the <see cref="buffer" /> and the
        /// <see cref="markers" />.
        /// </summary>
        std::mutex lock;

        /// <summary>
        /// The markers for the current <see cref="buffer" />.
        /// </summary>
        marker_list_type markers;

        /// <summary>
        /// Indicates whether the collector thread should continue running.
        /// </summary>
        std::atomic<bool> running;

        /// <summary>
        /// The desired sampling interval.
        /// </summary>
        std::chrono::microseconds sampling_interval;

        /// <summary>
        /// Indicates whether collecting sensor data requires a marker being
        /// set.
        /// </summary>
        /// <remarks>
        /// This flag is assumed to be immutable and therefore readable without
        /// holding a lock.
        /// </remarks>
        bool require_marker;

        /// <summary>
        /// The list of sensors.
        /// </summary>
        std::vector<std::unique_ptr<sensor>> sensors;

        /// <summary>
        /// The output stream for the results.
        /// </summary>
        std::wofstream stream;

        /// <summary>
        /// The resolution of the timestamps being created.
        /// </summary>
        timestamp_resolution timestamp_resolution;

        /// <summary>
        /// The I/O thread executing <see cref="write" />.
        /// </summary>
        std::thread writer_thread;

        /// <summary>
        /// Initialises a new instance.
        /// </summary>
        collector_impl(void);

        /// <summary>
        /// Finalises the instance.
        /// </summary>
        ~collector_impl(void);

        /// <summary>
        /// Answer whether data can be written to the buffer.
        /// </summary>
        /// <returns></returns>
        bool can_buffer(void) const;

        /// <summary>
        /// Runs in <see cref="thread" /> and collects the data.
        /// </summary>
        void collect(void);

        /// <summary>
        /// Inject a marker in the stream.
        /// </summary>
        /// <param name="marker"></param>
        void marker(const wchar_t *marker);

        /// <summary>
        /// Starts the collector thread if not running.
        /// </summary>
        void start(void);

        /// <summary>
        /// Stops the collector thread.
        /// </summary>
        void stop(void);

        /// <summary>
        /// Asynchronously writes data from <see cref="buffer" /> and
        /// <see cref="markers" /> to <see cref="stream" />.
        /// </summary>
        void write(void);
    };

} /* namespace detail */
} /* namespace power_overwhelming */
} /* namespace visus */
