#include "UtilTest.h"
#include <iostream>
#include <fstream>

std::string GL_DATA_PATH = "/home/username/Figaro/data";

std::string getConfigPath(uint32_t idx)
{
    return CONFIG_PATH + "test" + std::to_string(idx) + "/";
}

std::string getDataPath(uint32_t idx)
{
    return GL_DATA_PATH + "/unit_test_databases/test" + std::to_string(idx) + "/";
}

void readMatrixDense(const std::string& sPath, Figaro::MatrixEigenT& rmA, char sep)
{
    std::ifstream input(sPath);
    std::stringstream lineStream;
    std::string line;
    std::string cell;
    uint32_t rowNum = 0;
    uint32_t colNum = 0;
    uint32_t row = 0;
    uint32_t col = 0;

    input >> rowNum >> colNum;
    getline(input, line);
    FIGARO_LOG_DBG("PATH", sPath);
    FIGARO_LOG_DBG("RowNum:", rowNum, "ColNum:", colNum);
    rmA.resize(rowNum, colNum);
    while (getline(input, line))
    {
        lineStream << line;
        col = 0;
        // Iterates column by column in search for values
        // separated by sep.
        //
        while (std::getline(lineStream, cell, sep))
        {
            double val = std::stod(cell);
            //FIGARO_LOG_DBG("RowNum:", row, "ColNum:", col, "Val:", val);
            rmA(row, col) = val;
            col ++;
        }
        lineStream.clear();
        row ++;
    }
    input.close();
}

void compareMatrices(Figaro::MatrixEigenT& R, Figaro::MatrixEigenT& expR, bool cmpRowNum, bool cmpColNum)
{
    if (cmpRowNum)
    {
        EXPECT_EQ(R.rows(), expR.rows());
    }
    if (cmpColNum)
    {
        EXPECT_EQ(R.cols(), expR.cols());
    }
    for (uint32_t row = 0; row < expR.rows(); row ++)
    {
        for (uint32_t col = 0; col < expR.cols(); col++)
        {
            EXPECT_NEAR(R(row, col), expR(row, col), QR_TEST_PRECISION_ERROR);
        }
    }
}