#include "UtilTest.h"
#include "database/storage/ArrayStorage.h"
#include "database/storage/Matrix.h"
#include "database/storage/MatrixSparse.h"
#include "database/Database.h"
#include "database/Relation.h"
#include "database/query/Query.h"
#include "database/query/visitor/result/ASTVisitorResultQR.h"
#include "database/query/visitor/result/ASTVisitorResultSVD.h"
#include <vector>
#include <string>
#include <array>
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

TEST(MatrixSparse, Basic)
{
    static constexpr uint32_t NUM_ROWS = 5, NUM_COLS = 4;
    std::vector<double> vVals = {0, 1, 2, 3, 4, 5, 6, 7, 8,
        9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19};
    std::vector<int64_t> rowStartPos = {0, 4, 8, 12, 16, 20};
    std::vector<int64_t> vColIndcs = {0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3};
    Figaro::MatrixSparse<double> matrix{NUM_ROWS, NUM_COLS, rowStartPos.data(),  vColIndcs.data(), vVals.data()};
    long long int numRows;
    long long int numCols;
    long long int* pRowStartPos;
    long long int* pColInds;
    double* pVals;
    matrix.getElems(numRows, numCols, pRowStartPos, pColInds, pVals);

    EXPECT_EQ(numRows, NUM_ROWS);
    EXPECT_EQ(numCols, NUM_COLS);

    for (int idx = 0; idx < rowStartPos.size() - 1; idx++)
    {
        EXPECT_EQ(rowStartPos[idx], pRowStartPos[idx]);
    }

    for (int idx = 0; idx < vColIndcs.size(); idx++)
    {
        EXPECT_EQ(vColIndcs[idx], pColInds[idx]);
    }

    for (int idx = 0; idx < vVals.size(); idx++)
    {
        EXPECT_EQ(vVals[idx], pVals[idx]);
    }
}

TEST(Matrix, ResizeRows)
{
    static constexpr uint32_t NUM_ROWS = 5, NEW_NUM_ROWS = 3, NUM_COLS = 4;
    Figaro::Matrix<double> matrix(0, NUM_COLS);

    EXPECT_EQ(matrix.getNumRows(), 0);
    EXPECT_EQ(matrix.getNumCols(), NUM_COLS);

    matrix.resizeRows(NUM_ROWS);
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

    matrix.resizeRows(NEW_NUM_ROWS);
    EXPECT_EQ(matrix.getNumRows(), NEW_NUM_ROWS);
    for (uint32_t rowIdx = 0; rowIdx < matrix.getNumRows(); rowIdx ++)
    {
        for (uint32_t colIdx = 0; colIdx < matrix.getNumCols(); colIdx++)
        {
            EXPECT_EQ(matrix[rowIdx][colIdx], rowIdx * NUM_COLS + colIdx);
        }
    }
}

TEST(Matrix, BlockRowMajor)
{
    static constexpr uint32_t NUM_ROWS = 3,  NUM_COLS = 2;
    Figaro::Matrix<double, Figaro::MemoryLayout::ROW_MAJOR> matrix1(NUM_ROWS, NUM_COLS);

    matrix1(0, 0) = 1; matrix1(0, 1) = 2;
    matrix1(1, 0) = 3; matrix1(1, 1) = 4;
    matrix1(2, 0) = 5; matrix1(2, 1) = 6;

    const auto&& blockMat = matrix1.getBlock(1, 2, 0, 0);
    EXPECT_EQ(blockMat.getNumCols(), 1);
    EXPECT_EQ(blockMat.getNumRows(), 2);
    EXPECT_EQ(blockMat(0, 0), 3);
    EXPECT_EQ(blockMat(1, 0), 5);

    const auto&& rightCols = matrix1.getRightCols(1);
    EXPECT_EQ(rightCols.getNumCols(), 1);
    EXPECT_EQ(rightCols.getNumRows(), 3);
    EXPECT_EQ(rightCols(0, 0), 2);
    EXPECT_EQ(rightCols(1, 0), 4);
    EXPECT_EQ(rightCols(2, 0), 6);

    const auto&& leftCols = matrix1.getLeftCols(1);
    EXPECT_EQ(leftCols.getNumCols(), 1);
    EXPECT_EQ(leftCols.getNumRows(), 3);
    EXPECT_EQ(leftCols(0, 0), 1);
    EXPECT_EQ(leftCols(1, 0), 3);
    EXPECT_EQ(leftCols(2, 0), 5);

    const auto&& topRows = matrix1.getTopRows(1);
    EXPECT_EQ(topRows.getNumCols(), 2);
    EXPECT_EQ(topRows.getNumRows(), 1);
    EXPECT_EQ(topRows(0, 0), 1);
    EXPECT_EQ(topRows(0, 1), 2);

    const auto&& bottomRows = matrix1.getBottomRows(1);
    EXPECT_EQ(bottomRows.getNumCols(), 2);
    EXPECT_EQ(bottomRows.getNumRows(), 1);
    EXPECT_EQ(bottomRows(0, 0), 5);
    EXPECT_EQ(bottomRows(0, 1), 6);
}


TEST(Matrix, BlockColMajor)
{
    static constexpr uint32_t NUM_ROWS = 3,  NUM_COLS = 2;
    Figaro::Matrix<double, Figaro::MemoryLayout::COL_MAJOR> matrix1(NUM_ROWS, NUM_COLS);

    matrix1(0, 0) = 1; matrix1(0, 1) = 2;
    matrix1(0, 0) = 1; matrix1(0, 1) = 2;
    matrix1(1, 0) = 3; matrix1(1, 1) = 4;
    matrix1(2, 0) = 5; matrix1(2, 1) = 6;

    const auto&& blockMat = matrix1.getBlock(1, 2, 0, 0);
    EXPECT_EQ(blockMat.getNumCols(), 1);
    EXPECT_EQ(blockMat.getNumRows(), 2);
    EXPECT_EQ(blockMat(0, 0), 3);
    EXPECT_EQ(blockMat(1, 0), 5);

    const auto&& rightCols = matrix1.getRightCols(1);
    EXPECT_EQ(rightCols.getNumCols(), 1);
    EXPECT_EQ(rightCols.getNumRows(), 3);
    EXPECT_EQ(rightCols(0, 0), 2);
    EXPECT_EQ(rightCols(1, 0), 4);
    EXPECT_EQ(rightCols(2, 0), 6);

    const auto&& leftCols = matrix1.getLeftCols(1);
    EXPECT_EQ(leftCols.getNumCols(), 1);
    EXPECT_EQ(leftCols.getNumRows(), 3);
    EXPECT_EQ(leftCols(0, 0), 1);
    EXPECT_EQ(leftCols(1, 0), 3);
    EXPECT_EQ(leftCols(2, 0), 5);

    const auto&& topRows = matrix1.getTopRows(1);
    EXPECT_EQ(topRows.getNumCols(), 2);
    EXPECT_EQ(topRows.getNumRows(), 1);
    EXPECT_EQ(topRows(0, 0), 1);
    EXPECT_EQ(topRows(0, 1), 2);

    const auto&& bottomRows = matrix1.getBottomRows(1);
    EXPECT_EQ(bottomRows.getNumCols(), 2);
    EXPECT_EQ(bottomRows.getNumRows(), 1);
    EXPECT_EQ(bottomRows(0, 0), 5);
    EXPECT_EQ(bottomRows(0, 1), 6);
}

