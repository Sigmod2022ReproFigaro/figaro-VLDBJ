#ifndef _FIGARO_UTILS_TEST_H_
#define _FIGARO_UTILS_TEST_H_

#include <gtest/gtest.h>
#include <string>
#include "utils/Utils.h"
extern std::string GL_DATA_PATH;
const std::string CONFIG_PATH = "../tests/";
const std::string DB_CONFIG_PATH_IN = "database_specs.conf";
const std::string QUERY_CONFIG_PATH_IN = "query_specs.conf";
const std::string R_COMP_FILE_NAME = "expectedR.csv";
constexpr double QR_TEST_PRECISION_ERROR = 1e-12;
constexpr double GIVENS_TEST_PRECISION_ERROR = 1e-14;

std::string getConfigPath(uint32_t idx);
std::string getDataPath(uint32_t idx);
void readMatrixDense(const std::string& sPath, Figaro::MatrixEigenT& rmA, char sep = ',');
void compareMatrices(Figaro::MatrixEigenT& R, Figaro::MatrixEigenT& expR, bool cmpRowNum=true, bool cmpColNum=true);

#endif
