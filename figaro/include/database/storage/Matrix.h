#ifndef _FIGARO_MATRIX_H_
#define _FIGARO_MATRIX_H_

#include "utils/Logger.h"
#include "ArrayStorage.h"
#include "utils/Performance.h"
#include <boost/math/special_functions/sign.hpp>
#include <cmath>
#include <mkl.h>

namespace Figaro
{
    enum class  MemoryLayout: uint32_t
    {
        ROW_MAJOR = 0,
        COL_MAJOR = 1
    };

    enum class QRHintType
    {
        THIN_BOTTOM = 0,
        THIN_DIAG = 1,
        THICK_BOTTOM = 2,
        THICK_DIAG = 3,
        HOUSEHOLDER_LAPACK = 4
    };

    // Row-major order of storing elements of matrix is assumed.
    template <typename T, MemoryLayout L = MemoryLayout::ROW_MAJOR>
    class Matrix
    {
        static constexpr uint32_t MIN_COLS_PAR = UINT32_MAX;
        //static constexpr uint32_t MIN_COLS_PAR = 0;
        uint32_t m_numRows = 0, m_numCols = 0;
        ArrayStorage<T>* m_pStorage = nullptr;

        using MatrixType = Matrix<T, L>;
        using MatrixTypeCol = Matrix<T, MemoryLayout::COL_MAJOR>;
        using MatrixTypeRow = Matrix<T, MemoryLayout::ROW_MAJOR>;

        template <typename U, MemoryLayout V>
        friend class Matrix;

        void destroyData(void)
        {
            m_numRows = 0;
            m_numCols = 0;
            if (nullptr != m_pStorage)
            {
                delete m_pStorage;
                m_pStorage = nullptr;
            }
        }

        uint64_t getNumEntries(void) const
        {
            return (uint64_t) m_numRows * (uint64_t)m_numCols;
        }
        const T* getArrPt (void) const { return &((*m_pStorage)[0]); }
        T* getArrPt (void)  { return &((*m_pStorage)[0]); }
    public:

        Matrix(uint32_t numRows, uint32_t numCols)
        {
            m_numRows = numRows;
            m_numCols = numCols;
            m_pStorage = new ArrayStorage<T>(getNumEntries());
        }

        Matrix(const Matrix&) = delete;
        Matrix& operator=(const Matrix&) = delete;

        Matrix(Matrix&& other)
        {
            m_pStorage = other.m_pStorage;
            m_numRows = other.m_numRows;
            m_numCols = other.m_numCols;

            other.m_pStorage = nullptr;
            other.m_numCols = 0;
            other.m_numRows = 0;
        }
        Matrix& operator=(Matrix&& other)
        {
            if (this != &other)
            {
                destroyData();
                m_pStorage = other.m_pStorage;
                m_numRows = other.m_numRows;
                m_numCols = other.m_numCols;

                other.m_pStorage = nullptr;
                other.m_numCols = 0;
                other.m_numRows = 0;
            }
            return *this;
        }

        ~Matrix()
        {
            destroyData();
        }

        // No bound checks for colIdx. BE CAREFULL!!!
        T* operator[](uint32_t rowIdx)
        {
            FIGARO_LOG_ASSERT(rowIdx < m_numRows);
            return &((*m_pStorage)[(uint64_t)(rowIdx) * (uint64_t)(m_numCols)]);
        }

        const T* operator[](uint32_t rowIdx) const
        {
            FIGARO_LOG_ASSERT(rowIdx < m_numRows);
            return &((*m_pStorage)[(uint64_t)(rowIdx) * (uint64_t)(m_numCols)]);
        }

        T& operator()(uint32_t rowIdx, uint32_t colIdx)
        {
            FIGARO_LOG_ASSERT(rowIdx < m_numRows);
            FIGARO_LOG_ASSERT(colIdx < m_numCols);
            if constexpr (L == MemoryLayout::ROW_MAJOR)
            {
                return (*m_pStorage)[(uint64_t)(rowIdx) * (uint64_t)(m_numCols) + colIdx];
            }
            else
            {
                return (*m_pStorage)[(uint64_t)(colIdx) * (uint64_t)(m_numRows) + rowIdx];
            }
        }

        T operator()(uint32_t rowIdx, uint32_t colIdx) const
        {
            FIGARO_LOG_ASSERT(rowIdx < m_numRows);
            FIGARO_LOG_ASSERT(colIdx < m_numCols);
            if constexpr (L == MemoryLayout::ROW_MAJOR)
            {
                return (*m_pStorage)[(uint64_t)(rowIdx) * (uint64_t)(m_numCols) + (uint64_t)colIdx];
            }
            else
            {
                return (*m_pStorage)[(uint64_t)(colIdx) * (uint64_t)(m_numRows) + rowIdx];
            }
        }

        MatrixType operator*(const MatrixType& second) const
        {
            CBLAS_LAYOUT cBlasMemLayout = getCblasMajorOrder();
            const double* pA = getArrPt();
            const double* pB = second.getArrPt();
            uint32_t m = getNumRows();
            uint32_t n = second.getNumCols();
            uint32_t k = getNumCols();
            MatrixType matC{m, n};
            double* pC = matC.getArrPt();
            uint32_t ldA = getLeadingDimension();
            uint32_t ldB = second.getLeadingDimension();
            uint32_t ldC = matC.getLeadingDimension();

            cblas_dgemm(cBlasMemLayout, CblasNoTrans, CblasNoTrans,
                m, n, k, 1.0, pA, ldA, pB, ldB, 0.0, pC, ldC);
            return matC;
        }

        MatrixType add(const MatrixType& second,
            uint32_t numJoinAttr1, uint32_t numJoinAttr2) const
        {
            auto& matA = *this;
            uint32_t m = getNumRows();
            uint32_t n = getNumCols() - numJoinAttr1;
            MatrixType matC{m, numJoinAttr1 + n };
            for (uint32_t rowIdx = 0; rowIdx < m; rowIdx++)
            {
                for (uint32_t colIdx = 0; colIdx < getNumCols(); colIdx++)
                {
                    if (colIdx < numJoinAttr1)
                    {
                        matC[rowIdx][colIdx] = matA[rowIdx][colIdx];
                    }
                    else
                    {
                        matC[rowIdx][colIdx] = matA[rowIdx][colIdx] +
                        second[rowIdx][colIdx - numJoinAttr1 + numJoinAttr2];
                    }
                }
            }
            return matC;
        }


