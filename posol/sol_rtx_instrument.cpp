#include "sol_rtx_instrument.h"

#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>

#include "power_overwhelming/rtx_instrument.h"


void visus::power_overwhelming::sol_rtx_instrument(void* state) {
    sol::state_view lua(reinterpret_cast<lua_State*>(state));

    auto rtx_instrument_table = lua.new_usertype<rtx_instrument>(
        "rtx_instrument", sol::constructors<rtx_instrument(),
                              rtx_instrument(_In_z_ const char* path, _In_ const visa_instrument::timeout_type timeout),
                              rtx_instrument(_Out_ std::reference_wrapper<bool>, _In_z_ const char*,
                                  _In_ const visa_instrument::timeout_type)>());

    rtx_instrument_table["acquisition"] =
        sol::overload(static_cast<rtx_instrument& (rtx_instrument::*)(_In_ const oscilloscope_single_acquisition&,
                          _In_ const bool, _In_ const bool)>(&rtx_instrument::acquisition),
            static_cast<oscilloscope_acquisition_state (rtx_instrument::*)() const>(&rtx_instrument::acquisition)/*,
            static_cast<const rtx_instrument& (rtx_instrument::*)(_In_ const oscilloscope_acquisition_state,
                _In_ const bool) const>(&rtx_instrument::acquisition)*/);
}
