#include "UtilTest.h"
#include "database/Database.h"
#include "database/query/Query.h"
#include <vector>
#include <string>

TEST(DatabaseConfig, BasicInput) {
    static const std::string DB_CONFIG_PATH = getTestPath(1) + DB_CONFIG_PATH_IN;
    Figaro::Database database(DB_CONFIG_PATH);
    Figaro::ErrorCode initError = database.getInitializationErrorCode();
    EXPECT_EQ(initError, Figaro::ErrorCode::NO_ERROR);
}

TEST(DatabaseConfig, PathQuery3) {
    static const std::string DB_CONFIG_PATH = getTestPath(2) + DB_CONFIG_PATH_IN;
    Figaro::Database database(DB_CONFIG_PATH);
    Figaro::ErrorCode initError;
    Figaro::ErrorCode loadError;
    initError = database.getInitializationErrorCode();
    EXPECT_EQ(initError, Figaro::ErrorCode::NO_ERROR);
    loadError = database.loadData();
    EXPECT_EQ(loadError, Figaro::ErrorCode::NO_ERROR);
}

TEST(DatabaseConfig, ComputeSimpleHeadByOneAttrName)
{
    static const std::string DB_CONFIG_PATH = getTestPath(3) + DB_CONFIG_PATH_IN;
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

    database.sortRelation("S", {"A", "B"});
    database.computeHead("S", "A");

    database.sortRelation("T", {"C", "B"});
    database.computeHead("T", "C");
    
    database.sortRelation("U", {"C"});
    database.computeHead("U", "C");

    database.joinRelations({"S", "R"}, {{"A", "A"}} );
    database.swapAttributes("S", {"A1", "A2"} );
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
    static const std::string DB_CONFIG_PATH = getTestPath(4) + DB_CONFIG_PATH_IN;
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

    database.joinRelations({"S", "R"}, {{"A", "A"}} );
    database.swapAttributes("S", {"A11", "A21"} );
    database.swapAttributes("S", {"A12", "A21"} );
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