#include "utils/Utils.h"

#include <iostream>
#include <fstream>

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

    std::ostream& printMatrix(std::ostream& out, const MatrixT& matrix, char sep)
    {
        uint32_t rowNum = 0;
        uint32_t colNum = 0;
        uint32_t row = 0;
        uint32_t col = 0;

        out <<  "Matrix dimensions: " << matrix.rows() << " " << matrix.cols() << std::endl;
        for (uint32_t row = 0; row < matrix.rows(); row ++)
        {
            for (uint32_t col = 0; col < matrix.cols(); col++)
            {

                out << matrix(row, col);
                if (col != (matrix.cols() - 1))
                {
                    out << sep;
                }
            }
            out << std::endl;
        }
        return out;
    }
    
}