        MatrixType subtract(const MatrixType& second,
            uint32_t numJoinAttr1, uint32_t numJoinAttr2) const
        {
            auto& matA = *this;
            uint32_t m = getNumRows();
            uint32_t n = getNumCols() - numJoinAttr1;
            MatrixType matC{m, numJoinAttr1 + n };
            for (uint32_t rowIdx = 0; rowIdx < m; rowIdx++)
            {
                for (uint32_t colIdx = 0; colIdx < getNumCols(); colIdx++)
                {
                    if (colIdx < numJoinAttr1)
                    {
                        matC[rowIdx][colIdx] = matA[rowIdx][colIdx];
                    }
                    else
                    {
                        matC[rowIdx][colIdx] = matA[rowIdx][colIdx] -
                        second[rowIdx][colIdx - numJoinAttr1 + numJoinAttr2];
                    }
                }
            }
            return matC;
        }


        MatrixType multiply(const MatrixType& second,
            uint32_t numJoinAttr1, uint32_t numJoinAttr2,
            uint32_t startRowIdx1 = 0) const
        {
            // TODO: Based on type generate different code
            uint32_t m = getNumRows();
            uint32_t n = second.getNumCols() - numJoinAttr2;
            uint32_t k = getNumCols() - numJoinAttr1;
            MatrixType matC{m, n + numJoinAttr1};
            double* pC = matC.getArrPt() + numJoinAttr1;
            auto& matA = *this;

            uint32_t ldA = getNumCols();
            uint32_t ldB = second.getNumCols();
            uint32_t ldC = n + numJoinAttr1;

            const double* pA = getArrPt() + numJoinAttr1;
            const double* pB = second.getArrPt() + startRowIdx1 * ldB + numJoinAttr2;

            cblas_dgemm(CBLAS_ORDER::CblasRowMajor, CblasNoTrans, CblasNoTrans,
                m, n, k, 1.0, pA, ldA, pB, ldB, 0.0,
                    pC, ldC);
            for (uint32_t rowIdx = 0; rowIdx < getNumRows(); rowIdx++)
            {
                for (uint32_t colIdx = 0; colIdx < numJoinAttr1; colIdx++)
                {
                    matC[rowIdx][colIdx] = matA[rowIdx][colIdx];
                }
            }
            return matC;
        }

        MatrixType selfMatrixMultiply(
            uint32_t numJoinAttr) const
        {
            // TODO: Based on type generate different code
            uint32_t m = getNumCols() - numJoinAttr;
            uint32_t n = getNumCols() - numJoinAttr;
            uint32_t k = getNumRows();
            MatrixType matC{m, m};
            double* pC = matC.getArrPt();
            uint32_t ldA = getLeadingDimension();
            uint32_t ldB = getLeadingDimension();
            uint32_t ldC = matC.getLeadingDimension();
            CBLAS_LAYOUT cBlasMemLayout = getCblasMajorOrder();

            const double* pA = getArrPt() + numJoinAttr;
            const double* pB = getArrPt() + numJoinAttr;

            // TODO: Add fix for numJoinAttr

            cblas_dgemm(cBlasMemLayout, CblasTrans, CblasNoTrans,
                m, n, k, 1.0, pA, ldA, pB, ldB, 0.0, pC, ldC);
            return matC;
        }

        double norm(uint32_t numJoinAttr) const
        {
            uint32_t m = getNumRows();
            uint32_t n = getNumCols() - numJoinAttr;
            uint32_t ldA = getNumCols();
            const double* pA = getArrPt() + numJoinAttr;
            double normVal = LAPACKE_dlange(LAPACK_ROW_MAJOR, 'f', m, n, pA, ldA);
            return normVal;
        }

        // Changes the size of matrix while keeping the data.
        void resize(uint32_t newNumRows)
        {
            uint32_t oldNumRows = m_numRows;
            m_numRows = newNumRows;
            uint64_t newNumEntries = getNumEntries();
            if (nullptr == m_pStorage)
            {
                m_pStorage = new ArrayStorage<T>(newNumEntries);
            }
            else
            {
                if constexpr (L == MemoryLayout::ROW_MAJOR)
                {
                    m_pStorage->resize(newNumEntries);
                }
                else
                {
                    ArrayStorage<T>* pNewStorage = new ArrayStorage<T>(newNumEntries);
                    for (uint32_t rowIdx = 0; rowIdx < m_numRows; rowIdx ++)
                    {
                        for (uint32_t colIdx = 0; colIdx < m_numCols; colIdx++)
                        {
                            uint64_t oldStorageIdx = (uint64_t)colIdx * (uint64_t)oldNumRows
                                + (uint64_t)rowIdx;
                            uint64_t newStorageIdx = (uint64_t)colIdx * (uint64_t)m_numRows
                                + (uint64_t)rowIdx;

                            (*pNewStorage)[newStorageIdx] = (*m_pStorage)[oldStorageIdx];
                        }
                    }
                    delete m_pStorage;
                    m_pStorage = pNewStorage;
                }
            }
        }

        uint32_t getNumRows(void) const
        {
            return m_numRows;
        }

        uint32_t getNumCols(void) const
        {
            return m_numCols;
        }

        void setToZeros(void)
        {
            m_pStorage->setToZeros();
        }

        friend std::ostream& operator<<(std::ostream& out, const MatrixType& m)
        {
            out << "Figaro matrix" << std::endl;

            uint32_t rowNum;
            uint32_t colNum;

            colNum = m.getNumCols();
            rowNum = m.getNumRows();

            out << "Matrix" << std::endl;
            out <<  "Matrix dimensions: " << rowNum << " " << colNum << std::endl;

            for (uint32_t row = 0; row < rowNum; row ++)
            {
                for (uint32_t col = 0; col < colNum; col++)
                {

                    out << m(row, col);
                    if (col != (colNum - 1))
                    {
                        out << " ";
                    }
                }
                out << std::endl;
            }
            return out;
        }