TEST(Matrix, ConcatenateVertically)
{
    static constexpr uint32_t NUM_ROWS1 = 3, NUM_ROWS2 = 2, NUM_COLS = 2;
    Figaro::Matrix<double> matrix1(NUM_ROWS1, NUM_COLS);
    Figaro::Matrix<double> matrix2(NUM_ROWS2, NUM_COLS);

    matrix1[0][0] = 1; matrix1(0, 1) = 2;
    matrix1[1][0] = 3; matrix1[1][1] = 4;
    matrix1[2][0] = 5; matrix1[2][1] = 6;

    matrix2[0][0] = 7; matrix2(0, 1) = 8;
    matrix2[1][0] = 9; matrix2[1][1] = 10;

    Figaro::Matrix<double> matrix = matrix1.concatenateVertically(matrix2);
    matrix = matrix.concatenateVerticallyScalar(0, 2);

    EXPECT_EQ(matrix[0][0], 1);
    EXPECT_EQ(matrix(0, 1), 2);
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

    matrix1[0][0] = 1; matrix1(0, 1) = 2;
    matrix1[1][0] = 3; matrix1[1][1] = 4;

    matrix2[0][0] = 5; matrix2(0, 1) = 6; matrix2(0, 2) = 7;
    matrix2[1][0] = 8 ;matrix2[1][1] = 9; matrix2[1][2] = 10;

    Figaro::Matrix<double> matrix = matrix1.concatenateHorizontally(matrix2);
    matrix = matrix.concatenateHorizontallyScalar(0, 2);

    EXPECT_EQ(matrix[0][0], 1);
    EXPECT_EQ(matrix(0, 1), 2);
    EXPECT_EQ(matrix[1][0], 3);
    EXPECT_EQ(matrix[1][1], 4);

    EXPECT_EQ(matrix(0, 2), 5);
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


TEST(Matrix, ZerosRowMajor)
{
    static constexpr uint32_t NUM_ROWS = 2, NUM_COLS = 3;
    Figaro::Matrix<double, Figaro::MemoryLayout::ROW_MAJOR> matrix = Figaro::Matrix<double, Figaro::MemoryLayout::ROW_MAJOR>::zeros(NUM_ROWS, NUM_COLS);

    for(uint32_t rowIdx = 0; rowIdx < NUM_ROWS; rowIdx++)
    {
        for (uint32_t colIdx = 0; colIdx < NUM_COLS; colIdx++)
        {
            EXPECT_EQ(matrix(rowIdx, colIdx), 0);
        }
    }
}

TEST(Matrix, ZerosColMajor)
{
    static constexpr uint32_t NUM_ROWS = 2, NUM_COLS = 3;
    Figaro::Matrix<double, Figaro::MemoryLayout::COL_MAJOR> matrix = Figaro::Matrix<double, Figaro::MemoryLayout::COL_MAJOR>::zeros(NUM_ROWS, NUM_COLS);

    for(uint32_t rowIdx = 0; rowIdx < NUM_ROWS; rowIdx++)
    {
        for (uint32_t colIdx = 0; colIdx < NUM_COLS; colIdx++)
        {
            EXPECT_EQ(matrix(rowIdx, colIdx), 0);
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

TEST(Matrix, TransposeRowMajor)
{
    static constexpr uint32_t NUM_ROWS = 3, NUM_COLS = 2;
    Figaro::Matrix<double, Figaro::MemoryLayout::ROW_MAJOR> matrix(NUM_ROWS, NUM_COLS);

    matrix(0, 0) = 1; matrix(0, 1) = 2;
    matrix(1, 0) = 3; matrix(1, 1) = 4;
    matrix(2, 0) = 4; matrix(2, 1) = 5;

    auto tMatrix = matrix.transpose();
    EXPECT_EQ(tMatrix.getNumCols(), 3);
    EXPECT_EQ(tMatrix.getNumRows(), 2);

    EXPECT_NEAR(tMatrix(0, 0), 1, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(tMatrix(0, 1), 3, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(tMatrix(0, 2), 4, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(tMatrix(1, 0), 2, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(tMatrix(1, 1), 4, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(tMatrix(1, 2), 5, GIVENS_TEST_PRECISION_ERROR);
}


TEST(Matrix, TransposeColMajor)
{
    static constexpr uint32_t NUM_ROWS = 3, NUM_COLS = 2;
    Figaro::Matrix<double, Figaro::MemoryLayout::COL_MAJOR> matrix(NUM_ROWS, NUM_COLS);

    matrix(0, 0) = 1; matrix(0, 1) = 2;
    matrix(1, 0) = 3; matrix(1, 1) = 4;
    matrix(2, 0) = 4; matrix(2, 1) = 5;

    auto tMatrix = matrix.transpose();
    EXPECT_EQ(tMatrix.getNumCols(), 3);
    EXPECT_EQ(tMatrix.getNumRows(), 2);

    EXPECT_NEAR(tMatrix(0, 0), 1, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(tMatrix(0, 1), 3, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(tMatrix(0, 2), 4, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(tMatrix(1, 0), 2, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(tMatrix(1, 1), 4, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(tMatrix(1, 2), 5, GIVENS_TEST_PRECISION_ERROR);
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
    Figaro::Matrix<double, Figaro::MemoryLayout::ROW_MAJOR> matrix(NUM_ROWS, NUM_COLS);

    matrix(0, 0) = 1; matrix(0, 1) = 2;
    matrix(1, 0) = 3; matrix(1, 1) = 4;
    matrix(2, 0) = 4; matrix(2, 1) = 3;

    matrix.computeQRHouseholder();

    EXPECT_NEAR(std::abs(matrix(0, 0)), 5.099019513592785, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrix(0, 1)), 5.099019513592786, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(std::abs(matrix(1, 0)), 0, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrix(1, 1)), 1.732050807568877, GIVENS_TEST_PRECISION_ERROR);
}

TEST(MatrixSparse, computeQR)
{
    static constexpr uint32_t NUM_ROWS = 3, NUM_COLS = 2;
    std::vector<double> vVals = {1, 2, 3, 4, 4, 3};
    std::vector<int64_t> rowStartPos = {0, 2, 4, 6};
    std::vector<int64_t> vColIndcs = {0, 1, 0, 1, 0, 1};
    Figaro::MatrixSparse<double> matrix{NUM_ROWS, NUM_COLS, rowStartPos.data(),  vColIndcs.data(), vVals.data()};
    bool success = matrix.computeQR();
    EXPECT_EQ(success, true);
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


TEST(Matrix, computeCholeskyRowMajor)
{
    static constexpr uint32_t NUM_ROWS = 2, NUM_COLS = 2;
    Figaro::Matrix<double, Figaro::MemoryLayout::ROW_MAJOR> matrix(NUM_ROWS, NUM_COLS);

    matrix(0, 0) = 26; matrix(0, 1) = 26;
    matrix(1, 0) = 26; matrix(1, 1) = 29;
    matrix.computeCholesky();

    FIGARO_LOG_DBG("matrix", matrix)

    EXPECT_NEAR(std::abs(matrix(0, 0)), 5.099019513592785, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrix(0, 1)), 5.099019513592786, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(std::abs(matrix(1, 0)), 0, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrix(1, 1)), 1.732050807568877, GIVENS_TEST_PRECISION_ERROR);
}


TEST(Matrix, computeCholeskyColMajor)
{
    static constexpr uint32_t NUM_ROWS = 2, NUM_COLS = 2;
    Figaro::Matrix<double, Figaro::MemoryLayout::COL_MAJOR> matrix(NUM_ROWS, NUM_COLS);

    matrix(0, 0) = 26; matrix(0, 1) = 26;
    matrix(1, 0) = 26; matrix(1, 1) = 29;
    matrix.computeCholesky();

    FIGARO_LOG_DBG("matrix", matrix)

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

    matrix.computeQRCholesky(true, true, &matrixR, &matrixQ);

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

TEST(Matrix, computeQRCholesky2QRowMajor)
{
    static constexpr uint32_t NUM_ROWS = 3, NUM_COLS = 2;
    Figaro::Matrix<double, Figaro::MemoryLayout::ROW_MAJOR> matrix(NUM_ROWS, NUM_COLS);
    Figaro::Matrix<double, Figaro::MemoryLayout::ROW_MAJOR> matrixR(0, 0), matrixQ(0,0);

    matrix(0, 0) = 1; matrix(0, 1) = 2;
    matrix(1, 0) = 3; matrix(1, 1) = 4;
    matrix(2, 0) = 4; matrix(2, 1) = 3;

    matrix.computeQRCholesky2(true, true, &matrixR, &matrixQ);

    EXPECT_NEAR(std::abs(matrixR(0, 0)), 5.099019513592785, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixR(0, 1)), 5.099019513592786, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(std::abs(matrixR(1, 0)), 0, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixR(1, 1)), 1.732050807568877, GIVENS_TEST_PRECISION_ERROR);

    FIGARO_LOG_DBG("matrixR", matrixR)
    FIGARO_LOG_DBG("matrixQ", matrixQ)
    FIGARO_LOG_DBG("matrixQ orthogonality", matrixQ.getOrthogonality(0))

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

    matrix.computeQRCholesky(true, true, &matrixR, &matrixQ);

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


TEST(Matrix, computeEigenValueDecompositionQRIterRowMajor)
{
    static constexpr uint32_t NUM_ROWS = 4, NUM_COLS = 4;
    Figaro::Matrix<double, Figaro::MemoryLayout::ROW_MAJOR> matrix(NUM_ROWS, NUM_COLS);
    Figaro::Matrix<double, Figaro::MemoryLayout::ROW_MAJOR> matrixE(0, 0);
    Figaro::Matrix<double, Figaro::MemoryLayout::ROW_MAJOR> matrixEV(0, 0);


    matrix(0, 0) = 1; matrix(0, 1) = 2;
    matrix(0, 2) = 3; matrix(0, 3) = 4;

    matrix(1, 0) = 2; matrix(1, 1) = 5;
    matrix(1, 2) = 6; matrix(1, 3) = 7;

    matrix(2, 0) = 3; matrix(2, 1) = 6;
    matrix(2, 2) = 9; matrix(2, 3) = 10;

    matrix(3, 0) = 4; matrix(3, 1) = 7;
    matrix(3, 2) = 10; matrix(3, 3) = 11;

    matrix.computeEigenValueDecomposition(Figaro::EDHintType::QR_ITER, false,
        &matrixE, &matrixEV);

    FIGARO_LOG_DBG("matrixE", matrixE)
    FIGARO_LOG_DBG("matrixOriginal", matrix)
    FIGARO_LOG_DBG("orthogonality", matrix.selfMatrixMultiply(0))

/*
    EXPECT_NEAR(std::abs(matrixE(0, 0)), std::abs(0.551948448869983), GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixE(0, 1)), std::abs(0.432426561344364), GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixE(0, 2)), std::abs(-0.648451457665928), GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixE(0, 3)), std::abs(-0.504404303300050), GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(std::abs(matrixE(1, 0)), std::abs(0.291731531522893), GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixE(1, 1)), std::abs(-0.699239307273875), GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixE(1, 2)), std::abs(-0.330243829214656), GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixE(1, 3)), std::abs(-0.544068571154380), GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(std::abs(matrixE(2, 0)), std::abs(0.526866989555185), GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixE(2, 1)), std::abs(0.249192418330564), GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixE(2, 2)), std::abs(0.669569036618122), GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixE(2, 3)), std::abs(0.513743286769497), GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(std::abs(matrixE(3, 0)), std::abs(0.576764075356973), GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixE(3, 1)), std::abs(-0.511834737834006), GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixE(3, 2)), std::abs(0.148751556357754), GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixE(3, 3)), std::abs(0.430852090622404), GIVENS_TEST_PRECISION_ERROR);
*/
}


TEST(Matrix, computeEigenValueDecompositionDivAndConqRowMajor)
{
    static constexpr uint32_t NUM_ROWS = 4, NUM_COLS = 4;
    Figaro::Matrix<double, Figaro::MemoryLayout::ROW_MAJOR> matrix(NUM_ROWS, NUM_COLS);
    Figaro::Matrix<double, Figaro::MemoryLayout::ROW_MAJOR> matrixE(0, 0);
    Figaro::Matrix<double, Figaro::MemoryLayout::ROW_MAJOR> matrixEV(0, 0);


    matrix(0, 0) = 1; matrix(0, 1) = 2;
    matrix(0, 2) = 3; matrix(0, 3) = 4;

    matrix(1, 0) = 2; matrix(1, 1) = 5;
    matrix(1, 2) = 6; matrix(1, 3) = 7;

    matrix(2, 0) = 3; matrix(2, 1) = 6;
    matrix(2, 2) = 9; matrix(2, 3) = 10;

    matrix(3, 0) = 4; matrix(3, 1) = 7;
    matrix(3, 2) = 10; matrix(3, 3) = 11;

    matrix.computeEigenValueDecomposition(Figaro::EDHintType::DIV_AND_CONQ, false,
        &matrixE, &matrixEV);

    FIGARO_LOG_DBG("matrixE", matrixE)
    FIGARO_LOG_DBG("matrixE", matrixEV)
    FIGARO_LOG_DBG("matrixOriginal", matrix)
/*
    EXPECT_NEAR(std::abs(matrixE(0, 0)), std::abs(0.551948448869983), GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixE(0, 1)), std::abs(0.432426561344364), GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixE(0, 2)), std::abs(-0.648451457665928), GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixE(0, 3)), std::abs(-0.504404303300050), GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(std::abs(matrixE(1, 0)), std::abs(0.291731531522893), GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixE(1, 1)), std::abs(-0.699239307273875), GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixE(1, 2)), std::abs(-0.330243829214656), GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixE(1, 3)), std::abs(-0.544068571154380), GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(std::abs(matrixE(2, 0)), std::abs(0.526866989555185), GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixE(2, 1)), std::abs(0.249192418330564), GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixE(2, 2)), std::abs(0.669569036618122), GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixE(2, 3)), std::abs(0.513743286769497), GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(std::abs(matrixE(3, 0)), std::abs(0.576764075356973), GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixE(3, 1)), std::abs(-0.511834737834006), GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixE(3, 2)), std::abs(0.148751556357754), GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixE(3, 3)), std::abs(0.430852090622404), GIVENS_TEST_PRECISION_ERROR);
*/
}


TEST(Matrix, computeLeastSquaresRowMajor)
{
    static constexpr uint32_t NUM_ROWS = 4, NUM_COLS = 4, NUM_RHS = 2;
    Figaro::Matrix<double, Figaro::MemoryLayout::ROW_MAJOR> matrix(NUM_ROWS, NUM_COLS);
    Figaro::Matrix<double, Figaro::MemoryLayout::ROW_MAJOR> matrixXEx{NUM_COLS, NUM_RHS};
    Figaro::Matrix<double, Figaro::MemoryLayout::ROW_MAJOR> matrixB{NUM_ROWS, NUM_RHS};


    matrix(0, 0) = 1; matrix(0, 1) = 2;
    matrix(0, 2) = 3; matrix(0, 3) = 4;

    matrix(1, 0) = 2; matrix(1, 1) = 5;
    matrix(1, 2) = 6; matrix(1, 3) = 7;

    matrix(2, 0) = 3; matrix(2, 1) = 6;
    matrix(2, 2) = 9; matrix(2, 3) = 10;

    matrix(3, 0) = 4; matrix(3, 1) = 7;
    matrix(3, 2) = 10; matrix(3, 3) = 11;

    matrixXEx(0, 0) = 1; matrixXEx(0, 1) = 2;
    matrixXEx(1, 0) = 2; matrixXEx(1, 1) = 3;
    matrixXEx(2, 0) = 3; matrixXEx(2, 1) = 4;
    matrixXEx(3, 0) = 4; matrixXEx(3, 1) = 5;


    matrixB(0, 0) = 30; matrixB(0, 1) = 40;
    matrixB(1, 0) = 58; matrixB(1, 1) = 78;
    matrixB(2, 0) = 82; matrixB(2, 1) = 110;
    matrixB(3, 0) = 92; matrixB(3, 1) = 124;

    matrix.computeLeastSquares(matrixB);

    FIGARO_LOG_DBG(matrixXEx)
    for (uint32_t rowIdx = 0; rowIdx < matrix.getNumRows(); rowIdx++)
    {
        for (uint32_t colIdx = 0; colIdx < NUM_RHS; colIdx++)
        {
            EXPECT_NEAR(matrixB(rowIdx, colIdx), matrixXEx(rowIdx, colIdx), RELAX_GIVENS_TEST_PRECISION_ERROR);
        }
    }
}

TEST(Matrix, computeLeastSquaresColMajor)
{
    static constexpr uint32_t NUM_ROWS = 4, NUM_COLS = 4, NUM_RHS = 2;
    Figaro::Matrix<double, Figaro::MemoryLayout::COL_MAJOR> matrix(NUM_ROWS, NUM_COLS);
    Figaro::Matrix<double, Figaro::MemoryLayout::COL_MAJOR> matrixXEx{NUM_COLS, NUM_RHS};
    Figaro::Matrix<double, Figaro::MemoryLayout::COL_MAJOR> matrixB{NUM_ROWS, NUM_RHS};


    matrix(0, 0) = 1; matrix(0, 1) = 2;
    matrix(0, 2) = 3; matrix(0, 3) = 4;

    matrix(1, 0) = 2; matrix(1, 1) = 5;
    matrix(1, 2) = 6; matrix(1, 3) = 7;

    matrix(2, 0) = 3; matrix(2, 1) = 6;
    matrix(2, 2) = 9; matrix(2, 3) = 10;

    matrix(3, 0) = 4; matrix(3, 1) = 7;
    matrix(3, 2) = 10; matrix(3, 3) = 11;

    matrixXEx(0, 0) = 1; matrixXEx(0, 1) = 2;
    matrixXEx(1, 0) = 2; matrixXEx(1, 1) = 3;
    matrixXEx(2, 0) = 3; matrixXEx(2, 1) = 4;
    matrixXEx(3, 0) = 4; matrixXEx(3, 1) = 5;


    matrixB(0, 0) = 30; matrixB(0, 1) = 40;
    matrixB(1, 0) = 58; matrixB(1, 1) = 78;
    matrixB(2, 0) = 82; matrixB(2, 1) = 110;
    matrixB(3, 0) = 92; matrixB(3, 1) = 124;

    matrix.computeLeastSquares(matrixB);

    FIGARO_LOG_DBG(matrixXEx)
    for (uint32_t rowIdx = 0; rowIdx < matrix.getNumRows(); rowIdx++)
    {
        for (uint32_t colIdx = 0; colIdx < NUM_RHS; colIdx++)
        {
            EXPECT_NEAR(matrixB(rowIdx, colIdx), matrixXEx(rowIdx, colIdx), RELAX_GIVENS_TEST_PRECISION_ERROR);
        }
    }
}


TEST(MatrixSparse, computeLeastSquaresCSRRowMajor)
{
    static constexpr uint32_t NUM_ROWS = 4, NUM_COLS = 4, NUM_RHS = 1;
    Figaro::Matrix<double, Figaro::MemoryLayout::ROW_MAJOR> matrixXEx{NUM_COLS, NUM_RHS};
    Figaro::Matrix<double, Figaro::MemoryLayout::ROW_MAJOR> matrixB{NUM_ROWS, NUM_RHS};
    Figaro::Matrix<double, Figaro::MemoryLayout::ROW_MAJOR> matrixX{NUM_COLS, NUM_RHS};

    std::vector<double> vVals = {1, 2, 3, 4, 2, 5, 6, 7, 3, 6, 9, 10, 4, 7, 10, 11};
    std::vector<int64_t> rowStartPos = {0, 4, 8, 12, 16};
    std::vector<int64_t> vColIndcs = {0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3};
    Figaro::MatrixSparse<double, Figaro::MemoryLayout::CSR>
        matrix{NUM_ROWS, NUM_COLS, rowStartPos.data(),  vColIndcs.data(), vVals.data()};

    matrixXEx(0, 0) = 1; //matrixXEx(0, 1) = 2;
    matrixXEx(1, 0) = 2; //matrixXEx(1, 1) = 3;
    matrixXEx(2, 0) = 3; //matrixXEx(2, 1) = 4;
    matrixXEx(3, 0) = 4; //matrixXEx(3, 1) = 5;


    matrixB(0, 0) = 30; //matrixB(0, 1) = 40;
    matrixB(1, 0) = 58; //matrixB(1, 1) = 78;
    matrixB(2, 0) = 82; //matrixB(2, 1) = 110;
    matrixB(3, 0) = 92; //matrixB(3, 1) = 124;

    bool success = matrix.computeLeastSquares(matrixB, matrixX);
    EXPECT_EQ(success, true);

    for (uint32_t rowIdx = 0; rowIdx < matrix.getNumRows(); rowIdx++)
    {
        for (uint32_t colIdx = 0; colIdx < NUM_RHS; colIdx++)
        {
            EXPECT_NEAR(matrixX(rowIdx, colIdx), matrixXEx(rowIdx, colIdx), RELAX_GIVENS_TEST_PRECISION_ERROR);
        }
    }
}

TEST(MatrixSparse, computeLeastSquaresCSRColMajor)
{
    static constexpr uint32_t NUM_ROWS = 4, NUM_COLS = 4, NUM_RHS = 1;
    Figaro::Matrix<double, Figaro::MemoryLayout::COL_MAJOR> matrixXEx{NUM_COLS, NUM_RHS};
    Figaro::Matrix<double, Figaro::MemoryLayout::COL_MAJOR> matrixB{NUM_ROWS, NUM_RHS};
    Figaro::Matrix<double, Figaro::MemoryLayout::COL_MAJOR> matrixX{NUM_ROWS, NUM_RHS};

    std::vector<double> vVals = {1, 2, 3, 4, 2, 5, 6, 7, 3, 6, 9, 10, 4, 7, 10, 11};
    std::vector<int64_t> rowStartPos = {0, 4, 8, 12, 16};
    std::vector<int64_t> vColIndcs = {0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3};
    Figaro::MatrixSparse<double, Figaro::MemoryLayout::CSR>
        matrix{NUM_ROWS, NUM_COLS, rowStartPos.data(),  vColIndcs.data(), vVals.data()};

    matrixXEx(0, 0) = 1; //matrixXEx(0, 1) = 2;
    matrixXEx(1, 0) = 2; //matrixXEx(1, 1) = 3;
    matrixXEx(2, 0) = 3; //matrixXEx(2, 1) = 4;
    matrixXEx(3, 0) = 4; //matrixXEx(3, 1) = 5;


    matrixB(0, 0) = 30; //matrixB(0, 1) = 40;
    matrixB(1, 0) = 58; //matrixB(1, 1) = 78;
    matrixB(2, 0) = 82; //matrixB(2, 1) = 110;
    matrixB(3, 0) = 92; //matrixB(3, 1) = 124;

    bool success = matrix.computeLeastSquares(matrixB, matrixX);
    EXPECT_EQ(success, true);

    for (uint32_t rowIdx = 0; rowIdx < matrix.getNumRows(); rowIdx++)
    {
        for (uint32_t colIdx = 0; colIdx < NUM_RHS; colIdx++)
        {
            EXPECT_NEAR(matrixX(rowIdx, colIdx), matrixXEx(rowIdx, colIdx), RELAX_GIVENS_TEST_PRECISION_ERROR);
        }
    }
}

TEST(Matrix, computeSVDDivAndConqRowMajor)
{
    static constexpr uint32_t NUM_ROWS = 5, NUM_COLS = 3;
    Figaro::Matrix<double, Figaro::MemoryLayout::ROW_MAJOR> matrix(NUM_ROWS, NUM_COLS);
    Figaro::Matrix<double, Figaro::MemoryLayout::ROW_MAJOR> matrixU(0, 0);
    Figaro::Matrix<double, Figaro::MemoryLayout::ROW_MAJOR> matrixS(0, 0);
    Figaro::Matrix<double, Figaro::MemoryLayout::ROW_MAJOR> matrixVT(0, 0);

    matrix(0, 0) = 1; matrix(0, 1) = 2; matrix(0, 2) = 27;
    matrix(1, 0) = 3; matrix(1, 1) = 4; matrix(1, 2) = 12;
    matrix(2, 0) = 5; matrix(2, 1) = 6; matrix(2, 2) = 13;
    matrix(3, 0) = 7; matrix(3, 1) = 8; matrix(3, 2) = 14;
    matrix(4, 0) = 9; matrix(4, 1) = 23; matrix(4, 2) = 17;

    matrix.computeSVDDivAndConq(1, true,
        &matrixU, &matrixS, &matrixVT);

    EXPECT_NEAR(matrixU(0, 0), -0.532977758781636, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrixU(0, 1), -0.719504148787956, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrixU(0, 2), -0.382389696847683, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(matrixU(1, 0), -0.283948361179314, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrixU(1, 1), -0.134893419827008, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrixU(1, 2), 0.145821660497625, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(matrixU(2, 0), -0.334946954795978, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrixU(2, 1), -0.040622061532983, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrixU(2, 2), 0.422403208347773, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(matrixU(3, 0), -0.385945548412641, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrixU(3, 1), 0.053649296761042, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrixU(3, 2), 0.698984756197921, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(matrixU(4, 0), -0.611689960650861, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrixU(4, 1), 0.677930045239293, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrixU(4, 2), -0.406829206491980, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(matrixS(0, 0), 44.989193549900570, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrixS(1, 0), 17.417873990872451, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrixS(2, 0), 3.686479264511551, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(matrixVT(0, 0), -0.250424273299085, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrixVT(1, 0), 0.295651511272016, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrixVT(2, 0), 0.921888207552954, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(matrixVT(0, 1), -0.474955483467975, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrixVT(1, 1), 0.792247726429847, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrixVT(2, 1),  -0.383093759660007, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(matrixVT(0, 2), -0.843626085458675, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrixVT(1, 2), -0.533791835690010, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrixVT(2, 2), -0.057976754689808, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(matrixU.getOrthogonality(0), 0, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrixVT.getOrthogonality(0), 0, GIVENS_TEST_PRECISION_ERROR);
}

TEST(Matrix, computeSVDDivAndConqColMajor)
{
    static constexpr uint32_t NUM_ROWS = 5, NUM_COLS = 3;
    Figaro::Matrix<double, Figaro::MemoryLayout::COL_MAJOR> matrix(NUM_ROWS, NUM_COLS);
    Figaro::Matrix<double, Figaro::MemoryLayout::COL_MAJOR> matrixU(0, 0);
    Figaro::Matrix<double, Figaro::MemoryLayout::COL_MAJOR> matrixS(0, 0);
    Figaro::Matrix<double, Figaro::MemoryLayout::COL_MAJOR> matrixVT(0, 0);

    matrix(0, 0) = 1; matrix(0, 1) = 2; matrix(0, 2) = 27;
    matrix(1, 0) = 3; matrix(1, 1) = 4; matrix(1, 2) = 12;
    matrix(2, 0) = 5; matrix(2, 1) = 6; matrix(2, 2) = 13;
    matrix(3, 0) = 7; matrix(3, 1) = 8; matrix(3, 2) = 14;
    matrix(4, 0) = 9; matrix(4, 1) = 23; matrix(4, 2) = 17;

    matrix.computeSVDDivAndConq(1, true,
        &matrixU, &matrixS, &matrixVT);

    EXPECT_NEAR(matrixU(0, 0), -0.532977758781636, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrixU(0, 1), -0.719504148787956, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrixU(0, 2), -0.382389696847683, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(matrixU(1, 0), -0.283948361179314, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrixU(1, 1), -0.134893419827008, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrixU(1, 2), 0.145821660497625, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(matrixU(2, 0), -0.334946954795978, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrixU(2, 1), -0.040622061532983, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrixU(2, 2), 0.422403208347773, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(matrixU(3, 0), -0.385945548412641, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrixU(3, 1), 0.053649296761042, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrixU(3, 2), 0.698984756197921, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(matrixU(4, 0), -0.611689960650861, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrixU(4, 1), 0.677930045239293, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrixU(4, 2), -0.406829206491980, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(matrixS(0, 0), 44.989193549900570, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrixS(1, 0), 17.417873990872451, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrixS(2, 0), 3.686479264511551, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(matrixVT(0, 0), -0.250424273299085, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrixVT(1, 0), 0.295651511272016, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrixVT(2, 0), 0.921888207552954, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(matrixVT(0, 1), -0.474955483467975, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrixVT(1, 1), 0.792247726429847, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrixVT(2, 1),  -0.383093759660007, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(matrixVT(0, 2), -0.843626085458675, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrixVT(1, 2), -0.533791835690010, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrixVT(2, 2), -0.057976754689808, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(matrixU.getOrthogonality(0), 0, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrixVT.getOrthogonality(0), 0, GIVENS_TEST_PRECISION_ERROR);
}


TEST(Matrix, computeSVDDivAndConqRowMajorSingularValues)
{
    static constexpr uint32_t NUM_ROWS = 5, NUM_COLS = 3;
    Figaro::Matrix<double, Figaro::MemoryLayout::ROW_MAJOR> matrix(NUM_ROWS, NUM_COLS);
    Figaro::Matrix<double, Figaro::MemoryLayout::ROW_MAJOR> matrixS(0, 0);

    matrix(0, 0) = 1; matrix(0, 1) = 2; matrix(0, 2) = 27;
    matrix(1, 0) = 3; matrix(1, 1) = 4; matrix(1, 2) = 12;
    matrix(2, 0) = 5; matrix(2, 1) = 6; matrix(2, 2) = 13;
    matrix(3, 0) = 7; matrix(3, 1) = 8; matrix(3, 2) = 14;
    matrix(4, 0) = 9; matrix(4, 1) = 23; matrix(4, 2) = 17;

    matrix.computeSVDDivAndConq(1, true,
        nullptr, &matrixS, nullptr);

    EXPECT_NEAR(matrixS(0, 0), 44.989193549900570, RELAX_GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrixS(1, 0), 17.417873990872451, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrixS(2, 0), 3.686479264511551, GIVENS_TEST_PRECISION_ERROR);
}


TEST(Matrix, computeSVDDivAndConqColMajorSingularValues)
{
    static constexpr uint32_t NUM_ROWS = 5, NUM_COLS = 3;
    Figaro::Matrix<double, Figaro::MemoryLayout::COL_MAJOR> matrix(NUM_ROWS, NUM_COLS);
    Figaro::Matrix<double, Figaro::MemoryLayout::COL_MAJOR> matrixS(0, 0);

   matrix(0, 0) = 1; matrix(0, 1) = 2; matrix(0, 2) = 27;
    matrix(1, 0) = 3; matrix(1, 1) = 4; matrix(1, 2) = 12;
    matrix(2, 0) = 5; matrix(2, 1) = 6; matrix(2, 2) = 13;
    matrix(3, 0) = 7; matrix(3, 1) = 8; matrix(3, 2) = 14;
    matrix(4, 0) = 9; matrix(4, 1) = 23; matrix(4, 2) = 17;

    matrix.computeSVDDivAndConq(1, true,
        nullptr, &matrixS, nullptr);

    EXPECT_NEAR(matrixS(0, 0), 44.989193549900570, RELAX_GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrixS(1, 0), 17.417873990872451, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrixS(2, 0), 3.686479264511551, GIVENS_TEST_PRECISION_ERROR);
}


TEST(Matrix, computeSVDQRIterRowMajor)
{
    static constexpr uint32_t NUM_ROWS = 5, NUM_COLS = 3;
    Figaro::Matrix<double, Figaro::MemoryLayout::ROW_MAJOR> matrix(NUM_ROWS, NUM_COLS);
    Figaro::Matrix<double, Figaro::MemoryLayout::ROW_MAJOR> matrixU(0, 0);
    Figaro::Matrix<double, Figaro::MemoryLayout::ROW_MAJOR> matrixS(0, 0);
    Figaro::Matrix<double, Figaro::MemoryLayout::ROW_MAJOR> matrixVT(0, 0);

    matrix(0, 0) = 1; matrix(0, 1) = 2; matrix(0, 2) = 27;
    matrix(1, 0) = 3; matrix(1, 1) = 4; matrix(1, 2) = 12;
    matrix(2, 0) = 5; matrix(2, 1) = 6; matrix(2, 2) = 13;
    matrix(3, 0) = 7; matrix(3, 1) = 8; matrix(3, 2) = 14;
    matrix(4, 0) = 9; matrix(4, 1) = 23; matrix(4, 2) = 17;

    matrix.computeSVDQRIter(1, true, &matrixU, &matrixS, &matrixVT);

    EXPECT_NEAR(std::abs(matrixU(0, 0)), 0.532977758781636, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixU(0, 1)), 0.719504148787956, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixU(0, 2)), 0.382389696847683, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(std::abs(matrixU(1, 0)), 0.283948361179314, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixU(1, 1)), 0.134893419827008, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixU(1, 2)), 0.145821660497625, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(std::abs(matrixU(2, 0)), 0.334946954795978, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixU(2, 1)), 0.040622061532983, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixU(2, 2)), 0.422403208347773, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(std::abs(matrixU(3, 0)), 0.385945548412641, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixU(3, 1)), 0.053649296761042, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixU(3, 2)), 0.698984756197921, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(std::abs(matrixU(4, 0)), 0.611689960650861, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixU(4, 1)), 0.677930045239293, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixU(4, 2)), 0.406829206491980, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(matrixS(0, 0), 44.989193549900570, RELAX_GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrixS(1, 0), 17.417873990872451, RELAX_GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrixS(2, 0), 3.686479264511551, RELAX_GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(std::abs(matrixVT(0, 0)), 0.250424273299085, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixVT(1, 0)), 0.295651511272016, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixVT(2, 0)), 0.921888207552954, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(std::abs(matrixVT(0, 1)), 0.474955483467975, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixVT(1, 1)), 0.792247726429847, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixVT(2, 1)), 0.383093759660007, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(std::abs(matrixVT(0, 2)), 0.843626085458675, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixVT(1, 2)), 0.533791835690010, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixVT(2, 2)), 0.057976754689808, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(matrixU.getOrthogonality(0), 0, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrixVT.getOrthogonality(0), 0, GIVENS_TEST_PRECISION_ERROR);
}

TEST(Matrix, computeSVDQRIterColMajor)
{
    static constexpr uint32_t NUM_ROWS = 5, NUM_COLS = 3;
    Figaro::Matrix<double, Figaro::MemoryLayout::COL_MAJOR> matrix(NUM_ROWS, NUM_COLS);
    Figaro::Matrix<double, Figaro::MemoryLayout::COL_MAJOR> matrixU(0, 0);
    Figaro::Matrix<double, Figaro::MemoryLayout::COL_MAJOR> matrixS(0, 0);
    Figaro::Matrix<double, Figaro::MemoryLayout::COL_MAJOR> matrixVT(0, 0);

    matrix(0, 0) = 1; matrix(0, 1) = 2; matrix(0, 2) = 27;
    matrix(1, 0) = 3; matrix(1, 1) = 4; matrix(1, 2) = 12;
    matrix(2, 0) = 5; matrix(2, 1) = 6; matrix(2, 2) = 13;
    matrix(3, 0) = 7; matrix(3, 1) = 8; matrix(3, 2) = 14;
    matrix(4, 0) = 9; matrix(4, 1) = 23; matrix(4, 2) = 17;

    matrix.computeSVDQRIter(1, true, &matrixU, &matrixS, &matrixVT);

   EXPECT_NEAR(std::abs(matrixU(0, 0)), 0.532977758781636, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixU(0, 1)), 0.719504148787956, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixU(0, 2)), 0.382389696847683, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(std::abs(matrixU(1, 0)), 0.283948361179314, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixU(1, 1)), 0.134893419827008, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixU(1, 2)), 0.145821660497625, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(std::abs(matrixU(2, 0)), 0.334946954795978, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixU(2, 1)), 0.040622061532983, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixU(2, 2)), 0.422403208347773, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(std::abs(matrixU(3, 0)), 0.385945548412641, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixU(3, 1)), 0.053649296761042, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixU(3, 2)), 0.698984756197921, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(std::abs(matrixU(4, 0)), 0.611689960650861, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixU(4, 1)), 0.677930045239293, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixU(4, 2)), 0.406829206491980, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(matrixS(0, 0), 44.989193549900570, RELAX_GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrixS(1, 0), 17.417873990872451, RELAX_GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrixS(2, 0), 3.686479264511551, RELAX_GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(std::abs(matrixVT(0, 0)), 0.250424273299085, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixVT(1, 0)), 0.295651511272016, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixVT(2, 0)), 0.921888207552954, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(std::abs(matrixVT(0, 1)), 0.474955483467975, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixVT(1, 1)), 0.792247726429847, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixVT(2, 1)),  0.383093759660007, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(std::abs(matrixVT(0, 2)), 0.843626085458675, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixVT(1, 2)), 0.533791835690010, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixVT(2, 2)), 0.057976754689808, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(matrixU.getOrthogonality(0), 0, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrixVT.getOrthogonality(0), 0, GIVENS_TEST_PRECISION_ERROR);
}


TEST(Matrix, computeSVDQRIterRowMajorSingularValues)
{
    static constexpr uint32_t NUM_ROWS = 5, NUM_COLS = 3;
    Figaro::Matrix<double, Figaro::MemoryLayout::ROW_MAJOR> matrix(NUM_ROWS, NUM_COLS);
    Figaro::Matrix<double, Figaro::MemoryLayout::ROW_MAJOR> matrixS(0, 0);

    matrix(0, 0) = 1; matrix(0, 1) = 2; matrix(0, 2) = 27;
    matrix(1, 0) = 3; matrix(1, 1) = 4; matrix(1, 2) = 12;
    matrix(2, 0) = 5; matrix(2, 1) = 6; matrix(2, 2) = 13;
    matrix(3, 0) = 7; matrix(3, 1) = 8; matrix(3, 2) = 14;
    matrix(4, 0) = 9; matrix(4, 1) = 23; matrix(4, 2) = 17;

    matrix.computeSVDQRIter(1, true, nullptr, &matrixS, nullptr);

    EXPECT_NEAR(matrixS(0, 0), 44.989193549900570, RELAX_GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrixS(1, 0), 17.417873990872451, RELAX_GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrixS(2, 0), 3.686479264511551, RELAX_GIVENS_TEST_PRECISION_ERROR);
}

TEST(Matrix, computeSVDQRIterColMajorSingularValues)
{
    static constexpr uint32_t NUM_ROWS = 5, NUM_COLS = 3;
    Figaro::Matrix<double, Figaro::MemoryLayout::COL_MAJOR> matrix(NUM_ROWS, NUM_COLS);
    Figaro::Matrix<double, Figaro::MemoryLayout::COL_MAJOR> matrixU(0, 0);
    Figaro::Matrix<double, Figaro::MemoryLayout::COL_MAJOR> matrixS(0, 0);
    Figaro::Matrix<double, Figaro::MemoryLayout::COL_MAJOR> matrixVT(0, 0);

    matrix(0, 0) = 1; matrix(0, 1) = 2; matrix(0, 2) = 27;
    matrix(1, 0) = 3; matrix(1, 1) = 4; matrix(1, 2) = 12;
    matrix(2, 0) = 5; matrix(2, 1) = 6; matrix(2, 2) = 13;
    matrix(3, 0) = 7; matrix(3, 1) = 8; matrix(3, 2) = 14;
    matrix(4, 0) = 9; matrix(4, 1) = 23; matrix(4, 2) = 17;

    matrix.computeSVDQRIter(1, true, nullptr, &matrixS, nullptr);

    EXPECT_NEAR(matrixS(0, 0), 44.989193549900570, RELAX_GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrixS(1, 0), 17.417873990872451, RELAX_GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrixS(2, 0), 3.686479264511551, RELAX_GIVENS_TEST_PRECISION_ERROR);
}


TEST(Matrix, computeSVDPowerIterRowMajor)
{
    static constexpr uint32_t NUM_ROWS = 5, NUM_COLS = 3;
    static constexpr uint32_t NUM_ITERS = 10000;
    Figaro::Matrix<double, Figaro::MemoryLayout::ROW_MAJOR> matrix(NUM_ROWS, NUM_COLS);
    Figaro::Matrix<double, Figaro::MemoryLayout::ROW_MAJOR> matrixU(0, 0);
    Figaro::Matrix<double, Figaro::MemoryLayout::ROW_MAJOR> matrixS(0, 0);
    Figaro::Matrix<double, Figaro::MemoryLayout::ROW_MAJOR> matrixVT(0, 0);

    matrix(0, 0) = 1; matrix(0, 1) = 2; matrix(0, 2) = 27;
    matrix(1, 0) = 3; matrix(1, 1) = 4; matrix(1, 2) = 12;
    matrix(2, 0) = 5; matrix(2, 1) = 6; matrix(2, 2) = 13;
    matrix(3, 0) = 7; matrix(3, 1) = 8; matrix(3, 2) = 14;
    matrix(4, 0) = 9; matrix(4, 1) = 23; matrix(4, 2) = 17;

    matrix.computeSVDPowerIter(1, NUM_ITERS, true,
        &matrixU, &matrixS, &matrixVT);

    EXPECT_NEAR(std::abs(matrixU(0, 0)), 0.532977758781636, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixU(0, 1)), 0.719504148787956, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixU(0, 2)), 0.382389696847683, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(std::abs(matrixU(1, 0)), 0.283948361179314, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixU(1, 1)), 0.134893419827008, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixU(1, 2)), 0.145821660497625, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(std::abs(matrixU(2, 0)), 0.334946954795978, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixU(2, 1)), 0.040622061532983, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixU(2, 2)), 0.422403208347773, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(std::abs(matrixU(3, 0)), 0.385945548412641, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixU(3, 1)), 0.053649296761042, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixU(3, 2)), 0.698984756197921, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(std::abs(matrixU(4, 0)), 0.611689960650861, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixU(4, 1)), 0.677930045239293, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixU(4, 2)), 0.406829206491980, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(matrixS(0, 0), 44.989193549900570, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrixS(1, 0), 17.417873990872451, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrixS(2, 0), 3.686479264511551, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(std::abs(matrixVT(0, 0)), 0.250424273299085, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixVT(1, 0)), 0.295651511272016, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixVT(2, 0)), 0.921888207552954, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(std::abs(matrixVT(0, 1)), 0.474955483467975, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixVT(1, 1)), 0.792247726429847, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixVT(2, 1)), 0.383093759660007, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(std::abs(matrixVT(0, 2)), 0.843626085458675, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixVT(1, 2)), 0.533791835690010, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixVT(2, 2)), 0.057976754689808, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(matrixU.getOrthogonality(0), 0, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrixVT.getOrthogonality(0), 0, GIVENS_TEST_PRECISION_ERROR);
}


TEST(Matrix, computeSVDPowerIterColMajor)
{
    static constexpr uint32_t NUM_ROWS = 5, NUM_COLS = 3;
    static constexpr uint32_t NUM_ITERS = 10000;
    Figaro::Matrix<double, Figaro::MemoryLayout::COL_MAJOR> matrix(NUM_ROWS, NUM_COLS);
    Figaro::Matrix<double, Figaro::MemoryLayout::COL_MAJOR> matrixU(0, 0);
    Figaro::Matrix<double, Figaro::MemoryLayout::COL_MAJOR> matrixS(0, 0);
    Figaro::Matrix<double, Figaro::MemoryLayout::COL_MAJOR> matrixVT(0, 0);

    matrix(0, 0) = 1; matrix(0, 1) = 2; matrix(0, 2) = 27;
    matrix(1, 0) = 3; matrix(1, 1) = 4; matrix(1, 2) = 12;
    matrix(2, 0) = 5; matrix(2, 1) = 6; matrix(2, 2) = 13;
    matrix(3, 0) = 7; matrix(3, 1) = 8; matrix(3, 2) = 14;
    matrix(4, 0) = 9; matrix(4, 1) = 23; matrix(4, 2) = 17;

    matrix.computeSVDPowerIter(1, NUM_ITERS, true,
        &matrixU, &matrixS, &matrixVT);

    EXPECT_NEAR(std::abs(matrixU(0, 0)), 0.532977758781636, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixU(0, 1)), 0.719504148787956, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixU(0, 2)), 0.382389696847683, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(std::abs(matrixU(1, 0)), 0.283948361179314, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixU(1, 1)), 0.134893419827008, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixU(1, 2)), 0.145821660497625, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(std::abs(matrixU(2, 0)), 0.334946954795978, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixU(2, 1)), 0.040622061532983, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixU(2, 2)), 0.422403208347773, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(std::abs(matrixU(3, 0)), 0.385945548412641, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixU(3, 1)), 0.053649296761042, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixU(3, 2)), 0.698984756197921, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(std::abs(matrixU(4, 0)), 0.611689960650861, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixU(4, 1)), 0.677930045239293, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixU(4, 2)), 0.406829206491980, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(matrixS(0, 0), 44.989193549900570, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrixS(1, 0), 17.417873990872451, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrixS(2, 0), 3.686479264511551, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(std::abs(matrixVT(0, 0)), 0.250424273299085, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixVT(1, 0)), 0.295651511272016, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixVT(2, 0)), 0.921888207552954, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(std::abs(matrixVT(0, 1)), 0.474955483467975, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixVT(1, 1)), 0.792247726429847, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixVT(2, 1)), 0.383093759660007, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(std::abs(matrixVT(0, 2)), 0.843626085458675, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixVT(1, 2)), 0.533791835690010, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixVT(2, 2)), 0.057976754689808, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(matrixU.getOrthogonality(0), 0, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrixVT.getOrthogonality(0), 0, GIVENS_TEST_PRECISION_ERROR);
}


TEST(Matrix, computeSVDEigDecDivAndConqRowMajor)
{
    static constexpr uint32_t NUM_ROWS = 5, NUM_COLS = 3;
    Figaro::Matrix<double, Figaro::MemoryLayout::ROW_MAJOR> matrix(NUM_ROWS, NUM_COLS);
    Figaro::Matrix<double, Figaro::MemoryLayout::ROW_MAJOR> matrixU(0, 0);
    Figaro::Matrix<double, Figaro::MemoryLayout::ROW_MAJOR> matrixS(0, 0);
    Figaro::Matrix<double, Figaro::MemoryLayout::ROW_MAJOR> matrixVT(0, 0);

    matrix(0, 0) = 1; matrix(0, 1) = 2; matrix(0, 2) = 27;
    matrix(1, 0) = 3; matrix(1, 1) = 4; matrix(1, 2) = 12;
    matrix(2, 0) = 5; matrix(2, 1) = 6; matrix(2, 2) = 13;
    matrix(3, 0) = 7; matrix(3, 1) = 8; matrix(3, 2) = 14;
    matrix(4, 0) = 9; matrix(4, 1) = 23; matrix(4, 2) = 17;

    matrix.computeSVDEigenDec(1, Figaro::SVDHintType::EIGEN_DECOMP_DIV_AND_CONQ,
        true, true, &matrixU, &matrixS, &matrixVT);
    EXPECT_NEAR(std::abs(matrixU(0, 0)), 0.532977758781636, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixU(0, 1)), 0.719504148787956, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixU(0, 2)), 0.382389696847683, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(std::abs(matrixU(1, 0)), 0.283948361179314, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixU(1, 1)), 0.134893419827008, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixU(1, 2)), 0.145821660497625, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(std::abs(matrixU(2, 0)), 0.334946954795978, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixU(2, 1)), 0.040622061532983, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixU(2, 2)), 0.422403208347773, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(std::abs(matrixU(3, 0)), 0.385945548412641, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixU(3, 1)), 0.053649296761042, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixU(3, 2)), 0.698984756197921, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(std::abs(matrixU(4, 0)), 0.611689960650861, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixU(4, 1)), 0.677930045239293, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixU(4, 2)), 0.406829206491980, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(matrixS(0, 0), 44.989193549900570, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrixS(1, 0), 17.417873990872451, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrixS(2, 0), 3.686479264511551, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(std::abs(matrixVT(0, 0)), 0.250424273299085, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixVT(1, 0)), 0.295651511272016, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixVT(2, 0)), 0.921888207552954, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(std::abs(matrixVT(0, 1)), 0.474955483467975, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixVT(1, 1)), 0.792247726429847, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixVT(2, 1)), 0.383093759660007, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(std::abs(matrixVT(0, 2)), 0.843626085458675, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixVT(1, 2)), 0.533791835690010, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixVT(2, 2)), 0.057976754689808, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(matrixU.getOrthogonality(0), 0, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrixVT.getOrthogonality(0), 0, GIVENS_TEST_PRECISION_ERROR);
}


