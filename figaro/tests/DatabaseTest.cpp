#include "UtilTest.h"
#include "database/Database.h"
#include "database/query/Query.h"

TEST(DatabaseConfig, BasicInput) {
    static const std::string DB_CONFIG_PATH = getTestPath(1) + DB_CONFIG_PATH_IN;
    Figaro::Database database(DB_CONFIG_PATH);
    Figaro::ErrorCode initError = database.getInitializationErrorCode();
    EXPECT_EQ(initError, Figaro::ErrorCode::NO_ERROR);
}
