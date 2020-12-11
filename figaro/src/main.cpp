#include "database/Database.h"
#include "database/query/Query.h"
#include "utils/Performance.h"

#include <boost/program_options.hpp>
#include <fstream>

namespace po = boost::program_options;

const std::string queryConfigPath = "";

int main(int argc, char *argv[]) 
{
    std::string dump_path; 
    std::string db_config_path;
    bool dump = false;
    uint32_t precision;
    MICRO_BENCH_INIT(load);
    MICRO_BENCH_INIT(sort);
    MICRO_BENCH_INIT(main)

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
    MICRO_BENCH_START(load);
    Figaro::Database database(db_config_path);
    database.loadData();
    MICRO_BENCH_STOP(load);
    MICRO_BENCH_START(sort)
    database.sortData();
    MICRO_BENCH_STOP(sort)

    Figaro::Query query(&database);
    //query.loadQuery(queryConfigPath);
    //query.evaluateQuery();

    MICRO_BENCH_START(sort)
    database.sortRelation("R", {"A"});
    MICRO_BENCH_STOP(sort);
    database.computeHead("R", "A");

    MICRO_BENCH_START(sort)
    database.sortRelation("S", {"A", "B"});
    MICRO_BENCH_STOP(sort);
    database.computeHead("S", "A");

    MICRO_BENCH_START(sort)
    database.sortRelation("T", {"C", "B"});
    MICRO_BENCH_STOP(sort);
    database.computeHead("T", "C");
    
    MICRO_BENCH_START(sort)
    database.sortRelation("U", {"C"});
    MICRO_BENCH_STOP(sort);
    database.computeHead("U", "C");

    FIGARO_LOG_DBG("PASS sort and compute head")

    MICRO_BENCH_START(main)
    database.joinRelations({"S", "R"}, {{"A", "A"}} );
    FIGARO_LOG_DBG("Pass Join relations S R")
    database.swapAttributes("S", {"A1", "A2"} );
    FIGARO_LOG_DBG("Pass Join Swap ")
    database.joinRelations({"T", "U"}, {{"C", "C"}} );
    FIGARO_LOG_DBG("Pass Join relations T U")

    database.computeScaledCartesianProduct({"S", "T"}, "B");
    FIGARO_LOG_DBG("Pass Compute Scaled")
    database.computeQRDecompositionHouseholder("S", &R);
    MICRO_BENCH_STOP(main)
    FIGARO_LOG_DBG("Pass Compute Householder ")
    FIGARO_LOG_INFO(R);
    FIGARO_LOG_DBG("PRECISION", precision)
    FIGARO_LOG_BENCH("Figaro", "load", MICRO_BENCH_GET_TIMER(load));
    FIGARO_LOG_BENCH("Figaro", "sort", MICRO_BENCH_GET_TIMER(sort));
    FIGARO_LOG_BENCH("Figaro", "main", MICRO_BENCH_GET_TIMER(main));
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