        MatrixType getBlock(uint32_t rowIdxBegin, uint32_t rowIdxEnd,
                        uint32_t colIdxBegin, uint32_t colIdxEnd) const
        {
            MatrixType tmp(rowIdxEnd - rowIdxBegin + 1, colIdxEnd-colIdxBegin + 1);
            for (uint32_t rowIdx = rowIdxBegin; rowIdx <= rowIdxEnd; rowIdx++)
            {
                for (uint32_t colIdx = colIdxBegin; colIdx <= colIdxEnd; colIdx++)
                {
                    tmp[rowIdx - rowIdxBegin][colIdx - colIdxBegin] = (*this)[rowIdx][colIdx];
                }
            }
            return tmp;
        }

        MatrixType getRightCols(uint32_t numCols) const
        {
            return getBlock(0, m_numRows - 1, m_numCols - numCols, m_numCols - 1);
        }

        MatrixType getLeftCols(uint32_t numCols) const
        {
            return getBlock(0, m_numRows - 1, 0, numCols - 1);
        }

        MatrixType getTopRows(uint32_t numRows) const
        {
            return getBlock(0, numRows - 1, 0, m_numCols - 1);
        }

        MatrixType getBottomRows(uint32_t numRows) const
        {
            return getBlock(m_numRows - numRows, m_numRows - 1, 0, m_numCols - 1);
        }

        static MatrixType zeros(uint32_t numRows, uint32_t numCols)
        {
            MatrixType m(numRows, numCols);
            m.m_pStorage->setToZeros();
            return m;
        }

        static MatrixType identity(uint32_t numRows)
        {
            MatrixType matEye(numRows, numRows);
            matEye.m_pStorage->setToZeros();
            for (uint32_t rowIdx = 0; rowIdx < numRows; rowIdx++)
            {
                matEye[rowIdx][rowIdx] = 1;
            }
            return matEye;
        }



        // TODO: parallelization


        MatrixType concatenateHorizontally(const MatrixType& m) const
        {
            FIGARO_LOG_ASSERT(getNumRows() == m.getNumRows());
            MatrixType tmp(m_numRows, m_numCols + m.m_numCols);
            auto& thisRef = *this;

            FIGARO_LOG_DBG("Entered concatenateHorizontally")
            for (uint32_t rowIdx = 0; rowIdx < m_numRows; rowIdx++)
            {
                for (uint32_t colIdx = 0; colIdx < m_numCols; colIdx++)
                {
                    tmp[rowIdx][colIdx] = thisRef[rowIdx][colIdx];
                }
                for (uint32_t colIdx = 0; colIdx < m.m_numCols; colIdx++)
                {
                    tmp[rowIdx][m_numCols + colIdx] = m[rowIdx][colIdx];
                }
            }
            FIGARO_LOG_DBG("Exited concatenateHorizontally")
            return tmp;
        }

        MatrixType concatenateVertically(const MatrixType& m) const
        {
            FIGARO_LOG_ASSERT(m_numCols == m.m_numCols);
            MatrixType tmp(m_numRows + m.m_numRows, m_numCols);
            auto& thisRef = *this;

            for (uint32_t rowIdx = 0; rowIdx < m_numRows; rowIdx++)
            {
                for (uint32_t colIdx = 0; colIdx < m_numCols; colIdx++)
                {
                    tmp[rowIdx][colIdx] = thisRef[rowIdx][colIdx];
                }
            }

            for (uint32_t rowIdx = 0; rowIdx < m.m_numRows; rowIdx++)
            {
                for (uint32_t colIdx = 0; colIdx < m_numCols; colIdx++)
                {
                    tmp[rowIdx + m_numRows][colIdx] = m[rowIdx][colIdx];
                }
            }
            return tmp;
        }

        MatrixType concatenateHorizontallyScalar(T scalar, uint32_t numCols) const
        {
            MatrixType tmp(m_numRows, m_numCols + numCols);
            auto& thisRef = *this;

            FIGARO_LOG_DBG("Entered concatenateHorizontallyScalar")
            for (uint32_t rowIdx = 0; rowIdx < m_numRows; rowIdx++)
            {
                for (uint32_t colIdx = 0; colIdx < m_numCols; colIdx++)
                {
                    tmp[rowIdx][colIdx] = thisRef[rowIdx][colIdx];
                }
                for (uint32_t colIdx = 0; colIdx < numCols; colIdx++)
                {
                    tmp[rowIdx][m_numCols + colIdx] = scalar;
                }
            }
            FIGARO_LOG_DBG("FInished concatenateHorizontallyScalar")
            return tmp;
        }

        MatrixType concatenateVerticallyScalar(T scalar, uint32_t numRows) const
        {
            MatrixType tmp(m_numRows + numRows, m_numCols);
            auto& thisRef = *this;

            for (uint32_t rowIdx = 0; rowIdx < m_numRows; rowIdx++)
            {
                for (uint32_t colIdx = 0; colIdx < m_numCols; colIdx++)
                {
                    tmp[rowIdx][colIdx] = thisRef[rowIdx][colIdx];
                }
            }

            for (uint32_t rowIdx = 0; rowIdx < numRows; rowIdx++)
            {
                for (uint32_t colIdx = 0; colIdx < m_numCols; colIdx++)
                {
                    tmp[rowIdx + m_numRows][colIdx] = scalar;
                }
            }
            return tmp;
        }


        void copyBlockToThisMatrix(const MatrixType& matSource,
            uint32_t rowSrcBeginIdx, uint32_t rowSrcEndIdx,
            uint32_t colSrcBeginIdx, uint32_t colSrcEndIdx,
            uint32_t rowDstBeginIdx, uint32_t colDstBeginIdx)
        {
            auto& matA = *this;

            if constexpr (L == MemoryLayout::ROW_MAJOR)
            {
                for (uint32_t rowIdxSrc = rowSrcBeginIdx; rowIdxSrc <= rowSrcEndIdx; rowIdxSrc++)
                {
                    for (uint32_t colIdxSrc = colSrcBeginIdx; colIdxSrc <= colSrcEndIdx; colIdxSrc++)
                    {
                        uint32_t colIdxDst = colIdxSrc - colSrcBeginIdx + colDstBeginIdx;
                        uint32_t rowIdxDst = rowIdxSrc - rowSrcBeginIdx + rowDstBeginIdx;
                        matA(rowIdxDst, colIdxDst) = matSource(rowIdxSrc, colIdxSrc);
                    }
                }
            }
            else
            {
                for (uint32_t colIdxSrc = colSrcBeginIdx; colIdxSrc <= colSrcEndIdx; colIdxSrc++)
                {
                    for (uint32_t rowIdxSrc = rowSrcBeginIdx; rowIdxSrc <= rowSrcEndIdx; rowIdxSrc++)
                    {
                        uint32_t colIdxDst = colIdxSrc - colSrcBeginIdx + colDstBeginIdx;
                        uint32_t rowIdxDst = rowIdxSrc - rowSrcBeginIdx + rowDstBeginIdx;
                        matA(rowIdxDst, colIdxDst) = matSource(rowIdxSrc, colIdxSrc);
                    }
                }
            }
        }