TEST(Matrix, computeSVDEigDecDivAndConqColMajor)
{
   static constexpr uint32_t NUM_ROWS = 5, NUM_COLS = 3;
    Figaro::Matrix<double, Figaro::MemoryLayout::COL_MAJOR> matrix(NUM_ROWS, NUM_COLS);
    Figaro::Matrix<double, Figaro::MemoryLayout::COL_MAJOR> matrixU(0, 0);
    Figaro::Matrix<double, Figaro::MemoryLayout::COL_MAJOR> matrixS(0, 0);
    Figaro::Matrix<double, Figaro::MemoryLayout::COL_MAJOR> matrixVT(0, 0);

    matrix(0, 0) = 1; matrix(0, 1) = 2; matrix(0, 2) = 27;
    matrix(1, 0) = 3; matrix(1, 1) = 4; matrix(1, 2) = 12;
    matrix(2, 0) = 5; matrix(2, 1) = 6; matrix(2, 2) = 13;
    matrix(3, 0) = 7; matrix(3, 1) = 8; matrix(3, 2) = 14;
    matrix(4, 0) = 9; matrix(4, 1) = 23; matrix(4, 2) = 17;

    matrix.computeSVDEigenDec(1, Figaro::SVDHintType::EIGEN_DECOMP_DIV_AND_CONQ,
        true, true, &matrixU, &matrixS, &matrixVT);
    EXPECT_NEAR(std::abs(matrixU(0, 0)), 0.532977758781636, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixU(0, 1)), 0.719504148787956, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixU(0, 2)), 0.382389696847683, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(std::abs(matrixU(1, 0)), 0.283948361179314, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixU(1, 1)), 0.134893419827008, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixU(1, 2)), 0.145821660497625, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(std::abs(matrixU(2, 0)), 0.334946954795978, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixU(2, 1)), 0.040622061532983, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixU(2, 2)), 0.422403208347773, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(std::abs(matrixU(3, 0)), 0.385945548412641, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixU(3, 1)), 0.053649296761042, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixU(3, 2)), 0.698984756197921, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(std::abs(matrixU(4, 0)), 0.611689960650861, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixU(4, 1)), 0.677930045239293, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixU(4, 2)), 0.406829206491980, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(matrixS(0, 0), 44.989193549900570, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrixS(1, 0), 17.417873990872451, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrixS(2, 0), 3.686479264511551, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(std::abs(matrixVT(0, 0)), 0.250424273299085, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixVT(1, 0)), 0.295651511272016, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixVT(2, 0)), 0.921888207552954, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(std::abs(matrixVT(0, 1)), 0.474955483467975, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixVT(1, 1)), 0.792247726429847, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixVT(2, 1)), 0.383093759660007, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(std::abs(matrixVT(0, 2)), 0.843626085458675, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixVT(1, 2)), 0.533791835690010, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixVT(2, 2)), 0.057976754689808, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(matrixU.getOrthogonality(0), 0, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrixVT.getOrthogonality(0), 0, GIVENS_TEST_PRECISION_ERROR);
}


