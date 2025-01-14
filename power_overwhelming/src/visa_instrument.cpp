// <copyright file="visa_instrument.cpp" company="Visualisierungsinstitut der Universit�t Stuttgart">
// Copyright � 2021 - 2023 Visualisierungsinstitut der Universit�t Stuttgart. Alle Rechte vorbehalten.
// </copyright>
// <author>Christoph M�ller</author>

#include "power_overwhelming/visa_instrument.h"

#include <algorithm>
#include <cassert>
#include <memory>
#include <numeric>
#include <stdexcept>
#include <system_error>

#if !defined(_WIN32)
#include <sys/time.h>
#endif /* !defined(_WIN32) */

#include "power_overwhelming/convert_string.h"

#include "no_visa_error_msg.h"
#include "visa_exception.h"
#include "visa_instrument_impl.h"
#include "visa_library.h"


/*
 * visus::power_overwhelming::visa_instrument::rohde_und_schwarz
 */
constexpr const char *
visus::power_overwhelming::visa_instrument::rohde_und_schwarz;


/*
 * visus::power_overwhelming::visa_instrument::find_resources
 */
visus::power_overwhelming::blob
visus::power_overwhelming::visa_instrument::find_resources(
        _In_z_ const wchar_t *query) {
    typedef wchar_t char_type;
    if (query == nullptr) {
        throw std::invalid_argument("The instrument query cannot be null.");
    }

    auto q = convert_string<char>(query);
    auto devices = detail::visa_library::instance().find_resource(q.c_str());

    const auto len = std::accumulate(devices.begin(), devices.end(),
        static_cast<std::size_t>(0), [](std::size_t s, const std::string &d) {
            return s + d.size() + 1;
        }) + 1;

    blob retval(len * sizeof(char_type));
    auto cur = retval.as<char_type>();
    const auto end = reinterpret_cast<char_type *>(retval.end());
    assert(cur != nullptr);
    _Analysis_assume_(cur != nullptr);
    assert(end != nullptr);
    _Analysis_assume_(end != nullptr);

    for (auto& d : devices) {
        detail::convert_string(cur, end - cur, d.c_str(), d.size());
        cur += d.size();
        *cur++ = 0;
    }

    *cur = 0;

    return retval;
}


/*
 * visus::power_overwhelming::visa_instrument::find_resources
 */
visus::power_overwhelming::blob
visus::power_overwhelming::visa_instrument::find_resources(
        _In_z_ const char *query) {
    typedef char char_type;
    if (query == nullptr) {
        throw std::invalid_argument("The instrument query cannot be null.");
    }

    auto devices = detail::visa_library::instance().find_resource(query);

    const auto len = std::accumulate(devices.begin(), devices.end(),
        static_cast<std::size_t>(0), [](std::size_t s, const std::string& d) {
            return s + d.size() + 1;
        }) + 1;

    blob retval(len * sizeof(char_type));
    auto cur = retval.as<char_type>();
    assert(cur != nullptr);
    _Analysis_assume_(cur != nullptr);

    for (auto& d : devices) {
        std::copy(d.begin(), d.end(), cur);
        cur += d.size();
        *cur++ = 0;
    }

    *cur = 0;

    return retval;
}


/*
 * visus::power_overwhelming::visa_instrument::find_resources
 */
visus::power_overwhelming::blob
visus::power_overwhelming::visa_instrument::find_resources(
        _In_z_ const wchar_t *vendor_id, _In_z_ const wchar_t *instrument_id) {
    if (vendor_id == nullptr) {
        throw std::invalid_argument("The instrument vendor cannot be null.");
    }
    if (instrument_id == nullptr) {
        throw std::invalid_argument("The instrument ID cannot be null.");
    }

    std::wstring query(L"?*::");    // Any protocol
    query += vendor_id;             // Only specified vendor
    query += L"::";
    query += instrument_id;         // Only specified instrument
    query += L"::?*::INSTR";        // All serial numbers

    return find_resources(query.c_str());
}


/*
 * visus::power_overwhelming::visa_instrument::find_resources
 */
