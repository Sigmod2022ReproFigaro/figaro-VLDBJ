#ifndef _FIGARO_LOGGER_H_
#define _FIGARO_LOGGER_H_

#define FIGARO_LOG_LEVEL_DEBUG 0
#define FIGARO_LOG_LEVEL_INFO  1
#define FIGARO_LOG_LEVEL_BENCH 2
#define FIGARO_LOG_LEVEL_WARNING 3
#define FIGARO_LOG_LEVEL_ERROR 4

// In the case if log level is defined by CmakeLists.txt we do not want
// want to redefine it. 
#ifndef FIGARO_LOG_LEVEL 
#define FIGARO_LOG_LEVEL 0
#endif 

#include <iostream>

// TODO:
// 1) Add module definition for benchmark

template<typename Arg1, typename Arg2, typename ...Args>
void figaroLogStd(Arg1&& severity, Arg2&& arg, Args&& ...args)
{
    std::cout << severity << " ";
    std::cout << arg;
    ((std::cout << " " << std::forward<Args>(args)), ...) << std::endl;
}

template<typename Arg1, typename Arg2>
void figaroLogStd(Arg1&& severity, Arg2&& arg)
{
    std::cout << severity << " ";
    std::cout << arg << std::endl;
}

template<typename Arg, typename ...Args>
void figaroLogBench(Arg&& module, Args&& ...args)
{
    std::cout << "##" << module << "##";
    ((std::cout << "##" << std::forward<Args>(args)), ...) << std::endl;
}


#define FIGARO_LOG_INTERNAL_FUN(SEVERITY, ...) \
do                                            \
{                                             \
    figaroLogStd(#SEVERITY, __VA_ARGS__);      \
} while(0);


#define FIGARO_LOG_INTERNAL(SEVERITY, ...)     \
FIGARO_LOG_INTERNAL_FUN(SEVERITY, __FILE__,    \
     __FUNCTION__, __LINE__, __VA_ARGS__);    \

#define FIGARO_LOG_BENCH_INTERNAL(...) \
do                                        \
{                                         \
    figaroLogStd(__VA_ARGS__);             \
} while(0);

#define FIGARO_LOG_DBG(...)
#define FIGARO_LOG_INFO(...)
#define FIGARO_LOG_BENCH(...)
#define FIGARO_LOG_WARNING(...)
#define FIGARO_LOG_ERROR(...)


#if FIGARO_LOG_LEVEL <= FIGARO_LOG_LEVEL_DEBUG
#undef FIGARO_LOG_DBG
//#define FIGARO_LOG_DBG(...) FIGARO_LOG_INTERNAL(DEBUG, MSG)
#define FIGARO_LOG_DBG(...) FIGARO_LOG_INTERNAL(DEBUG, __VA_ARGS__)
#endif

#if FIGARO_LOG_LEVEL <= FIGARO_LOG_LEVEL_INFO
#undef FIGARO_LOG_INFO
#define FIGARO_LOG_INFO(...) FIGARO_LOG_INTERNAL(INFO, __VA_ARGS__)
#endif

#if FIGARO_LOG_LEVEL <= FIGARO_LOG_LEVEL_BENCH
#undef FIGARO_LOG_BENCH
#define FIGARO_LOG_BENCH(...) FIGARO_LOG_BENCH_INTERNAL(__VA_ARGS__)
#endif

#if FIGARO_LOG_LEVEL <= FIGARO_LOG_LEVEL_WARNING
#undef FIGARO_LOG_WARNING
#define FIGARO_LOG_WARNING(...) FIGARO_LOG_INTERNAL(WARNING, __VA_ARGS__)
#endif

#if FIGARO_LOG_LEVEL <= FIGARO_LOG_LEVEL_ERROR
#undef FIGARO_LOG_ERROR
#define FIGARO_LOG_ERROR(...) FIGARO_LOG_INTERNAL(ERRROR, __VA_ARGS__)
#endif

#endif