TEST(Matrix, computeSVDEigDecDivAndConqSingValuesRowMajor)
{
    static constexpr uint32_t NUM_ROWS = 5, NUM_COLS = 3;
    Figaro::Matrix<double, Figaro::MemoryLayout::ROW_MAJOR> matrix(NUM_ROWS, NUM_COLS);
    Figaro::Matrix<double, Figaro::MemoryLayout::ROW_MAJOR> matrixU(0, 0);
    Figaro::Matrix<double, Figaro::MemoryLayout::ROW_MAJOR> matrixS(0, 0);
    Figaro::Matrix<double, Figaro::MemoryLayout::ROW_MAJOR> matrixVT(0, 0);

    matrix(0, 0) = 1; matrix(0, 1) = 2; matrix(0, 2) = 27;
    matrix(1, 0) = 3; matrix(1, 1) = 4; matrix(1, 2) = 12;
    matrix(2, 0) = 5; matrix(2, 1) = 6; matrix(2, 2) = 13;
    matrix(3, 0) = 7; matrix(3, 1) = 8; matrix(3, 2) = 14;
    matrix(4, 0) = 9; matrix(4, 1) = 23; matrix(4, 2) = 17;

    matrix.computeSVDEigenDec(1, Figaro::SVDHintType::EIGEN_DECOMP_DIV_AND_CONQ,
        false, false, nullptr, &matrixS, nullptr);

    EXPECT_NEAR(matrixS(0, 0), 44.989193549900570, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrixS(1, 0), 17.417873990872451, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrixS(2, 0), 3.686479264511551, GIVENS_TEST_PRECISION_ERROR);
}


