// <copyright file="graphics_device.cpp" company="Visualisierungsinstitut der Universit�t Stuttgart">
// Copyright � 2022 Visualisierungsinstitut der Universit�t Stuttgart. Alle Rechte vorbehalten.
// </copyright>
// <author>Christoph M�ller</author>

#include "power_overwhelming/graphics_device.h"

#include <memory>

#include <wrl.h>

#include "com_error_category.h"
#include "safe_com.h"



/*
 * visus::power_overwhelming::graphics_device::all
 */
std::size_t visus::power_overwhelming::graphics_device::all(
        graphics_device *outDevices, const std::size_t cntDevices,
        const bool onlyHardware) {
    Microsoft::WRL::ComPtr<IDXGIFactory1> factory;
    auto hr = S_OK;
    std::size_t retval = 0;

    if ((cntDevices > 0) && (outDevices == nullptr)) {
        throw std::invalid_argument("The output pointer can only be nullptr if "
            "the size of the output array is zero.");
    }

    hr = ::CreateDXGIFactory1(::IID_IDXGIFactory1,
        reinterpret_cast<void **>(factory.GetAddressOf()));
    if (FAILED(hr)) {
        throw std::system_error(hr, detail::com_category());
    }

    for (UINT a = 0; (hr != DXGI_ERROR_NOT_FOUND); ++a) {
        Microsoft::WRL::ComPtr<IDXGIAdapter> adapter;

        hr = factory->EnumAdapters(a, &adapter);

        if (SUCCEEDED(hr) && onlyHardware) {
            DXGI_ADAPTER_DESC desc;

            hr = adapter->GetDesc(&desc);
            if (FAILED(hr)) {
                throw std::system_error(hr, detail::com_category());
            }

            if ((desc.VendorId == 0x1414) && (desc.DeviceId == 0x8c)) {
                // Skip Microsoft's software emulation (cf.
                // https://msdn.microsoft.com/en-us/library/windows/desktop/bb205075(v=vs.85).aspx)
                continue;
            }
        }

        if (SUCCEEDED(hr) && (retval < cntDevices)) {
            Microsoft::WRL::ComPtr<device_type> device;
            D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;

#if (POWER_OVERWHELMING_GPU_ABSTRACTION == 11)
            hr = ::D3D11CreateDevice(adapter.Get(), D3D_DRIVER_TYPE_UNKNOWN,
                NULL, 0, NULL, 0, D3D11_SDK_VERSION, device.GetAddressOf(),
                &featureLevel, nullptr);
            if (FAILED(hr)) {
                throw std::system_error(hr, detail::com_category());
            }
#elif (POWER_OVERWHELMING_GPU_ABSTRACTION == 12)
            hr = ::D3D12CreateDevice(adapter.Get(), featureLevel,
                IID_PPV_ARGS(&device));
            if (FAILED(hr)) {
                throw std::system_error(hr, detail::com_category());
            }
#endif /* (POWER_OVERWHELMING_GPU_ABSTRACTION == 11) */

            detail::safe_replace(outDevices[retval]._adapter, adapter.Get());
            detail::safe_replace(outDevices[retval]._device, device.Get());
        }

        if (SUCCEEDED(hr)) {
            ++retval;
        }
    }

    return retval;
}


/*
 * visus::power_overwhelming::graphics_device::graphics_device
 */
visus::power_overwhelming::graphics_device::graphics_device(void) noexcept
    : _adapter(nullptr), _device(nullptr), _id(nullptr), _name(nullptr) { }


/*
 * visus::power_overwhelming::graphics_device::graphics_device
 */
visus::power_overwhelming::graphics_device::graphics_device(
        const graphics_device& rhs)
    : _adapter(rhs._adapter), _device(rhs._device),
        _id(nullptr), _name(nullptr) {
#if (POWER_OVERWHELMING_GPU_ABSTRACTION >= 11)
    detail::safe_add_ref(this->_adapter);
    detail::safe_add_ref(this->_device);
#endif /* (POWER_OVERWHELMING_GPU_ABSTRACTION >= 11) */
}


/*
 * visus::power_overwhelming::graphics_device::graphics_device
 */
