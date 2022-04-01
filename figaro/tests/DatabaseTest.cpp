#include "UtilTest.h"
#include "database/storage/ArrayStorage.h"
#include "database/storage/Matrix.h"
#include "database/Database.h"
#include "database/Relation.h"
#include "database/query/Query.h"
#include <vector>
#include <string>
#include <cmath>

using namespace Figaro;

TEST(Storage, SimpleArrayStorage)
{
    static constexpr uint32_t SIZE = 5;
    Figaro::ArrayStorage<double> arrayStorage(SIZE);
    EXPECT_EQ(arrayStorage.getSize(), SIZE);
    for (uint32_t idx = 0; idx < arrayStorage.getSize(); idx++)
    {
        arrayStorage[idx] = idx;
    }

    for (uint32_t idx = 0; idx < arrayStorage.getSize(); idx++)
    {
        EXPECT_EQ(arrayStorage[idx], idx);
    }
}

TEST(Matrix, Basic)
{
    static constexpr uint32_t NUM_ROWS = 5, NUM_COLS = 4;
    Figaro::Matrix<double> matrix(NUM_ROWS, NUM_COLS);

    EXPECT_EQ(matrix.getNumRows(), NUM_ROWS);
    EXPECT_EQ(matrix.getNumCols(), NUM_COLS);
    for (uint32_t rowIdx = 0; rowIdx < matrix.getNumRows(); rowIdx ++)
    {
        for (uint32_t colIdx = 0; colIdx < matrix.getNumCols(); colIdx++)
        {
            matrix[rowIdx][colIdx] = rowIdx * NUM_COLS + colIdx;
        }
    }

    for (uint32_t rowIdx = 0; rowIdx < matrix.getNumRows(); rowIdx ++)
    {
        for (uint32_t colIdx = 0; colIdx < matrix.getNumCols(); colIdx++)
        {
            EXPECT_EQ(matrix[rowIdx][colIdx], rowIdx * NUM_COLS + colIdx);
        }
    }
}

TEST(Matrix, Resize)
{
    static constexpr uint32_t NUM_ROWS = 5, NEW_NUM_ROWS = 3, NUM_COLS = 4;
    Figaro::Matrix<double> matrix(0, NUM_COLS);

    EXPECT_EQ(matrix.getNumRows(), 0);
    EXPECT_EQ(matrix.getNumCols(), NUM_COLS);

    matrix.resize(NUM_ROWS);
    EXPECT_EQ(matrix.getNumRows(), NUM_ROWS);
    for (uint32_t rowIdx = 0; rowIdx < matrix.getNumRows(); rowIdx ++)
    {
        for (uint32_t colIdx = 0; colIdx < matrix.getNumCols(); colIdx++)
        {
            matrix[rowIdx][colIdx] = rowIdx * NUM_COLS + colIdx;
        }
    }

    for (uint32_t rowIdx = 0; rowIdx < matrix.getNumRows(); rowIdx ++)
    {
        for (uint32_t colIdx = 0; colIdx < matrix.getNumCols(); colIdx++)
        {
            EXPECT_EQ(matrix[rowIdx][colIdx], rowIdx * NUM_COLS + colIdx);
        }
    }

    matrix.resize(NEW_NUM_ROWS);
    EXPECT_EQ(matrix.getNumRows(), NEW_NUM_ROWS);
    for (uint32_t rowIdx = 0; rowIdx < matrix.getNumRows(); rowIdx ++)
    {
        for (uint32_t colIdx = 0; colIdx < matrix.getNumCols(); colIdx++)
        {
            EXPECT_EQ(matrix[rowIdx][colIdx], rowIdx * NUM_COLS + colIdx);
        }
    }
}

TEST(Matrix, Block)
{
    static constexpr uint32_t NUM_ROWS = 3,  NUM_COLS = 2;
    Figaro::Matrix<double> matrix1(NUM_ROWS, NUM_COLS);

    matrix1[0][0] = 1; matrix1[0][1] = 2;
    matrix1[1][0] = 3; matrix1[1][1] = 4;
    matrix1[2][0] = 5; matrix1[2][1] = 6;

    const auto&& blockMat = matrix1.getBlock(1, 2, 0, 0);
    EXPECT_EQ(blockMat.getNumCols(), 1);
    EXPECT_EQ(blockMat.getNumRows(), 2);
    EXPECT_EQ(blockMat[0][0], 3);
    EXPECT_EQ(blockMat[0][1], 5);

    const auto&& rightCols = matrix1.getRightCols(1);
    EXPECT_EQ(rightCols.getNumCols(), 1);
    EXPECT_EQ(rightCols.getNumRows(), 3);
    EXPECT_EQ(rightCols[0][0], 2);
    EXPECT_EQ(rightCols[0][1], 4);
    EXPECT_EQ(rightCols[0][2], 6);

    const auto&& leftCols = matrix1.getLeftCols(1);
    EXPECT_EQ(leftCols.getNumCols(), 1);
    EXPECT_EQ(leftCols.getNumRows(), 3);
    EXPECT_EQ(leftCols[0][0], 1);
    EXPECT_EQ(leftCols[0][1], 3);
    EXPECT_EQ(leftCols[0][2], 5);

    const auto&& topRows = matrix1.getTopRows(1);
    EXPECT_EQ(topRows.getNumCols(), 2);
    EXPECT_EQ(topRows.getNumRows(), 1);
    EXPECT_EQ(topRows[0][0], 1);
    EXPECT_EQ(topRows[0][1], 2);

    const auto&& bottomRows = matrix1.getBottomRows(1);
    EXPECT_EQ(bottomRows.getNumCols(), 2);
    EXPECT_EQ(bottomRows.getNumRows(), 1);
    EXPECT_EQ(bottomRows[0][0], 5);
    EXPECT_EQ(bottomRows[0][1], 6);

}

TEST(Matrix, ConcatenateVertically)
{
    static constexpr uint32_t NUM_ROWS1 = 3, NUM_ROWS2 = 2, NUM_COLS = 2;
    Figaro::Matrix<double> matrix1(NUM_ROWS1, NUM_COLS);
    Figaro::Matrix<double> matrix2(NUM_ROWS2, NUM_COLS);

    matrix1[0][0] = 1; matrix1[0][1] = 2;
    matrix1[1][0] = 3; matrix1[1][1] = 4;
    matrix1[2][0] = 5; matrix1[2][1] = 6;

    matrix2[0][0] = 7; matrix2[0][1] = 8;
    matrix2[1][0] = 9; matrix2[1][1] = 10;

    Figaro::Matrix<double> matrix = matrix1.concatenateVertically(matrix2);
    matrix = matrix.concatenateVerticallyScalar(0, 2);

    EXPECT_EQ(matrix[0][0], 1);
    EXPECT_EQ(matrix[0][1], 2);
    EXPECT_EQ(matrix[1][0], 3);
    EXPECT_EQ(matrix[1][1], 4);
    EXPECT_EQ(matrix[2][0], 5);
    EXPECT_EQ(matrix[2][1], 6);
    EXPECT_EQ(matrix[3][0], 7);
    EXPECT_EQ(matrix[3][1], 8);
    EXPECT_EQ(matrix[4][0], 9);
    EXPECT_EQ(matrix[4][1], 10);

    EXPECT_EQ(matrix[5][0], 0);
    EXPECT_EQ(matrix[5][1], 0);
    EXPECT_EQ(matrix[6][0], 0);
    EXPECT_EQ(matrix[6][1], 0);
}

TEST(Matrix, ConcatenateHorizontally)
{
    static constexpr uint32_t NUM_ROWS = 2, NUM_COLS1 = 2, NUM_COLS2 = 3;
    Figaro::Matrix<double> matrix1(NUM_ROWS, NUM_COLS1);
    Figaro::Matrix<double> matrix2(NUM_ROWS, NUM_COLS2);

    matrix1[0][0] = 1; matrix1[0][1] = 2;
    matrix1[1][0] = 3; matrix1[1][1] = 4;

    matrix2[0][0] = 5; matrix2[0][1] = 6; matrix2[0][2] = 7;
    matrix2[1][0] = 8 ;matrix2[1][1] = 9; matrix2[1][2] = 10;

    Figaro::Matrix<double> matrix = matrix1.concatenateHorizontally(matrix2);
    matrix = matrix.concatenateHorizontallyScalar(0, 2);

    EXPECT_EQ(matrix[0][0], 1);
    EXPECT_EQ(matrix[0][1], 2);
    EXPECT_EQ(matrix[1][0], 3);
    EXPECT_EQ(matrix[1][1], 4);

    EXPECT_EQ(matrix[0][2], 5);
    EXPECT_EQ(matrix[0][3], 6);
    EXPECT_EQ(matrix[0][4], 7);
    EXPECT_EQ(matrix[1][2], 8);
    EXPECT_EQ(matrix[1][3], 9);
    EXPECT_EQ(matrix[1][4], 10);

    EXPECT_EQ(matrix[0][5], 0);
    EXPECT_EQ(matrix[0][6], 0);
    EXPECT_EQ(matrix[1][5], 0);
    EXPECT_EQ(matrix[1][6], 0);
}


