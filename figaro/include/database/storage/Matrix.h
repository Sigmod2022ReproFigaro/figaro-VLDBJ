#ifndef _FIGARO_MATRIX_H_
#define _FIGARO_MATRIX_H_

#include "ArrayStorage.h"

namespace Figaro
{
    // Row-major order of storing elements of matrix is assumed. 
    template <typename T>
    class Matrix 
    {
        ArrayStorage<T> m_storage; 
        uint32_t m_numRows = 0, m_numCols = 0;
    public:
        Matrix(uint32_t numRows, uint32_t numCols): m_numRows(numRows), m_numCols(numCols), m_storage(numRows * numCols)
        {}

        // No bound checks for colIdx. BE CAREFULL!!!
        T* operator[](uint32_t rowIdx)
        {
            FIGARO_LOG_ASSERT(rowIdx < m_numRows);
            return &m_storage[rowIdx * m_numCols];
        }

        const T* operator[](uint32_t rowIdx) const
        {
            FIGARO_LOG_ASSERT(rowIdx < m_numRows);
            return &m_storage[rowIdx * m_numCols];
        }
        
        // Changes the size of matrix while keeping the data. 
        void resize(uint32_t newNumRows)
        {
            m_storage.resize(newNumRows * m_numCols);
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

        void transferOwnershipTo(Matrix<T>& m)
        {
            m = *this;
            m_numCols = 0;
            m_numCols = 0;
            m_storage = ArrayStorage<T>(0);
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
            return RowIterator(m_storage.getData(), m_numCols);
        }

        RowIterator end()
        {
            return RowIterator(m_storage.getData() + m_storage.getSize(), m_numCols);    
        }
    };

} 

#endif