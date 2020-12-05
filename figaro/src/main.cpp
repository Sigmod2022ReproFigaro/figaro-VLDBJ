#include "database/Database.h"
#include "database/query/Query.h"

#include <boost/program_options.hpp>

namespace po = boost::program_options;

const std::string DB_CONFIG_PATH = "/home/popina/Figaro/figaro-code/system_tests/test3/database_specs.conf";
const std::string queryConfigPath = "";

int main(int argc, char *argv[]) 
{
    boost::program_options::options_description desc("figaro - allowed options");
    Figaro::MatrixT R;
    Figaro::Database database(DB_CONFIG_PATH);
    database.loadData();
    database.sortData();

    Figaro::Query query(&database);
    //query.loadQuery(queryConfigPath);
    //query.evaluateQuery();


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
    database.computeQRDecompositionHouseholder("S", &R);

    FIGARO_LOG_INFO("Figaro program has terminated")
    return 0;
}