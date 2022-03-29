#ifndef _FIGARO_UTILS_H_
#define _FIGARO_UTILS_H_

#include <eigen3/Eigen/Dense>
// There is a bug because Eigen include C complex.h which has I defined as macro
// while boost uses I as classname, and thus there is a clash in naming.
// I is not used anywhere in eigen as a variable except in src/SparseLU/SparseLU_gemm_kernel.h:
// which doesn't include any files, thus I is not included.
//
#undef I
#include <nlohmann/json.hpp>
#include "utils/Logger.h"
#include "utils/ErrorCode.h"

namespace Figaro
{
    typedef nlohmann::json json;
    typedef Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::ColMajor> MatrixEigenT;
    typedef Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> MatrixEigenTR;
    typedef Eigen::VectorXd VectorT;
    typedef Eigen::ArrayXd ArrayT;

    // TODO: move to Utils namespace
    uint32_t getNumberOfLines(const std::string& filePath);

    std::ostream& outputMatrixTToCSV(std::ostream& out,
        const Figaro::MatrixEigenT& matrix,
        char sep = ' ', uint32_t precision = 6);

    std::vector<std::string> setIntersection(
        const std::vector<std::string>& vStr1,
        const std::vector<std::string>& vStr2);

    uint32_t getNumberOfThreads(void);
}

template <class T>
inline void hash_combine(std::size_t& seed, T v)
{
    seed ^= std::hash<T>()(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

template<class... TupleArgs>
struct std::hash<std::tuple<TupleArgs...> >
{
    private:
        //  this is a termination condition
        //  N == sizeof...(TupleTypes)
        //
        template<size_t Idx = 0, typename... TupleTypes>
        inline typename std::enable_if<Idx == sizeof...(TupleTypes), void>::type
        hash_combine_tup(
            [[maybe_unused]] size_t& seed,
            [[maybe_unused]] const std::tuple<TupleTypes...>& tup) const
        {
        }

        //  this is the computation workhorse
        //  N < sizeof...(TupleTypes)
        //
        template<size_t Idx = 0, typename... TupleTypes>
        inline typename std::enable_if<Idx < sizeof...(TupleTypes), void>::type
        hash_combine_tup(size_t& seed, const std::tuple<TupleTypes...>& tup) const
        {
            hash_combine(seed, std::get<Idx>(tup));

            //  on to next element
            hash_combine_tup<Idx + 1>(seed, tup);
        }

    public:
        size_t operator()(std::tuple<TupleArgs...> tupleValue) const
        {
            size_t seed = 0;
            hash_combine_tup<0>(seed, tupleValue);
            return seed;
        }
};

std::ostream& operator<<(std::ostream& out, const Figaro::MatrixEigenT& matrix);

#endif