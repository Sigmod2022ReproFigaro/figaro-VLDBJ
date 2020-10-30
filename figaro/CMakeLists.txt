cmake_minimum_required(VERSION 3.9)

project(figaro VERSION 1.0.0 LANGUAGES CXX)

set(CMKAKE_CXX_STANDARD 17)
set(CMKAKE_CXX_STANDARD REQUIRED)

#TODO: Replace flags with compiler options. 
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -std=c++17 -pthread -g -Wall -Wextra -Og")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -std=c++17 -pthread -Ofast -mtune=native -fassociative-math -freciprocal-math -fno-signed-zeros -frename-registers -fopenmp")
else()
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -std=c++17 -pthread -g -Wall -Wextra -O0")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -std=c++17  -Ofast")
endif()
set(CMAKE_CXX_FLAGS_BENCHMARK "${CMAKE_CXX_FLAGS_RELEASE} -DBENCH")
# Set to Debug, Benchmark or Release.
set(CMAKE_BUILD_TYPE Benchmark)

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

# Add libraries.
if (FIGARO_RUN)
  target_link_libraries(figaro ${Boost_LIBRARIES})
endif()

if (FIGARO_TEST)
  add_compile_definitions(FIGARO_LOG_LEVEL=1)
  # Take care of Gtest library.
  find_package(GTest REQUIRED)
  target_include_directories(figaro_test PRIVATE ${GTEST_INCLUDE_DIRS})
  target_link_libraries(figaro_test ${Boost_LIBRARIES} ${GTEST_LIBRARIES})
endif()


#Builds static library which will be loaded in runtime.
if (FIGARO_LIB)
  add_library(figarolib STATIC ${SRC_FILES} )
endif()
