#pragma once

#include <chrono>

using timestamp = std::chrono::time_point<std::chrono::high_resolution_clock, std::chrono::microseconds>;
using duration_us = std::chrono::microseconds;

// Declare literal operators
duration_us operator"" _us(unsigned long long us);
duration_us operator"" _ns(unsigned long long ns);
duration_us operator"" _ms(unsigned long long ms);
duration_us operator"" _s(unsigned long long s);
duration_us operator"" _min(unsigned long long min);
duration_us operator"" _h(unsigned long long h);