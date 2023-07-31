#pragma once

namespace visus::power_overwhelming {

void sol_rtx_instrument(void* state);

void sol_oscilloscope_single_acquisition(void* state);

void sol_oscilloscope_reference_point(void* state);

void sol_oscilloscope_channel(void* state);

void sol_oscilloscope_edge_trigger(void* state);
} // namespace visus::power_overwhelming
