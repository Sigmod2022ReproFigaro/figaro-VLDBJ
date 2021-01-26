#include "utils/Utils.h"

#include <iostream>
#include <fstream>
#include <iomanip>

namespace Figaro
{
    uint32_t getNumberOfLines(const std::string& filePath)
    {
        std::string line; 
        std::ifstream file(filePath);
        uint32_t cntLines = 0;
        if (file.fail())
        {
            FIGARO_LOG_ERROR("Path to the file is wrong:", filePath);
            return 0;
        }

        while (std::getline(file, line))
        {
            cntLines ++;
        }
        return cntLines;
    }
}

std::ostream& Figaro::outputMatrixTToCSV(std::ostream& out, 
    const Figaro::MatrixEigenT& matrix, char sep, uint32_t precision)
{
    for (uint32_t row = 0; row < matrix.rows(); row ++)
    {
        for (uint32_t col = 0; col < matrix.cols(); col++)
        {

            out << std::setprecision(precision) << std::fixed << matrix(row, col);
            if (col != (matrix.cols() - 1))
            {
                out << sep;
            }
        }
        out << std::endl;
    }
    return out;
}


std::ostream& operator<<(std::ostream& out, const Figaro::MatrixEigenT& matrix)
{
    out << "Matrix Eigen" << std::endl;
    out <<  "Matrix dimensions: " << matrix.rows() << " " << matrix.cols() << std::endl;
    return Figaro::outputMatrixTToCSV(out, matrix);
}