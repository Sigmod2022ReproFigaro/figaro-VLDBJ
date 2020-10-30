#ifndef _FIGARO_UTILS_TEST_H_
#define _FIGARO_UTILS_TEST_H_

#include <gtest/gtest.h>
#include <string>

const std::string TEST_PATH = "../tests/data/";
const std::string DB_CONFIG_PATH_IN = "database_specs.conf";
std::string getTestPath(uint32_t idx);

#endif
