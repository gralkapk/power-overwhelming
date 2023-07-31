#pragma once

namespace visus::power_overwhelming {

inline void sol_rtx_instrument(void* state);

inline void sol_oscilloscope_single_acquisition(void* state);

inline void sol_oscilloscope_reference_point(void* state);

inline void sol_oscilloscope_channel(void* state);

inline void sol_oscilloscope_edge_trigger(void* state);

inline void sol_oscilloscope_quantity(void* state);

inline void sol_oscilloscope_label(void* state);

inline void sol_register_all(void* state);
} // namespace visus::power_overwhelming