TEST(Matrix, Zeros)
{
    static constexpr uint32_t NUM_ROWS = 2, NUM_COLS = 3;
    Figaro::Matrix<double> matrix = Figaro::Matrix<double>::zeros(NUM_ROWS, NUM_COLS);

    for(uint32_t rowIdx = 0; rowIdx < NUM_ROWS; rowIdx++)
    {
        for (uint32_t colIdx = 0; colIdx < NUM_COLS; colIdx++)
        {
            EXPECT_EQ(matrix[rowIdx][colIdx], 0);
        }
    }
}

TEST(Matrix, ApplyGivens)
{
    static constexpr uint32_t NUM_ROWS = 3, NUM_COLS = 2;
    Figaro::Matrix<double> matrix(NUM_ROWS, NUM_COLS);

    matrix[0][0] = 1; matrix[0][1] = 2;
    matrix[1][0] = 3; matrix[1][1] = 4;
    matrix[2][0] = 4; matrix[2][1] = 3;

    double upperVal = matrix[1][0];
    double lowerVal = matrix[2][0];
    double r = std::sqrt(upperVal * upperVal + lowerVal * lowerVal);
    double cosTheta = upperVal / r;
    double sinTheta = -lowerVal / r;

    matrix.applyGivens(1, 2, 0, sinTheta, cosTheta);

    EXPECT_EQ(matrix[0][0], 1);
    EXPECT_EQ(matrix[0][1], 2);

    EXPECT_NEAR(matrix[1][0], 5, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrix[1][1], 4.8, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(matrix[2][0], 0, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrix[2][1], -1.4, GIVENS_TEST_PRECISION_ERROR);
}

TEST(Matrix, computeQRGivensThinR)
{
    static constexpr uint32_t NUM_ROWS = 3, NUM_COLS = 2;
    Figaro::Matrix<double> matrix(NUM_ROWS, NUM_COLS);

    matrix[0][0] = 1; matrix[0][1] = 2;
    matrix[1][0] = 3; matrix[1][1] = 4;
    matrix[2][0] = 4; matrix[2][1] = 3;

    matrix.computeQR();

    EXPECT_NEAR(matrix[0][0], 5.099019513592785, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrix[0][1], 5.099019513592786, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(matrix[1][0], 0, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrix[1][1], 1.732050807568877, GIVENS_TEST_PRECISION_ERROR);
}


TEST(Matrix, computeQRHouseholderRRowMajor)
{
    static constexpr uint32_t NUM_ROWS = 3, NUM_COLS = 2;
    Figaro::Matrix<double, Figaro::MemoryLayout::COL_MAJOR> matrix(NUM_ROWS, NUM_COLS);

    matrix[0][0] = 1; matrix[0][1] = 2;
    matrix[1][0] = 3; matrix[1][1] = 4;
    matrix[2][0] = 4; matrix[2][1] = 3;

    matrix.computeQRHouseholder();

    EXPECT_NEAR(std::abs(matrix[0][0]), 5.099019513592785, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrix[0][1]), 5.099019513592786, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(std::abs(matrix[1][0]), 0, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrix[1][1]), 1.732050807568877, GIVENS_TEST_PRECISION_ERROR);
}

TEST(Matrix, computeQRHouseholderRColMajor)
{
    static constexpr uint32_t NUM_ROWS = 3, NUM_COLS = 2;
    Figaro::Matrix<double, Figaro::MemoryLayout::COL_MAJOR> matrix(NUM_ROWS, NUM_COLS);

    matrix(0, 0) = 1; matrix(0, 1) = 2;
    matrix(1, 0) = 3; matrix(1, 1) = 4;
    matrix(2, 0) = 4; matrix(2, 1) = 3;

    matrix.computeQRHouseholder();

    EXPECT_NEAR(std::abs(matrix(0, 0)), 5.099019513592785, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrix(0, 1)), 5.099019513592786, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(std::abs(matrix(1, 0)), 0, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrix(1, 1)), 1.732050807568877, GIVENS_TEST_PRECISION_ERROR);
}

TEST(Matrix, computeQRCholeskyRRowMajor)
{
    static constexpr uint32_t NUM_ROWS = 3, NUM_COLS = 2;
    Figaro::Matrix<double, Figaro::MemoryLayout::ROW_MAJOR> matrix(NUM_ROWS, NUM_COLS);
    Figaro::Matrix<double, Figaro::MemoryLayout::ROW_MAJOR> matrixR(0, 0);

    matrix(0, 0) = 1; matrix(0, 1) = 2;
    matrix(1, 0) = 3; matrix(1, 1) = 4;
    matrix(2, 0) = 4; matrix(2, 1) = 3;

    matrix.computeQRCholesky(false, false, &matrixR);

    EXPECT_EQ(matrixR.getNumRows(), matrix.getNumCols());
    EXPECT_EQ(matrixR.getNumCols(), matrix.getNumCols());

    EXPECT_NEAR(std::abs(matrixR(0, 0)), 5.099019513592785, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixR(0, 1)), 5.099019513592786, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(std::abs(matrixR(1, 0)), 0, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixR(1, 1)), 1.732050807568877, GIVENS_TEST_PRECISION_ERROR);
}

TEST(Matrix, computeQRCholeskyRColMajor)
{
    static constexpr uint32_t NUM_ROWS = 3, NUM_COLS = 2;
    Figaro::Matrix<double, Figaro::MemoryLayout::COL_MAJOR> matrix(NUM_ROWS, NUM_COLS);
    Figaro::Matrix<double, Figaro::MemoryLayout::COL_MAJOR> matrixR(0, 0);

    matrix(0, 0) = 1; matrix(0, 1) = 2;
    matrix(1, 0) = 3; matrix(1, 1) = 4;
    matrix(2, 0) = 4; matrix(2, 1) = 3;

    matrix.computeQRCholesky(false, false, &matrixR);

    EXPECT_EQ(matrixR.getNumRows(), matrix.getNumCols());
    EXPECT_EQ(matrixR.getNumCols(), matrix.getNumCols());

    EXPECT_NEAR(std::abs(matrixR(0, 0)), 5.099019513592785, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixR(0, 1)), 5.099019513592786, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(std::abs(matrixR(1, 0)), 0, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixR(1, 1)), 1.732050807568877, GIVENS_TEST_PRECISION_ERROR);
}


TEST(Matrix, computeQRHouseholderQRowMajor)
{
    static constexpr uint32_t NUM_ROWS = 3, NUM_COLS = 2;
    Figaro::Matrix<double> matrix(NUM_ROWS, NUM_COLS);
    Figaro::Matrix<double> matrixR(0, 0), matrixQ(0,0);

    matrix[0][0] = 1; matrix[0][1] = 2;
    matrix[1][0] = 3; matrix[1][1] = 4;
    matrix[2][0] = 4; matrix[2][1] = 3;

    matrix.computeQRHouseholder(true, true, &matrixR, &matrixQ);

    EXPECT_NEAR(std::abs(matrixR[0][0]), 5.099019513592785, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixR[0][1]), 5.099019513592786, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(std::abs(matrixR[1][0]), 0, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixR[1][1]), 1.732050807568877, GIVENS_TEST_PRECISION_ERROR);

    FIGARO_LOG_DBG("matrixR", matrixR)
    FIGARO_LOG_DBG("matrixQ", matrixQ)

    EXPECT_NEAR(std::abs(matrixQ[0][0]), std::abs(-0.196116135138184), GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixQ[1][0]), std::abs(-0.588348405414552), GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixQ[2][0]), std::abs(-0.784464540552736), GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(std::abs(matrixQ[0][1]), std::abs(0.577350269189625), GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixQ[1][1]), std::abs(-0.577350269189626), GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixQ[2][1]), std::abs(0.577350269189626), GIVENS_TEST_PRECISION_ERROR);
}

TEST(Matrix, computeQRHouseholderQColMajor)
{
    static constexpr uint32_t NUM_ROWS = 3, NUM_COLS = 2;
    Figaro::Matrix<double, Figaro::MemoryLayout::COL_MAJOR> matrix(NUM_ROWS, NUM_COLS);
    Figaro::Matrix<double, Figaro::MemoryLayout::COL_MAJOR> matrixR(0, 0), matrixQ(0,0);

    matrix(0, 0) = 1; matrix(0, 1) = 2;
    matrix(1, 0) = 3; matrix(1, 1) = 4;
    matrix(2, 0) = 4; matrix(2, 1) = 3;

    matrix.computeQRHouseholder(true, true, &matrixR, &matrixQ);

    EXPECT_NEAR(std::abs(matrixR(0, 0)), 5.099019513592785, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixR(0, 1)), 5.099019513592786, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(std::abs(matrixR(1, 0)), 0, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixR(1, 1)), 1.732050807568877, GIVENS_TEST_PRECISION_ERROR);

    FIGARO_LOG_DBG("matrixR", matrixR)
    FIGARO_LOG_DBG("matrixQ", matrixQ)

    EXPECT_NEAR(std::abs(matrixQ(0, 0)), std::abs(-0.196116135138184), GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixQ(1, 0)), std::abs(-0.588348405414552), GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixQ(2, 0)), std::abs(-0.784464540552736), GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(std::abs(matrixQ(0, 1)), std::abs(0.577350269189625), GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixQ(1, 1)), std::abs(-0.577350269189626), GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixQ(2, 1)), std::abs(0.577350269189626), GIVENS_TEST_PRECISION_ERROR);
}


