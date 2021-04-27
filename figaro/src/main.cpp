#include "database/Database.h"
#include "database/query/Query.h"
#include "utils/Performance.h"

#include <boost/program_options.hpp>
#include <fstream>
#include <omp.h>

namespace po = boost::program_options;

void initGlobalState(void)
{
    omp_set_nested(1);
    omp_set_dynamic(0);
    uint32_t numberOfCores =  omp_get_num_procs();
    omp_set_num_threads(numberOfCores);
}

int main(int argc, char *argv[])
{
    std::string dumpFilePath;
    std::string dbConfigPath;
    std::string queryConfigPath;
    bool dump = false;
    uint32_t precision;

    initGlobalState();

    po::options_description desc("figaro - allowed options");
    desc.add_options()
    ("help", "produce help message")
    ("dump_file_path", po::value<std::string>(&dumpFilePath))
    ("db_config_path", po::value<std::string>(&dbConfigPath))
    ("query_config_path", po::value<std::string>(&queryConfigPath))
    ("precision", po::value<uint32_t>(&precision))
    ;

    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(desc).run(), vm);
    po::notify(vm);

    if (vm.count("dump_file_path"))
    {
        dumpFilePath = vm["dump_file_path"].as<std::string>();
        dump = true;
        FIGARO_LOG_INFO(dumpFilePath)
    }

    dbConfigPath = vm["db_config_path"].as<std::string>();
    queryConfigPath = vm["query_config_path"].as<std::string>();
    FIGARO_LOG_INFO(dbConfigPath)
    Figaro::Database database(dbConfigPath);
    database.loadData();
    Figaro::Query query(&database);
    query.loadQuery(queryConfigPath);

    MICRO_BENCH_INIT(main)
    MICRO_BENCH_START(main)
    query.evaluateQuery(true, true, true, true);
    MICRO_BENCH_STOP(main)
    FIGARO_LOG_BENCH("Figaro", "query evaluation",  MICRO_BENCH_GET_TIMER_LAP(main));

    Figaro::MatrixEigenT& R = query.getResult();
    if (dump)
    {
        FIGARO_LOG_INFO("Dumping R to the path", dumpFilePath);
        std::ofstream fileDumpR(dumpFilePath, std::ofstream::out);
        Figaro::outputMatrixTToCSV(fileDumpR, R.topRightCorner(R.cols(), R.cols()), ',', precision);
    }


    FIGARO_LOG_INFO("Figaro program has terminated")
    return 0;
}