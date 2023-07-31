#include "posol/sol_rtx_instrument.h"

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

    lua.set_function("find_resources", [](const char*, const char*) -> std::vector<rtx_instrument> {
        std::vector<rtx_instrument> ret;

        auto devices = visa_instrument::find_resources("0x0AAD", "0x01D6");

        for (auto d = devices.as<char>(); (d != nullptr) && (*d != 0); d += strlen(d) + 1) {
            ret.emplace_back(d);
        }

        return ret;
    });
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


void visus::power_overwhelming::sol_oscilloscope_reference_point(void* state) {
    sol::state_view lua(reinterpret_cast<lua_State*>(state));

    lua.new_enum<oscilloscope_reference_point>("oscilloscope_reference_point",
        {{"left", oscilloscope_reference_point::left}, {"middle", oscilloscope_reference_point::left},
            {"right", oscilloscope_reference_point::right}});
}


void visus::power_overwhelming::sol_oscilloscope_channel(void* state) {
    sol::state_view lua(reinterpret_cast<lua_State*>(state));

    auto channel_table = lua.new_usertype<oscilloscope_channel>(
        "oscilloscope_channel", sol::constructors<oscilloscope_channel(const std::uint32_t),
                                    oscilloscope_channel(const std::uint32_t, const oscilloscope_channel&)>());

    channel_table["attenuation"] =
        static_cast<oscilloscope_channel& (oscilloscope_channel::*)(const oscilloscope_quantity&)>(
            &oscilloscope_channel::attenuation);

    channel_table["label"] = static_cast<oscilloscope_channel& (oscilloscope_channel::*)(const oscilloscope_label&)>(
        &oscilloscope_channel::label);

    channel_table["state"] =
        static_cast<oscilloscope_channel& (oscilloscope_channel::*)(const bool)>(&oscilloscope_channel::state);
}


void visus::power_overwhelming::sol_oscilloscope_edge_trigger(void* state) {
    sol::state_view lua(reinterpret_cast<lua_State*>(state));

    auto trigger_table = lua.new_usertype<oscilloscope_edge_trigger>(
        "oscilloscope_edge_trigger", sol::constructors<oscilloscope_edge_trigger(const char*)>());

    trigger_table["level"] = sol::overload(
        static_cast<oscilloscope_edge_trigger& (
            oscilloscope_edge_trigger::*)(const oscilloscope_edge_trigger::input_type, const oscilloscope_quantity&)>(
            &oscilloscope_edge_trigger::level),
        static_cast<oscilloscope_edge_trigger& (oscilloscope_edge_trigger::*)(const oscilloscope_quantity&)>(
            &oscilloscope_edge_trigger::level));

    trigger_table["slope"] =
        static_cast<oscilloscope_edge_trigger& (oscilloscope_edge_trigger::*)(const oscilloscope_trigger_slope)>(
            &oscilloscope_edge_trigger::slope);

    trigger_table["mode"] =
        static_cast<oscilloscope_trigger& (oscilloscope_edge_trigger::*)(const oscilloscope_trigger_mode)>(
            &oscilloscope_edge_trigger::mode);

    lua.new_enum<oscilloscope_trigger_slope>("oscilloscope_trigger_slope",
        {{"both", oscilloscope_trigger_slope::both}, {"rising", oscilloscope_trigger_slope::rising},
            {"falling", oscilloscope_trigger_slope::falling}});

    lua.new_enum<oscilloscope_trigger_mode>("oscilloscope_trigger_mode",
        {{"automatic", oscilloscope_trigger_mode::automatic}, {"normal", oscilloscope_trigger_mode::normal}});
}


void visus::power_overwhelming::sol_oscilloscope_quantity(void* state) {
    sol::state_view lua(reinterpret_cast<lua_State*>(state));

    auto quant_table = lua.new_usertype<oscilloscope_quantity>(
        "oscilloscope_quantity", sol::constructors<oscilloscope_quantity(const float, const char* unit)>());
}


void visus::power_overwhelming::sol_oscilloscope_label(void* state) {
    sol::state_view lua(reinterpret_cast<lua_State*>(state));

    auto label_table = lua.new_usertype<oscilloscope_label>(
        "oscilloscope_label", sol::constructors<oscilloscope_label(), oscilloscope_label(const char*, const bool)>());
}


void visus::power_overwhelming::sol_register_all(void* state) {
    sol_rtx_instrument(state);

    sol_oscilloscope_single_acquisition(state);

    sol_oscilloscope_reference_point(state);

    sol_oscilloscope_channel(state);

    sol_oscilloscope_edge_trigger(state);

    sol_oscilloscope_quantity(state);

    sol_oscilloscope_label(state);
}