TEST(Matrix, computeQRCholeskyQRowMajor)
{
    static constexpr uint32_t NUM_ROWS = 3, NUM_COLS = 2;
    Figaro::Matrix<double, Figaro::MemoryLayout::ROW_MAJOR> matrix(NUM_ROWS, NUM_COLS);
    Figaro::Matrix<double, Figaro::MemoryLayout::ROW_MAJOR> matrixR(0, 0), matrixQ(0,0);

    matrix(0, 0) = 1; matrix(0, 1) = 2;
    matrix(1, 0) = 3; matrix(1, 1) = 4;
    matrix(2, 0) = 4; matrix(2, 1) = 3;

    matrix.computeQRHouseholder(true, true, &matrixR, &matrixQ);

    EXPECT_NEAR(std::abs(matrixR(0, 0)), 5.099019513592785, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixR(0, 1)), 5.099019513592786, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(std::abs(matrixR(1, 0)), 0, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixR(1, 1)), 1.732050807568877, GIVENS_TEST_PRECISION_ERROR);

    FIGARO_LOG_DBG("matrixR", matrixR)
    FIGARO_LOG_DBG("matrixQ", matrixQ)

    EXPECT_NEAR(std::abs(matrixQ(0, 0)), std::abs(-0.196116135138184), GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixQ(1, 0)), std::abs(-0.588348405414552), GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixQ(2, 0)), std::abs(-0.784464540552736), GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(std::abs(matrixQ(0, 1)), std::abs(0.577350269189625), GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixQ(1, 1)), std::abs(-0.577350269189626), GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixQ(2, 1)), std::abs(0.577350269189626), GIVENS_TEST_PRECISION_ERROR);
}


TEST(Matrix, computeQRCholeskyQColMajor)
{
    static constexpr uint32_t NUM_ROWS = 3, NUM_COLS = 2;
    Figaro::Matrix<double, Figaro::MemoryLayout::COL_MAJOR> matrix(NUM_ROWS, NUM_COLS);
    Figaro::Matrix<double, Figaro::MemoryLayout::COL_MAJOR> matrixR(0, 0), matrixQ(0,0);

    matrix(0, 0) = 1; matrix(0, 1) = 2;
    matrix(1, 0) = 3; matrix(1, 1) = 4;
    matrix(2, 0) = 4; matrix(2, 1) = 3;

    matrix.computeQRHouseholder(true, true, &matrixR, &matrixQ);

    EXPECT_NEAR(std::abs(matrixR(0, 0)), 5.099019513592785, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixR(0, 1)), 5.099019513592786, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(std::abs(matrixR(1, 0)), 0, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixR(1, 1)), 1.732050807568877, GIVENS_TEST_PRECISION_ERROR);

    FIGARO_LOG_DBG("matrixR", matrixR)
    FIGARO_LOG_DBG("matrixQ", matrixQ)

    EXPECT_NEAR(std::abs(matrixQ(0, 0)), std::abs(-0.196116135138184), GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixQ(1, 0)), std::abs(-0.588348405414552), GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixQ(2, 0)), std::abs(-0.784464540552736), GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(std::abs(matrixQ(0, 1)), std::abs(0.577350269189625), GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixQ(1, 1)), std::abs(-0.577350269189626), GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixQ(2, 1)), std::abs(0.577350269189626), GIVENS_TEST_PRECISION_ERROR);
}


TEST(Matrix, computeCholeskyRowMajor)
{
    static constexpr uint32_t NUM_ROWS = 2, NUM_COLS = 2;
    Figaro::Matrix<double> matrix(NUM_ROWS, NUM_COLS);

    matrix(0, 0) = 26; matrix(0, 1) = 26;
    matrix(1, 0) = 26; matrix(1, 1) = 29;
    FIGARO_LOG_DBG("HOHOH")
    matrix.computeCholesky();

    FIGARO_LOG_DBG("matrix", matrix)

    EXPECT_NEAR(std::abs(matrix(0, 0)), 5.099019513592785, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrix(0, 1)), 5.099019513592786, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(std::abs(matrix(1, 0)), 0, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrix(1, 1)), 1.732050807568877, GIVENS_TEST_PRECISION_ERROR);
}


