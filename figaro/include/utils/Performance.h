#ifndef _FIGARO_PERFORMANCE_H_

#include "utils/Utils.h"
#include <chrono>

#define MICRO_BENCH_INIT(timer)\
auto begin_##timer = std::chrono::high_resolution_clock::now();\
auto finish_##timer = std::chrono::high_resolution_clock::now();\
std::chrono::duration<double>  elapsed_##timer = finish_##timer - begin_##timer;\
double time_spent_##timer = elapsed_##timer.count();\
double global_time_##timer = 0;

#define MICRO_BENCH_START(timer) begin_##timer = std::chrono::high_resolution_clock::now();

#define MICRO_BENCH_STOP(timer) finish_##timer = std::chrono::high_resolution_clock::now();\
elapsed_##timer = finish_##timer - begin_##timer;\
time_spent_##timer = elapsed_##timer.count();\
global_time_##timer += time_spent_##timer;

#define MICRO_BENCH_GET_TIMER(timer)  std::to_string(global_time_##timer)
#define MICRO_BENCH_GET_TIMER_LAP(timer)  std::to_string(time_spent_##timer)

#endif