visus::power_overwhelming::blob
visus::power_overwhelming::visa_instrument::find_resources(
        _In_z_ const char *vendor_id, _In_z_ const char *instrument_id) {
    if (vendor_id == nullptr) {
        throw std::invalid_argument("The instrument vendor cannot be null.");
    }
    if (instrument_id == nullptr) {
        throw std::invalid_argument("The instrument ID cannot be null.");
    }

    std::string query("?*::");  // Any protocol
    query += vendor_id;         // Only specified vendor
    query += "::";
    query += instrument_id;     // Only specified instrument
    query += "::?*::INSTR";     // All serial numbers

    return find_resources(query.c_str());
}


/*
 * visus::power_overwhelming::visa_instrument::visa_instrument
 */
visus::power_overwhelming::visa_instrument::visa_instrument(void)
    : _impl(nullptr) { }


/*
 * visus::power_overwhelming::visa_instrument::visa_instrument
 */
visus::power_overwhelming::visa_instrument::visa_instrument(
        _In_z_ const wchar_t *path, _In_ const timeout_type timeout)
        : _impl(nullptr) {
    this->_impl = detail::visa_instrument_impl::create(path, timeout);
}


/*
 * visus::power_overwhelming::visa_instrument::visa_instrument
 */
visus::power_overwhelming::visa_instrument::visa_instrument(
        _In_z_ const char *path, _In_ const timeout_type timeout)
        : _impl(nullptr) {
    this->_impl = detail::visa_instrument_impl::create(path, timeout);
}


/*
 * visus::power_overwhelming::visa_instrument::~visa_instrument
 */
visus::power_overwhelming::visa_instrument::~visa_instrument(void) {
    if (this->_impl != nullptr) {
        this->_impl->release();
    }
}


#if defined(POWER_OVERWHELMING_WITH_VISA)
/*
 * visus::power_overwhelming::visa_instrument::attribute
 */
const visus::power_overwhelming::visa_instrument &
visus::power_overwhelming::visa_instrument::attribute(
        _Out_ void *dst, _In_ ViAttr name) const {
    visa_exception::throw_on_error(detail::visa_library::instance()
        .viGetAttribute(this->check_not_disposed().session, name, &dst));
    return *this;
}
#endif /*defined(POWER_OVERWHELMING_WITH_VISA) */


#if defined(POWER_OVERWHELMING_WITH_VISA)
/*
 * visus::power_overwhelming::visa_instrument::attribute
 */
visus::power_overwhelming::visa_instrument&
visus::power_overwhelming::visa_instrument::attribute(_In_ ViAttr name,
        _In_ ViAttrState value) {
    visa_exception::throw_on_error(detail::visa_library::instance()
        .viSetAttribute(this->check_not_disposed().session, name, value));
    return *this;
}
#endif /*defined(POWER_OVERWHELMING_WITH_VISA) */


/*
 * visus::power_overwhelming::visa_instrument::buffer
 */
visus::power_overwhelming::visa_instrument&
visus::power_overwhelming::visa_instrument::buffer(
        _In_ const std::uint16_t mask, _In_ const std::uint32_t size) {
#if defined(POWER_OVERWHELMING_WITH_VISA)
    visa_exception::throw_on_error(detail::visa_library::instance()
        .viSetBuf(this->check_not_disposed().session, mask, size));
#endif /*defined(POWER_OVERWHELMING_WITH_VISA) */
    return *this;
}


/*
 * visus::power_overwhelming::visa_instrument::clear
 */
visus::power_overwhelming::visa_instrument&
visus::power_overwhelming::visa_instrument::clear(void) {
#if defined(POWER_OVERWHELMING_WITH_VISA)
    visa_exception::throw_on_error(detail::visa_library::instance()
        .viClear(this->check_not_disposed().session));
#endif /*defined(POWER_OVERWHELMING_WITH_VISA) */
    return *this;
}


/*
 * visus::power_overwhelming::visa_instrument::clear_status
 */
visus::power_overwhelming::visa_instrument&
visus::power_overwhelming::visa_instrument::clear_status(void) {
    this->check_not_disposed().write("*CLS\n");
    return *this;
}


/*
 * visus::power_overwhelming::visa_instrument::identify
 */
