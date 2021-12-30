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

    enum class QRGivensHintType
    {
        THIN_BOTTOM = 0,
        THIN_DIAG = 1,
        THICK_BOTTOM = 2,
        THICK_DIAG = 3,
        LAPACK = 4
    };

    // Row-major order of storing elements of matrix is assumed.
    template <typename T, MemoryLayout L = MemoryLayout::ROW_MAJOR>
    class Matrix
    {
        static constexpr uint32_t MIN_COLS_PAR = UINT32_MAX;
        //static constexpr uint32_t MIN_COLS_PAR = 0;
        uint32_t m_numRows = 0, m_numCols = 0;
        ArrayStorage<T>* m_pStorage = nullptr;
        void destroyData(void)
        {
            m_numRows = 0;
            m_numCols = 0;
            //FIGARO_LOG_DBG("Tried destroying data");
            if (nullptr != m_pStorage)
            {
                //FIGARO_LOG_DBG("Not nullptr", m_pStorage)
                delete m_pStorage;
                m_pStorage = nullptr;
            }
            //FIGARO_LOG_DBG("Destroyed data");
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
            //FIGARO_LOG_DBG("Entered move constructor")
            m_pStorage = other.m_pStorage;
            m_numRows = other.m_numRows;
            m_numCols = other.m_numCols;

            other.m_pStorage = nullptr;
            other.m_numCols = 0;
            other.m_numRows = 0;
            //FIGARO_LOG_DBG("Finished move constructor")
        }
        Matrix& operator=(Matrix&& other)
        {
            FIGARO_LOG_DBG("Entered move assignment")
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
            //FIGARO_LOG_DBG("Finished move assignment")
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

        Matrix<T, L> operator*(const Matrix<T, L>& second) const
        {
            // TODO: Based on type generate different code
            const double* pA = getArrPt();
            const double* pB = second.getArrPt();
            uint32_t m = getNumRows();
            uint32_t n = second.getNumCols();
            uint32_t k = getNumCols();
            Matrix<T, L> matC{m, n};
            double* pC = matC.getArrPt();
            cblas_dgemm(CBLAS_ORDER::CblasRowMajor, CblasNoTrans, CblasNoTrans,
                m, n, k, 1.0, pA, k, pB, n, 0.0, pC, n);
            return matC;
        }

        Matrix<T, L> multiply(const Matrix<T, L>& second,
            uint32_t numJoinAttr1, uint32_t numJoinAttr2,
            uint32_t startRowIdx1 = 0) const
        {
            // TODO: Based on type generate different code
            uint32_t m = getNumRows();
            uint32_t n = second.getNumCols() - numJoinAttr2;
            uint32_t k = getNumCols() - numJoinAttr1;
            Matrix<T, L> matC{m, n + numJoinAttr1};
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

        friend std::ostream& operator<<(std::ostream& out, const Matrix<T, L>& m)
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

                    out << m[row][col];
                    if (col != (colNum - 1))
                    {
                        out << " ";
                    }
                }
                out << std::endl;
            }
            return out;
        }


        Matrix<T, L> getBlock(uint32_t rowIdxBegin, uint32_t rowIdxEnd,
                        uint32_t colIdxBegin, uint32_t colIdxEnd) const
        {
            Matrix<T, L> tmp(rowIdxEnd - rowIdxBegin + 1, colIdxEnd-colIdxBegin + 1);
            for (uint32_t rowIdx = rowIdxBegin; rowIdx <= rowIdxEnd; rowIdx++)
            {
                for (uint32_t colIdx = colIdxBegin; colIdx <= colIdxEnd; colIdx++)
                {
                    tmp[rowIdx - rowIdxBegin][colIdx - colIdxBegin] = (*this)[rowIdx][colIdx];
                }
            }
            return tmp;
        }

        Matrix<T, L> getRightCols(uint32_t numCols) const
        {
            return getBlock(0, m_numRows - 1, m_numCols - numCols, m_numCols - 1);
        }

        Matrix<T, L> getLeftCols(uint32_t numCols) const
        {
            return getBlock(0, m_numRows - 1, 0, numCols - 1);
        }

        Matrix<T, L> getTopRows(uint32_t numRows) const
        {
            return getBlock(0, numRows - 1, 0, m_numCols - 1);
        }

        Matrix<T, L> getBottomRows(uint32_t numRows) const
        {
            return getBlock(m_numRows - numRows, m_numRows - 1, 0, m_numCols - 1);
        }

        static Matrix<T, L> zeros(uint32_t numRows, uint32_t numCols)
        {
            Matrix<T, L> m(numRows, numCols);
            FIGARO_LOG_DBG("Entered zeros")
            m.m_pStorage->setToZeros();
            FIGARO_LOG_DBG("Exited zeros")
            return m;
        }

        // TODO: parallelization


        Matrix<T, L> concatenateHorizontally(const Matrix<T, L>& m) const
        {
            FIGARO_LOG_ASSERT(getNumRows() == m.getNumRows());
            Matrix<T, L> tmp(m_numRows, m_numCols + m.m_numCols);
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

        Matrix<T, L> concatenateVertically(const Matrix<T, L>& m) const
        {
            FIGARO_LOG_ASSERT(m_numCols == m.m_numCols);
            Matrix<T, L> tmp(m_numRows + m.m_numRows, m_numCols);
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

        Matrix<T, L> concatenateHorizontallyScalar(T scalar, uint32_t numCols) const
        {
            Matrix<T, L> tmp(m_numRows, m_numCols + numCols);
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

        Matrix<T, L> concatenateVerticallyScalar(T scalar, uint32_t numRows) const
        {
            Matrix<T, L> tmp(m_numRows + numRows, m_numCols);
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


        void copyBlockToThisMatrix(const Matrix<T, L>& matSource,
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
                    matA[rowIdxDst][colIdxDst] = matSource[rowIdxSrc][colIdxSrc];
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
         * @return Matrix<T, L>
         */
        Matrix<T, L> computeInverse(uint32_t numJoinAttr = 0) const
        {
            Matrix<T, L> inverse {m_numRows, m_numCols};
            inverse.copyBlockToThisMatrix(*this,
                0, m_numRows - 1, 0, m_numCols - 1, 0, 0);
            uint32_t M = m_numRows;
            uint32_t N = m_numCols - numJoinAttr;
            long long int * pIpivot = new long long int[std::min(M, N)];
            uint32_t ldA = N + numJoinAttr;
            double* pA = inverse.getArrPt() + numJoinAttr;
            LAPACKE_dgetrf(
                LAPACK_ROW_MAJOR, M, N,
                pA, ldA, pIpivot);

            LAPACKE_dgetri(LAPACK_ROW_MAJOR, M, pA,
                    ldA, pIpivot);
            delete [ ] pIpivot;
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
            Figaro::QRGivensHintType qrType)
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
                if (qrType == QRGivensHintType::THIN_BOTTOM)
                {
                    computeQRGivensSequentialBlockBottom(rowBlockBeginIdx,
                        rowBlockEndIdx, 0, m_numCols - 1);
                }
                else if (qrType == QRGivensHintType::THIN_DIAG)
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

            if (qrType == QRGivensHintType::THIN_BOTTOM)
            {
                computeQRGivensParallelBlockBottom(0, rowTotalEndIdx, 0, m_numCols - 1, numThreads);
            }
            else if (qrType == QRGivensHintType::THIN_DIAG)
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


        void computeQRGivensParallelizedThickMatrix(uint32_t numThreads, Figaro::QRGivensHintType qrType)
        {
            if (qrType == QRGivensHintType::THICK_DIAG)
            {
                computeQRGivensParallelBlockDiag(0, m_numRows - 1, 0, m_numCols - 1, numThreads);

            }
            else if (qrType == QRGivensHintType::THICK_BOTTOM)
            {
                computeQRGivensParallelBlockBottom(0, m_numRows - 1, 0, m_numCols - 1, numThreads);
            }
            this->resize(std::min(m_numCols, m_numRows));
        }

        void computeQRGivensParallelizedLaPack(bool computeQ)
        {
            auto& matA = *this;
            uint32_t rank = std::min(getNumRows(), getNumCols());
            double* tau = new double [getNumCols()];
            double* pMat = getArrPt();
            if constexpr (L == MemoryLayout::ROW_MAJOR)
            {
                LAPACKE_dgeqrf(
                LAPACK_ROW_MAJOR,
                m_numRows,
                m_numCols,
                pMat /* *a */,
                m_numCols,/*lda*/
                tau/* tau */
                );
                if (computeQ)
                {
                    LAPACKE_dorgqr(LAPACK_ROW_MAJOR, m_numRows, rank, rank,
                        pMat, m_numCols, tau);
                }
            }
            else
            {
                LAPACKE_dgeqrf(
                LAPACK_COL_MAJOR,
                m_numRows,
                m_numCols,
                pMat /* *a */,
                m_numRows,/*lda*/
                tau/* tau */
                );
                if (computeQ)
                {
                    LAPACKE_dorgqr(LAPACK_COL_MAJOR, m_numRows, rank, rank,
                        pMat, m_numRows, tau);
                }
            }

            matA.resize(rank);
            for (uint32_t rowIdx = 0; rowIdx < rank; rowIdx++)
            {
                for (uint32_t colIdx = 0; colIdx < std::min(rowIdx, m_numCols); colIdx++)
                {
                    matA(rowIdx, colIdx) = 0;
                }
            }
            delete [] tau;
        }

        /**
         * @brief Computes in-place upper triangular R in QR decomposition for the current matrix.
         * Trailing zero rows are discarded, and the size is adjusted accordingly.
         * @param numThreads denotes number of threads available for the computation in the case of parallelization.
         */
        void computeQRGivens(uint32_t numThreads = 1, bool useHint = false, Figaro::QRGivensHintType qrTypeHint = QRGivensHintType::THIN_DIAG,
        bool computeQ = false)
        {
            Figaro::QRGivensHintType qrType;
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
                    qrType = QRGivensHintType::THICK_DIAG;
                }
                else
                {
                    qrType = QRGivensHintType::THIN_DIAG;
                }
            }

            if ((qrType == QRGivensHintType::THICK_DIAG) ||
                (qrType == QRGivensHintType::THICK_BOTTOM))
            {
                FIGARO_LOG_INFO("Thick version")
                computeQRGivensParallelizedThickMatrix(numThreads, qrType);
            }
            else if ((qrType == QRGivensHintType::THIN_DIAG) ||
                (qrType == QRGivensHintType::THIN_BOTTOM))
            {
                FIGARO_LOG_INFO("Thin version")
                computeQRGivensParallelizedThinMatrix(numThreads, qrType);
            }
            else if (qrType == QRGivensHintType::LAPACK)
            {
                FIGARO_LOG_INFO("Lapack")
                computeQRGivensParallelizedLaPack(computeQ);
            }
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