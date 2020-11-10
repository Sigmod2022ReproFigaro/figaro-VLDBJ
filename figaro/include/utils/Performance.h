#ifndef _FIGARO_PERFORMANCE_H_

#define INIT_MICRO_BENCH(timer)\
auto begin_##timer = std::chrono::high_resolution_clock::now();\
auto finish_##timer = std::chrono::high_resolution_clock::now();\
std::chrono::duration<double>  elapsed_##timer = finish_##timer - begin_##timer;\
double time_spent_##timer = elapsed_##timer.count();\
double global_time_##timer = 0;

#define BEGIN_MICRO_BENCH(timer) begin_##timer = std::chrono::high_resolution_clock::now();

#define END_MICRO_BENCH(timer) finish_##timer = std::chrono::high_resolution_clock::now();\
elapsed_##timer = finish_##timer - begin_##timer;\
time_spent_##timer = elapsed_##timer.count();\
global_time_##timer += time_spent_##timer;

#define PRINT_MICRO_BENCH(timer) std::cout << #timer << ": " << std::to_string(global_time_##timer) << std::endl;

#endif 