        void copyBlockToThisMatrixFromCol( Matrix<T, MemoryLayout::COL_MAJOR>& matSource,
            uint32_t rowSrcBeginIdx, uint32_t rowSrcEndIdx,
            uint32_t colSrcBeginIdx, uint32_t colSrcEndIdx,
            uint32_t rowDstBeginIdx, uint32_t colDstBeginIdx)
        {
            auto& matA = *this;
            for (uint32_t rowIdxSrc = rowSrcBeginIdx; rowIdxSrc <= rowSrcEndIdx; rowIdxSrc++)
            {
                for (uint32_t colIdxSrc = colSrcBeginIdx; colIdxSrc <= colSrcEndIdx; colIdxSrc++)
                {
                    uint32_t colIdxDst = colIdxSrc - colSrcBeginIdx + colDstBeginIdx;
                    uint32_t rowIdxDst = rowIdxSrc - rowSrcBeginIdx + rowDstBeginIdx;
                    matA(rowIdxDst, colIdxDst) = matSource(rowIdxSrc, colIdxSrc) ;
                }
            }
        }

          /**
         *
         * @return MatrixType
         */
        MatrixType computeInverse(uint32_t numJoinAttr = 0, bool isTriangular = false) const
        {
            MatrixType inverse {m_numRows, m_numCols};
            inverse.copyBlockToThisMatrix(*this,
                0, m_numRows - 1, 0, m_numCols - 1, 0, 0);
            uint32_t M = m_numRows;
            uint32_t N = m_numCols;
            uint32_t ldA = getLeadingDimension();
            uint32_t rank = std::min(M, N);
            double* pA = inverse.getArrPt();
            CBLAS_LAYOUT cBlasMemLayout = getCblasMajorOrder();

            if (isTriangular)
            {
                LAPACKE_dtrtri(cBlasMemLayout, 'U', 'N', N, pA, ldA);
            }
            else
            {
                long long int* pIpivot = new long long int[rank];
                LAPACKE_dgetrf(cBlasMemLayout, M, N,
                    pA, ldA, pIpivot);

                LAPACKE_dgetri(cBlasMemLayout, M,
                    pA, ldA, pIpivot);
                delete [] pIpivot;
            }

            return inverse;
        }


        void computeGivensRotation(T a, T b, T& cos, T& sin, T& r)
        {
            if (b == 0.0)
            {
                cos = boost::math::sign(a);
                sin = 0;
                r = std::abs(a);
            }
            else if (a == 0.0)
            {
                cos = 0.0;
                sin = -boost::math::sign(b);
                r = std::abs(b);
            }
            else if (std::abs(a) > std::abs(b))
            {
                T t = b / a;
                T u = boost::math::sign(a) *
                    std::abs(std::sqrt(1 + t * t));
                cos = 1 / u;
                sin = -cos * t;
                r = a * u;
            }
            else
            {
                T t = a / b;
                T u = boost::math::sign(b) * std::abs(std::sqrt(1 + t * t));
                sin = - 1 / u;
                cos = -sin * t;
                r = b * u;
            }
        }


        void applyGivens(uint32_t rowIdxUpper, uint32_t rowIdxLower, uint32_t startColIdx,
                         T sin, T cos)
        {
            auto& matA = *this;

            T tmpUpperVal;
            T tmpLowerVal;

            tmpUpperVal = matA[rowIdxUpper][startColIdx];
            tmpLowerVal = matA[rowIdxLower][startColIdx];
            matA[rowIdxUpper][startColIdx] = cos * tmpUpperVal - sin * tmpLowerVal;
            matA[rowIdxLower][startColIdx] = 0;

            for (uint32_t colIdx = startColIdx + 1; colIdx < matA.m_numCols; colIdx++)
            {
                tmpUpperVal = matA[rowIdxUpper][colIdx];
                tmpLowerVal = matA[rowIdxLower][colIdx];
                matA[rowIdxUpper][colIdx] = cos * tmpUpperVal - sin * tmpLowerVal;
                matA[rowIdxLower][colIdx] = sin * tmpUpperVal + cos * tmpLowerVal;
            }
        }


        void computeQRGivensSequentialBlockBottom(
            uint32_t rowBeginIdx,
            uint32_t rowEndIdx,
            uint32_t colBeginIdx,
            uint32_t colEndIdx)
        {
            auto& matA = *this;
            for (uint32_t colIdx = colBeginIdx; colIdx <= colEndIdx; colIdx++)
            {
                for (uint32_t rowIdx = rowEndIdx;
                    rowIdx > colIdx + rowBeginIdx; rowIdx--)
                {
                    T upperVal = matA[rowIdx - 1][colIdx];
                    T lowerVal = matA[rowIdx][colIdx];
                    T cos, sin, r;
                    if (lowerVal == 0.0)
                    {
                        continue;
                    }
                    computeGivensRotation(upperVal, lowerVal, cos, sin, r);
                    if (std::abs(r) > 0.0)
                    {
                        applyGivens(rowIdx - 1, rowIdx, colIdx, sin, cos);
                    }
                }
            }
        }


