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
    Figaro::Database database(DB_CONFIG_PATH);
    Figaro::ErrorCode initError;
    Figaro::ErrorCode loadError;
    std::vector<std::string> vRelationNames{"S", "R"};
    std::vector<std::tuple<std::string, std::string> > vJoinAttributes
    { std::tuple<std::string, std::string>{"A", "A"} };

    initError = database.getInitializationErrorCode();
    EXPECT_EQ(initError, Figaro::ErrorCode::NO_ERROR);
    loadError = database.loadData();
    EXPECT_EQ(loadError, Figaro::ErrorCode::NO_ERROR);
    
    database.computeHead("R", "A");
    database.computeHead("S", "A");
    database.joinRelations(vRelationNames, vJoinAttributes);
}
