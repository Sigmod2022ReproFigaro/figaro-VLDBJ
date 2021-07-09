#ifndef _FIGARO_MATRIX_H_
#define _FIGARO_MATRIX_H_

#include "ArrayStorage.h"
#include "utils/Performance.h"

namespace Figaro
{
    // Row-major order of storing elements of matrix is assumed.
    template <typename T>
    class Matrix
    {
        static constexpr uint32_t MIN_COLS_PAR = 0;
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
    public:
        Matrix(uint32_t numRows, uint32_t numCols)
        {
            m_numRows = numRows;
            m_numCols = numCols;
            m_pStorage = new ArrayStorage<T>(numRows * numCols);
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
            return &((*m_pStorage)[rowIdx * m_numCols]);
        }

        const T* operator[](uint32_t rowIdx) const
        {
            FIGARO_LOG_ASSERT(rowIdx < m_numRows);
            return &((*m_pStorage)[rowIdx * m_numCols]);
        }

        // Changes the size of matrix while keeping the data.
        void resize(uint32_t newNumRows)
        {
            if (nullptr == m_pStorage)
            {
                m_pStorage = new ArrayStorage<T>(newNumRows * m_numCols);
            }
            else
            {
                m_pStorage->resize(newNumRows * m_numCols);
            }
            m_numRows = newNumRows;
        }

        uint32_t getNumRows(void) const
        {
            return m_numRows;
        }

        uint32_t getNumCols(void) const
        {
            return m_numCols;
        }


        Matrix<T> getBlock(uint32_t rowIdxBegin, uint32_t rowIdxEnd,
                        uint32_t colIdxBegin, uint32_t colIdxEnd) const
        {
            Matrix<T> tmp(rowIdxEnd - rowIdxBegin + 1, colIdxEnd-colIdxBegin + 1);
            for (uint32_t rowIdx = rowIdxBegin; rowIdx <= rowIdxEnd; rowIdx++)
            {
                for (uint32_t colIdx = colIdxBegin; colIdx <= colIdxEnd; colIdx++)
                {
                    tmp[rowIdx - rowIdxBegin][colIdx - colIdxBegin] = (*this)[rowIdx][colIdx];
                }
            }
            return tmp;
        }

        Matrix<T> getRightCols(uint32_t numCols) const
        {
            return getBlock(0, m_numRows - 1, m_numCols - numCols, m_numCols - 1);
        }

        Matrix<T> getLeftCols(uint32_t numCols) const
        {
            return getBlock(0, m_numRows - 1, 0, numCols - 1);
        }

        Matrix<T> getTopRows(uint32_t numRows) const
        {
            return getBlock(0, numRows - 1, 0, m_numCols - 1);
        }

        Matrix<T> getBottomRows(uint32_t numRows) const
        {
            return getBlock(m_numRows - numRows, m_numRows - 1, 0, m_numCols - 1);
        }

        friend std::ostream& operator<<(std::ostream& out, const Matrix<T>& m)
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

        static Matrix<T> zeros(uint32_t numRows, uint32_t numCols)
        {
            Matrix<T> m(numRows, numCols);
            FIGARO_LOG_DBG("Entered zeros")
            m.m_pStorage->setToZeros();
            FIGARO_LOG_DBG("Exited zeros")
            return m;
        }

        // TODO: parallelization


