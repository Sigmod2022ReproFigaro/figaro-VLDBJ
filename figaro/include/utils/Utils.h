#ifndef _FIGARO_UTILS_H_
#define _FIGARO_UTILS_H_

#include <Eigen/Dense>
// There is a bug because Eigen include C complex.h which has I defined as macro
// while boost uses I as classname, and thus there is a clash in naming.
// I is not used anywhere in eigen as a variable except in src/SparseLU/SparseLU_gemm_kernel.h:
// which doesn't include any files, thus I is not included.
//
#undef I
#include <nlohmann/json.hpp>
#include <string>
#include <fstream>
#include "utils/Logger.h"

namespace Figaro 
{
    typedef nlohmann::json json;
    typedef Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> MatrixT;

    uint32_t getNumberOfLines(const std::string& filePath);

    std::ostream& printMatrix(std::ostream& out, const MatrixT& matrix, char sep = ' ');
}

#endif