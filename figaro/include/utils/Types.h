#ifndef _UTIL_TYPES_H_
#define _UTIL_TYPES_H_

namespace Figaro
{
#
    enum class  MemoryLayout: uint32_t
    {
        ROW_MAJOR = 0,
        COL_MAJOR = 1
    };

    enum class QRHintType
    {
        GIV_THIN_BOTTOM = 0,
        GIV_THIN_DIAG = 1,
        GIV_THICK_BOTTOM = 2,
        GIV_THICK_DIAG = 3,
        HOUSEHOLDER = 4
    };

    enum class SVDHintType
    {
        DIV_AND_CONQ = 0,
        QR_ITER = 1,
        POWER_ITER = 2,
        EIGEN_DECOMP = 3,
        EIGEN_DECOMP_DIV_AND_CONQ = 4,
        EIGEN_DECOMP_QR_ITER = 5,
        EIGEN_DECOMP_RRR = 6,
        QR = 7
    };

    enum class PCAHintType
    {
        DIV_AND_CONQ = 0,
        QR_ITER = 1,
        POWER_ITER = 2,
        EIGEN_DECOMP = 3,
        EIGEN_DECOMP_DIV_AND_CONQ = 4,
        EIGEN_DECOMP_QR_ITER = 5,
        EIGEN_DECOMP_RRR = 6,
        QR = 7
    };

    enum class LUHintType
    {
        THIN_DIAG = 0,
        PART_PIVOT_LAPACK = 1
    };

    enum class EDHintType
    {
        QR_ITER = 0,
        DIV_AND_CONQ = 1,
        RRR = 2
    };
}

#endif