std::size_t visus::power_overwhelming::visa_instrument::identify(
        _Out_writes_opt_z_(cnt) wchar_t *dst,
        _In_ const std::size_t cnt) const {
    auto retval = this->check_not_disposed().identify();

    if (dst != nullptr) {
        auto converted = convert_string<wchar_t>(retval);
        ::wcsncpy(dst, converted.c_str(), cnt);
        dst[cnt - 1] = 0;
    }

    return (retval.size() + 1);
}


/*
 * visus::power_overwhelming::visa_instrument::identify
 */
std::size_t visus::power_overwhelming::visa_instrument::identify(
        _Out_writes_opt_z_(cnt) char *dst,
        _In_ const std::size_t cnt) const {
    auto retval = this->check_not_disposed().identify();

    if (dst != nullptr) {
        ::strncpy(dst, retval.c_str(), cnt);
        dst[cnt - 1] = 0;
    }

    return (retval.size() + 1);
}


/*
 * visus::power_overwhelming::visa_instrument::interface_type
 */
std::uint16_t visus::power_overwhelming::visa_instrument::interface_type(
        void) const {
    return this->check_not_disposed().interface_type();
}

/*

        else if (rsSession->sessionType == RS_INTF_GPIB)
        {
            rsSession->vxiCapable = (addFlags & 2) > 0 ? VI_FALSE : VI_TRUE;
        }
        else if (rsSession->sessionType == RS_INTF_TCPIP)
        {
            rsSession->vxiCapable = (addFlags & 4) > 0 ? VI_FALSE : VI_TRUE;
        }
        else if (rsSession->sessionType == RS_INTF_USB)
        {
            rsSession->vxiCapable = (addFlags & 1) > 0 ? VI_FALSE : VI_TRUE;
        }

*/


/*
 * visus::power_overwhelming::visa_instrument::path
 */
_Ret_maybenull_z_ const char *
visus::power_overwhelming::visa_instrument::path(void) const noexcept {
    return (*this) ? this->_impl->path().c_str() : nullptr;
}


/*
 * visus::power_overwhelming::visa_instrument::query
 */
visus::power_overwhelming::blob
visus::power_overwhelming::visa_instrument::query(
        _In_reads_bytes_(cnt) const byte_type *query,
        _In_ const std::size_t cnt,
        _In_ const std::size_t buffer_size) const {
    return this->write_all(query, cnt).read_all(buffer_size);
}


/*
 * visus::power_overwhelming::visa_instrument::query
 */
visus::power_overwhelming::blob
visus::power_overwhelming::visa_instrument::query(_In_z_ const char *query,
        _In_ const std::size_t buffer_size) const {
    return this->write(query).read_all(buffer_size);
}


/*
 * visus::power_overwhelming::visa_instrument::query
 */
visus::power_overwhelming::blob
visus::power_overwhelming::visa_instrument::query(_In_z_ const wchar_t *query,
        _In_ const std::size_t buffer_size) const {
    return this->write(query).read_all(buffer_size);
}


/*
 * visus::power_overwhelming::visa_instrument::read
 */
std::size_t visus::power_overwhelming::visa_instrument::read(
        _Out_writes_bytes_(cnt) byte_type *buffer,
        _In_ const std::size_t cnt) const {
    if (buffer == nullptr) {
        throw std::invalid_argument("The buffer to receive data from a VISA "
            "instrument must not be null.");
    }

    return this->check_not_disposed().read(buffer, cnt);
}


/*
 * visus::power_overwhelming::visa_instrument::read_all
 */
visus::power_overwhelming::blob
visus::power_overwhelming::visa_instrument::read_all(
        _In_ const std::size_t buffer_size) const {
    return this->check_not_disposed().read_all(buffer_size);
}


/*
 * visus::power_overwhelming::visa_instrument::reset
 */
visus::power_overwhelming::visa_instrument&
visus::power_overwhelming::visa_instrument::reset(void) {
    this->check_not_disposed().format("*RST\n");
    this->throw_on_system_error();
    return *this;
}


/*
 * visus::power_overwhelming::visa_instrument::synchronise_clock
 */
