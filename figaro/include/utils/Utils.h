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
#include "utils/Logger.h"
#include "utils/ErrorCode.h"

namespace Figaro 
{
    typedef nlohmann::json json;
    typedef Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::ColMajor> MatrixT;
    typedef Eigen::VectorXd VectorT;
    typedef Eigen::ArrayXd ArrayT;
    
    // TODO: move to Utils namespace
    uint32_t getNumberOfLines(const std::string& filePath);   
}

std::ostream& operator<<(std::ostream& out, const Figaro::MatrixT& matrix);

#endif