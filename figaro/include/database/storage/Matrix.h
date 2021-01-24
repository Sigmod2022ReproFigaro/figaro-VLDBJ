#ifndef _FIGARO_MATRIX_H_
#define _FIGARO_MATRIX_H_

#include "ArrayStorage.h"

namespace Figaro
{
    // Row-major order of storing elements of matrix is assumed. 
    template <typename T>
    class Matrix 
    {
        uint32_t m_numRows = 0, m_numCols = 0;
        ArrayStorage<T>* m_pStorage = nullptr; 
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
            return &(*m_pStorage)[rowIdx * m_numCols];
        }

        const T* operator[](uint32_t rowIdx) const
        {
            FIGARO_LOG_ASSERT(rowIdx < m_numRows);
            return &(*m_pStorage)[rowIdx * m_numCols];
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

        void applyGivens(uint32_t rowIdxUpper, uint32_t rowIdxLower, double sin, double cos)
        {
            auto& matA = *this;
            for (uint32_t colIdx = 0; colIdx < matA.m_numCols; colIdx++)
            {
                double tmpUpperVal = matA[rowIdxUpper][colIdx];
                double tmpLowerVal = matA[rowIdxLower][colIdx];
                matA[rowIdxUpper][colIdx] = cos * tmpUpperVal - sin * tmpLowerVal;
                matA[rowIdxLower][colIdx] = sin * tmpUpperVal + cos * tmpLowerVal;
            }
        }

        void computeQRGivens(void)
        {
            auto& matA = *this;
            for (uint32_t colIdx = 0; colIdx < m_numCols; colIdx++)
            {
                for (uint32_t rowIdx = m_numRows -1 ; rowIdx > colIdx; rowIdx--)
                {
                    double upperVal = matA[rowIdx - 1][colIdx];
                    double lowerVal = matA[rowIdx][colIdx];
                    double r = std::sqrt(upperVal * upperVal + lowerVal * lowerVal);
                    if (r > 0.0)
                    {
                        double sinTheta = -lowerVal / r;
                        double cosTheta = upperVal / r;
                        applyGivens(rowIdx - 1, rowIdx, sinTheta, cosTheta);
                    }
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