        void computeQRGivensSequentialBlockDiag(
            uint32_t rowBeginIdx,
            uint32_t rowEndIdx,
            uint32_t colBeginIdx,
            uint32_t colEndIdx)
        {
            auto& matA = *this;
            for (uint32_t colIdx = colBeginIdx; colIdx <= colEndIdx; colIdx++)
            {
                /*for (uint32_t rowIdx = rowEndIdx;
                    rowIdx > colIdx + rowBeginIdx; rowIdx--)*/
                for (uint32_t rowIdx = colIdx + rowBeginIdx + 1;
                    rowIdx <= rowEndIdx; rowIdx++)
                {
                    T upperVal = matA[colIdx + rowBeginIdx][colIdx];
                    T lowerVal = matA[rowIdx][colIdx];
                    T cos, sin, r;
                    if (lowerVal == 0.0)
                    {
                        continue;
                    }
                    computeGivensRotation(upperVal, lowerVal, cos, sin, r);
                    if (std::abs(r) > 0.0)
                    {
                        applyGivens(colIdx + rowBeginIdx, rowIdx, colIdx, sin, cos);
                    }
                }
            }
        }


        void computeQRGivensParallelBlockBottom(
            uint32_t rowBeginIdx,
            uint32_t rowEndIdx,
            uint32_t colBeginIdx,
            uint32_t colEndIdx,
            uint32_t numThreads)
        {
            auto& matA = *this;
            uint32_t rowStartIdx;
            uint32_t numBatches;
            uint32_t batchSize;
            uint32_t numRows;
            uint32_t numCols;

            numRows = rowEndIdx - rowBeginIdx + 1;
            numCols = colEndIdx - colBeginIdx + 1;
            batchSize = numThreads;
            omp_set_num_threads(numThreads);
            // Ceil division
            numBatches = (numCols + batchSize - 1) / batchSize;
            // Needed for handling the case of fat tables (number of rows less than number of cols).
            if (numRows >= numCols)
            {
                rowStartIdx = rowEndIdx;
            }
            else
            {
                rowStartIdx = colEndIdx;
            }

            for (uint32_t batchIdx = 0; batchIdx < numBatches; batchIdx++)
            {
                #pragma omp parallel
                {
                    uint32_t threadId;
                    uint32_t colIdx;
                    threadId = omp_get_thread_num();
                    colIdx = colBeginIdx + batchIdx * batchSize + threadId;

                    // Each thread will get equal number of rows to process.
                    // Although some threads will do dummy processing in the begining
                    for (uint32_t rowIdx = rowStartIdx + 2 * threadId; rowIdx > colIdx; rowIdx--)
                    {
                        if ((rowIdx <= rowEndIdx) && (colIdx <= colEndIdx) && (colIdx < rowEndIdx - 1))
                        {
                            T upperVal = matA[rowIdx - 1][colIdx];
                            T lowerVal = matA[rowIdx][colIdx];
                            T cos, sin, r;
                            if (lowerVal != 0.0)
                            {
                                computeGivensRotation(upperVal, lowerVal, cos, sin, r);
                                if (std::abs(r) > 0.0)
                                {
                                    applyGivens(rowIdx - 1, rowIdx, colIdx, sin, cos);
                                }
                            }

                        }
                        #pragma omp barrier
                    }
                    // Extra dummy loops needed for barier synchronization.
                    for (uint32_t idx = 0; idx < batchSize - threadId - 1; idx++)
                    {
                        #pragma omp barrier
                    }
                }
            }
        }

        void computeQRGivensParallelBlockDiag(
            uint32_t rowBeginIdx,
            uint32_t rowEndIdx,
            uint32_t colBeginIdx,
            uint32_t colEndIdx,
            uint32_t numThreads)
        {
            auto& matA = *this;
            uint32_t rowStartIdx;
            uint32_t numBatches;
            uint32_t batchSize;
            uint32_t numRows;
            uint32_t numCols;

            numRows = rowEndIdx - rowBeginIdx + 1;
            numCols = colEndIdx - colBeginIdx + 1;
            batchSize = numThreads;
            omp_set_num_threads(numThreads);
            // Ceil division
            numBatches = (numCols + batchSize - 1) / batchSize;
            // Needed for handling the case of fat tables (number of rows less than number of cols).
            if (numRows >= numCols)
            {
                rowStartIdx = rowEndIdx;
            }
            else
            {
                rowStartIdx = colEndIdx;
            }

            for (uint32_t batchIdx = 0; batchIdx < numBatches; batchIdx++)
            {
                #pragma omp parallel
                {
                    uint32_t threadId;
                    uint32_t colIdx;
                    threadId = omp_get_thread_num();
                    colIdx = colBeginIdx + batchIdx * batchSize + threadId;

                    // Each thread will get equal number of rows to process.
                    // Although some threads will do dummy processing in the begining
                    //for (uint32_t rowIdx = rowStartIdx + 2 * threadId; rowIdx > colIdx; rowIdx--)
                    for (int32_t rowIdx = (int32_t)colIdx + 1 - 2 * (int32_t)threadId;
                        rowIdx <= (int32_t)rowStartIdx; rowIdx++)
                    {
                        if ((colIdx <= colEndIdx) && (rowIdx <= (int32_t)rowEndIdx) &&
                        (rowIdx > (int32_t)colIdx) )
                        {
                            T upperVal = matA[colIdx + rowBeginIdx][colIdx];
                            T lowerVal = matA[rowIdx][colIdx];
                            T cos, sin, r;
                            if (lowerVal != 0.0)
                            {
                                computeGivensRotation(upperVal, lowerVal, cos, sin, r);
                                if (std::abs(r) > 0.0)
                                {
                                    applyGivens(colIdx + rowBeginIdx, rowIdx, colIdx, sin, cos);
                                }
                            }

                        }
                        #pragma omp barrier
                    }
                    // Extra dummy loops needed for barier synchronization.
                    for (uint32_t idx = 0; idx < batchSize - threadId - 1; idx++)
                    {
                        #pragma omp barrier
                    }
                }
            }
        }

