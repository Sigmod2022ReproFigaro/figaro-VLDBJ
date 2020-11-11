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
    database.joinRelations({"T", "U"}, {{"C", "C"}} );

    database.computeScaledCartesianProduct({"S", "T"}, "B");

}
