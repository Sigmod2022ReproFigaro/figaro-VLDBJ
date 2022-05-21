#include "utils/Utils.h"

#include <iostream>
#include <fstream>
#include <iomanip>
#include <omp.h>
#include <unistd.h>
#include <ios>
#include <string>

namespace Figaro
{
    uint32_t getNumberOfLines(const std::string& filePath)
    {
        std::string line;
        std::ifstream file(filePath);
        uint32_t cntLines = 0;
        if (file.fail())
        {
            FIGARO_LOG_ERROR("Path to the file is wrong:", filePath);
            return 0;
        }

        while (std::getline(file, line))
        {
            cntLines ++;
        }
        return cntLines;
    }

    uint32_t getNumberOfThreads(void)
    {
        uint32_t numThreads;
        #pragma omp parallel
        {
            #pragma omp single
            numThreads = omp_get_num_threads();
        }
        return numThreads;
    }

    std::vector<std::string> setIntersection(
        const std::vector<std::string>& vStr1,
        const std::vector<std::string>& vStr2)
    {
        std::map<std::string, bool> sStrAppears;
        std::vector<std::string> vIntersection;
        for (const auto& str: vStr1)
        {
            sStrAppears[str] = false;
        }
        for (const auto& str: vStr2)
        {
            if (sStrAppears.find(str) != sStrAppears.end())
            {
                sStrAppears[str] = true;
            }
        }
        for (const auto&[key, exists]: sStrAppears)
        {
            if (exists)
            {
                vIntersection.push_back(key);
            }
        }
        return vIntersection;
    }
}

std::ostream& Figaro::outputMatrixTToCSV(std::ostream& out,
    const Figaro::MatrixEigenT& matrix, char sep, uint32_t precision)
{
    for (uint32_t row = 0; row < matrix.rows(); row ++)
    {
        for (uint32_t col = 0; col < matrix.cols(); col++)
        {

            out << std::setprecision(precision) << std::scientific << matrix(row, col);
            if (col != (matrix.cols() - 1))
            {
                out << sep;
            }
        }
        out << std::endl;
    }
    return out;
}


std::ostream& operator<<(std::ostream& out, const Figaro::MatrixEigenT& matrix)
{
    out << "Matrix Eigen" << std::endl;
    out <<  "Matrix dimensions: " << matrix.rows() << " " << matrix.cols() << std::endl;
    return Figaro::outputMatrixTToCSV(out, matrix);
}

//////////////////////////////////////////////////////////////////////////////
//
// process_mem_usage(double &, double &) - takes two doubles by reference,
// attempts to read the system-dependent data for a process' virtual memory
// size and resident set size, and return the results in KB.
//
// On failure, returns 0.0, 0.0

void processMemUsage(double& vmUsage, double& residentSet)
{
   using std::ios_base;
   using std::ifstream;
   using std::string;

   vmUsage     = 0.0;
   residentSet = 0.0;

   // 'file' stat seems to give the most reliable results
   //
   ifstream stat_stream("/proc/self/stat", ios_base::in);

   // dummy vars for leading entries in stat that we don't care about
   //
   string pid, comm, state, ppid, pgrp, session, tty_nr;
   string tpgid, flags, minflt, cminflt, majflt, cmajflt;
   string utime, stime, cutime, cstime, priority, nice;
   string O, itrealvalue, starttime;

   // the two fields we want
   //
   unsigned long vsize;
   long rss;

   stat_stream >> pid >> comm >> state >> ppid >> pgrp >> session >> tty_nr
               >> tpgid >> flags >> minflt >> cminflt >> majflt >> cmajflt
               >> utime >> stime >> cutime >> cstime >> priority >> nice
               >> O >> itrealvalue >> starttime >> vsize >> rss; // don't care about the rest

   stat_stream.close();

   long pageSizeKB = sysconf(_SC_PAGE_SIZE) / 1024; // in case x86-64 is configured to use 2MB pages
   vmUsage     = vsize / 1024.0;
   residentSet = rss * pageSizeKB;
}