TEST(Matrix, computeSVDEigDecDivAndConqSingValuesColMajor)
{
   static constexpr uint32_t NUM_ROWS = 5, NUM_COLS = 3;
    Figaro::Matrix<double, Figaro::MemoryLayout::COL_MAJOR> matrix(NUM_ROWS, NUM_COLS);
    Figaro::Matrix<double, Figaro::MemoryLayout::COL_MAJOR> matrixU(0, 0);
    Figaro::Matrix<double, Figaro::MemoryLayout::COL_MAJOR> matrixS(0, 0);
    Figaro::Matrix<double, Figaro::MemoryLayout::COL_MAJOR> matrixVT(0, 0);

    matrix(0, 0) = 1; matrix(0, 1) = 2; matrix(0, 2) = 27;
    matrix(1, 0) = 3; matrix(1, 1) = 4; matrix(1, 2) = 12;
    matrix(2, 0) = 5; matrix(2, 1) = 6; matrix(2, 2) = 13;
    matrix(3, 0) = 7; matrix(3, 1) = 8; matrix(3, 2) = 14;
    matrix(4, 0) = 9; matrix(4, 1) = 23; matrix(4, 2) = 17;

    matrix.computeSVDEigenDec(1, Figaro::SVDHintType::EIGEN_DECOMP_DIV_AND_CONQ,
        false, false, nullptr, &matrixS, nullptr);

    EXPECT_NEAR(matrixS(0, 0), 44.989193549900570, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrixS(1, 0), 17.417873990872451, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrixS(2, 0), 3.686479264511551, GIVENS_TEST_PRECISION_ERROR);
}


TEST(Matrix, computeSVDEigDecQrIterRowMajor)
{
    static constexpr uint32_t NUM_ROWS = 5, NUM_COLS = 3;
    Figaro::Matrix<double, Figaro::MemoryLayout::ROW_MAJOR> matrix(NUM_ROWS, NUM_COLS);
    Figaro::Matrix<double, Figaro::MemoryLayout::ROW_MAJOR> matrixU(0, 0);
    Figaro::Matrix<double, Figaro::MemoryLayout::ROW_MAJOR> matrixS(0, 0);
    Figaro::Matrix<double, Figaro::MemoryLayout::ROW_MAJOR> matrixVT(0, 0);

    matrix(0, 0) = 1; matrix(0, 1) = 2; matrix(0, 2) = 27;
    matrix(1, 0) = 3; matrix(1, 1) = 4; matrix(1, 2) = 12;
    matrix(2, 0) = 5; matrix(2, 1) = 6; matrix(2, 2) = 13;
    matrix(3, 0) = 7; matrix(3, 1) = 8; matrix(3, 2) = 14;
    matrix(4, 0) = 9; matrix(4, 1) = 23; matrix(4, 2) = 17;

    matrix.computeSVDEigenDec(1, Figaro::SVDHintType::EIGEN_DECOMP_QR_ITER,
        true, true, &matrixU, &matrixS, &matrixVT);
    EXPECT_NEAR(std::abs(matrixU(0, 0)), 0.532977758781636, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixU(0, 1)), 0.719504148787956, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixU(0, 2)), 0.382389696847683, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(std::abs(matrixU(1, 0)), 0.283948361179314, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixU(1, 1)), 0.134893419827008, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixU(1, 2)), 0.145821660497625, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(std::abs(matrixU(2, 0)), 0.334946954795978, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixU(2, 1)), 0.040622061532983, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixU(2, 2)), 0.422403208347773, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(std::abs(matrixU(3, 0)), 0.385945548412641, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixU(3, 1)), 0.053649296761042, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixU(3, 2)), 0.698984756197921, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(std::abs(matrixU(4, 0)), 0.611689960650861, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixU(4, 1)), 0.677930045239293, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixU(4, 2)), 0.406829206491980, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(matrixS(0, 0), 44.989193549900570, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrixS(1, 0), 17.417873990872451, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrixS(2, 0), 3.686479264511551, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(std::abs(matrixVT(0, 0)), 0.250424273299085, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixVT(1, 0)), 0.295651511272016, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixVT(2, 0)), 0.921888207552954, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(std::abs(matrixVT(0, 1)), 0.474955483467975, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixVT(1, 1)), 0.792247726429847, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixVT(2, 1)), 0.383093759660007, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(std::abs(matrixVT(0, 2)), 0.843626085458675, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixVT(1, 2)), 0.533791835690010, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixVT(2, 2)), 0.057976754689808, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(matrixU.getOrthogonality(0), 0, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrixVT.getOrthogonality(0), 0, GIVENS_TEST_PRECISION_ERROR);
}


TEST(Matrix, computeSVDEigDecQrIterColMajor)
{
   static constexpr uint32_t NUM_ROWS = 5, NUM_COLS = 3;
    Figaro::Matrix<double, Figaro::MemoryLayout::COL_MAJOR> matrix(NUM_ROWS, NUM_COLS);
    Figaro::Matrix<double, Figaro::MemoryLayout::COL_MAJOR> matrixU(0, 0);
    Figaro::Matrix<double, Figaro::MemoryLayout::COL_MAJOR> matrixS(0, 0);
    Figaro::Matrix<double, Figaro::MemoryLayout::COL_MAJOR> matrixVT(0, 0);

    matrix(0, 0) = 1; matrix(0, 1) = 2; matrix(0, 2) = 27;
    matrix(1, 0) = 3; matrix(1, 1) = 4; matrix(1, 2) = 12;
    matrix(2, 0) = 5; matrix(2, 1) = 6; matrix(2, 2) = 13;
    matrix(3, 0) = 7; matrix(3, 1) = 8; matrix(3, 2) = 14;
    matrix(4, 0) = 9; matrix(4, 1) = 23; matrix(4, 2) = 17;

    matrix.computeSVDEigenDec(1, Figaro::SVDHintType::EIGEN_DECOMP_QR_ITER,
        true, true, &matrixU, &matrixS, &matrixVT);
    EXPECT_NEAR(std::abs(matrixU(0, 0)), 0.532977758781636, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixU(0, 1)), 0.719504148787956, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixU(0, 2)), 0.382389696847683, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(std::abs(matrixU(1, 0)), 0.283948361179314, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixU(1, 1)), 0.134893419827008, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixU(1, 2)), 0.145821660497625, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(std::abs(matrixU(2, 0)), 0.334946954795978, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixU(2, 1)), 0.040622061532983, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixU(2, 2)), 0.422403208347773, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(std::abs(matrixU(3, 0)), 0.385945548412641, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixU(3, 1)), 0.053649296761042, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixU(3, 2)), 0.698984756197921, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(std::abs(matrixU(4, 0)), 0.611689960650861, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixU(4, 1)), 0.677930045239293, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixU(4, 2)), 0.406829206491980, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(matrixS(0, 0), 44.989193549900570, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrixS(1, 0), 17.417873990872451, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrixS(2, 0), 3.686479264511551, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(std::abs(matrixVT(0, 0)), 0.250424273299085, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixVT(1, 0)), 0.295651511272016, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixVT(2, 0)), 0.921888207552954, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(std::abs(matrixVT(0, 1)), 0.474955483467975, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixVT(1, 1)), 0.792247726429847, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixVT(2, 1)), 0.383093759660007, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(std::abs(matrixVT(0, 2)), 0.843626085458675, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixVT(1, 2)), 0.533791835690010, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixVT(2, 2)), 0.057976754689808, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(matrixU.getOrthogonality(0), 0, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrixVT.getOrthogonality(0), 0, GIVENS_TEST_PRECISION_ERROR);
}


TEST(Matrix, computeSVDEigDecQrIterSingValuesRowMajor)
{
    static constexpr uint32_t NUM_ROWS = 5, NUM_COLS = 3;
    Figaro::Matrix<double, Figaro::MemoryLayout::ROW_MAJOR> matrix(NUM_ROWS, NUM_COLS);
    Figaro::Matrix<double, Figaro::MemoryLayout::ROW_MAJOR> matrixU(0, 0);
    Figaro::Matrix<double, Figaro::MemoryLayout::ROW_MAJOR> matrixS(0, 0);
    Figaro::Matrix<double, Figaro::MemoryLayout::ROW_MAJOR> matrixVT(0, 0);

    matrix(0, 0) = 1; matrix(0, 1) = 2; matrix(0, 2) = 27;
    matrix(1, 0) = 3; matrix(1, 1) = 4; matrix(1, 2) = 12;
    matrix(2, 0) = 5; matrix(2, 1) = 6; matrix(2, 2) = 13;
    matrix(3, 0) = 7; matrix(3, 1) = 8; matrix(3, 2) = 14;
    matrix(4, 0) = 9; matrix(4, 1) = 23; matrix(4, 2) = 17;

    matrix.computeSVDEigenDec(1, Figaro::SVDHintType::EIGEN_DECOMP_QR_ITER,
        false, false, nullptr, &matrixS, nullptr);

    EXPECT_NEAR(matrixS(0, 0), 44.989193549900570, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrixS(1, 0), 17.417873990872451, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrixS(2, 0), 3.686479264511551, GIVENS_TEST_PRECISION_ERROR);
}


TEST(Matrix, computeSVDEigDecQrIterSingValuesColMajor)
{
   static constexpr uint32_t NUM_ROWS = 5, NUM_COLS = 3;
    Figaro::Matrix<double, Figaro::MemoryLayout::COL_MAJOR> matrix(NUM_ROWS, NUM_COLS);
    Figaro::Matrix<double, Figaro::MemoryLayout::COL_MAJOR> matrixU(0, 0);
    Figaro::Matrix<double, Figaro::MemoryLayout::COL_MAJOR> matrixS(0, 0);
    Figaro::Matrix<double, Figaro::MemoryLayout::COL_MAJOR> matrixVT(0, 0);

    matrix(0, 0) = 1; matrix(0, 1) = 2; matrix(0, 2) = 27;
    matrix(1, 0) = 3; matrix(1, 1) = 4; matrix(1, 2) = 12;
    matrix(2, 0) = 5; matrix(2, 1) = 6; matrix(2, 2) = 13;
    matrix(3, 0) = 7; matrix(3, 1) = 8; matrix(3, 2) = 14;
    matrix(4, 0) = 9; matrix(4, 1) = 23; matrix(4, 2) = 17;

    matrix.computeSVDEigenDec(1, Figaro::SVDHintType::EIGEN_DECOMP_QR_ITER,
        false, false, nullptr, &matrixS, nullptr);

    EXPECT_NEAR(matrixS(0, 0), 44.989193549900570, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrixS(1, 0), 17.417873990872451, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrixS(2, 0), 3.686479264511551, GIVENS_TEST_PRECISION_ERROR);
}


TEST(Matrix, computeSVDEigDecRRRRowMajor)
{
    static constexpr uint32_t NUM_ROWS = 5, NUM_COLS = 3;
    Figaro::Matrix<double, Figaro::MemoryLayout::ROW_MAJOR> matrix(NUM_ROWS, NUM_COLS);
    Figaro::Matrix<double, Figaro::MemoryLayout::ROW_MAJOR> matrixU(0, 0);
    Figaro::Matrix<double, Figaro::MemoryLayout::ROW_MAJOR> matrixS(0, 0);
    Figaro::Matrix<double, Figaro::MemoryLayout::ROW_MAJOR> matrixVT(0, 0);

    matrix(0, 0) = 1; matrix(0, 1) = 2; matrix(0, 2) = 27;
    matrix(1, 0) = 3; matrix(1, 1) = 4; matrix(1, 2) = 12;
    matrix(2, 0) = 5; matrix(2, 1) = 6; matrix(2, 2) = 13;
    matrix(3, 0) = 7; matrix(3, 1) = 8; matrix(3, 2) = 14;
    matrix(4, 0) = 9; matrix(4, 1) = 23; matrix(4, 2) = 17;

    matrix.computeSVDEigenDec(1, Figaro::SVDHintType::EIGEN_DECOMP_RRR,
        true, true, &matrixU, &matrixS, &matrixVT);
    EXPECT_NEAR(std::abs(matrixU(0, 0)), 0.532977758781636, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixU(0, 1)), 0.719504148787956, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixU(0, 2)), 0.382389696847683, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(std::abs(matrixU(1, 0)), 0.283948361179314, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixU(1, 1)), 0.134893419827008, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixU(1, 2)), 0.145821660497625, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(std::abs(matrixU(2, 0)), 0.334946954795978, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixU(2, 1)), 0.040622061532983, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixU(2, 2)), 0.422403208347773, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(std::abs(matrixU(3, 0)), 0.385945548412641, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixU(3, 1)), 0.053649296761042, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixU(3, 2)), 0.698984756197921, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(std::abs(matrixU(4, 0)), 0.611689960650861, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixU(4, 1)), 0.677930045239293, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixU(4, 2)), 0.406829206491980, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(matrixS(0, 0), 44.989193549900570, RELAX_GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrixS(1, 0), 17.417873990872451, RELAX_GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrixS(2, 0), 3.686479264511551, RELAX_GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(std::abs(matrixVT(0, 0)), 0.250424273299085, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixVT(1, 0)), 0.295651511272016, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixVT(2, 0)), 0.921888207552954, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(std::abs(matrixVT(0, 1)), 0.474955483467975, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixVT(1, 1)), 0.792247726429847, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixVT(2, 1)), 0.383093759660007, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(std::abs(matrixVT(0, 2)), 0.843626085458675, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixVT(1, 2)), 0.533791835690010, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixVT(2, 2)), 0.057976754689808, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(matrixU.getOrthogonality(0), 0, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrixVT.getOrthogonality(0), 0, GIVENS_TEST_PRECISION_ERROR);
}


