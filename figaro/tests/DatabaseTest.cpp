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

TEST(Storage, Matrix)
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

TEST(Storage, MatrixResize)
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

    matrix.applyGivens(1, 2, sinTheta, cosTheta);

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
    Figaro::MatrixT R, expectedR;

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

    FIGARO_LOG_DBG("DB CONFIG PATH", DB_CONFIG_PATH)
    FIGARO_LOG_DBG("R", R)
    readMatrixDense(FILE_INPUT_EXP_R, expectedR);
    FIGARO_LOG_DBG("expectedR", expectedR)
    compareMatrices(R, expectedR, false, false);
    FIGARO_LOG_DBG(R)
}


TEST(DatabaseConfig, ComputeSimpleHeadByOneMultipleAttributes)
{
    static const std::string DB_CONFIG_PATH = getConfigPath(4) + DB_CONFIG_PATH_IN;
    static const std::string FILE_INPUT_EXP_R = getDataPath(4) + R_COMP_FILE_NAME;
    Figaro::Database database(DB_CONFIG_PATH);
    Figaro::ErrorCode initError;
    Figaro::ErrorCode loadError;
    Figaro::MatrixT R, expectedR;

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

    FIGARO_LOG_DBG("DB CONFIG PATH", DB_CONFIG_PATH)
    FIGARO_LOG_DBG("R", R)
    readMatrixDense(FILE_INPUT_EXP_R, expectedR);
    FIGARO_LOG_DBG("expectedR", expectedR)
    compareMatrices(R, expectedR, false, true);
    FIGARO_LOG_DBG(R)
}


TEST(DatabaseConfig, BasicQueryParsing)
{
    static const std::string QUERY_CONFIG_PATH = getConfigPath(5) + QUERY_CONFIG_PATH_IN;
    Figaro::Query query(nullptr);
    EXPECT_EQ(query.loadQuery(QUERY_CONFIG_PATH), Figaro::ErrorCode::NO_ERROR);
    query.evaluateQuery();
}