        /**
         * @brief
         *
         * @param numThreads
         *
         */
        void computeQRGivensParallelizedThinMatrix(uint32_t numThreads,
            Figaro::QRHintType qrType)
        {
            uint32_t blockSize;
            uint32_t numBlocks;
            uint32_t numRedRows;
            uint32_t rowTotalEndIdx;
            uint32_t numRedEndRows;

            std::vector<uint32_t> vRowBlockBeginIdx;
            std::vector<uint32_t> vRowRedBlockBeginIdx;
            std::vector<uint32_t> vRowBlockEndIdx;
            std::vector<uint32_t> vRowRedBlockEndIdx;
            std::vector<uint32_t> vRowRedSrcBlockEndIdx;

            auto& matA = *this;

            numBlocks = std::min(m_numRows, numThreads);
            // ceil(m_numRows / numBlocks)
            blockSize =  (m_numRows + numBlocks - 1) / numBlocks;
            numRedRows = std::min(blockSize, m_numCols);
            omp_set_num_threads(numThreads);

            for (uint32_t blockIdx = 0; blockIdx < numBlocks; blockIdx++)
            {
                uint32_t beginBlockIdx = blockIdx * blockSize;
                uint32_t rowRedSrcEndIdx;
                // This is needed for dummy threads case.
                if (beginBlockIdx >= m_numRows)
                {
                    break;
                }
                vRowBlockBeginIdx.push_back(beginBlockIdx);
                vRowRedBlockBeginIdx.push_back(blockIdx * numRedRows);
                vRowBlockEndIdx.push_back(std::min((blockIdx + 1) * blockSize - 1,
                                        m_numRows - 1));
                rowRedSrcEndIdx = std::min(beginBlockIdx + numRedRows - 1,
                                m_numRows - 1);
                vRowRedSrcBlockEndIdx.push_back(rowRedSrcEndIdx);
                vRowRedBlockEndIdx.push_back(vRowRedBlockBeginIdx[blockIdx] +
                    rowRedSrcEndIdx - beginBlockIdx);
            }
            numBlocks = vRowBlockBeginIdx.size();

            FIGARO_LOG_DBG("m_numRows, blockSize", m_numRows, blockSize, numRedRows)
            //MICRO_BENCH_INIT(qrGivensPar)
            //MICRO_BENCH_START(qrGivensPar)
            #pragma omp parallel for schedule(static)
            for (uint32_t blockIdx = 0; blockIdx < numBlocks; blockIdx++)
            {
                uint32_t rowBlockBeginIdx;
                uint32_t rowBlockEndIdx;
                rowBlockBeginIdx = vRowBlockBeginIdx[blockIdx];
                rowBlockEndIdx = vRowBlockEndIdx[blockIdx];
                if (qrType == QRHintType::THIN_BOTTOM)
                {
                    computeQRGivensSequentialBlockBottom(rowBlockBeginIdx,
                        rowBlockEndIdx, 0, m_numCols - 1);
                }
                else if (qrType == QRHintType::THIN_DIAG)
                {
                    computeQRGivensSequentialBlockDiag(rowBlockBeginIdx,
                        rowBlockEndIdx, 0, m_numCols - 1);
                }
            }
            //MICRO_BENCH_STOP(qrGivensPar)
            //FIGARO_LOG_BENCH("Time Parallel", MICRO_BENCH_GET_TIMER_LAP(qrGivensPar))
            FIGARO_LOG_INFO("Number of blocks", numBlocks)
            FIGARO_LOG_DBG("After parallel", matA)
            //MICRO_BENCH_INIT(qrGivensPar2)
            //MICRO_BENCH_START(qrGivensPar2)

            for (uint32_t blockIdx = 0; blockIdx < numBlocks; blockIdx++)
            {
                uint32_t rowRedSrcBeginIdx;
                uint32_t rowRedSrcEndIdx;
                uint32_t rowRedBeginIdx;

                rowRedSrcBeginIdx = vRowBlockBeginIdx[blockIdx];
                rowRedSrcEndIdx = vRowRedSrcBlockEndIdx[blockIdx];
                rowRedBeginIdx = vRowRedBlockBeginIdx[blockIdx];

                copyBlockToThisMatrix(matA, rowRedSrcBeginIdx, rowRedSrcEndIdx,
                    0, m_numCols - 1, rowRedBeginIdx, 0);
            }

            rowTotalEndIdx = vRowRedBlockEndIdx.back();

            if (qrType == QRHintType::THIN_BOTTOM)
            {
                computeQRGivensParallelBlockBottom(0, rowTotalEndIdx, 0, m_numCols - 1, numThreads);
            }
            else if (qrType == QRHintType::THIN_DIAG)
            {
                computeQRGivensParallelBlockDiag(0, rowTotalEndIdx, 0, m_numCols - 1, numThreads);
            }
            //computeQRGivensSequentialBlock(0, rowTotalEndIdx, 0, m_numCols - 1);

            numRedEndRows = std::min(rowTotalEndIdx + 1, m_numCols);
            //FIGARO_LOG_INFO("rowTotalEndIdx, numEndRows", rowTotalEndIdx, numRedEndRows)
            this->resize(numRedEndRows);
            //FIGARO_LOG_INFO("After processing", matA)
            //MICRO_BENCH_STOP(qrGivensPar2)
            //FIGARO_LOG_BENCH("Time sequential", MICRO_BENCH_GET_TIMER_LAP(qrGivensPar2))
        }


        void computeQRGivensParallelizedThickMatrix(uint32_t numThreads, Figaro::QRHintType qrType)
        {
            if (qrType == QRHintType::THICK_DIAG)
            {
                computeQRGivensParallelBlockDiag(0, m_numRows - 1, 0, m_numCols - 1, numThreads);

            }
            else if (qrType == QRHintType::THICK_BOTTOM)
            {
                computeQRGivensParallelBlockBottom(0, m_numRows - 1, 0, m_numCols - 1, numThreads);
            }
            this->resize(std::min(m_numCols, m_numRows));
        }

        constexpr uint32_t getLapackMajorOrder(void) const
        {
            if constexpr (L == MemoryLayout::ROW_MAJOR)
            {
                return LAPACK_ROW_MAJOR;
            }
            else
            {
                return LAPACK_COL_MAJOR;
            }
        }

        constexpr CBLAS_LAYOUT getCblasMajorOrder(void) const
        {
            if constexpr (L == MemoryLayout::ROW_MAJOR)
            {
                return CBLAS_ORDER::CblasRowMajor;
            }
            else
            {
                return CBLAS_ORDER::CblasColMajor;
            }
        }

