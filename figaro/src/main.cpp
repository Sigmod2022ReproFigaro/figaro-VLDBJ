#include "database/Database.h"
#include "database/query/Query.h"

const std::string DB_CONFIG_PATH = "/home/popina/Figaro/figaro-code/system_tests/test1/database_specs.conf";
const std::string queryConfigPath = "";

int main() 
{
    Figaro::Database database(DB_CONFIG_PATH);
    database.loadData();

    Figaro::Query query(&database);
    query.loadQuery(queryConfigPath);
    query.evaluateQuery();

    return 0;
}