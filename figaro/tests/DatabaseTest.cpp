#include "UtilTest.h"
#include "database/storage/ArrayStorage.h"
#include "database/storage/Matrix.h"
#include "database/Database.h"
#include "database/query/Query.h"
#include <vector>
#include <string>

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

TEST(Matrix, computeQRGivens)
{
    static constexpr uint32_t NUM_ROWS = 3, NUM_COLS = 2;
    Figaro::Matrix<double> matrix(NUM_ROWS, NUM_COLS);

    matrix[0][0] = 1; matrix[0][1] = 2;
    matrix[1][0] = 3; matrix[1][1] = 4;
    matrix[2][0] = 4; matrix[2][1] = 3;

    matrix.computeQRGivens();

    EXPECT_NEAR(matrix[0][0], 5.099019513592785, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrix[0][1], 5.099019513592786, GIVENS_TEST_PRECISION_ERROR);

    EXPECT_NEAR(matrix[1][0], 0, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrix[1][1], 1.732050807568877, GIVENS_TEST_PRECISION_ERROR);
    
    EXPECT_NEAR(matrix[2][0], 0, GIVENS_TEST_PRECISION_ERROR);
    EXPECT_NEAR(matrix[2][1], 0, GIVENS_TEST_PRECISION_ERROR);
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

TEST(DatabaseConfig, ComputeSimpleHeadByOneAttrName)
{
    static const std::string DB_CONFIG_PATH = getConfigPath(3) + DB_CONFIG_PATH_IN;
     static const std::string FILE_INPUT_EXP_R = getDataPath(3) + R_COMP_FILE_NAME;
    Figaro::Database database(DB_CONFIG_PATH);
    Figaro::ErrorCode initError;
    Figaro::ErrorCode loadError;
    Figaro::MatrixEigenT R, expectedR;

    initError = database.getInitializationErrorCode();
    EXPECT_EQ(initError, Figaro::ErrorCode::NO_ERROR);
    loadError = database.loadData();
    EXPECT_EQ(loadError, Figaro::ErrorCode::NO_ERROR);
    
    database.sortRelation("R", {"A"});
    database.computeHead("R", "A");

    database.sortRelation("S", {"B", "A"});
    database.computeHead("S", "A");

    database.sortRelation("T", {"B", "C"});
    database.computeHead("T", "C");
    
    database.sortRelation("U", {"C"});
    database.computeHead("U", "C");

    database.joinRelations({"S", "R"}, {{"A", "A"}} , true);
    database.joinRelations({"T", "U"}, {{"C", "C"}} );

    database.computeScaledCartesianProduct({"S", "T"}, "B");
    database.computeQRDecompositionHouseholder("S", &R);

    readMatrixDense(FILE_INPUT_EXP_R, expectedR);
    compareMatrices(R, expectedR, false, false);
}


TEST(DISABLED_DatabaseConfig, ComputeSimpleHeadByOneMultipleAttributes)
{
    static const std::string DB_CONFIG_PATH = getConfigPath(4) + DB_CONFIG_PATH_IN;
    static const std::string FILE_INPUT_EXP_R = getDataPath(4) + R_COMP_FILE_NAME;
    Figaro::Database database(DB_CONFIG_PATH);
    Figaro::ErrorCode initError;
    Figaro::ErrorCode loadError;
    Figaro::MatrixEigenT R, expectedR;

    initError = database.getInitializationErrorCode();
    EXPECT_EQ(initError, Figaro::ErrorCode::NO_ERROR);
    loadError = database.loadData();
    EXPECT_EQ(loadError, Figaro::ErrorCode::NO_ERROR);
    
    database.sortRelation("R", {"A"});
    database.computeHead("R", "A");

    database.sortRelation("S", {"A", "B"});
    database.computeHead("S", "A");

    database.sortRelation("T", {"C", "B"});
    database.computeHead("T", "C");
    
    database.sortRelation("U", {"C"});
    database.computeHead("U", "C");

    database.joinRelations({"S", "R"}, {{"A", "A"}}, true);
    database.joinRelations({"T", "U"}, {{"C", "C"}} );

    database.computeScaledCartesianProduct({"S", "T"}, "B");
    database.computeQRDecompositionHouseholder("S", &R);

    readMatrixDense(FILE_INPUT_EXP_R, expectedR);
    compareMatrices(R, expectedR, false, true);
    FIGARO_LOG_DBG(R)
}


TEST(DatabaseConfig, BasicQueryParsing)
{
    static const std::string DB_CONFIG_PATH = getConfigPath(5) + DB_CONFIG_PATH_IN;
    static const std::string QUERY_CONFIG_PATH = getConfigPath(5) + QUERY_CONFIG_PATH_IN;

    Figaro::Database database(DB_CONFIG_PATH);
    Figaro::ErrorCode initError;
    Figaro::ErrorCode loadError;

    initError = database.getInitializationErrorCode();
    EXPECT_EQ(initError, Figaro::ErrorCode::NO_ERROR);
    loadError = database.loadData();
    EXPECT_EQ(loadError, Figaro::ErrorCode::NO_ERROR);

    Figaro::Query query(&database);
    EXPECT_EQ(query.loadQuery(QUERY_CONFIG_PATH), Figaro::ErrorCode::NO_ERROR);
    query.evaluateQuery();
}