        constexpr uint32_t getLeadingDimension(void) const
        {
            if constexpr (L == MemoryLayout::ROW_MAJOR)
            {
                return m_numCols;
            }
            else
            {
                return m_numRows;
            }
        }

        void setToZeroBelowMainDiagonal(void)
        {
            MatrixType& matA = *this;
            uint32_t rank = std::min(m_numRows, m_numCols);
            if constexpr (L == MemoryLayout::ROW_MAJOR)
            {
                this->resize(rank);
                for (uint32_t rowIdx = 0; rowIdx < rank; rowIdx++)
                {
                    for (uint32_t colIdx = 0; colIdx < m_numCols; colIdx++)
                    {
                        if (colIdx < rowIdx)
                        {
                            matA(rowIdx, colIdx) = 0;
                        }
                    }
                }
            }
            else
            {
                for (uint32_t colIdx = 0; colIdx < m_numCols; colIdx++)
                {
                    for (uint32_t rowIdx = 0; rowIdx < rank; rowIdx++)
                    {
                        if (colIdx < rowIdx)
                        {
                            matA(rowIdx, colIdx) = 0;
                        }
                    }
                }
            }
        }

        /**
         * @brief If computeQ is set to false, computed R in place will be an upper triangular
         * matrix. If compute Q is set to true, computed R in place will not be
         * upper triangular.
         *
         */
        void computeQRLapack(bool computeQ, bool saveResult,
            MatrixType *pMatR, MatrixType *pMatQ)
        {
            auto& matA = *this;
            uint32_t rank = std::min(getNumRows(), getNumCols());
            double* tau = new double [getNumCols()];
            double* pMat = getArrPt();
            uint32_t ldA;
            uint32_t memLayout = getLapackMajorOrder();

            if constexpr (L == MemoryLayout::ROW_MAJOR)
            {
                ldA = m_numCols;
            }
            else
            {
                ldA = m_numRows;
            }

            LAPACKE_dgeqrf(memLayout, m_numRows, m_numCols,
                pMat /* *a */, ldA,/*lda*/ tau/* tau */);

            if (!computeQ)
            {
                matA.setToZeroBelowMainDiagonal();
            }

            if (saveResult)
            {
                FIGARO_LOG_ASSERT(pMatR != nullptr);
                MatrixType& matR = *pMatR;
                // Copying R to a newly allocated matrix.
                matR = std::move(MatrixType{rank, m_numCols});
                if constexpr (L == MemoryLayout::ROW_MAJOR)
                {
                    for (uint32_t rowIdx = 0; rowIdx < rank; rowIdx++)
                    {
                        for (uint32_t colIdx = 0; colIdx < m_numCols; colIdx++)
                        {
                            if (colIdx < rowIdx)
                            {
                                matR(rowIdx, colIdx) = 0;
                            }
                            else
                            {
                                matR(rowIdx, colIdx) = matA(rowIdx, colIdx); // copy elements.
                            }
                        }
                    }
                }
                else
                {
                    for (uint32_t colIdx = 0; colIdx < m_numCols; colIdx++)
                    {
                        for (uint32_t rowIdx = 0; rowIdx < rank; rowIdx++)
                        {
                            if (colIdx < rowIdx)
                            {
                                matR(rowIdx, colIdx) = 0;
                            }
                            else
                            {
                                matR(rowIdx, colIdx) = matA(rowIdx, colIdx); // copy elements.
                            }
                        }
                    }
                }
            }

            // Copying Q
            if (computeQ)
            {
                LAPACKE_dorgqr(memLayout, m_numRows, rank, rank,
                    pMat, ldA, tau);
                FIGARO_LOG_DBG("matQ", *this);
                if (saveResult)
                {
                    MatrixType& matQ = *pMatQ;
                    FIGARO_LOG_ASSERT(pMatQ != nullptr);
                    matQ = std::move(MatrixType{m_numRows, rank});
                    if constexpr (L == MemoryLayout::ROW_MAJOR)
                    {
                        for (uint32_t rowIdx = 0; rowIdx < m_numRows; rowIdx++)
                        {
                            for (uint32_t colIdx = 0; colIdx < rank; colIdx++)
                            {
                                matQ(rowIdx, colIdx) = matA(rowIdx, colIdx);
                            }
                        }
                    }
                    else
                    {
                        for (uint32_t colIdx = 0; colIdx < rank; colIdx++)
                        {
                            for (uint32_t rowIdx = 0; rowIdx < m_numRows; rowIdx++)
                            {
                                matQ(rowIdx, colIdx) = matA(rowIdx, colIdx);
                            }
                        }
                    }
                }
            }

            delete [] tau;
        }

        /**
         * @brief Computes in-place upper triangular R in QR decomposition for the current matrix.
         * Trailing zero rows are discarded, and the size is adjusted accordingly.
         * @param numThreads denotes number of threads available for the computation in the case of parallelization.
         */
        void computeQR(uint32_t numThreads = 1, bool useHint = false, Figaro::QRHintType qrTypeHint = QRHintType::THIN_DIAG,
        bool computeQ = false, bool saveResult = false,
        MatrixType* pMatR = nullptr, MatrixType* pMatQ = nullptr)
        {
            Figaro::QRHintType qrType;
            if ((0 == m_numRows) || (0 == m_numCols))
            {
                return;
            }
            if (useHint)
            {
                qrType = qrTypeHint;
            }
            else
            {
                if (m_numCols > MIN_COLS_PAR)
                {
                    qrType = QRHintType::THICK_DIAG;
                }
                else
                {
                    qrType = QRHintType::THIN_DIAG;
                }
            }

            if ((qrType == QRHintType::THICK_DIAG) ||
                (qrType == QRHintType::THICK_BOTTOM))
            {
                FIGARO_LOG_INFO("Thick version")
                computeQRGivensParallelizedThickMatrix(numThreads, qrType);
            }
            else if ((qrType == QRHintType::THIN_DIAG) ||
                (qrType == QRHintType::THIN_BOTTOM))
            {
                FIGARO_LOG_INFO("Thin version")
                computeQRGivensParallelizedThinMatrix(numThreads, qrType);
            }
            else if (qrType == QRHintType::HOUSEHOLDER_LAPACK)
            {
                FIGARO_LOG_INFO("HOUSEHOLDER_LAPACK")
                computeQRLapack(computeQ, saveResult, pMatR, pMatQ);
            }
        }

