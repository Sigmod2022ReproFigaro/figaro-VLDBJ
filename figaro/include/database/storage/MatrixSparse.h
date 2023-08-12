#ifndef _FIGARO_MATRIX_SPARSE_H_
#define _FIGARO_MATRIX_SPARSE_H_

#include "utils/Logger.h"
#include "ArrayStorage.h"
#include "utils/Performance.h"
#include <cmath>
#include <mkl.h>
#include <mkl_spblas.h>
#include <random>
#include "utils/Types.h"

namespace Figaro
{
    template <typename T, SparseMemoryLayout Layout = SparseMemoryLayout::CSR>
    class MatrixSparse
    {
        sparse_matrix_t m_pMatrix;
        uint32_t m_numRows;
        uint32_t m_numCols;
    public:
        MatrixSparse(uint32_t numRows, uint32_t numCols,
            int64_t* pRows,
            int64_t* pColInds,
            double* pVals)
        {
            mkl_sparse_d_create_csr(&m_pMatrix,
                sparse_index_base_t::SPARSE_INDEX_BASE_ZERO,
                numRows,  numCols, (long long int*)pRows, (long long int*)(pRows + 1),
                (long long int*)pColInds, pVals);
            m_numRows = numRows;
            m_numCols = numCols;
        }

        ~MatrixSparse()
        {
            mkl_sparse_destroy(m_pMatrix);
        }

        uint32_t getNumRows(void) const
        {
            return m_numRows;
        }

        uint32_t getNumCols(void) const
        {
            return m_numCols;
        }

        void getElems(long long int& numRows, long long int& numCols, long long int*& pRows,
            long long int*& pColInds,
            double*& pVals)
        {
            long long int* pRowsEnd;
            sparse_index_base_t base;
            mkl_sparse_d_export_csr(m_pMatrix,
                &base,
                &numRows,  &numCols, &pRows, &pRowsEnd,
                &pColInds, &pVals);
        }
        /*
        T& operator()(uint32_t rowIdx, uint32_t colIdx)
        {
            FIGARO_LOG_ASSERT(rowIdx < m_numRows);
            FIGARO_LOG_ASSERT(colIdx < m_numCols);
            if constexpr (Layout == SparseMemoryLayout::CSR)
            {
                return (*m_pStorage)[(uint64_t)(rowIdx) * (uint64_t)(m_numCols) + colIdx];
            }
            else
            {
                return (*m_pStorage)[(uint64_t)(colIdx) * (uint64_t)(m_numRows) + rowIdx];
            }
        }
        */

    };
}


#endif