TEST(Matrix, computeSVDLapackRowMajor)
{
    static constexpr uint32_t NUM_ROWS = 3, NUM_COLS = 2;
    Figaro::Matrix<double, Figaro::MemoryLayout::ROW_MAJOR> matrix(NUM_ROWS, NUM_COLS);
    Figaro::Matrix<double, Figaro::MemoryLayout::ROW_MAJOR> matrixU(0, 0);
    Figaro::Matrix<double, Figaro::MemoryLayout::ROW_MAJOR> matrixS(0, 0);
    Figaro::Matrix<double, Figaro::MemoryLayout::ROW_MAJOR> matrixVT(0, 0);

    matrix(0, 0) = 1; matrix(0, 1) = 2;
    matrix(1, 0) = 3; matrix(1, 1) = 4;
    matrix(2, 0) = 4; matrix(2, 1) = 3;
    matrix.computeSingularValueDecomposition(1,
        &matrixU, &matrixS, &matrixVT);

    EXPECT_NEAR(matrixU(0, 0), -0.292567560669349, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrixU(1, 0), -0.678945554804918, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrixU(2, 0), -0.673377424669574, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrixU(0, 1), -0.534975788460430, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrixU(1, 1), -0.467461349309033, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrixU(2, 1), 0.703761886338924, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(matrixS(0, 0), 7.317324188951234, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrixS(1, 0), 1.206965912438775, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(matrixVT(0, 0), -0.686441353978375, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrixVT(1, 0), 0.727185167304955, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrixVT(0, 1), -0.727185167304955, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrixVT(1, 1), -0.686441353978375, GIVENS_TEST_PRECISION_ERROR);

    FIGARO_LOG_DBG("matrixU", matrixU)
    FIGARO_LOG_DBG("matrixS", matrixS)
    FIGARO_LOG_DBG("matrixVT", matrixVT)
}

TEST(Matrix, computeSVDLapackColMajor)
{
    static constexpr uint32_t NUM_ROWS = 3, NUM_COLS = 2;
    Figaro::Matrix<double, Figaro::MemoryLayout::COL_MAJOR> matrix(NUM_ROWS, NUM_COLS);
    Figaro::Matrix<double, Figaro::MemoryLayout::COL_MAJOR> matrixU(0, 0);
    Figaro::Matrix<double, Figaro::MemoryLayout::COL_MAJOR> matrixS(0, 0);
    Figaro::Matrix<double, Figaro::MemoryLayout::COL_MAJOR> matrixVT(0, 0);

    matrix(0, 0) = 1; matrix(0, 1) = 2;
    matrix(1, 0) = 3; matrix(1, 1) = 4;
    matrix(2, 0) = 4; matrix(2, 1) = 3;
    matrix.computeSingularValueDecomposition(1,
        &matrixU, &matrixS, &matrixVT);

    EXPECT_NEAR(matrixU(0, 0), -0.292567560669349, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrixU(1, 0), -0.678945554804918, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrixU(2, 0), -0.673377424669574, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrixU(0, 1), -0.534975788460430, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrixU(1, 1), -0.467461349309033, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrixU(2, 1), 0.703761886338924, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(matrixS(0, 0), 7.317324188951234, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrixS(1, 0), 1.206965912438775, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(matrixVT(0, 0), -0.686441353978375, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrixVT(1, 0), 0.727185167304955, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrixVT(0, 1), -0.727185167304955, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrixVT(1, 1), -0.686441353978375, GIVENS_TEST_PRECISION_ERROR);

    FIGARO_LOG_DBG("matrixU", matrixU)
    FIGARO_LOG_DBG("matrixS", matrixS)
    FIGARO_LOG_DBG("matrixVT", matrixVT)
}


TEST(Matrix, MultiplicationRowMajor)
{
    static constexpr uint32_t M = 3, N = 2, K = 3;
    Figaro::Matrix<double, Figaro::MemoryLayout::ROW_MAJOR> A(M, K), B(K, N), expC(M, N);

    EXPECT_EQ(A.getNumRows(), M);
    EXPECT_EQ(A.getNumCols(), K);

    EXPECT_EQ(B.getNumRows(), K);
    EXPECT_EQ(B.getNumCols(), N);


    A(0, 0) = 0;
    A(0, 1) = 1;
    A(0, 2) = 2;

    A(1, 0) = 3;
    A(1, 1) = 4;
    A(1, 2) = 5;

    A(2, 0) = 6;
    A(2, 1) = 7;
    A(2, 2) = 8;

    B(0, 0) = 0;
    B(0, 1) = 1;

    B(1, 0) = 2;
    B(1, 1) = 3;

    B(2, 0) = 4;
    B(2, 1) = 5;

    expC(0, 0) = 10;
    expC(0, 1) = 13;

    expC(1, 0) = 28;
    expC(1, 1) = 40;

    expC(2, 0) = 46;
    expC(2, 1) = 67;

    Figaro::Matrix<double, Figaro::MemoryLayout::ROW_MAJOR> C = A * B;
    EXPECT_EQ(C.getNumRows(), expC.getNumRows());
    EXPECT_EQ(C.getNumCols(), expC.getNumCols());

    for (uint32_t row = 0; row < expC.getNumRows(); row ++)
    {
        for (uint32_t col = 0; col < expC.getNumCols(); col++)
        {
            EXPECT_NEAR(C(row, col), expC(row, col), QR_TEST_PRECISION_ERROR);
        }
    }
}


TEST(Matrix, MultiplicationColMajor)
{
    static constexpr uint32_t M = 3, N = 2, K = 3;
    Figaro::Matrix<double, Figaro::MemoryLayout::COL_MAJOR> A(M, K), B(K, N), expC(M, N);

    EXPECT_EQ(A.getNumRows(), M);
    EXPECT_EQ(A.getNumCols(), K);

    EXPECT_EQ(B.getNumRows(), K);
    EXPECT_EQ(B.getNumCols(), N);


    A(0, 0) = 0;
    A(0, 1) = 1;
    A(0, 2) = 2;

    A(1, 0) = 3;
    A(1, 1) = 4;
    A(1, 2) = 5;

    A(2, 0) = 6;
    A(2, 1) = 7;
    A(2, 2) = 8;

    B(0, 0) = 0;
    B(0, 1) = 1;

    B(1, 0) = 2;
    B(1, 1) = 3;

    B(2, 0) = 4;
    B(2, 1) = 5;

    expC(0, 0) = 10;
    expC(0, 1) = 13;

    expC(1, 0) = 28;
    expC(1, 1) = 40;

    expC(2, 0) = 46;
    expC(2, 1) = 67;


    Figaro::Matrix<double, Figaro::MemoryLayout::COL_MAJOR> C = A * B;
    EXPECT_EQ(C.getNumRows(), expC.getNumRows());
    EXPECT_EQ(C.getNumCols(), expC.getNumCols());

    for (uint32_t row = 0; row < expC.getNumRows(); row ++)
    {
        for (uint32_t col = 0; col < expC.getNumCols(); col++)
        {
            EXPECT_NEAR(C(row, col), expC(row, col), QR_TEST_PRECISION_ERROR);
        }
    }
}



TEST(Matrix, SelfMatrixMultiplyRowMajor)
{
    static constexpr uint32_t M = 3, N = 4;
    Figaro::Matrix<double, Figaro::MemoryLayout::ROW_MAJOR> A(M, N), expB(N, N);

    A(0, 0) = 1;
    A(0, 1) = 2;
    A(0, 2) = 2;
    A(0, 3) = 3;

    A(1, 0) = 1;
    A(1, 1) = 2;
    A(1, 2) = 4;
    A(1, 3) = 6;

    A(2, 0) = 1;
    A(2, 1) = 2;
    A(2, 2) = 6;
    A(2, 3) = 7;


    expB(0, 0) = 3;
    expB(0, 1) = 6;
    expB(0, 2) = 12;
    expB(0, 3) = 16;

    expB(1, 0) = 6;
    expB(1, 1) = 12;
    expB(1, 2) = 24;
    expB(1, 3) = 32;

    expB(2, 0) = 12;
    expB(2, 1) = 24;
    expB(2, 2) = 56;
    expB(2, 3) = 72;

    expB(3, 0) = 16;
    expB(3, 1) = 32;
    expB(3, 2) = 72;
    expB(3, 3) = 94;

    Figaro::Matrix<double, Figaro::MemoryLayout::ROW_MAJOR> B = A.selfMatrixMultiply(0);

    EXPECT_EQ(B.getNumRows(), expB.getNumRows());
    EXPECT_EQ(B.getNumCols(), expB.getNumCols());

    for (uint32_t row = 0; row < expB.getNumRows(); row ++)
    {
        for (uint32_t col = 0; col < expB.getNumCols(); col++)
        {
            EXPECT_NEAR(B(row, col), expB(row, col), QR_TEST_PRECISION_ERROR);
        }
    }
}

TEST(Matrix, SelfMatrixMultiplyColMajor)
{
    static constexpr uint32_t M = 3, N = 4;
    Figaro::Matrix<double, Figaro::MemoryLayout::COL_MAJOR> A(M, N), expB(N, N);

    A(0, 0) = 1;
    A(0, 1) = 2;
    A(0, 2) = 2;
    A(0, 3) = 3;

    A(1, 0) = 1;
    A(1, 1) = 2;
    A(1, 2) = 4;
    A(1, 3) = 6;

    A(2, 0) = 1;
    A(2, 1) = 2;
    A(2, 2) = 6;
    A(2, 3) = 7;

    expB(0, 0) = 3;
    expB(0, 1) = 6;
    expB(0, 2) = 12;
    expB(0, 3) = 16;

    expB(1, 0) = 6;
    expB(1, 1) = 12;
    expB(1, 2) = 24;
    expB(1, 3) = 32;

    expB(2, 0) = 12;
    expB(2, 1) = 24;
    expB(2, 2) = 56;
    expB(2, 3) = 72;

    expB(3, 0) = 16;
    expB(3, 1) = 32;
    expB(3, 2) = 72;
    expB(3, 3) = 94;

    Figaro::Matrix<double, Figaro::MemoryLayout::COL_MAJOR> B = A.selfMatrixMultiply(0);

    EXPECT_EQ(B.getNumRows(), expB.getNumRows());
    EXPECT_EQ(B.getNumCols(), expB.getNumCols());

    for (uint32_t row = 0; row < expB.getNumRows(); row ++)
    {
        for (uint32_t col = 0; col < expB.getNumCols(); col++)
        {
            EXPECT_NEAR(B(row, col), expB(row, col), QR_TEST_PRECISION_ERROR);
        }
    }
}


TEST(Matrix, InverseRowMajor)
{
    static constexpr uint32_t M = 3, N = 3;
    Figaro::Matrix<double, Figaro::MemoryLayout::ROW_MAJOR> A(M, N), expInv(M, N);

    EXPECT_EQ(A.getNumRows(), M);
    EXPECT_EQ(A.getNumCols(), N);

    A(0, 0) = 1;
    A(0, 1) = 2;
    A(0, 2) = 3;

    A(1, 0) = 4;
    A(1, 1) = 5;
    A(1, 2) = 6;

    A(2, 0) = 10;
    A(2, 1) = 11;
    A(2, 2) = 13;

    expInv(0, 0) = 0.3333333333333333;
    expInv(0, 1) = -2.3333333333333333;
    expInv(0, 2) = 1;

    expInv(1, 0) = -2.6666666666666667;
    expInv(1, 1) = 5.66666666666666667;
    expInv(1, 2) = -2;

    expInv(2, 0) = 2;
    expInv(2, 1) = -3;
    expInv(2, 2) = 1;

    Figaro::Matrix<double, Figaro::MemoryLayout::ROW_MAJOR> inv = A.computeInverse();
    EXPECT_EQ(inv.getNumRows(), expInv.getNumRows());
    EXPECT_EQ(inv.getNumCols(), expInv.getNumCols());

    for (uint32_t row = 0; row < expInv.getNumRows(); row ++)
    {
        for (uint32_t col = 0; col < expInv.getNumCols(); col++)
        {
            EXPECT_NEAR(inv(row, col), expInv(row, col), QR_TEST_PRECISION_ERROR);
        }
    }
}



TEST(Matrix, InverseColMajor)
{
    static constexpr uint32_t M = 3, N = 3;
    Figaro::Matrix<double, Figaro::MemoryLayout::COL_MAJOR> A(M, N), expInv(M, N);

    EXPECT_EQ(A.getNumRows(), M);
    EXPECT_EQ(A.getNumCols(), N);

    A(0, 0) = 1;
    A(0, 1) = 2;
    A(0, 2) = 3;

    A(1, 0) = 4;
    A(1, 1) = 5;
    A(1, 2) = 6;

    A(2, 0) = 10;
    A(2, 1) = 11;
    A(2, 2) = 13;

    expInv(0, 0) = 0.3333333333333333;
    expInv(0, 1) = -2.3333333333333333;
    expInv(0, 2) = 1;

    expInv(1, 0) = -2.6666666666666667;
    expInv(1, 1) = 5.66666666666666667;
    expInv(1, 2) = -2;

    expInv(2, 0) = 2;
    expInv(2, 1) = -3;
    expInv(2, 2) = 1;

    Figaro::Matrix<double, Figaro::MemoryLayout::COL_MAJOR> inv = A.computeInverse();
    EXPECT_EQ(inv.getNumRows(), expInv.getNumRows());
    EXPECT_EQ(inv.getNumCols(), expInv.getNumCols());

    for (uint32_t row = 0; row < expInv.getNumRows(); row ++)
    {
        for (uint32_t col = 0; col < expInv.getNumCols(); col++)
        {
            EXPECT_NEAR(inv(row, col), expInv(row, col), QR_TEST_PRECISION_ERROR);
        }
    }
}


TEST(Matrix, InverseTriangularRowMajor)
{
    static constexpr uint32_t M = 3, N = 3;
    Figaro::Matrix<double, Figaro::MemoryLayout::ROW_MAJOR> A(M, N), expInv(M, N);

    EXPECT_EQ(A.getNumRows(), M);
    EXPECT_EQ(A.getNumCols(), N);

    A(0, 0) = 1;
    A(0, 1) = 2;
    A(0, 2) = 3;

    A(1, 0) = 0;
    A(1, 1) = 4;
    A(1, 2) = 5;

    A(2, 0) = 0;
    A(2, 1) = 0;
    A(2, 2) = 7;

    expInv(0, 0) = 1;
    expInv(0, 1) = -0.500000000000000;
    expInv(0, 2) = -0.071428571428571;

    expInv(1, 0) = 0;
    expInv(1, 1) = 0.250000000000000;
    expInv(1, 2) = -0.178571428571429;

    expInv(2, 0) = 0;
    expInv(2, 1) = 0;
    expInv(2, 2) = 0.142857142857143;

    Figaro::Matrix<double, Figaro::MemoryLayout::ROW_MAJOR> inv = A.computeInverse(0, true);
    FIGARO_LOG_DBG(expInv)
    EXPECT_EQ(inv.getNumRows(), expInv.getNumRows());
    EXPECT_EQ(inv.getNumCols(), expInv.getNumCols());

    for (uint32_t row = 0; row < expInv.getNumRows(); row ++)
    {
        for (uint32_t col = 0; col < expInv.getNumCols(); col++)
        {
            EXPECT_NEAR(inv(row, col), expInv(row, col), QR_TEST_PRECISION_ERROR);
        }
    }
}


TEST(Matrix, InverseTriangularColMajor)
{
    static constexpr uint32_t M = 3, N = 3;
    Figaro::Matrix<double, Figaro::MemoryLayout::COL_MAJOR> A(M, N), expInv(M, N);

    EXPECT_EQ(A.getNumRows(), M);
    EXPECT_EQ(A.getNumCols(), N);

    A(0, 0) = 1;
    A(0, 1) = 2;
    A(0, 2) = 3;

    A(1, 0) = 0;
    A(1, 1) = 4;
    A(1, 2) = 5;

    A(2, 0) = 0;
    A(2, 1) = 0;
    A(2, 2) = 7;

    expInv(0, 0) = 1;
    expInv(0, 1) = -0.500000000000000;
    expInv(0, 2) = -0.071428571428571;

    expInv(1, 0) = 0;
    expInv(1, 1) = 0.250000000000000;
    expInv(1, 2) = -0.178571428571429;

    expInv(2, 0) = 0;
    expInv(2, 1) = 0;
    expInv(2, 2) = 0.142857142857143;

    Figaro::Matrix<double, Figaro::MemoryLayout::COL_MAJOR> inv = A.computeInverse(0, true);
    FIGARO_LOG_DBG(expInv)
    EXPECT_EQ(inv.getNumRows(), expInv.getNumRows());
    EXPECT_EQ(inv.getNumCols(), expInv.getNumCols());

    for (uint32_t row = 0; row < expInv.getNumRows(); row ++)
    {
        for (uint32_t col = 0; col < expInv.getNumCols(); col++)
        {
            EXPECT_NEAR(inv(row, col), expInv(row, col), QR_TEST_PRECISION_ERROR);
        }
    }
}


TEST(Storage, MatrixIterator)
{
    static constexpr uint32_t NUM_ROWS = 5, NUM_COLS = 4;
    Figaro::Matrix<double> matrix(NUM_ROWS, NUM_COLS);

    EXPECT_EQ(matrix.getNumRows(), NUM_ROWS);
    EXPECT_EQ(matrix.getNumCols(), NUM_COLS);
     for (uint32_t rowIdx = 0; rowIdx < matrix.getNumRows(); rowIdx ++)
    {
        for (uint32_t colIdx = 0; colIdx < matrix.getNumCols(); colIdx++)
        {
            matrix[rowIdx][colIdx] = rowIdx * NUM_COLS + colIdx;
        }
    }

    uint32_t rowIdx = 0;
    for (auto row: matrix)
    {
        for (uint32_t colIdx = 0; colIdx < matrix.getNumCols(); colIdx++)
        {
            EXPECT_EQ(row[colIdx], rowIdx * NUM_COLS + colIdx);
        }
        rowIdx++;
    }
    EXPECT_EQ(rowIdx, matrix.getNumRows());

    rowIdx = 0;
    for (auto it = matrix.begin(); it != matrix.end(); it++)
    {
        for (uint32_t colIdx = 0; colIdx < matrix.getNumCols(); colIdx++)
        {
            EXPECT_EQ((*it)[colIdx], rowIdx * NUM_COLS + colIdx);
        }
        rowIdx++;
    }
    EXPECT_EQ(rowIdx, matrix.getNumRows());

    auto it = matrix.begin();
    for (rowIdx = matrix.getNumRows(); rowIdx > 0; )
    {
        rowIdx--;
        for (uint32_t colIdx = 0; colIdx < matrix.getNumCols(); colIdx++)
        {
            EXPECT_EQ((*(it + rowIdx))[colIdx], rowIdx * NUM_COLS + colIdx);
        }
    }
    EXPECT_EQ(rowIdx, 0);
}

TEST(DatabaseConfig, DropAttrs) {
    static const std::string DB_CONFIG_PATH = getConfigPath(5) + DB_CONFIG_PATH_IN;
    Figaro::Database database(DB_CONFIG_PATH);
    Figaro::ErrorCode initError = database.getInitializationErrorCode();
    Figaro::ErrorCode loadError;
    std::vector<std::string> attrNames;
    std::vector<std::string> expAttrNames = {"X25", "Y52", "Y53"};
    EXPECT_EQ(initError, Figaro::ErrorCode::NO_ERROR);
    loadError = database.loadData();
    EXPECT_EQ(loadError, Figaro::ErrorCode::NO_ERROR);

    database.dropAttributesFromRelations({"Y51"});

    attrNames = database.getRelationAttributeNames("R5");

    for (uint32_t idx = 0; idx < expAttrNames.size(); idx++)
    {
        EXPECT_EQ(attrNames[idx], expAttrNames[idx]);
    }
}

TEST(DatabaseConfig, BasicInput) {
    static const std::string DB_CONFIG_PATH = getConfigPath(1) + DB_CONFIG_PATH_IN;
    Figaro::Database database(DB_CONFIG_PATH);
    Figaro::ErrorCode initError = database.getInitializationErrorCode();
    EXPECT_EQ(initError, Figaro::ErrorCode::NO_ERROR);
}


TEST(DatabaseConfig, PathQuery3) {
    static const std::string DB_CONFIG_PATH = getConfigPath(2) + DB_CONFIG_PATH_IN;
    Figaro::Database database(DB_CONFIG_PATH);
    Figaro::ErrorCode initError;
    Figaro::ErrorCode loadError;
    initError = database.getInitializationErrorCode();
    EXPECT_EQ(initError, Figaro::ErrorCode::NO_ERROR);
    loadError = database.loadData();
    EXPECT_EQ(loadError, Figaro::ErrorCode::NO_ERROR);
    database.sortData();
}



TEST(DatabaseConfig, BasicQueryParsing)
{
    static const std::string DB_CONFIG_PATH = getConfigPath(5) + DB_CONFIG_PATH_IN;
    static const std::string QUERY_CONFIG_PATH = getConfigPath(5) + QUERY_CONFIG_PATH_IN;

    Figaro::Database database(DB_CONFIG_PATH);
    Figaro::ErrorCode initError;
    Figaro::ErrorCode loadError;
    std::map<std::vector<double>, uint32_t> downCounts;
    std::map<std::vector<double>, uint32_t> downParCounts;
    std::map<std::vector<double>, uint32_t> upParCounts;
    std::map<std::vector<double>, uint32_t> circCounts;

    initError = database.getInitializationErrorCode();
    EXPECT_EQ(initError, Figaro::ErrorCode::NO_ERROR);
    loadError = database.loadData();
    EXPECT_EQ(loadError, Figaro::ErrorCode::NO_ERROR);

    Figaro::Query query(&database);
    EXPECT_EQ(query.loadQuery(QUERY_CONFIG_PATH), Figaro::ErrorCode::NO_ERROR);
}

TEST(DatabaseConfig, ComputingCounts)
{
    static const std::string DB_CONFIG_PATH = getConfigPath(5) + DB_CONFIG_PATH_IN;
    static const std::string QUERY_CONFIG_PATH = getConfigPath(5) + QUERY_CONFIG_PATH_IN;

    Figaro::Database database(DB_CONFIG_PATH);
    Figaro::ErrorCode initError;
    Figaro::ErrorCode loadError;
    std::map<std::vector<uint32_t>, uint32_t> downCounts;
    std::map<std::vector<uint32_t>, uint32_t> downParCounts;
    std::map<std::vector<uint32_t>, uint32_t> upParCounts;
    std::map<std::vector<uint32_t>, uint32_t> circCounts;

    initError = database.getInitializationErrorCode();
    EXPECT_EQ(initError, Figaro::ErrorCode::NO_ERROR);
    loadError = database.loadData();
    EXPECT_EQ(loadError, Figaro::ErrorCode::NO_ERROR);

    Figaro::Query query(&database);
    EXPECT_EQ(query.loadQuery(QUERY_CONFIG_PATH), Figaro::ErrorCode::NO_ERROR);
    query.evaluateQuery(true, false, false, false);

    /*********************************** R4 ****************/
    downCounts =  database.getDownCounts("R4");
    EXPECT_EQ(downCounts.at({1}), 2);
    EXPECT_EQ(downCounts.at({2}), 3);
    EXPECT_EQ(downCounts.at({3}), 1);

    downParCounts = database.getParDownCnts("R4", {"X24"});
    EXPECT_EQ(downParCounts.at({1}), 2);
    EXPECT_EQ(downParCounts.at({2}), 3);
    EXPECT_EQ(downParCounts.at({3}), 1);

    upParCounts = database.getParUpCnts("R4", {"X24"});
    EXPECT_EQ(upParCounts.at({1}), 35);
    EXPECT_EQ(upParCounts.at({2}), 32);
    EXPECT_EQ(upParCounts.at({3}), 8);

    circCounts = database.getCircCounts("R4");
    EXPECT_EQ(circCounts.at({1}), 35);
    EXPECT_EQ(circCounts.at({2}), 32);
    EXPECT_EQ(circCounts.at({3}), 8);

    /*********************************** R5 ****************/
    downCounts =  database.getDownCounts("R5");
    EXPECT_EQ(downCounts.at({1}), 3);
    EXPECT_EQ(downCounts.at({2}), 1);
    EXPECT_EQ(downCounts.at({3}), 2);

    downParCounts = database.getParDownCnts("R5", {"X25"});
    EXPECT_EQ(downParCounts.at({1}), 3);
    EXPECT_EQ(downParCounts.at({2}), 1);
    EXPECT_EQ(downParCounts.at({3}), 2);

    upParCounts = database.getParUpCnts("R5", {"X25"});
    EXPECT_EQ(upParCounts.at({1}), 29);
    EXPECT_EQ(upParCounts.at({2}), 27);
    EXPECT_EQ(upParCounts.at({3}), 30);

    circCounts = database.getCircCounts("R5");
    EXPECT_EQ(circCounts.at({1}), 29);
    EXPECT_EQ(circCounts.at({2}), 27);
    EXPECT_EQ(circCounts.at({3}), 30);

    /*********************************** R3 ****************/
    downCounts =  database.getDownCounts("R3");
    EXPECT_EQ(downCounts.at({1}), 1);
    EXPECT_EQ(downCounts.at({2}), 2);
    EXPECT_EQ(downCounts.at({3}), 3);

    downParCounts = database.getParDownCnts("R3", {"X13"});
    EXPECT_EQ(downParCounts.at({1}), 1);
    EXPECT_EQ(downParCounts.at({2}), 2);
    EXPECT_EQ(downParCounts.at({3}), 3);

    upParCounts = database.getParUpCnts("R3", {"X13"});
    EXPECT_EQ(upParCounts.at({1}), 38);
    EXPECT_EQ(upParCounts.at({2}), 20);
    EXPECT_EQ(upParCounts.at({3}), 32);

    circCounts = database.getCircCounts("R3");
    EXPECT_EQ(circCounts.at({1}), 38);
    EXPECT_EQ(circCounts.at({2}), 20);
    EXPECT_EQ(circCounts.at({3}), 32);

    /*********************************** R2 ****************/
    downCounts =  database.getDownCounts("R2");
    EXPECT_EQ(downCounts.at({1, 1, 1}), 6);
    EXPECT_EQ(downCounts.at({1, 1, 3}), 4);
    EXPECT_EQ(downCounts.at({1, 2, 1}), 9);
    EXPECT_EQ(downCounts.at({1, 2, 3}), 6);
    EXPECT_EQ(downCounts.at({2, 2, 1}), 9);
    EXPECT_EQ(downCounts.at({2, 2, 2}), 3);
    EXPECT_EQ(downCounts.at({2, 3, 2}), 1);
    EXPECT_EQ(downCounts.at({3, 1, 2}), 2);
    EXPECT_EQ(downCounts.at({3, 1, 3}), 4);
    EXPECT_EQ(downCounts.at({3, 3, 2}), 1);

    downParCounts = database.getParDownCnts("R2", {"X12"});
    EXPECT_EQ(downParCounts.at({1}), 25);
    EXPECT_EQ(downParCounts.at({2}), 13);
    EXPECT_EQ(downParCounts.at({3}), 7);

    upParCounts = database.getParUpCnts("R2", {"X12"});
    EXPECT_EQ(upParCounts.at({1}), 4);
    EXPECT_EQ(upParCounts.at({2}), 3);
    EXPECT_EQ(upParCounts.at({3}), 5);

    circCounts =  database.getCircCounts("R2");
    EXPECT_EQ(circCounts.at({1, 1, 1}), 24);
    EXPECT_EQ(circCounts.at({1, 1, 3}), 16);
    EXPECT_EQ(circCounts.at({1, 2, 1}), 36);
    EXPECT_EQ(circCounts.at({1, 2, 3}), 24);
    EXPECT_EQ(circCounts.at({2, 2, 1}), 27);
    EXPECT_EQ(circCounts.at({2, 2, 2}), 9);
    EXPECT_EQ(circCounts.at({2, 3, 2}), 3);
    EXPECT_EQ(circCounts.at({3, 1, 2}), 10);
    EXPECT_EQ(circCounts.at({3, 1, 3}), 20);
    EXPECT_EQ(circCounts.at({3, 3, 2}), 5);

    /*********************************** R1 ****************/
    downCounts =  database.getDownCounts("R1");
    EXPECT_EQ(downCounts.at({1, 1}), 25);
    EXPECT_EQ(downCounts.at({1, 3}), 75);
    EXPECT_EQ(downCounts.at({2, 1}), 13);
    EXPECT_EQ(downCounts.at({2, 2}), 26);
    EXPECT_EQ(downCounts.at({3, 2}), 14);
    EXPECT_EQ(downCounts.at({3, 3}), 21);

    circCounts = database.getCircCounts("R1");
    EXPECT_EQ(circCounts.at({1, 1}), 25);
    EXPECT_EQ(circCounts.at({1, 3}), 75);
    EXPECT_EQ(circCounts.at({2, 1}), 13);
    EXPECT_EQ(circCounts.at({2, 2}), 26);
    EXPECT_EQ(circCounts.at({3, 2}), 14);
    EXPECT_EQ(circCounts.at({3, 3}), 21);
}

TEST(DatabaseConfig, FigaroFirstPass)
{
    static const std::string DB_CONFIG_PATH = getConfigPath(5) + DB_CONFIG_PATH_IN;
    static const std::string QUERY_CONFIG_PATH = getConfigPath(5) + QUERY_CONFIG_PATH_IN;

    Figaro::Database database(DB_CONFIG_PATH);
    Figaro::ErrorCode initError;
    Figaro::ErrorCode loadError;
    static constexpr uint32_t NUM_RELS = 5;
    std::array<Figaro::MatrixEigenT, NUM_RELS> head;
    std::array<Figaro::MatrixEigenT, NUM_RELS> expHead;
    std::array<Figaro::MatrixEigenT, NUM_RELS> tail;
    std::array<Figaro::MatrixEigenT, NUM_RELS> expTail;
    std::array<Figaro::MatrixEigenT, NUM_RELS> scales;
    std::array<Figaro::MatrixEigenT, NUM_RELS> expScales;
    std::array<Figaro::MatrixEigenT, NUM_RELS> dataScales;
    std::array<Figaro::MatrixEigenT, NUM_RELS> expDataScales;
    std::array<std::string, NUM_RELS> fileInputExpHead;
    std::array<std::string, NUM_RELS> fileInputExpTail;
    std::array<std::string, NUM_RELS> fileInputExpScales;
    std::array<std::string, NUM_RELS> fileInputExpDataScales;


    for (uint32_t idxRel = 0; idxRel < NUM_RELS; idxRel ++)
    {
        fileInputExpHead[idxRel] = getDataPath(5) + "expectedHead" +
            std::to_string(idxRel + 1) + ".csv";
        fileInputExpTail[idxRel] = getDataPath(5) + "expectedTail" +
            std::to_string(idxRel + 1) + ".csv";
        fileInputExpScales[idxRel] = getDataPath(5) + "expectedScalesFirstPass" +
            std::to_string(idxRel + 1) + ".csv";
        fileInputExpDataScales[idxRel] = getDataPath(5) + "expectedDataScalesFirstPass" +
            std::to_string(idxRel + 1) + ".csv";

        readMatrixDense(fileInputExpHead[idxRel], expHead[idxRel]);
        readMatrixDense(fileInputExpTail[idxRel], expTail[idxRel]);
        readMatrixDense(fileInputExpScales[idxRel], expScales[idxRel]);
        readMatrixDense(fileInputExpDataScales[idxRel], expDataScales[idxRel]);
    }

    initError = database.getInitializationErrorCode();
    EXPECT_EQ(initError, Figaro::ErrorCode::NO_ERROR);
    loadError = database.loadData();
    EXPECT_EQ(loadError, Figaro::ErrorCode::NO_ERROR);

    Figaro::Query query(&database);
    EXPECT_EQ(query.loadQuery(QUERY_CONFIG_PATH), Figaro::ErrorCode::NO_ERROR);
    query.evaluateQuery(true, true, false, false);

    for (uint32_t idxRel = 0; idxRel < NUM_RELS; idxRel++)
    {
        const std::string relName = "R" + std::to_string(idxRel + 1);
        FIGARO_LOG_INFO("Relation", relName)
        const auto& headDT = database.getHead(relName);
        const auto& tailDT = database.getTail(relName);
        const auto& scaleDT = database.getScales(relName);
        const auto& dataScaleDT = database.getDataScales(relName);

        Figaro::Relation::copyMatrixDTToMatrixEigen(headDT, head[idxRel]);
        Figaro::Relation::copyMatrixDTToMatrixEigen(tailDT, tail[idxRel]);
        Figaro::Relation::copyMatrixDTToMatrixEigen(scaleDT, scales[idxRel]);
        Figaro::Relation::copyMatrixDTToMatrixEigen(dataScaleDT, dataScales[idxRel]);
        compareMatrices(head[idxRel], expHead[idxRel], true, true);
        compareMatrices(tail[idxRel], expTail[idxRel], true, true);
        compareMatrices(scales[idxRel], expScales[idxRel], true, true);
        compareMatrices(dataScales[idxRel], expDataScales[idxRel], true, true);
    }
}

TEST(DatabaseConfig, FigaroSecondPass)
{
    static const std::string DB_CONFIG_PATH = getConfigPath(5) + DB_CONFIG_PATH_IN;
    static const std::string QUERY_CONFIG_PATH = getConfigPath(5) + QUERY_CONFIG_PATH_IN;

    Figaro::Database database(DB_CONFIG_PATH);
    Figaro::ErrorCode initError;
    Figaro::ErrorCode loadError;
    Figaro::MatrixEigenT headGen1, headGen2, tailGen2;
    Figaro::MatrixEigenT expHeadGen1, expHeadGen2, expTailGen2;

    std::string fileInputExpHead2 = getDataPath(5) + "expectedHeadGen2.csv";
    std::string fileInputExpHead1 = getDataPath(5) + "expectedHeadGen1.csv";
    std::string fileInputExpTail2 = getDataPath(5) + "expectedTailGen2.csv";

    readMatrixDense(fileInputExpHead2, expHeadGen2);
    readMatrixDense(fileInputExpHead1, expHeadGen1);
    readMatrixDense(fileInputExpTail2, expTailGen2);

    initError = database.getInitializationErrorCode();
    EXPECT_EQ(initError, Figaro::ErrorCode::NO_ERROR);
    loadError = database.loadData();
    EXPECT_EQ(loadError, Figaro::ErrorCode::NO_ERROR);

    Figaro::Query query(&database);
    EXPECT_EQ(query.loadQuery(QUERY_CONFIG_PATH), Figaro::ErrorCode::NO_ERROR);
    query.evaluateQuery(true, true, true, false);
    const auto& headDT = database.getHead("R2");
    const auto& tailDT = database.getGeneralizedTail("R2");
    Figaro::Relation::copyMatrixDTToMatrixEigen(headDT, headGen2);
    Figaro::Relation::copyMatrixDTToMatrixEigen(tailDT, tailGen2);
    compareMatrices(headGen2, expHeadGen2, true, true);
    compareMatrices(tailGen2, expTailGen2, true, true);
    // TODO: Add tests for scales, datascales, and allscales
}


TEST(DatabaseConfig, FigaroQR)
{
    static const std::string DB_CONFIG_PATH = getConfigPath(5) + DB_CONFIG_PATH_IN;
    static const std::string QUERY_CONFIG_PATH = getConfigPath(5) + QUERY_CONFIG_PATH_IN;

    Figaro::Database database(DB_CONFIG_PATH);
    Figaro::ErrorCode initError;
    Figaro::ErrorCode loadError;
    Figaro::MatrixEigenT headGen1, headGen2, tailGen2;
    Figaro::MatrixEigenT expHeadGen1, expHeadGen2, expTailGen2;


    initError = database.getInitializationErrorCode();
    EXPECT_EQ(initError, Figaro::ErrorCode::NO_ERROR);
    loadError = database.loadData();
    EXPECT_EQ(loadError, Figaro::ErrorCode::NO_ERROR);

    Figaro::Query query(&database);
    EXPECT_EQ(query.loadQuery(QUERY_CONFIG_PATH), Figaro::ErrorCode::NO_ERROR);
    query.evaluateQuery(true, true, true, true);
}



TEST(Relation, Join)
{
    static constexpr uint32_t M = 3, N = 3, K= 2;
    Relation::MatrixDT A(M, N), B(K, N), C(K, K);

    A[0][0] = 1;
    A[0][1] = 2;
    A[0][2] = 3;

    A[1][0] = 1;
    A[1][1] = 4;
    A[1][2] = 6;

    A[2][0] = 1;
    A[2][1] = 6;
    A[2][2] = 7;

    B[0][0] = 1;
    B[0][1] = 1;
    B[0][2] = 4;


    B[1][0] = 1;
    B[1][1] = 2;
    B[1][2] = 5;

    C[0][0] = 1;
    C[0][1] = 2;

    C[1][0] = 1;
    C[1][1] = 3;


    Relation relA("A", std::move(A),
        {Relation::Attribute("A", Relation::AttributeType::FLOAT),
         Relation::Attribute("A1", Relation::AttributeType::FLOAT),
         Relation::Attribute("A2", Relation::AttributeType::FLOAT)});

    Relation relB("B", std::move(B),
        {Relation::Attribute("A", Relation::AttributeType::FLOAT),
         Relation::Attribute("B1", Relation::AttributeType::FLOAT),
         Relation::Attribute("B2", Relation::AttributeType::FLOAT)});

    Relation relC("C", std::move(C),
        {Relation::Attribute("A", Relation::AttributeType::FLOAT),
         Relation::Attribute("C1", Relation::AttributeType::FLOAT)});

    FIGARO_LOG_DBG("relA", relA)
    FIGARO_LOG_DBG("relB", relB)
    FIGARO_LOG_DBG("relC", relC)
    Relation joinRel1 = relA.joinRelations({&relB, &relC}, {"A"}, {"A"}, {{"A"},{"A"}}, false);
    Relation joinRel2 = relA.joinRelations({&relB}, {"A"}, {}, {{"A"}}, false);

    FIGARO_LOG_DBG(joinRel1);
    FIGARO_LOG_DBG(joinRel2);
}


TEST(Relation, JoinLeapFrog)
{
    static constexpr uint32_t M = 3, N = 3, K= 2;
    Relation::MatrixDT A(M, N), B(K, N), C(K, K);

    A[0][0] = 1;
    A[0][1] = 2;
    A[0][2] = 3;

    A[1][0] = 1;
    A[1][1] = 4;
    A[1][2] = 6;

    A[2][0] = 1;
    A[2][1] = 6;
    A[2][2] = 7;

    B[0][0] = 1;
    B[0][1] = 1;
    B[0][2] = 4;


    B[1][0] = 1;
    B[1][1] = 2;
    B[1][2] = 5;

    C[0][0] = 1;
    C[0][1] = 2;

    C[1][0] = 1;
    C[1][1] = 3;


    Relation relA("A", std::move(A),
        {Relation::Attribute("A", Relation::AttributeType::FLOAT),
         Relation::Attribute("A1", Relation::AttributeType::FLOAT),
         Relation::Attribute("A2", Relation::AttributeType::FLOAT)});

    Relation relB("B", std::move(B),
        {Relation::Attribute("A", Relation::AttributeType::FLOAT),
         Relation::Attribute("B1", Relation::AttributeType::FLOAT),
         Relation::Attribute("B2", Relation::AttributeType::FLOAT)});

    Relation relC("C", std::move(C),
        {Relation::Attribute("A", Relation::AttributeType::FLOAT),
         Relation::Attribute("C1", Relation::AttributeType::FLOAT)});
    FIGARO_LOG_DBG("relA", relA)
    FIGARO_LOG_DBG("relB", relB)
    FIGARO_LOG_DBG("relC", relC)
    std::vector<Relation*> vRels = {&relA, &relB, &relC};
    std::vector<Relation*> vParRels = {nullptr, &relA, &relA};
    std::vector<std::vector<std::string> > vvJoinAttrs = {{"A"}, {"A"}, {"A"}};
    std::vector<std::vector<std::string> > vvParJoinAttrs = {{}, {"A"}, {"A"}};
    FIGARO_LOG_DBG("MAJMUN")
    Relation joinRel = Relation::joinRelations(vRels, vParRels, vvJoinAttrs, vvParJoinAttrs, 12);
    FIGARO_LOG_DBG(joinRel);
}

TEST(Relation, Multiply)
{
    static constexpr uint32_t M = 3, N = 4, K= 2;
    Relation::MatrixDT A(M, N), B(K, K);

    A[0][0] = 1;
    A[0][1] = 2;
    A[0][2] = 2;
    A[0][3] = 3;

    A[1][0] = 1;
    A[1][1] = 2;
    A[1][2] = 4;
    A[1][3] = 6;

    A[2][0] = 1;
    A[2][1] = 2;
    A[2][2] = 6;
    A[2][3] = 7;

    B[0][0] = 1;
    B[0][1] = 1;

    B[1][0] = 1;
    B[1][1] = 2;


    Relation relA("A", std::move(A),
        {Relation::Attribute("A", Relation::AttributeType::FLOAT),
         Relation::Attribute("AA", Relation::AttributeType::FLOAT),
         Relation::Attribute("A1", Relation::AttributeType::FLOAT),
         Relation::Attribute("A2", Relation::AttributeType::FLOAT)});

    Relation relB("B", std::move(B),
        {Relation::Attribute("A", Relation::AttributeType::FLOAT),
         Relation::Attribute("B1", Relation::AttributeType::FLOAT)});

    FIGARO_LOG_DBG("relA", relA)
    FIGARO_LOG_DBG("relB", relB)
    Relation rel = relA.multiply(relB, {"A", "AA"}, {"A"});
    FIGARO_LOG_DBG("rel", rel)
}

TEST(Relation, SelfMatrixMultiply)
{
    static constexpr uint32_t M = 3, N = 4;
    Relation::MatrixDT A(M, N);

    A[0][0] = 1;
    A[0][1] = 2;
    A[0][2] = 2;
    A[0][3] = 3;

    A[1][0] = 1;
    A[1][1] = 2;
    A[1][2] = 4;
    A[1][3] = 6;

    A[2][0] = 1;
    A[2][1] = 2;
    A[2][2] = 6;
    A[2][3] = 7;


    Relation relA("A", std::move(A),
        {Relation::Attribute("A", Relation::AttributeType::FLOAT),
         Relation::Attribute("AA", Relation::AttributeType::FLOAT),
         Relation::Attribute("A1", Relation::AttributeType::FLOAT),
         Relation::Attribute("A2", Relation::AttributeType::FLOAT)});

    FIGARO_LOG_DBG("relA", relA)
    Relation rel = relA.selfMatrixMultiply({});
    FIGARO_LOG_DBG("rel", rel)
}


TEST(Relation, CheckOrthogonality)
{
    static constexpr uint32_t M = 3, N = 3;
    Relation::MatrixDT A(M, N);

    A[0][0] = 1 + GIVENS_TEST_PRECISION_ERROR;
    A[0][1] = 0;
    A[0][2] = 0;

    A[1][0] = 0;
    A[1][1] = 1 + GIVENS_TEST_PRECISION_ERROR;
    A[1][2] = 0;

    A[2][0] = 0;
    A[2][1] = 0;
    A[2][2] = 1 + GIVENS_TEST_PRECISION_ERROR;


    Relation relA("A", std::move(A),
        {Relation::Attribute("A1", Relation::AttributeType::FLOAT),
         Relation::Attribute("A2", Relation::AttributeType::FLOAT),
         Relation::Attribute("A3", Relation::AttributeType::FLOAT)});

    FIGARO_LOG_DBG("relA", relA)
    double orth = relA.checkOrthogonality({});
    EXPECT_NEAR(orth, 0, QR_TEST_PRECISION_ERROR);
    FIGARO_LOG_DBG("orth", orth)
}


TEST(Relation, Norm)
{
    static constexpr uint32_t M = 3, N = 4;
    Relation::MatrixDT A(M, N);

    A[0][0] = 1;
    A[0][1] = 2;
    A[0][2] = 2;
    A[0][3] = 3;

    A[1][0] = 1;
    A[1][1] = 2;
    A[1][2] = 4;
    A[1][3] = 6;

    A[2][0] = 1;
    A[2][1] = 2;
    A[2][2] = 6;
    A[2][3] = 7;


    Relation relA("A", std::move(A),
        {Relation::Attribute("A", Relation::AttributeType::FLOAT),
         Relation::Attribute("AA", Relation::AttributeType::FLOAT),
         Relation::Attribute("A1", Relation::AttributeType::FLOAT),
         Relation::Attribute("A2", Relation::AttributeType::FLOAT)});

    FIGARO_LOG_DBG("relA", relA)
    double norm = relA.norm({"A", "AA"});
    EXPECT_NEAR(norm, 12.247448713916, QR_TEST_PRECISION_ERROR);
    FIGARO_LOG_DBG("norm", norm)
}

TEST(Relation, AdditionAndSubtraction)
{
    static constexpr uint32_t M = 2, N = 3;
    Relation::MatrixDT A(M, N), B(M, N);
    A[0][0] = 1;
    A[0][1] = 2;
    A[0][2] = 3;

    A[1][0] = 4;
    A[1][1] = 5;
    A[1][2] = 6;

    B[0][0] = 7;
    B[0][1] = 8;
    B[0][2] = 9;

    B[1][0] = 10;
    B[1][1] = 11;
    B[1][2] = 12;

    Relation relA("A", std::move(A),
        {Relation::Attribute("A", Relation::AttributeType::FLOAT),
         Relation::Attribute("A1", Relation::AttributeType::FLOAT),
         Relation::Attribute("A2", Relation::AttributeType::FLOAT)});

    Relation relB("B", std::move(B),
        {Relation::Attribute("B", Relation::AttributeType::FLOAT),
         Relation::Attribute("B1", Relation::AttributeType::FLOAT),
         Relation::Attribute("B2", Relation::AttributeType::FLOAT)});

    auto relC = relA.addRelation(relB, {"A"}, {"B"});
    auto relD = relA.subtractRelation(relB, {"A"}, {"B"});
    FIGARO_LOG_DBG("relA", relA)
    FIGARO_LOG_DBG("relB", relB)
    FIGARO_LOG_DBG("relC", relC)
    FIGARO_LOG_DBG("relD", relD)
}

TEST(Database, Multiply2)
{
    static constexpr uint32_t M = 3, N = 4, K= 2;
    Relation::MatrixDT A(M, N), B(K, K), R(N, N);

    A[0][0] = 1;
    A[0][1] = 2;
    A[0][2] = 2;
    A[0][3] = 3;

    A[1][0] = 1;
    A[1][1] = 2;
    A[1][2] = 4;
    A[1][3] = 6;

    A[2][0] = 1;
    A[2][1] = 2;
    A[2][2] = 6;
    A[2][3] = 7;

    B[0][0] = 1;
    B[0][1] = 1;

    B[1][0] = 1;
    B[1][1] = 2;


    R[0][0] = -4.899;
    R[0][1] = -9.798;
    R[0][2] = -13.0639;
    R[0][3] = -3.6742;

    R[1][0] = 0;
    R[1][1] = 4;
    R[1][2] = 4;
    R[1][3] = 0;

    R[2][0] = 0;
    R[2][1] = 0;
    R[2][2] = -1.1547;
    R[2][3] = 0;

    R[3][0] = 0;
    R[3][1] = 0;
    R[3][2] = 0;
    R[3][3] = -1.2247;

    Relation relA("A", std::move(A),
        {Relation::Attribute("A", Relation::AttributeType::FLOAT),
         Relation::Attribute("AA", Relation::AttributeType::FLOAT),
         Relation::Attribute("A1", Relation::AttributeType::FLOAT),
         Relation::Attribute("A2", Relation::AttributeType::FLOAT)});

    Relation relB("B", std::move(B),
        {Relation::Attribute("A", Relation::AttributeType::FLOAT),
         Relation::Attribute("B1", Relation::AttributeType::FLOAT)});

    Relation relR("R", std::move(R),
        {Relation::Attribute("R1", Relation::AttributeType::FLOAT),
         Relation::Attribute("R2", Relation::AttributeType::FLOAT),
         Relation::Attribute("R3", Relation::AttributeType::FLOAT),
         Relation::Attribute("R4", Relation::AttributeType::FLOAT)});

    std::vector<Relation> vRels;

    vRels.emplace_back(std::move(relA.copyRelation()));
    vRels.emplace_back(std::move(relB.copyRelation()));
    vRels.emplace_back(std::move(relR.copyRelation()));

    Database database(std::move(vRels));
    FIGARO_LOG_DBG(relA, relB);
    auto joinRelName = database.joinRelations("COPY_A", {"COPY_B"}, {"A"}, {}, {{"A"}}, false);
    auto rInvName = database.inverse("COPY_R", {});

    auto qName = database.multiply(joinRelName, rInvName, {}, {}, 0);
    auto AInvRname = database.multiply("COPY_A", rInvName, {"A"}, {}, 0);
    auto BInvRname = database.multiply("COPY_B", rInvName, {"A"}, {}, 3);
    auto qWay2 = database.joinRelationsAndAddColumns(
            AInvRname, {BInvRname}, {"A"}, {}, {{"A"}}, false );

    database.outputRelation(rInvName);
    database.outputRelation("COPY_A");
    database.outputRelation("COPY_B");
    //database.outputRelation(joinRelName);

    database.outputRelation(AInvRname);
    database.outputRelation(BInvRname);
    database.outputRelation(qName);
    database.outputRelation(qWay2);
}

