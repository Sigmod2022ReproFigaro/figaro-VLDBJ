#include "database/Database.h"
#include "database/query/Query.h"
#include "database/query/visitor/result/ASTVisitorResultQR.h"
#include "database/query/visitor/result/ASTVisitorResultSVD.h"
#include "utils/Performance.h"

#include <boost/program_options.hpp>
#include <fstream>
#include <omp.h>
#include "utils/Utils.h"

namespace po = boost::program_options;

void initGlobalState(uint32_t numThreads = 48)
{
    omp_set_nested(1);
    omp_set_dynamic(0);
    omp_set_num_threads(numThreads);
    FIGARO_LOG_INFO("Number of threads is", numThreads)
}

int main(int argc, char *argv[])
{
    std::string dumpFilePath;
    std::string dbConfigPath;
    std::string strMemoryLayout;
    std::string queryConfigPath;
    std::string decompositionAlgorithm;

    Figaro::MemoryLayout memoryLayout = Figaro::MemoryLayout::ROW_MAJOR;
    bool dump = false;
    uint32_t precision;
    uint32_t numThreads = 1;
    bool computeAll = false;
    std::string computeAllStr = "false";

    std::string strArgs = "";
    for (uint32_t idx = 0; idx < argc; idx++)
    {
        strArgs += argv[idx] + std::string(" ");
    }
    FIGARO_LOG_INFO("Command line args", strArgs)


    po::options_description desc("figaro - allowed options");
    desc.add_options()
    ("help", "produce help message")
    ("dump_file_path", po::value<std::string>(&dumpFilePath))
    ("db_config_path", po::value<std::string>(&dbConfigPath))
    ("query_config_path", po::value<std::string>(&queryConfigPath))
    ("precision", po::value<uint32_t>(&precision))
    ("num_threads", po::value<uint32_t>(&numThreads))
    ("compute_all",  boost::program_options::value<bool>())
    ("decomposition_algorithm", po::value<std::string>(&decompositionAlgorithm))
    ("memory_layout", po::value<std::string>(&strMemoryLayout))
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

    if (vm.count("num_threads"))
    {
        numThreads = vm["num_threads"].as<std::uint32_t>();
    }

    if (vm.count("memory_layout"))
    {
        strMemoryLayout = vm["memory_layout"].as<std::string>();
        if (strMemoryLayout == "ROW_MAJOR")
        {
            memoryLayout = Figaro::MemoryLayout::ROW_MAJOR;
        }
        else if (strMemoryLayout == "COL_MAJOR")
        {
            memoryLayout = Figaro::MemoryLayout::COL_MAJOR;
        }
    }

    if (vm.count("compute_all"))
    {
        computeAll = vm["compute_all"].as<bool>();
        computeAllStr = computeAll ? "true" : "false";
    }

    dbConfigPath = vm["db_config_path"].as<std::string>();
    queryConfigPath = vm["query_config_path"].as<std::string>();
    FIGARO_LOG_INFO(dbConfigPath)


    initGlobalState(numThreads);

    Figaro::Database database(dbConfigPath);
    database.loadData();

    Figaro::Query query(&database);
    query.loadQuery(queryConfigPath, {{"decomp_alg", decompositionAlgorithm},
                {"compute_all",  computeAllStr}});
    Figaro::Query::OpType opType = query.getOpType();
    switch (opType)
    {
        case Figaro::Query::OpType::DECOMP_QR:
        {
            query.evaluateQuery(true, {{"headsAndTails", true}, {"generalizedHeadsAndTails", true}, {"postProcessing", true}}, memoryLayout, dump);
            break;
        }
        case Figaro::Query::OpType::DECOMP_LU:
        {
            query.evaluateQuery(true, {{"headsAndTails", true}, {"generalizedHeadsAndTails", true}, {"postProcessing", true}, {"computeL", true}}, memoryLayout, dump);
            break;
        }
        case Figaro::Query::OpType::DECOMP_SVD:
        {
            query.evaluateQuery(true, {{"headsAndTails", true}, {"generalizedHeadsAndTails", true},{"postProcessing", true}}, memoryLayout, dump);
            break;
        }
        case Figaro::Query::OpType::DECOMP_PCA:
        {
             query.evaluateQuery(true, {{"headsAndTails", true}, {"generalizedHeadsAndTails", true}, {"postProcessing", true}}, memoryLayout, dump);
                break;
        }
    }
    Figaro::ASTVisitorResultAbs* pResult = query.getResult();
    if (dump)
    {
        Figaro::ASTVisitorResultAbs* pResult = query.getResult();
        bool computeAll = query.isComputeAll();
        FIGARO_LOG_BENCH("Dumping R to the path", dumpFilePath);
        switch (opType)
        {
            case Figaro::Query::OpType::DECOMP_QR:
            {
                Figaro::ASTVisitorResultQR* pQrResult = (Figaro::ASTVisitorResultQR*)pResult;
                std::ofstream fileDumpR(dumpFilePath, std::ofstream::out);
                database.outputRelationToFile(fileDumpR,
                pQrResult->getRRelationName(), ',', precision);
                if (computeAll)
                {
                    double ortMeasure = database.checkOrthogonality(pQrResult->getQRelationName(), {});
                    FIGARO_LOG_BENCH("Orthogonality of Q",  ortMeasure);
                }
            }
            case Figaro::Query::OpType::DECOMP_LU:
            {
                break;
            }
            case Figaro::Query::OpType::DECOMP_SVD:
            {
                Figaro::ASTVisitorResultSVD* pSVDResult = (Figaro::ASTVisitorResultSVD*)pResult;
                std::ofstream fileDumpS(dumpFilePath, std::ofstream::out);
                database.outputRelationToFile(fileDumpS,
                pSVDResult->getSRelationName(), ',', precision);
                if (computeAll)
                {
                    double ortMeasureU = database.checkOrthogonality(pSVDResult->getURelationName(), {});
                    FIGARO_LOG_BENCH("Orthogonality of U",  ortMeasureU);
                      double ortMeasureV = database.checkOrthogonality(pSVDResult->getVRelationName(), {});
                    FIGARO_LOG_BENCH("Orthogonality of V",  ortMeasureV);
                }
                break;
            }
            case Figaro::Query::OpType::DECOMP_PCA:
            {
                Figaro::ASTVisitorResultSVD* pSVDResult = (Figaro::ASTVisitorResultSVD*)pResult;
                std::ofstream fileDumpS(dumpFilePath, std::ofstream::out);
                database.outputRelationToFile(fileDumpS,
                pSVDResult->getSRelationName(), ',', precision);
                if (computeAll)
                {
                    /*
                    double ortMeasureU = database.checkOrthogonality(pSVDResult->getURelationName(), {});
                    FIGARO_LOG_BENCH("Orthogonality of U",  ortMeasureU);
                      double ortMeasureV = database.checkOrthogonality(pSVDResult->getVRelationName(), {});
                    FIARO_LOG_BENCH("Orthogonality of V",  ortMeasureV);
                    */
                }
                break;
            }
        }
    }

    FIGARO_LOG_INFO("Figaro program has terminated")
    return 0;
}