visus::power_overwhelming::graphics_device::graphics_device(
        graphics_device&& rhs) noexcept
    : _adapter(rhs._adapter), _device(rhs._device),
        _id(rhs._id), _name(rhs._name) {
    rhs._adapter = nullptr;
    rhs._device = nullptr;
    rhs._id = nullptr;
    rhs._name = nullptr;
}


/*
 * visus::power_overwhelming::graphics_device::~graphics_device
 */
visus::power_overwhelming::graphics_device::~graphics_device(void) {
#if (POWER_OVERWHELMING_GPU_ABSTRACTION >= 11)
    detail::safe_release(this->_adapter);
    detail::safe_release(this->_device);
#endif /* (POWER_OVERWHELMING_GPU_ABSTRACTION >= 11) */

    delete[] this->_id;
    delete[] this->_name;
}


/*
 * visus::power_overwhelming::graphics_device::operator =
 */
visus::power_overwhelming::graphics_device
visus::power_overwhelming::graphics_device::operator =(
        const graphics_device& rhs) {
    if (this != std::addressof(rhs)) {
#if (POWER_OVERWHELMING_GPU_ABSTRACTION >= 11)
        detail::safe_replace(this->_adapter, rhs._adapter);
        detail::safe_replace(this->_device, rhs._device);
#endif /* (POWER_OVERWHELMING_GPU_ABSTRACTION >= 11) */

        // Just clear any existing cached data. The copy will be allocated
        // as necessary.
        delete[] this->_id;
        this->_id = nullptr;
        delete[] this->_name;
        this->_name = nullptr;
    }

    return *this;
}


/*
 * visus::power_overwhelming::graphics_device::operator =
 */
visus::power_overwhelming::graphics_device
visus::power_overwhelming::graphics_device::operator =(
        graphics_device&& rhs) noexcept {
    if (this != std::addressof(rhs)) {
        detail::safe_move(this->_adapter, std::move(rhs._adapter));
        detail::safe_move(this->_device, std::move(rhs._device));

        // Transfer the cached data as well.
        delete[] this->_id;
        this->_id = rhs._id;
        rhs._id = nullptr;
        delete[] this->_name;
        this->_name = rhs._name;
        rhs._name = nullptr;
    }

    return *this;
}


/*
 * visus::power_overwhelming::graphics_device::operator bool
 */
visus::power_overwhelming::graphics_device::operator bool(void) const noexcept {
    return ((this->_adapter != nullptr)
        && (this->_device != nullptr));
}


/*
 * visus::power_overwhelming::graphics_device::assert_id_and_name
 */
void visus::power_overwhelming::graphics_device::assert_id_and_name(void) const {
#if (POWER_OVERWHELMING_GPU_ABSTRACTION >= 11)
    if ((this->_adapter != nullptr) && (this->_id == nullptr)
            && (this->_name == nullptr)) {
        DXGI_ADAPTER_DESC desc;

        {
            auto hr = this->_adapter->GetDesc(&desc);
            if (FAILED(hr)) {
                throw std::system_error(hr, detail::com_category());
            }
        }

        if (this->_id == nullptr) {
            DWORD len = 0;
            if (!::CryptBinaryToStringW(
                    reinterpret_cast<BYTE *>(&desc.AdapterLuid),
                    sizeof(desc.AdapterLuid),
                    CRYPT_STRING_HEXRAW | CRYPT_STRING_NOCRLF,
                    nullptr, &len)) {
                throw std::system_error(::GetLastError(),
                    std::system_category());
            }

            this->_id = new wchar_t[len];

            if (!::CryptBinaryToStringW(
                    reinterpret_cast<BYTE *>(&desc.AdapterLuid),
                    sizeof(desc.AdapterLuid),
                    CRYPT_STRING_HEXRAW | CRYPT_STRING_NOCRLF,
                    this->_id, &len)) {
                throw std::system_error(::GetLastError(),
                    std::system_category());
            }
        }

        if (this->_name == nullptr) {
            auto len = ::wcslen(desc.Description) + 1;
            this->_name = new wchar_t[len];
            ::wcscpy_s(this->_name, len, desc.Description);
        }
    }
#endif /* (POWER_OVERWHELMING_GPU_ABSTRACTION >= 11) */
}