TEST(Matrix, computeSVDEigDecRRRColMajor)
{
   static constexpr uint32_t NUM_ROWS = 5, NUM_COLS = 3;
    Figaro::Matrix<double, Figaro::MemoryLayout::COL_MAJOR> matrix(NUM_ROWS, NUM_COLS);
    Figaro::Matrix<double, Figaro::MemoryLayout::COL_MAJOR> matrixU(0, 0);
    Figaro::Matrix<double, Figaro::MemoryLayout::COL_MAJOR> matrixS(0, 0);
    Figaro::Matrix<double, Figaro::MemoryLayout::COL_MAJOR> matrixVT(0, 0);

    matrix(0, 0) = 1; matrix(0, 1) = 2; matrix(0, 2) = 27;
    matrix(1, 0) = 3; matrix(1, 1) = 4; matrix(1, 2) = 12;
    matrix(2, 0) = 5; matrix(2, 1) = 6; matrix(2, 2) = 13;
    matrix(3, 0) = 7; matrix(3, 1) = 8; matrix(3, 2) = 14;
    matrix(4, 0) = 9; matrix(4, 1) = 23; matrix(4, 2) = 17;

    matrix.computeSVDEigenDec(1, Figaro::SVDHintType::EIGEN_DECOMP_RRR,
        true, true, &matrixU, &matrixS, &matrixVT);
    EXPECT_NEAR(std::abs(matrixU(0, 0)), 0.532977758781636, RELAX_GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixU(0, 1)), 0.719504148787956, RELAX_GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixU(0, 2)), 0.382389696847683, RELAX_GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(std::abs(matrixU(1, 0)), 0.283948361179314, RELAX_GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixU(1, 1)), 0.134893419827008, RELAX_GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixU(1, 2)), 0.145821660497625, RELAX_GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(std::abs(matrixU(2, 0)), 0.334946954795978, RELAX_GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixU(2, 1)), 0.040622061532983, RELAX_GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixU(2, 2)), 0.422403208347773, RELAX_GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(std::abs(matrixU(3, 0)), 0.385945548412641, RELAX_GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixU(3, 1)), 0.053649296761042, RELAX_GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixU(3, 2)), 0.698984756197921, RELAX_GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(std::abs(matrixU(4, 0)), 0.611689960650861, RELAX_GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixU(4, 1)), 0.677930045239293, RELAX_GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixU(4, 2)), 0.406829206491980, RELAX_GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(matrixS(0, 0), 44.989193549900570, RELAX_GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrixS(1, 0), 17.417873990872451, RELAX_GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrixS(2, 0), 3.686479264511551, RELAX_GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(std::abs(matrixVT(0, 0)), 0.250424273299085, RELAX_GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixVT(1, 0)), 0.295651511272016, RELAX_GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixVT(2, 0)), 0.921888207552954, RELAX_GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(std::abs(matrixVT(0, 1)), 0.474955483467975, RELAX_GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixVT(1, 1)), 0.792247726429847, RELAX_GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixVT(2, 1)), 0.383093759660007, RELAX_GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(std::abs(matrixVT(0, 2)), 0.843626085458675, RELAX_GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixVT(1, 2)), 0.533791835690010, RELAX_GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixVT(2, 2)), 0.057976754689808, RELAX_GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(matrixU.getOrthogonality(0), 0, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrixVT.getOrthogonality(0), 0, GIVENS_TEST_PRECISION_ERROR);
}


TEST(Matrix, computeSVDEigDecRRRSingValuesRowMajor)
{
    static constexpr uint32_t NUM_ROWS = 5, NUM_COLS = 3;
    Figaro::Matrix<double, Figaro::MemoryLayout::ROW_MAJOR> matrix(NUM_ROWS, NUM_COLS);
    Figaro::Matrix<double, Figaro::MemoryLayout::ROW_MAJOR> matrixU(0, 0);
    Figaro::Matrix<double, Figaro::MemoryLayout::ROW_MAJOR> matrixS(0, 0);
    Figaro::Matrix<double, Figaro::MemoryLayout::ROW_MAJOR> matrixVT(0, 0);

    matrix(0, 0) = 1; matrix(0, 1) = 2; matrix(0, 2) = 27;
    matrix(1, 0) = 3; matrix(1, 1) = 4; matrix(1, 2) = 12;
    matrix(2, 0) = 5; matrix(2, 1) = 6; matrix(2, 2) = 13;
    matrix(3, 0) = 7; matrix(3, 1) = 8; matrix(3, 2) = 14;
    matrix(4, 0) = 9; matrix(4, 1) = 23; matrix(4, 2) = 17;

    matrix.computeSVDEigenDec(1, Figaro::SVDHintType::EIGEN_DECOMP_RRR,
        false, false, nullptr, &matrixS, nullptr);

    EXPECT_NEAR(matrixS(0, 0), 44.989193549900570, RELAX_GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrixS(1, 0), 17.417873990872451, RELAX_GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrixS(2, 0), 3.686479264511551, RELAX_GIVENS_TEST_PRECISION_ERROR);
}


TEST(Matrix, computeSVDEigDecRRRSingValuesColMajor)
{
   static constexpr uint32_t NUM_ROWS = 5, NUM_COLS = 3;
    Figaro::Matrix<double, Figaro::MemoryLayout::COL_MAJOR> matrix(NUM_ROWS, NUM_COLS);
    Figaro::Matrix<double, Figaro::MemoryLayout::COL_MAJOR> matrixU(0, 0);
    Figaro::Matrix<double, Figaro::MemoryLayout::COL_MAJOR> matrixS(0, 0);
    Figaro::Matrix<double, Figaro::MemoryLayout::COL_MAJOR> matrixVT(0, 0);

    matrix(0, 0) = 1; matrix(0, 1) = 2; matrix(0, 2) = 27;
    matrix(1, 0) = 3; matrix(1, 1) = 4; matrix(1, 2) = 12;
    matrix(2, 0) = 5; matrix(2, 1) = 6; matrix(2, 2) = 13;
    matrix(3, 0) = 7; matrix(3, 1) = 8; matrix(3, 2) = 14;
    matrix(4, 0) = 9; matrix(4, 1) = 23; matrix(4, 2) = 17;

    matrix.computeSVDEigenDec(1, Figaro::SVDHintType::EIGEN_DECOMP_RRR,
        false, false, nullptr, &matrixS, nullptr);

    EXPECT_NEAR(matrixS(0, 0), 44.989193549900570, RELAX_GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrixS(1, 0), 17.417873990872451, RELAX_GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrixS(2, 0), 3.686479264511551, RELAX_GIVENS_TEST_PRECISION_ERROR);
}


TEST(Matrix, DISABLED_computePCADivAndConqEigValsAndEigVectRowMajor)
{
    static constexpr uint32_t NUM_ROWS = 5, NUM_COLS = 3;
    static constexpr uint32_t NUM_ITERS = 10000;
    Figaro::Matrix<double, Figaro::MemoryLayout::ROW_MAJOR> matrix(NUM_ROWS, NUM_COLS);
    Figaro::Matrix<double, Figaro::MemoryLayout::ROW_MAJOR> matrixU(0, 0);
    Figaro::Matrix<double, Figaro::MemoryLayout::ROW_MAJOR> matrixS(0, 0);
    Figaro::Matrix<double, Figaro::MemoryLayout::ROW_MAJOR> matrixVT(0, 0);

    matrix(0, 0) = 1; matrix(0, 1) = 2; matrix(0, 2) = 27;
    matrix(1, 0) = 3; matrix(1, 1) = 4; matrix(1, 2) = 12;
    matrix(2, 0) = 5; matrix(2, 1) = 6; matrix(2, 2) = 13;
    matrix(3, 0) = 7; matrix(3, 1) = 8; matrix(3, 2) = 14;
    matrix(4, 0) = 9; matrix(4, 1) = 23; matrix(4, 2) = 17;

    matrix.computePCA(1, true,
        Figaro::PCAHintType::DIV_AND_CONQ, true, true, 1, NUM_ITERS, false,
        nullptr, &matrixS, &matrixVT);

    EXPECT_NEAR(matrixS(0, 0), 44.989193549900570, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrixS(1, 0), 17.417873990872451, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrixS(2, 0), 3.686479264511551, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(std::abs(matrixVT(0, 0)), 0.250424273299085, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixVT(1, 0)), 0.295651511272016, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixVT(2, 0)), 0.921888207552954, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(std::abs(matrixVT(0, 1)), 0.474955483467975, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixVT(1, 1)), 0.792247726429847, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixVT(2, 1)),  0.383093759660007, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(std::abs(matrixVT(0, 2)), 0.843626085458675, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixVT(1, 2)), 0.533791835690010, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixVT(2, 2)), 0.057976754689808, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(matrixVT.getOrthogonality(0), 0, GIVENS_TEST_PRECISION_ERROR);
}

TEST(Matrix, computePCADivAndConqEigValsAndEigVectColMajor)
{
    static constexpr uint32_t NUM_ROWS = 5, NUM_COLS = 3;
    static constexpr uint32_t NUM_ITERS = 10000;
    Figaro::Matrix<double, Figaro::MemoryLayout::COL_MAJOR> matrix(NUM_ROWS, NUM_COLS);
    Figaro::Matrix<double, Figaro::MemoryLayout::COL_MAJOR> matrixU(0, 0);
    Figaro::Matrix<double, Figaro::MemoryLayout::COL_MAJOR> matrixS(0, 0);
    Figaro::Matrix<double, Figaro::MemoryLayout::COL_MAJOR> matrixVT(0, 0);

    matrix(0, 0) = 1; matrix(0, 1) = 2; matrix(0, 2) = 27;
    matrix(1, 0) = 3; matrix(1, 1) = 4; matrix(1, 2) = 12;
    matrix(2, 0) = 5; matrix(2, 1) = 6; matrix(2, 2) = 13;
    matrix(3, 0) = 7; matrix(3, 1) = 8; matrix(3, 2) = 14;
    matrix(4, 0) = 9; matrix(4, 1) = 23; matrix(4, 2) = 17;

     matrix.computePCA(1, true,
        Figaro::PCAHintType::DIV_AND_CONQ, true, true, 1, NUM_ITERS, false,
        nullptr, &matrixS, &matrixVT);

    EXPECT_NEAR(matrixS(0, 0), 44.989193549900570, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrixS(1, 0), 17.417873990872451, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrixS(2, 0), 3.686479264511551, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(std::abs(matrixVT(0, 0)), 0.250424273299085, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixVT(1, 0)), 0.295651511272016, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixVT(2, 0)), 0.921888207552954, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(std::abs(matrixVT(0, 1)), 0.474955483467975, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixVT(1, 1)), 0.792247726429847, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixVT(2, 1)),  0.383093759660007, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(std::abs(matrixVT(0, 2)), 0.843626085458675, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixVT(1, 2)), 0.533791835690010, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixVT(2, 2)), 0.057976754689808, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(matrixVT.getOrthogonality(0), 0, GIVENS_TEST_PRECISION_ERROR);
}


TEST(Matrix, computePCADivAndConqRowMajor)
{
    static constexpr uint32_t NUM_ROWS = 5, NUM_COLS = 3;
    static constexpr uint32_t NUM_ITERS = 10000;
    Figaro::Matrix<double, Figaro::MemoryLayout::ROW_MAJOR> matrix(NUM_ROWS, NUM_COLS);
    Figaro::Matrix<double, Figaro::MemoryLayout::ROW_MAJOR> matrixU(0, 0);
    Figaro::Matrix<double, Figaro::MemoryLayout::ROW_MAJOR> matrixS(0, 0);
    Figaro::Matrix<double, Figaro::MemoryLayout::ROW_MAJOR> matrixVT(0, 0);

    matrix(0, 0) = 1; matrix(0, 1) = 2; matrix(0, 2) = 27;
    matrix(1, 0) = 3; matrix(1, 1) = 4; matrix(1, 2) = 12;
    matrix(2, 0) = 5; matrix(2, 1) = 6; matrix(2, 2) = 13;
    matrix(3, 0) = 7; matrix(3, 1) = 8; matrix(3, 2) = 14;
    matrix(4, 0) = 9; matrix(4, 1) = 23; matrix(4, 2) = 17;

    matrix.computePCA(1, true,
        Figaro::PCAHintType::DIV_AND_CONQ, true, true, 2, NUM_ITERS, false,
        &matrixU, &matrixS, &matrixVT);

    EXPECT_EQ(matrixU.getNumRows(), 5);
    EXPECT_EQ(matrixU.getNumCols(), 2);

    FIGARO_LOG_DBG(matrixU)

    EXPECT_NEAR(std::abs(matrixU(0, 0)),  23.978239547619253, RELAX_GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixU(0, 1)), 12.532232599498553, RELAX_GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(std::abs(matrixU(1, 0)),  12.774607779273254, RELAX_GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixU(1, 1)), 2.349556588744679, RELAX_GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(std::abs(matrixU(2, 0)), 15.068993378266047,  RELAX_GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixU(2, 1)), 0.707549949030961, RELAX_GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(std::abs(matrixU(3, 0)), 17.363378977258840, RELAX_GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixU(3, 1)), 0.934456690682756, RELAX_GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(std::abs(matrixU(4, 0)), 27.519438032252665, RELAX_GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixU(4, 1)), 11.808100102604470, RELAX_GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(matrixS(0, 0), 44.989193549900570, RELAX_GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrixS(1, 0), 17.417873990872451, RELAX_GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrixS(2, 0), 3.686479264511551, RELAX_GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(std::abs(matrixVT(0, 0)), 0.250424273299085, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixVT(1, 0)), 0.295651511272016, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixVT(2, 0)), 0.921888207552954, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(std::abs(matrixVT(0, 1)), 0.474955483467975, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixVT(1, 1)), 0.792247726429847, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixVT(2, 1)),  0.383093759660007, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(std::abs(matrixVT(0, 2)), 0.843626085458675, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixVT(1, 2)), 0.533791835690010, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixVT(2, 2)), 0.057976754689808, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(matrixVT.getOrthogonality(0), 0, GIVENS_TEST_PRECISION_ERROR);
}