visus::power_overwhelming::visa_instrument&
visus::power_overwhelming::visa_instrument::synchronise_clock(
        _In_ const bool utc) {
#if defined(_WIN32)
    SYSTEMTIME time;
    if (utc) {
        ::GetSystemTime(&time);
    } else {
        ::GetLocalTime(&time);
    }

    this->check_not_disposed().format("SYST:TIME %d, %d, %d\n",
        time.wHour, time.wMinute, time.wSecond);
    this->throw_on_system_error();
    this->check_not_disposed().format("SYST:DATE %d, %d, %d\n",
        time.wYear, time.wMonth, time.wDay);
    this->throw_on_system_error();

#else /* defined(_WIN32) */
    struct timeval tv;
    struct timezone tz;

    if (::gettimeofday(&tv, &tz) != 0) {
        throw std::system_error(errno, std::system_category());
    }

    if (utc) {
        tv.tv_sec -= tz.tz_minuteswest * 60;
    }

    auto time = localtime(&tv.tv_sec);

    this->check_not_disposed().format("SYST:TIME %d, %d, %d\n",
        time->tm_hour, time->tm_min, time->tm_sec);
    this->throw_on_system_error();
    this->check_not_disposed().format("SYST:DATE %d, %d, %d\n",
        time->tm_year + 1900, time->tm_mon + 1, time->tm_mday);
    this->throw_on_system_error();

#endif /* defined(_WIN32) */

    return *this;
}


/*
 * visus::power_overwhelming::visa_instrument::status
 */
std::int32_t visus::power_overwhelming::visa_instrument::status(void) {
#if defined(POWER_OVERWHELMING_WITH_VISA)
    ViUInt16 retval;
    visa_exception::throw_on_error(detail::visa_library::instance()
        .viReadSTB(this->check_not_disposed().session, &retval));
    return static_cast<std::int32_t>(retval);

    // Note: R&S does the following, but NI's documentation suggests that
    // viReadSTB will issue *STB? by itself as a fallback, so we try the easier
    // one ...

    //if (this->check_not_disposed().vxi) {
    //    ViUInt16 retval;
    //    visa_exception::throw_on_error(detail::visa_library::instance()
    //        .viReadSTB(this->check_not_disposed().session, &retval));
    //    return static_cast<std::int32_t>(retval);

    //} else {
    //    this->write("*STB?\n");
    //    auto response = this->read_all();
    //    *response.rend() = 0;
    //    return std::atoi(response.as<char>());
    //}
#else /*defined(POWER_OVERWHELMING_WITH_VISA) */
    throw std::logic_error(detail::no_visa_error_msg);
#endif /*defined(POWER_OVERWHELMING_WITH_VISA) */
}


/*
 * visus::power_overwhelming::visa_instrument::system_error
 */
int visus::power_overwhelming::visa_instrument::system_error(void) const {
#if defined(POWER_OVERWHELMING_WITH_VISA)
    auto status = this->query(":SYST:ERR?\n");

    if (!status.empty()) {
        _Analysis_assume_(status.begin() != nullptr);
        _Analysis_assume_(status.end() != nullptr);
        auto delimiter = std::find_if(status.begin(),
            status.end(),
            [](const byte_type b) { return b == ','; });

        if (delimiter != status.end()) {
            *delimiter = '\0';
            return std::atoi(reinterpret_cast<char *>(status.begin()));
        }
    }

    throw std::runtime_error("The instrument responded unexpectedly when "
        "retrieving its error status.");
#else /*defined(POWER_OVERWHELMING_WITH_VISA) */
    return 0;
#endif /*defined(POWER_OVERWHELMING_WITH_VISA) */
}


/*
 * visus::power_overwhelming::visa_instrument::throw_on_system_error
 */
void visus::power_overwhelming::visa_instrument::throw_on_system_error(void) {
    if (this->_impl != nullptr) {
        std::string message;
        auto error = this->_impl->system_error(message);

        if (error != 0) {
            if (message.empty()) {
                message = std::to_string(error);
            }

            throw std::runtime_error(message);
        }
    }
}


/*
 * visus::power_overwhelming::visa_instrument::timeout
 */
