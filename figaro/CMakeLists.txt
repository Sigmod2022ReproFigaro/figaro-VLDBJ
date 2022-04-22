cmake_minimum_required(VERSION 3.9)

project(figaro VERSION 1.0.0 LANGUAGES CXX)

set(CMKAKE_CXX_STANDARD 20)
set(CMKAKE_CXX_STANDARD REQUIRED)

#TODO: Replace flags with compiler options.
# Checking compiler
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -std=c++20 -g -Wall -Wextra -Og -fopenmp -m64")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -std=c++20 -pthread -Ofast -mtune=native -fassociative-math -freciprocal-math -fno-signed-zeros -frename-registers -fopenmp -m64 ")
endif()
#set(CMAKE_CXX_FLAGS_BENCHMARK "${CMAKE_CXX_FLAGS_RELEASE} -DBENCH")
# Set to Debug, Benchmark or Release.

if (FIGARO_DEBUG)
  set(CMAKE_BUILD_TYPE Debug)
  message("Debug")
else()
  message("Release")
  set(CMAKE_BUILD_TYPE Release)
endif()

# MKL linkage
#set(FIGARO_MKL_GCC_LINK_PATH "/local/scratch/local/intel/tbb/2021.2.0/lib/intel64/gcc4.8")
#set(FIGARO_MKL_INCLUDE_PATH "/local/scratch/local/intel/mkl/2021.2.0/include")
set(FIGARO_MKL_INCLUDE_PATH)


add_definitions( -DEIGEN_USE_BLAS -DEIGEN_USE_LAPACKE -DEIGEN_USE_MKL_ALL	 -DMKL_ILP64 )
#add_definitions( -DEIGEN_USE_BLAS -DEIGEN_USE_LAPACKE)

# TODO: Clean this.
set(FIGARO_ROOT .)
set(FIGARO_TEST_PATH ${FIGARO_ROOT}/tests)
set(FIGARO_INCLUDE ${FIGARO_ROOT}/include)

include_directories(${FIGARO_INCLUDE})
# TODO: Remove GLOB. Add include directories to remove this clutter.
file(GLOB_RECURSE SRC_FILES src/*.cpp)
list(FILTER SRC_FILES EXCLUDE REGEX ".*src/main.cpp$")
if (FIGARO_RUN)
  add_executable(figaro src/main.cpp ${SRC_FILES})
endif()

if (FIGARO_TEST)
  file(GLOB_RECURSE TEST_SRC_FILES tests/*.cpp)
  list(FILTER TEST_SRC_FILES EXCLUDE REGEX ".*tests/main.cpp$")
  # Test environment.
  add_executable(figaro_test tests/main.cpp ${SRC_FILES} ${TEST_SRC_FILES})
  target_include_directories(figaro_test PRIVATE ${FIGARO_TEST_PATH})
endif()

set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost REQUIRED COMPONENTS system thread iostreams program_options)
include_directories(${Boost_INCLUDE_DIRS})

if (FIGARO_DEBUG OR FIGARO_TEST)
  add_compile_definitions(FIGARO_LOG_LEVEL=0)
elseif(FIGARO_INFO)
  add_compile_definitions(FIGARO_LOG_LEVEL=1)
endif()


#link_directories(${FIGARO_MKL_GCC_LINK_PATH})

# Add libraries.
if (FIGARO_RUN)
  #target_link_libraries(figaro ${Boost_LIBRARIES} -ltbb -lopenblas -llapacke)

  target_include_directories(figaro PRIVATE ${FIGARO_MKL_INCLUDE_PATH})
  target_link_libraries(figaro -Wl,--start-group libmkl_intel_ilp64.a libmkl_gnu_thread.a libmkl_core.a -Wl,--end-group -lgomp  -lstdc++ -lpthread -ltbb  -lm -ldl ${Boost_LIBRARIES})
endif()

if (FIGARO_TEST)
  # Take care of Gtest library.
  #target_link_libraries(figaro_test ${Boost_LIBRARIES} ${GTEST_LIBRARIES} -ltbb -lopenblas -llapacke  -Wl,--verbose)
  find_package(GTest REQUIRED)
  target_include_directories(figaro_test PRIVATE ${GTEST_INCLUDE_DIRS} ${FIGARO_MKL_INCLUDE_PATH})
  target_link_libraries(figaro_test -Wl,--start-group libmkl_intel_ilp64.a libmkl_gnu_thread.a libmkl_core.a -Wl,--end-group -lgomp  -lstdc++ -lpthread -ltbb  -lm -ldl  ${Boost_LIBRARIES} ${GTEST_LIBRARIES} -Wl,--verbose )
endif()


#Builds static library which will be loaded in runtime.
if (FIGARO_LIB)
  add_library(figarolib STATIC ${SRC_FILES} )
endif()