TEST(Matrix, computePCADivAndConqColMajor)
{
    static constexpr uint32_t NUM_ROWS = 5, NUM_COLS = 3;
    static constexpr uint32_t NUM_ITERS = 10000;
    Figaro::Matrix<double, Figaro::MemoryLayout::COL_MAJOR> matrix(NUM_ROWS, NUM_COLS);
    Figaro::Matrix<double, Figaro::MemoryLayout::COL_MAJOR> matrixU(0, 0);
    Figaro::Matrix<double, Figaro::MemoryLayout::COL_MAJOR> matrixS(0, 0);
    Figaro::Matrix<double, Figaro::MemoryLayout::COL_MAJOR> matrixVT(0, 0);

    matrix(0, 0) = 1; matrix(0, 1) = 2; matrix(0, 2) = 27;
    matrix(1, 0) = 3; matrix(1, 1) = 4; matrix(1, 2) = 12;
    matrix(2, 0) = 5; matrix(2, 1) = 6; matrix(2, 2) = 13;
    matrix(3, 0) = 7; matrix(3, 1) = 8; matrix(3, 2) = 14;
    matrix(4, 0) = 9; matrix(4, 1) = 23; matrix(4, 2) = 17;

     matrix.computePCA(1, true,
        Figaro::PCAHintType::DIV_AND_CONQ, true, true, 2, NUM_ITERS, false,
        &matrixU, &matrixS, &matrixVT);

    EXPECT_EQ(matrixU.getNumRows(), 5);
    EXPECT_EQ(matrixU.getNumCols(), 2);

    FIGARO_LOG_DBG(matrixU)

    EXPECT_NEAR(std::abs(matrixU(0, 0)),  23.978239547619253, RELAX_GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixU(0, 1)), 12.532232599498553, RELAX_GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(std::abs(matrixU(1, 0)),  12.774607779273254, RELAX_GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixU(1, 1)), 2.349556588744679, RELAX_GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(std::abs(matrixU(2, 0)), 15.068993378266047,  RELAX_GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixU(2, 1)), 0.707549949030961, RELAX_GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(std::abs(matrixU(3, 0)), 17.363378977258840, RELAX_GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixU(3, 1)), 0.934456690682756, RELAX_GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(std::abs(matrixU(4, 0)), 27.519438032252665, RELAX_GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixU(4, 1)), 11.808100102604470, RELAX_GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(matrixS(0, 0), 44.989193549900570, RELAX_GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrixS(1, 0), 17.417873990872451, RELAX_GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrixS(2, 0), 3.686479264511551, RELAX_GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(std::abs(matrixVT(0, 0)), 0.250424273299085, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixVT(1, 0)), 0.295651511272016, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixVT(2, 0)), 0.921888207552954, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(std::abs(matrixVT(0, 1)), 0.474955483467975, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixVT(1, 1)), 0.792247726429847, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixVT(2, 1)),  0.383093759660007, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(std::abs(matrixVT(0, 2)), 0.843626085458675, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixVT(1, 2)), 0.533791835690010, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixVT(2, 2)), 0.057976754689808, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(matrixVT.getOrthogonality(0), 0, GIVENS_TEST_PRECISION_ERROR);
}



TEST(Matrix, computePCAQRIterEigValsAndEigVectRowMajor)
{
    static constexpr uint32_t NUM_ROWS = 5, NUM_COLS = 3;
    static constexpr uint32_t NUM_ITERS = 10000;
    Figaro::Matrix<double, Figaro::MemoryLayout::ROW_MAJOR> matrix(NUM_ROWS, NUM_COLS);
    Figaro::Matrix<double, Figaro::MemoryLayout::ROW_MAJOR> matrixU(0, 0);
    Figaro::Matrix<double, Figaro::MemoryLayout::ROW_MAJOR> matrixS(0, 0);
    Figaro::Matrix<double, Figaro::MemoryLayout::ROW_MAJOR> matrixVT(0, 0);

    matrix(0, 0) = 1; matrix(0, 1) = 2; matrix(0, 2) = 27;
    matrix(1, 0) = 3; matrix(1, 1) = 4; matrix(1, 2) = 12;
    matrix(2, 0) = 5; matrix(2, 1) = 6; matrix(2, 2) = 13;
    matrix(3, 0) = 7; matrix(3, 1) = 8; matrix(3, 2) = 14;
    matrix(4, 0) = 9; matrix(4, 1) = 23; matrix(4, 2) = 17;

    matrix.computePCA(1, true,
        Figaro::PCAHintType::DIV_AND_CONQ, true, true, 1, NUM_ITERS, false,
        nullptr, &matrixS, &matrixVT);

    EXPECT_NEAR(matrixS(0, 0), 44.989193549900570, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrixS(1, 0), 17.417873990872451, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrixS(2, 0), 3.686479264511551, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(std::abs(matrixVT(0, 0)), 0.250424273299085, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixVT(1, 0)), 0.295651511272016, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixVT(2, 0)), 0.921888207552954, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(std::abs(matrixVT(0, 1)), 0.474955483467975, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixVT(1, 1)), 0.792247726429847, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixVT(2, 1)),  0.383093759660007, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(std::abs(matrixVT(0, 2)), 0.843626085458675, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixVT(1, 2)), 0.533791835690010, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixVT(2, 2)), 0.057976754689808, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(matrixVT.getOrthogonality(0), 0, GIVENS_TEST_PRECISION_ERROR);
}

TEST(Matrix, computePCAQRIterEigValsAndEigVectColMajor)
{
    static constexpr uint32_t NUM_ROWS = 5, NUM_COLS = 3;
    static constexpr uint32_t NUM_ITERS = 10000;
    Figaro::Matrix<double, Figaro::MemoryLayout::COL_MAJOR> matrix(NUM_ROWS, NUM_COLS);
    Figaro::Matrix<double, Figaro::MemoryLayout::COL_MAJOR> matrixU(0, 0);
    Figaro::Matrix<double, Figaro::MemoryLayout::COL_MAJOR> matrixS(0, 0);
    Figaro::Matrix<double, Figaro::MemoryLayout::COL_MAJOR> matrixVT(0, 0);

    matrix(0, 0) = 1; matrix(0, 1) = 2; matrix(0, 2) = 27;
    matrix(1, 0) = 3; matrix(1, 1) = 4; matrix(1, 2) = 12;
    matrix(2, 0) = 5; matrix(2, 1) = 6; matrix(2, 2) = 13;
    matrix(3, 0) = 7; matrix(3, 1) = 8; matrix(3, 2) = 14;
    matrix(4, 0) = 9; matrix(4, 1) = 23; matrix(4, 2) = 17;

     matrix.computePCA(1, true,
        Figaro::PCAHintType::QR_ITER, true, true, 1, NUM_ITERS, false,
        nullptr, &matrixS, &matrixVT);

    EXPECT_NEAR(matrixS(0, 0), 44.989193549900570, RELAX_GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrixS(1, 0), 17.417873990872451, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrixS(2, 0), 3.686479264511551, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(std::abs(matrixVT(0, 0)), 0.250424273299085, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixVT(1, 0)), 0.295651511272016, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixVT(2, 0)), 0.921888207552954, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(std::abs(matrixVT(0, 1)), 0.474955483467975, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixVT(1, 1)), 0.792247726429847, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixVT(2, 1)),  0.383093759660007, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(std::abs(matrixVT(0, 2)), 0.843626085458675, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixVT(1, 2)), 0.533791835690010, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixVT(2, 2)), 0.057976754689808, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(matrixVT.getOrthogonality(0), 0, GIVENS_TEST_PRECISION_ERROR);
}


TEST(Matrix, computePCAQRIterRowMajor)
{
    static constexpr uint32_t NUM_ROWS = 5, NUM_COLS = 3;
    static constexpr uint32_t NUM_ITERS = 10000;
    Figaro::Matrix<double, Figaro::MemoryLayout::ROW_MAJOR> matrix(NUM_ROWS, NUM_COLS);
    Figaro::Matrix<double, Figaro::MemoryLayout::ROW_MAJOR> matrixU(0, 0);
    Figaro::Matrix<double, Figaro::MemoryLayout::ROW_MAJOR> matrixS(0, 0);
    Figaro::Matrix<double, Figaro::MemoryLayout::ROW_MAJOR> matrixVT(0, 0);

    matrix(0, 0) = 1; matrix(0, 1) = 2; matrix(0, 2) = 27;
    matrix(1, 0) = 3; matrix(1, 1) = 4; matrix(1, 2) = 12;
    matrix(2, 0) = 5; matrix(2, 1) = 6; matrix(2, 2) = 13;
    matrix(3, 0) = 7; matrix(3, 1) = 8; matrix(3, 2) = 14;
    matrix(4, 0) = 9; matrix(4, 1) = 23; matrix(4, 2) = 17;

    matrix.computePCA(1, true,
        Figaro::PCAHintType::QR_ITER, true, true, 2, NUM_ITERS, false,
        &matrixU, &matrixS, &matrixVT);

    EXPECT_EQ(matrixU.getNumRows(), 5);
    EXPECT_EQ(matrixU.getNumCols(), 2);

    FIGARO_LOG_DBG(matrixU)

    EXPECT_NEAR(std::abs(matrixU(0, 0)),  23.978239547619253, RELAX_GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixU(0, 1)), 12.532232599498553, RELAX_GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(std::abs(matrixU(1, 0)),  12.774607779273254, RELAX_GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixU(1, 1)), 2.349556588744679, RELAX_GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(std::abs(matrixU(2, 0)), 15.068993378266047,  RELAX_GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixU(2, 1)), 0.707549949030961, RELAX_GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(std::abs(matrixU(3, 0)), 17.363378977258840, RELAX_GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixU(3, 1)), 0.934456690682756, RELAX_GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(std::abs(matrixU(4, 0)), 27.519438032252665, RELAX_GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixU(4, 1)), 11.808100102604470, RELAX_GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(matrixS(0, 0), 44.989193549900570, RELAX_GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrixS(1, 0), 17.417873990872451, RELAX_GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrixS(2, 0), 3.686479264511551, RELAX_GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(std::abs(matrixVT(0, 0)), 0.250424273299085, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixVT(1, 0)), 0.295651511272016, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixVT(2, 0)), 0.921888207552954, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(std::abs(matrixVT(0, 1)), 0.474955483467975, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixVT(1, 1)), 0.792247726429847, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixVT(2, 1)),  0.383093759660007, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(std::abs(matrixVT(0, 2)), 0.843626085458675, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixVT(1, 2)), 0.533791835690010, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixVT(2, 2)), 0.057976754689808, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(matrixVT.getOrthogonality(0), 0, GIVENS_TEST_PRECISION_ERROR);
}


TEST(Matrix, computePCAQRIterColMajor)
{
    static constexpr uint32_t NUM_ROWS = 5, NUM_COLS = 3;
    static constexpr uint32_t NUM_ITERS = 10000;
    Figaro::Matrix<double, Figaro::MemoryLayout::COL_MAJOR> matrix(NUM_ROWS, NUM_COLS);
    Figaro::Matrix<double, Figaro::MemoryLayout::COL_MAJOR> matrixU(0, 0);
    Figaro::Matrix<double, Figaro::MemoryLayout::COL_MAJOR> matrixS(0, 0);
    Figaro::Matrix<double, Figaro::MemoryLayout::COL_MAJOR> matrixVT(0, 0);

    matrix(0, 0) = 1; matrix(0, 1) = 2; matrix(0, 2) = 27;
    matrix(1, 0) = 3; matrix(1, 1) = 4; matrix(1, 2) = 12;
    matrix(2, 0) = 5; matrix(2, 1) = 6; matrix(2, 2) = 13;
    matrix(3, 0) = 7; matrix(3, 1) = 8; matrix(3, 2) = 14;
    matrix(4, 0) = 9; matrix(4, 1) = 23; matrix(4, 2) = 17;

     matrix.computePCA(1, true,
        Figaro::PCAHintType::QR_ITER, true, true, 2, NUM_ITERS, false,
        &matrixU, &matrixS, &matrixVT);

    EXPECT_EQ(matrixU.getNumRows(), 5);
    EXPECT_EQ(matrixU.getNumCols(), 2);

    FIGARO_LOG_DBG(matrixU)

    EXPECT_NEAR(std::abs(matrixU(0, 0)),  23.978239547619253, RELAX_GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixU(0, 1)), 12.532232599498553, RELAX_GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(std::abs(matrixU(1, 0)),  12.774607779273254, RELAX_GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixU(1, 1)), 2.349556588744679, RELAX_GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(std::abs(matrixU(2, 0)), 15.068993378266047,  RELAX_GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixU(2, 1)), 0.707549949030961, RELAX_GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(std::abs(matrixU(3, 0)), 17.363378977258840, RELAX_GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixU(3, 1)), 0.934456690682756, RELAX_GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(std::abs(matrixU(4, 0)), 27.519438032252665, RELAX_GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixU(4, 1)), 11.808100102604470, RELAX_GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(matrixS(0, 0), 44.989193549900570, RELAX_GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrixS(1, 0), 17.417873990872451, RELAX_GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrixS(2, 0), 3.686479264511551, RELAX_GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(std::abs(matrixVT(0, 0)), 0.250424273299085, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixVT(1, 0)), 0.295651511272016, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixVT(2, 0)), 0.921888207552954, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(std::abs(matrixVT(0, 1)), 0.474955483467975, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixVT(1, 1)), 0.792247726429847, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixVT(2, 1)),  0.383093759660007, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(std::abs(matrixVT(0, 2)), 0.843626085458675, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixVT(1, 2)), 0.533791835690010, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(std::abs(matrixVT(2, 2)), 0.057976754689808, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(matrixVT.getOrthogonality(0), 0, GIVENS_TEST_PRECISION_ERROR);
}


