#ifndef _FIGARO_UTILS_TEST_H_
#define _FIGARO_UTILS_TEST_H_

#include <gtest/gtest.h>
#include <string>
#include "utils/Utils.h"
const std::string TEST_PATH = "../tests/";
const std::string DB_CONFIG_PATH_IN = "database_specs.conf";
const std::string QUERY_CONFIG_PATH_IN = "query_specs.conf";
const std::string R_COMP_FILE_NAME = "expectedR.csv";
constexpr double QR_TEST_PRECISION_ERROR = 1e-13;

std::string getTestPath(uint32_t idx);
std::string getDataPath(uint32_t idx);
void readMatrixDense(const std::string& sPath, Figaro::MatrixT& rmA, char sep = ',');
void compareMatrices(Figaro::MatrixT& R, Figaro::MatrixT& expR, bool cmpRowNum=true, bool cmpColNum=true);

#endif
