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
        JACOBI = 0,
        POWER_ITER = 1,
        EIGEN_DECOMP = 2,
        QR = 4
    };

    enum class LUHintType
    {
        THIN_DIAG = 0,
        PART_PIVOT_LAPACK = 1
    };
}

#endif