TEST(Matrix, computeLULapackRowMajor)
{
    static constexpr uint32_t NUM_ROWS = 3, NUM_COLS = 2;
    Figaro::Matrix<double, Figaro::MemoryLayout::ROW_MAJOR> matrix(NUM_ROWS, NUM_COLS);
    Figaro::Matrix<double, Figaro::MemoryLayout::ROW_MAJOR> matrixL(0, 0);
    Figaro::Matrix<double, Figaro::MemoryLayout::ROW_MAJOR> matrixU(0, 0);
    Figaro::Matrix<uint32_t, Figaro::MemoryLayout::ROW_MAJOR> matrixP(0, 0);

    matrix(0, 0) = 1; matrix(0, 1) = 2;
    matrix(1, 0) = 3; matrix(1, 1) = 4;
    matrix(2, 0) = 5; matrix(2, 1) = 6;
    matrix.computeLU(1, LUHintType::PART_PIVOT_LAPACK, true, true,
            &matrixL, &matrixU, &matrixP, true);

    EXPECT_NEAR(matrixL(0, 0), 1.0, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrixL(1, 0), 0.2, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrixL(2, 0), 0.6, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrixL(0, 1), 0, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrixL(1, 1), 1, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrixL(2, 1), 0.5, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(matrixU(0, 0), 5.0, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrixU(0, 1), 6.0, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrixU(1, 0), 0, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrixU(1, 1), 0.8, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_EQ(matrixP(0, 0), 2);
    EXPECT_EQ(matrixP(1, 0), 0);
    EXPECT_EQ(matrixP(2, 0), 1);


    FIGARO_LOG_DBG("matrixL", matrixL)
    FIGARO_LOG_DBG("matrixU", matrixU)
    FIGARO_LOG_DBG("matrixP", matrixP)
}


TEST(Matrix, computeLULapackColMajor)
{
    static constexpr uint32_t NUM_ROWS = 3, NUM_COLS = 2;
    Figaro::Matrix<double, Figaro::MemoryLayout::COL_MAJOR> matrix(NUM_ROWS, NUM_COLS);
    Figaro::Matrix<double, Figaro::MemoryLayout::COL_MAJOR> matrixL(0, 0);
    Figaro::Matrix<double, Figaro::MemoryLayout::COL_MAJOR> matrixU(0, 0);
    Figaro::Matrix<uint32_t, Figaro::MemoryLayout::COL_MAJOR> matrixP(0, 0);

    matrix(0, 0) = 1; matrix(0, 1) = 2;
    matrix(1, 0) = 3; matrix(1, 1) = 4;
    matrix(2, 0) = 5; matrix(2, 1) = 6;
    matrix.computeLU(1, LUHintType::PART_PIVOT_LAPACK, true, true,
            &matrixL, &matrixU, &matrixP, true);

    EXPECT_NEAR(matrixL(0, 0), 1.0, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrixL(1, 0), 0.2, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrixL(2, 0), 0.6, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrixL(0, 1), 0, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrixL(1, 1), 1, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrixL(2, 1), 0.5, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(matrixU(0, 0), 5.0, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrixU(0, 1), 6.0, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrixU(1, 0), 0, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrixU(1, 1), 0.8, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_EQ(matrixP(0, 0), 2);
    EXPECT_EQ(matrixP(1, 0), 0);
    EXPECT_EQ(matrixP(2, 0), 1);


    FIGARO_LOG_DBG("matrixL", matrixL)
    FIGARO_LOG_DBG("matrixU", matrixU)
    FIGARO_LOG_DBG("matrixP", matrixP)
}

TEST(Matrix, computeLUThinDiagRowMajor)
{
    static constexpr uint32_t NUM_ROWS = 3, NUM_COLS = 2;
    Figaro::Matrix<double, Figaro::MemoryLayout::ROW_MAJOR> matrix(NUM_ROWS, NUM_COLS);
    Figaro::Matrix<double, Figaro::MemoryLayout::ROW_MAJOR> matrixL(0, 0);
    Figaro::Matrix<double, Figaro::MemoryLayout::ROW_MAJOR> matrixU(0, 0);

    matrix(0, 0) = 0; matrix(0, 1) = 2;
    matrix(1, 0) = 3; matrix(1, 1) = 4;
    matrix(2, 0) = 5; matrix(2, 1) = 6;
    matrix.computeLU(1, Figaro::LUHintType::THIN_DIAG, false, true, &matrixL, &matrixU,
        nullptr, true);
/*
    EXPECT_NEAR(matrixL(0, 0), 1.0, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrixL(1, 0), 0.2, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrixL(2, 0), 0.6, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrixL(0, 1), 0, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrixL(1, 1), 1, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrixL(2, 1), 0.5, GIVENS_TEST_PRECISION_ERROR);
*/
    EXPECT_NEAR(matrixU(0, 0), 3, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrixU(0, 1), 4, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrixU(1, 0), 0, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrixU(1, 1), 2, GIVENS_TEST_PRECISION_ERROR);

 //   FIGARO_LOG_DBG("matrixL", matrixL)
    FIGARO_LOG_DBG("matrixU", matrixU)
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



TEST(Matrix, MultiplicationMulRowMajor)
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

    Figaro::Matrix<double, Figaro::MemoryLayout::ROW_MAJOR> C = A.multiply(B, 0, 0, 0, false);
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


TEST(Matrix, MultiplicationMulColMajor)
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


    Figaro::Matrix<double, Figaro::MemoryLayout::COL_MAJOR> C = A.multiply(B, 0, 0, 0, false);
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

TEST(Matrix, MultiplicationTranRowMajor)
{
    static constexpr uint32_t M = 3, N = 2, K = 3;
    Figaro::Matrix<double, Figaro::MemoryLayout::ROW_MAJOR> A(M, K), B(M, N), expC(K, N);

    EXPECT_EQ(A.getNumRows(), M);
    EXPECT_EQ(A.getNumCols(), K);

    EXPECT_EQ(B.getNumRows(), M);
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

    expC(0, 0) = 30;
    expC(0, 1) = 39;

    expC(1, 0) = 36;
    expC(1, 1) = 48;

    expC(2, 0) = 42;
    expC(2, 1) = 57;

    Figaro::Matrix<double, Figaro::MemoryLayout::ROW_MAJOR> C = A.multiply(B, 0, 0, 0, true);
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

TEST(Matrix, MultiplicationTranColMajor)
{
    static constexpr uint32_t M = 4, N = 2, K = 3;
    Figaro::Matrix<double, Figaro::MemoryLayout::ROW_MAJOR> A(M, K), B(M, N), expC(K, N);

    EXPECT_EQ(A.getNumRows(), M);
    EXPECT_EQ(A.getNumCols(), K);

    EXPECT_EQ(B.getNumRows(), M);
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

    A(3, 0) = 9;
    A(3, 1) = 10;
    A(3, 2) = 11;

    B(0, 0) = 0;
    B(0, 1) = 1;

    B(1, 0) = 2;
    B(1, 1) = 3;

    B(2, 0) = 4;
    B(2, 1) = 5;

    B(3, 0) = 6;
    B(3, 1) = 7;

    expC(0, 0) = 84;
    expC(0, 1) = 102;

    expC(1, 0) = 96;
    expC(1, 1) = 118;

    expC(2, 0) = 108;
    expC(2, 1) = 134;

    Figaro::Matrix<double, Figaro::MemoryLayout::ROW_MAJOR> C = A.multiply(B, 0, 0, 0, true);
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


TEST(MatrixSparse, MultiplicationRowMajor)
{
    static constexpr uint32_t M = 3, N = 2, K = 3;
    std::vector<double> vVals = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    std::vector<int64_t> rowStartPos = {0, 3, 6, 9};
    std::vector<int64_t> vColIndcs = {0, 1, 2, 0, 1, 2, 0, 1, 2};

    Figaro::MatrixSparse<double> matrix{M, K, rowStartPos.data(),  vColIndcs.data(), vVals.data()};
    long long int numRows;
    long long int numCols;
    long long int* pRowStartPos;
    long long int* pColInds;
    double* pVals;

    Figaro::Matrix<double, Figaro::MemoryLayout::ROW_MAJOR>  A(M, K), B(K, N), expC(M, N);

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

    Figaro::Matrix<double, Figaro::MemoryLayout::ROW_MAJOR> C = matrix.multiply(A, B, 0, 0, 0);
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

TEST(MatrixSparse, MultiplicationRowMajorShift)
{
    static constexpr uint32_t M = 3, N = 2, K = 3;
    std::vector<double> vVals = {0, 1, 3, 4,  6, 7};
    std::vector<int64_t> rowStartPos = {0, 2, 4, 6};
    std::vector<int64_t> vColIndcs = {0, 1, 0, 1, 0, 1};

    Figaro::MatrixSparse<double> matrix{M, K, rowStartPos.data(),  vColIndcs.data(), vVals.data()};
    long long int numRows;
    long long int numCols;
    long long int* pRowStartPos;
    long long int* pColInds;
    double* pVals;

    Figaro::Matrix<double, Figaro::MemoryLayout::ROW_MAJOR>  A(M, 2), B(K, N), expC(M, N);

    EXPECT_EQ(A.getNumRows(), M);
    EXPECT_EQ(A.getNumCols(), 2);

    EXPECT_EQ(B.getNumRows(), K);
    EXPECT_EQ(B.getNumCols(), N);
    A(0, 0) = 0;
    A(0, 1) = 1;

    A(1, 0) = 3;
    A(1, 1) = 4;

    A(2, 0) = 6;
    A(2, 1) = 7;

    B(0, 0) = 0;
    B(0, 1) = 1;

    B(1, 0) = 2;
    B(1, 1) = 3;

    B(2, 0) = 4;
    B(2, 1) = 5;

    expC(0, 0) = 4;
    expC(0, 1) = 5;

    expC(1, 0) = 22;
    expC(1, 1) = 29;

    expC(2, 0) = 40;
    expC(2, 1) = 53;

    Figaro::Matrix<double, Figaro::MemoryLayout::ROW_MAJOR> C = matrix.multiply(A, B, 0, 0, 1);
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


TEST(MatrixSparse, MultiplicationRowMajorShiftJoinAttrs)
{
    static constexpr uint32_t M = 3, N = 2, K = 3;
    std::vector<double> vVals = {0, 1, 2, 3, 4, 5, 6, 7, 8};
    std::vector<int64_t> rowStartPos = {0, 3, 6, 9};
    std::vector<int64_t> vColIndcs = {0, 1, 2, 0, 1, 2, 0, 1, 2};

    Figaro::MatrixSparse<double> matrix{M, K, rowStartPos.data(),  vColIndcs.data(), vVals.data()};
    long long int numRows;
    long long int numCols;
    long long int* pRowStartPos;
    long long int* pColInds;
    double* pVals;

    Figaro::Matrix<double, Figaro::MemoryLayout::ROW_MAJOR>  A(M, K), B(K, N), expC(M, N + 1);

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
/* 0 1
3 4
6 7

0 1
2 3
4 5
*/
    B(0, 0) = 0;
    B(0, 1) = 1;

    B(1, 0) = 2;
    B(1, 1) = 3;

    B(2, 0) = 4;
    B(2, 1) = 5;

    expC(0, 0) = 0;
    expC(0, 1) = 10;
    expC(0, 2) = 13;

    expC(1, 0) = 3;
    expC(1, 1) = 28;
    expC(1, 2) = 37;

    expC(2, 0) = 6;
    expC(2, 1) = 46;
    expC(2, 2) = 61;

    Figaro::Matrix<double, Figaro::MemoryLayout::ROW_MAJOR> C = matrix.multiply(A, B, 1, 0, 1);
    EXPECT_EQ(C.getNumRows(), expC.getNumRows());
    EXPECT_EQ(C.getNumCols(), expC.getNumCols());

    FIGARO_LOG_DBG("MatC", C)
    FIGARO_LOG_DBG("expC", expC)

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


TEST(Matrix, SelfMatrixMultiplyRowMajor2)
{
    static constexpr uint32_t M = 3, N = 2;
    Figaro::Matrix<double, Figaro::MemoryLayout::ROW_MAJOR> A(M, N), expB(N, N);

    A(0, 0) = 1;
    A(0, 1) = 2;

    A(1, 0) = 1;
    A(1, 1) = 2;

    A(2, 0) = 1;
    A(2, 1) = 2;


    expB(0, 0) = 3;
    expB(0, 1) = 6;

    expB(1, 0) = 6;
    expB(1, 1) = 12;

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


TEST(Matrix, SelfMatrixMultiplyColMajor2)
{
    static constexpr uint32_t M = 3, N = 2;
    Figaro::Matrix<double, Figaro::MemoryLayout::COL_MAJOR> A(M, N), expB(N, N);

    A(0, 0) = 1;
    A(0, 1) = 2;

    A(1, 0) = 1;
    A(1, 1) = 2;

    A(2, 0) = 1;
    A(2, 1) = 2;


    expB(0, 0) = 3;
    expB(0, 1) = 6;

    expB(1, 0) = 6;
    expB(1, 1) = 12;

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


TEST(Matrix, MultiplicationRectDiagRowMajor)
{
    static constexpr uint32_t M = 3, N = 2;
    Figaro::Matrix<double, Figaro::MemoryLayout::ROW_MAJOR> A(M, N), B(N, 1), expC(M, N);

    EXPECT_EQ(A.getNumRows(), M);
    EXPECT_EQ(A.getNumCols(), N);

    EXPECT_EQ(B.getNumRows(), N);
    EXPECT_EQ(B.getNumCols(), 1);


    A(0, 0) = 0;
    A(0, 1) = 1;

    A(1, 0) = 3;
    A(1, 1) = 4;

    A(2, 0) = 6;
    A(2, 1) = 7;

    B(0, 0) = 2;
    B(1, 0) = 3;

    expC(0, 0) = 0;
    expC(1, 0) = 6;
    expC(2, 0) = 12;

    expC(0, 1) = 3;
    expC(1, 1) = 12;
    expC(2, 1) = 21;

    Figaro::Matrix<double, Figaro::MemoryLayout::ROW_MAJOR> C =
        A.computeMatrixProductRecDiag(B);
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


TEST(Matrix, MultiplicationRectDiagColMajor)
{
    static constexpr uint32_t M = 3, N = 2;
    Figaro::Matrix<double, Figaro::MemoryLayout::COL_MAJOR> A(M, N), B(N, 1), expC(M, N);

    EXPECT_EQ(A.getNumRows(), M);
    EXPECT_EQ(A.getNumCols(), N);

    EXPECT_EQ(B.getNumRows(), N);
    EXPECT_EQ(B.getNumCols(), 1);


    A(0, 0) = 0;
    A(0, 1) = 1;

    A(1, 0) = 3;
    A(1, 1) = 4;

    A(2, 0) = 6;
    A(2, 1) = 7;

    B(0, 0) = 2;
    B(1, 0) = 3;

    expC(0, 0) = 0;
    expC(1, 0) = 6;
    expC(2, 0) = 12;

    expC(0, 1) = 3;
    expC(1, 1) = 12;
    expC(2, 1) = 21;

    Figaro::Matrix<double, Figaro::MemoryLayout::COL_MAJOR> C =
        A.computeMatrixProductRecDiag(B);
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