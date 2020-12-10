#include "database/Database.h"
#include "database/query/Query.h"

#include <boost/program_options.hpp>
#include <fstream>

namespace po = boost::program_options;

//const std::string DB_CONFIG_PATH = "/home/popina/Figaro/figaro-code/system_tests/test3/database_specs.conf";
const std::string queryConfigPath = "";

int main(int argc, char *argv[]) 
{
    std::string dump_path; 
    std::string db_config_path;
    bool dump = false;
    uint32_t precision;

    po::options_description desc("figaro - allowed options");   
    desc.add_options()
    ("help", "produce help message")
    ("dump_path", po::value<std::string>(&dump_path))
    ("db_config_path", po::value<std::string>(&db_config_path))
    ("precision", po::value<uint32_t>(&precision))
    ;

    //po::positional_options_description p;
    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(desc).run(), vm);
    po::notify(vm);

    if (vm.count("dump_path"))
    {
        dump_path = vm["dump_path"].as<std::string>();
        dump = true;
        FIGARO_LOG_INFO(dump_path)
    }

    db_config_path = vm["db_config_path"].as<std::string>();
    FIGARO_LOG_INFO(db_config_path)
    Figaro::MatrixT R;
    Figaro::Database database(db_config_path);
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

    FIGARO_LOG_DBG("PASS sort and compute head")

    database.joinRelations({"S", "R"}, {{"A", "A"}} );
    FIGARO_LOG_DBG("Pass Join relations S R")
    database.swapAttributes("S", {"A1", "A2"} );
    FIGARO_LOG_DBG("Pass Join Swap ")
    database.joinRelations({"T", "U"}, {{"C", "C"}} );
    FIGARO_LOG_DBG("Pass Join relations T U")

    database.computeScaledCartesianProduct({"S", "T"}, "B");
    FIGARO_LOG_DBG("Pass Compute Scaled")
    database.computeQRDecompositionHouseholder("S", &R);
    FIGARO_LOG_DBG("Pass Compute Householder ")
    FIGARO_LOG_INFO(R);
    FIGARO_LOG_DBG("PRECISION", precision)
    if (dump)
    {
        std::string dumpFileName = dump_path + "/R.csv";
        FIGARO_LOG_INFO("Dumping R to the path", dumpFileName);
        std::ofstream fileDumpR(dumpFileName, std::ofstream::out);
        Figaro::outputMatrixTToCSV(fileDumpR, R.topRightCorner(R.cols(), R.cols()), ',', precision);
    }


    FIGARO_LOG_INFO("Figaro program has terminated")
    return 0;
}