        void computeQRHouseholder(
            bool computeQ = false, bool saveResult = false,
            MatrixType* pMatR = nullptr, MatrixType* pMatQ = nullptr)
        {
            computeQRLapack(computeQ, saveResult, pMatR, pMatQ);
        }

        void computeCholesky(uint32_t numThreads = 1,
            MatrixType* pMatR = nullptr)
        {
            double *pArr = getArrPt();
            uint32_t memLayout = getLapackMajorOrder();
            MatrixType& matA = *this;
            uint32_t ldA;
            if constexpr (L == MemoryLayout::ROW_MAJOR)
            {
                ldA = m_numCols;
            }
            else
            {
                ldA = m_numRows;
            }
            matA.setToZeroBelowMainDiagonal();

            LAPACKE_dpotrf(memLayout, 'U', m_numCols, pArr, ldA);
        }

        void computeQRCholesky(bool computeQ = false, bool saveResult = false,
            MatrixType* pMatR = nullptr, MatrixType* pMatQ = nullptr)
        {
            MatrixType& matR = *pMatR;
            MatrixType& matQ = *pMatQ;
            MatrixType& matA = *this;

            matR = selfMatrixMultiply(0);
            matR.computeCholesky();

            if (computeQ)
            {
                matQ = matA * matR.computeInverse(0, true);
            }
        }


        void computeSingularValueDecomposition(uint32_t numThreads,
            MatrixType* pMatU, MatrixType* pMatS,
            MatrixType* pMatV)
        {
            double *pArr = getArrPt();
            uint32_t memLayout = getLapackMajorOrder();
            uint32_t ldA, ldU, ldvT;
            MatrixType& matU = *pMatU;
            MatrixType& matS = *pMatS;
            MatrixType& matV = *pMatV;

            uint32_t rank;

            rank = std::min(m_numRows, m_numCols);
            if constexpr (L == MemoryLayout::ROW_MAJOR)
            {
                ldA = m_numCols;
                ldU = rank;
            }
            else
            {
                ldA = m_numRows;
                ldU = m_numRows;
            }
            ldvT = rank;
            matU = std::move(MatrixType{m_numRows, rank});
            matS = std::move(MatrixType{rank, 1});
            matV = std::move(MatrixType{rank, m_numCols});

            LAPACKE_dgesdd(memLayout, 'S', m_numRows, m_numCols, pArr, ldA,
                pMatS->getArrPt(), pMatU->getArrPt(), ldU,
                pMatV->getArrPt(), ldvT);
        }


        void makeDiagonalElementsPositiveInR(void)
        {
            auto& matA = *this;
            uint32_t rowEnd = std::min(m_numRows, m_numCols);
            for (uint32_t rowIdx = 0; rowIdx < m_numRows; rowIdx++)
            {
                T signDiag;
                signDiag = boost::math::sign(matA(rowIdx, rowIdx));
                for (uint32_t colIdx = 0; colIdx < m_numCols; colIdx++)
                {
                    matA(rowIdx, colIdx) *= signDiag;
                }
            }
        }


        class RowIterator
        {
            T* m_rowPt = nullptr;
            uint32_t m_numCols = 1;
        public:
            using difference_type = uint32_t;
            using pointer = const T*;
            using reference = const T*;
            using value_type = T*;
            using iterator_category = std::random_access_iterator_tag;

            RowIterator(T* rowPt, uint32_t numCols):
                m_rowPt(rowPt), m_numCols(numCols) {}

            RowIterator& operator+=(difference_type diff)
            {
                m_rowPt += diff * m_numCols;
                return *this;
            }
            RowIterator& operator-=(difference_type diff)
            {
                m_rowPt -= diff * m_numCols;
                return *this;
            }
            T* operator*() const { return m_rowPt; }
            T* operator->() const {return m_rowPt;}
            T* operator[](difference_type rhs) const
            {
                return m_rowPt + rhs * m_numCols;
            }

            RowIterator& operator++() { m_rowPt += m_numCols; return *this;}
            RowIterator& operator--() { m_rowPt -= m_numCols; return *this;}
            RowIterator operator++(int)
            {
                RowIterator tmp = *this;
                ++(*this);
                return tmp;
            }
            RowIterator operator--(int)
            {
                RowIterator tmp =*this;
                --(*this);
                return tmp;
            }
            difference_type operator-(const RowIterator& rhs) const
            {
                return (m_rowPt - rhs.m_rowPt) / m_numCols;
            }
            RowIterator operator+(difference_type diff) const
            {
                return RowIterator(m_rowPt + diff * m_numCols, m_numCols);
            }
            RowIterator operator-(difference_type diff) const
            {
                return RowIterator(m_rowPt - diff * m_numCols, m_numCols);
            }

            friend RowIterator operator+(difference_type diff, const RowIterator& rhs)
            {
                return RowIterator(diff + rhs.m_rowPt, rhs.sm_numCols);
            }
            friend  RowIterator operator-(difference_type diff, const RowIterator& rhs)
            {
                return RowIterator(diff - rhs.m_rowPt, rhs.m_numCols);
            }

            bool operator==(const RowIterator& rhs) const { return m_rowPt == rhs.m_rowPt; }
            bool operator!=(const RowIterator& rhs) const { return m_rowPt != rhs.m_rowPt; }
            bool operator>(const RowIterator& rhs) const {return m_rowPt > rhs.m_rowPt; }
            bool operator<(const RowIterator& rhs) const {return m_rowPt < rhs.m_rowPt; }
            bool operator>=(const RowIterator& rhs) const {return m_rowPt >= rhs.m_rowPt; }
            bool operator<=(const RowIterator& rhs) const {return m_rowPt <= rhs.m_rowPt; }
        };

        RowIterator begin()
        {
            FIGARO_LOG_ASSERT(m_pStorage != nullptr)
            return RowIterator(m_pStorage->getData(), m_numCols);
        }

        RowIterator end()
        {
            FIGARO_LOG_ASSERT(m_pStorage != nullptr)
            return RowIterator(m_pStorage->getData() + m_pStorage->getSize(), m_numCols);
        }
    };
}

#endif