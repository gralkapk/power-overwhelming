#include "sol_rtx_instrument.h"

#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>

#include "power_overwhelming/rtx_instrument.h"


void visus::power_overwhelming::sol_rtx_instrument(void* state) {
    sol::state_view lua(reinterpret_cast<lua_State*>(state));

    auto rtx_instrument_table = lua.new_usertype<rtx_instrument>("rtx_instrument",
        sol::constructors<rtx_instrument(),
            rtx_instrument(const char* path, const visa_instrument::timeout_type timeout),
            rtx_instrument(std::reference_wrapper<bool>, const char*, const visa_instrument::timeout_type)>());
#if 0
    rtx_instrument_table["acquisition"] =
        sol::overload(static_cast<rtx_instrument& (rtx_instrument::*)(_In_ const oscilloscope_single_acquisition&,
                          _In_ const bool, _In_ const bool)>(&rtx_instrument::acquisition),
            static_cast<oscilloscope_acquisition_state (rtx_instrument::*)() const>(&rtx_instrument::acquisition)/*,
            static_cast<const rtx_instrument& (rtx_instrument::*)(_In_ const oscilloscope_acquisition_state,
                _In_ const bool) const>(&rtx_instrument::acquisition)*/);
#endif

    rtx_instrument_table["acquisition"] =
        static_cast<rtx_instrument& (rtx_instrument::*)(const oscilloscope_single_acquisition&, const bool,
            const bool)>(&rtx_instrument::acquisition);

    rtx_instrument_table["channel"] =
        static_cast<rtx_instrument& (rtx_instrument::*)(const oscilloscope_channel&)>(&rtx_instrument::channel);

    rtx_instrument_table["reference_position"] =
        static_cast<rtx_instrument& (rtx_instrument::*)(const oscilloscope_reference_point)>(
            &rtx_instrument::reference_position);

    rtx_instrument_table["trigger_position"] = &rtx_instrument::trigger_position;

    rtx_instrument_table["trigger"] =
        static_cast<rtx_instrument& (rtx_instrument::*)(const oscilloscope_trigger&)>(&rtx_instrument::trigger);
}


void visus::power_overwhelming::sol_oscilloscope_single_acquisition(void* state) {
    sol::state_view lua(reinterpret_cast<lua_State*>(state));

    auto acq_table = lua.new_usertype<oscilloscope_single_acquisition>(
        "oscilloscope_single_acquisition", sol::constructors<oscilloscope_single_acquisition()>());

    acq_table["count"] =
        static_cast<oscilloscope_single_acquisition& (oscilloscope_single_acquisition::*)(const unsigned int)>(
            &oscilloscope_single_acquisition::count);

    acq_table["points"] =
        static_cast<oscilloscope_single_acquisition& (oscilloscope_single_acquisition::*)(const unsigned int)>(
            &oscilloscope_single_acquisition::points);

    acq_table["segmented"] =
        static_cast<oscilloscope_single_acquisition& (oscilloscope_single_acquisition::*)(const bool)>(
            &oscilloscope_single_acquisition::segmented);
}