        Matrix<T> concatenateHorizontally(const Matrix<T>& m) const
        {
            FIGARO_LOG_ASSERT(getNumRows() == m.getNumRows());
            Matrix<T> tmp(m_numRows, m_numCols + m.m_numCols);
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

        Matrix<T> concatenateVertically(const Matrix<T>& m) const
        {
            FIGARO_LOG_ASSERT(m_numCols == m.m_numCols);
            Matrix<T> tmp(m_numRows + m.m_numRows, m_numCols);
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

        Matrix<T> concatenateHorizontallyScalar(T scalar, uint32_t numCols) const
        {
            Matrix<T> tmp(m_numRows, m_numCols + numCols);
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

        Matrix<T> concatenateVerticallyScalar(T scalar, uint32_t numRows) const
        {
            Matrix<T> tmp(m_numRows + numRows, m_numCols);
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


        void copyBlockToThisMatrix(const Matrix<T>& matSource,
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


        void applyGivens(uint32_t rowIdxUpper, uint32_t rowIdxLower, uint32_t startColIdx,
                         double sin, double cos)
        {
            auto& matA = *this;

            for (uint32_t colIdx = startColIdx; colIdx < matA.m_numCols; colIdx++)
            {
                double tmpUpperVal = matA[rowIdxUpper][colIdx];
                double tmpLowerVal = matA[rowIdxLower][colIdx];
                matA[rowIdxUpper][colIdx] = cos * tmpUpperVal - sin * tmpLowerVal;
                matA[rowIdxLower][colIdx] = sin * tmpUpperVal + cos * tmpLowerVal;
            }
             matA[rowIdxLower][startColIdx] = 0;
        }


        void computeQRGivensSequentialBlock(
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
                    double upperVal = matA[rowIdx - 1][colIdx];
                    double lowerVal = matA[rowIdx][colIdx];
                    double rSquare = upperVal * upperVal + lowerVal * lowerVal;
                    double r = std::sqrt(rSquare);
                    if (rSquare > 0.0)
                    {
                        double sinTheta = -lowerVal / r;
                        double cosTheta = upperVal / r;
                        applyGivens(rowIdx - 1, rowIdx, colIdx, sinTheta, cosTheta);
                    }
                }
            }
        }

        void computeQRGivensSequential(void)
        {
            computeQRGivensSequentialBlock(0, m_numRows - 1, 0, m_numCols - 1);
        }


        /**
         * @brief
         *
         * @param numThreads
         *
         * @pre Each of the blocks on which threads operate have at least
         * number of rows greater than the number of cols. This does not hold
         * only for the last block.
         */
        void computeQRGivensParallelizedThinMatrix(uint32_t numThreads)
        {
            uint32_t blockSize;
            uint32_t numBlocks;
            uint32_t numRedRows;
            uint32_t rowTotalEndIdx;
            auto& matA = *this;

            numBlocks = std::min(m_numRows, numThreads);
            // ceil(m_numRows / numBlocks)
            blockSize =  (m_numRows + numBlocks - 1) / numBlocks;
            numRedRows = std::min(blockSize, m_numCols);
            omp_set_num_threads(numThreads);

            FIGARO_LOG_DBG("m_numRows, blockSize", m_numRows, blockSize, numRedRows)
            MICRO_BENCH_INIT(qrGivensPar)
            MICRO_BENCH_START(qrGivensPar)
            #pragma omp parallel for schedule(static)
            for (uint32_t blockIdx = 0; blockIdx < numBlocks; blockIdx++)
            {
                uint32_t rowBeginIdx;
                uint32_t rowEndIdx;
                rowBeginIdx = blockIdx * blockSize;
                rowEndIdx = std::min((blockIdx + 1) * blockSize - 1, m_numRows - 1);
                computeQRGivensSequentialBlock(rowBeginIdx, rowEndIdx, 0, m_numCols - 1);
            }
            MICRO_BENCH_STOP(qrGivensPar)
            FIGARO_LOG_BENCH("Time Parallel", MICRO_BENCH_GET_TIMER_LAP(qrGivensPar))
            FIGARO_LOG_DBG("After parallel", matA)
            MICRO_BENCH_INIT(qrGivensPar2)
            MICRO_BENCH_START(qrGivensPar2)
            for (uint32_t blockIdx = 0; blockIdx < numBlocks; blockIdx++)
            {
                uint32_t rowBeginIdx;
                uint32_t rowEndIdx;
                rowBeginIdx = blockIdx * blockSize;
                rowEndIdx = std::min(rowBeginIdx + numRedRows - 1, m_numRows - 1);
                copyBlockToThisMatrix(matA, rowBeginIdx, rowEndIdx, 0, m_numCols - 1,
                    blockIdx * numRedRows, 0);
            }
            rowTotalEndIdx = std::min(numBlocks * numRedRows - 1, m_numRows - 1);
            computeQRGivensSequentialBlock(0, rowTotalEndIdx, 0, m_numCols - 1);
            MICRO_BENCH_STOP(qrGivensPar2)
            FIGARO_LOG_BENCH("Time sequential", MICRO_BENCH_GET_TIMER_LAP(qrGivensPar2))
        }


        void computeQRGivensParallelized(uint32_t numThreads)
        {
            auto& matA = *this;
            constexpr double epsilon = 0.0;
            uint32_t numBatches;
            uint32_t batchSize;
            batchSize = numThreads;
            omp_set_num_threads(numThreads);
            // Ceil division
            numBatches = (m_numCols + batchSize - 1) / batchSize;
            for (uint32_t batchIdx = 0; batchIdx < numBatches; batchIdx++)
            {
                #pragma omp parallel
                {
                    uint32_t threadId;
                    uint32_t colIdx;
                    threadId = omp_get_thread_num();
                    colIdx = batchIdx * batchSize + threadId;

                    // Each thread will get equal number of rows to process.
                    // Although some threads will do dummy processing in the begining
                    for (uint32_t rowIdx = m_numRows - 1 + 2 * threadId; rowIdx > colIdx; rowIdx--)
                    {
                        if ((rowIdx <= (m_numRows - 1)) && (colIdx < m_numCols))
                        {
                            double upperVal = matA[rowIdx - 1][colIdx];
                            double lowerVal = matA[rowIdx][colIdx];
                            double r = std::sqrt(upperVal * upperVal + lowerVal * lowerVal);
                            if (r > epsilon)
                            {
                                double sinTheta = -lowerVal / r;
                                double cosTheta = upperVal / r;
                                applyGivens(rowIdx - 1, rowIdx, colIdx, sinTheta, cosTheta);
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

        // numThreads denotes number of threads available for the computation in the case of parallelization.
        void computeQRGivens(uint32_t numThreads = 1)
        {
            if ((0 == m_numRows) || (0 == m_numCols))
            {
                return;
            }
            if (m_numCols > MIN_COLS_PAR)
            {
                FIGARO_LOG_INFO("Parallelized version")
                computeQRGivensParallelized(numThreads);
            }
            else
            {
                FIGARO_LOG_INFO("Sequential version")
                computeQRGivensParallelizedThinMatrix(numThreads);
                //computeQRGivensSequential();
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