visus::power_overwhelming::visa_instrument&
visus::power_overwhelming::visa_instrument::timeout(
        _In_ const timeout_type timeout) {
#if defined(POWER_OVERWHELMING_WITH_VISA)
    return this->attribute(VI_ATTR_TMO_VALUE, timeout);
#else /* defined(POWER_OVERWHELMING_WITH_VISA) */
    return *this;
#endif /* defined(POWER_OVERWHELMING_WITH_VISA) */
}


/*
 * visus::power_overwhelming::visa_instrument::wait
 */
visus::power_overwhelming::visa_instrument&
visus::power_overwhelming::visa_instrument::wait(void) {
#if defined(POWER_OVERWHELMING_WITH_VISA)
    this->query("*OPC?");
#endif /* defined(POWER_OVERWHELMING_WITH_VISA) */
    return *this;
}


/*
 * visus::power_overwhelming::visa_instrument::write
 */
std::size_t visus::power_overwhelming::visa_instrument::write(
        _In_reads_bytes_(cnt) const byte_type *buffer,
        _In_ const std::size_t cnt) const {
    if (buffer == nullptr) {
        throw std::invalid_argument("The buffer being written to the "
            "instrument must not be null.");
    }

    return this->check_not_disposed().write(buffer, cnt);
}


/*
 * visus::power_overwhelming::visa_instrument::write_all
 */
const visus::power_overwhelming::visa_instrument&
visus::power_overwhelming::visa_instrument::write_all(
        _In_reads_bytes_(cnt) const byte_type *buffer,
        _In_ const std::size_t cnt) const {
    if (buffer == nullptr) {
        throw std::invalid_argument("The buffer being written to the "
            "instrument must not be null.");
    }

    this->check_not_disposed().write_all(buffer, cnt);
    return *this;
}


/*
 * visus::power_overwhelming::visa_instrument::write
 */
const visus::power_overwhelming::visa_instrument&
visus::power_overwhelming::visa_instrument::write(
        _In_z_ const char *str) const {
    if (str == nullptr) {
        throw std::invalid_argument("The string to write to the device must "
            "not be null.");
    }

    this->check_not_disposed();
    this->_impl->write(str);
    return *this;
}


/*
 * visus::power_overwhelming::visa_instrument::write
 */
const visus::power_overwhelming::visa_instrument&
visus::power_overwhelming::visa_instrument::write(
        _In_z_ const wchar_t *str) const {
    if (str == nullptr) {
        throw std::invalid_argument("The string to write to the device must "
            "not be null.");
    }

    this->check_not_disposed();
    auto s = convert_string<char>(str);
    this->_impl->write_all(reinterpret_cast<const byte_type *>(s.data()),
        s.size());
    return *this;
}


/*
 * visus::power_overwhelming::visa_instrument::operator =
 */
visus::power_overwhelming::visa_instrument&
visus::power_overwhelming::visa_instrument::operator =(
        _Inout_ visa_instrument&& rhs) noexcept {
    if (this != std::addressof(rhs)) {
        this->_impl = rhs._impl;
        rhs._impl = nullptr;
    }

    return *this;
}

/*
 * visus::power_overwhelming::visa_instrument::operator bool
 */
visus::power_overwhelming::visa_instrument::operator bool(
        void) const noexcept {
    return (this->_impl != nullptr);
}


/*
 * visus::power_overwhelming::visa_instrument::check_not_disposed
 */
visus::power_overwhelming::detail::visa_instrument_impl&
visus::power_overwhelming::visa_instrument::check_not_disposed(void) {
    if (*this) {
        assert(this->_impl != nullptr);
        return *this->_impl;
    } else {
        throw std::runtime_error("An instrument which has been disposed by "
            "a move operation cannot be used anymore.");
    }
}


/*
 * visus::power_overwhelming::visa_instrument::check_not_disposed
 */
const visus::power_overwhelming::detail::visa_instrument_impl&
visus::power_overwhelming::visa_instrument::check_not_disposed(void) const {
    if (*this) {
        assert(this->_impl != nullptr);
        return *this->_impl;
    } else {
        throw std::runtime_error("An instrument which has been disposed by "
            "a move operation cannot be used anymore.");
    }
}
