#ifndef _FIGARO_MATRIX_SPARSE_H_
#define _FIGARO_MATRIX_SPARSE_H_

#include "utils/Logger.h"
#include "Matrix.h"
#include <mkl_spblas.h>
#include "utils/Types.h"

namespace Figaro
{
    template <typename T, MemoryLayout Layout = MemoryLayout::CSR>
    class MatrixSparse
    {
        sparse_matrix_t m_pMatrix = nullptr;
        using MatrixType = MatrixSparse<T, Layout>;
        uint32_t m_numRows;
        uint32_t m_numCols;

        void destroyData(void)
        {
            m_numRows = 0;
            m_numCols = 0;
            if (nullptr != m_pMatrix)
            {
                mkl_sparse_destroy(m_pMatrix);
                m_pMatrix = nullptr;
            }
        }
    public:
        MatrixSparse()
        {
            m_numRows = 0;
            m_numCols = 0;
            m_pMatrix = nullptr;
        }
        MatrixSparse(uint32_t numRows, uint32_t numCols,
            int64_t* pRows,
            int64_t* pColInds,
            double* pVals,
            bool copy = true)
        {
            if (copy)
            {
                sparse_matrix_t pMatrixCopy;
                matrix_descr mDescr;
                mkl_sparse_d_create_csr(&pMatrixCopy,
                    sparse_index_base_t::SPARSE_INDEX_BASE_ZERO,
                    numRows,  numCols, (long long int*)pRows, (long long int*)(pRows + 1),
                    (long long int*)pColInds, pVals);
                mDescr.type = SPARSE_MATRIX_TYPE_GENERAL;
                // Create a copy of data, since the data might be lost

                mkl_sparse_copy(pMatrixCopy, mDescr, &m_pMatrix);
            }
            else
            {
                mkl_sparse_d_create_csr(&m_pMatrix,
                    sparse_index_base_t::SPARSE_INDEX_BASE_ZERO,
                    numRows,  numCols, (long long int*)pRows, (long long int*)(pRows + 1),
                    (long long int*)pColInds, pVals);
            }
            m_numRows = numRows;
            m_numCols = numCols;
        }

        MatrixSparse(const MatrixSparse&) = delete;
        MatrixSparse& operator=(const MatrixSparse&) = delete;

        MatrixSparse(MatrixSparse&& other)
        {
            m_pMatrix = other.m_pMatrix;
            m_numRows = other.m_numRows;
            m_numCols = other.m_numCols;

            other.m_pMatrix = nullptr;
            other.m_numCols = 0;
            other.m_numRows = 0;
        }

        MatrixSparse& operator=(MatrixSparse&& other)
        {
            if (this != &other)
            {
                destroyData();
                m_pMatrix = other.m_pMatrix;
                m_numRows = other.m_numRows;
                m_numCols = other.m_numCols;

                other.m_pMatrix = nullptr;
                other.m_numCols = 0;
                other.m_numRows = 0;
            }
            return *this;
        }

        ~MatrixSparse()
        {
            destroyData();
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
        * https://www.intel.com/content/www/us/en/docs/onemkl/developer-reference-c/2023-0/mkl-sparse-qr.html
        **/
        bool computeQR(void)
        {
            matrix_descr mDescr;
            mDescr.type = SPARSE_MATRIX_TYPE_GENERAL;
            sparse_status_t status;
            FIGARO_MIC_BEN_INIT(reorder)
            FIGARO_MIC_BEN_START(reorder)
            status = mkl_sparse_qr_reorder(m_pMatrix, mDescr);
            FIGARO_MIC_BEN_STOP(reorder)
            FIGARO_LOG_MIC_BEN("Reorder", FIGARO_MIC_BEN_GET_TIMER_LAP(reorder));
            FIGARO_LOG_ASSERT(status == SPARSE_STATUS_SUCCESS)
            if (status != SPARSE_STATUS_SUCCESS)
            {
                FIGARO_LOG_ERROR("Sit and cry")
                return false;
            }

            FIGARO_MIC_BEN_INIT(sparseQr)
            FIGARO_MIC_BEN_START(sparseQr)
            status = mkl_sparse_d_qr_factorize(m_pMatrix, nullptr);
            FIGARO_MIC_BEN_STOP(sparseQr)
            FIGARO_LOG_MIC_BEN("SparseQR", FIGARO_MIC_BEN_GET_TIMER_LAP(sparseQr));
            FIGARO_LOG_ASSERT(status == SPARSE_STATUS_SUCCESS)
            if (status != SPARSE_STATUS_SUCCESS)
            {
                FIGARO_LOG_ERROR("Sit and cry")
                return false;
            }
            return true;
        }


        template <typename SolMemType, MemoryLayout SolMemLayout>
        bool computeLeastSquares(
            Figaro::Matrix<SolMemType, SolMemLayout>& matB,
            Figaro::Matrix<SolMemType, SolMemLayout>& matX)
        {
            /* computeQR*/
            sparse_status_t status;
            double* pMatB = matB.getArrPt();
            double* pMatX = matX.getArrPt();
            uint32_t ldB = matB.getLeadingDimension();
            uint32_t ldX = matX.getLeadingDimension();
            sparse_layout_t sparseLayout;
            FIGARO_LOG_DBG("ldB", ldB)
            FIGARO_LOG_DBG("ldX", ldX)
            FIGARO_LOG_DBG("numCols", matB.getNumCols())
            if constexpr (SolMemLayout == MemoryLayout::ROW_MAJOR)
            {
                sparseLayout = SPARSE_LAYOUT_ROW_MAJOR;
                FIGARO_LOG_DBG("Row major layout")
            }
            else
            {
                sparseLayout = SPARSE_LAYOUT_COLUMN_MAJOR;
                FIGARO_LOG_DBG("Column major layout")
            }

            bool success = computeQR();
            if (!success)
            {
                FIGARO_LOG_ERROR("DAmn")
                return false;
            }

            /*  ( sparse_operation_t operation, sparse_matrix_t A, double *alt_values, sparse_layout_t layout, MKL_INT columns, double *x, MKL_INT ldx, const double *b, MKL_INT ldb );*/
            status = mkl_sparse_d_qr_solve(SPARSE_OPERATION_NON_TRANSPOSE, m_pMatrix, nullptr,
                                    sparseLayout, matB.getNumCols(), pMatX, ldX, pMatB, ldB);
            if (status != SPARSE_STATUS_SUCCESS)
            {
                if (status == SPARSE_STATUS_NOT_INITIALIZED)
                {
                    FIGARO_LOG_ERROR("Not initialized")
                }
                if (status == SPARSE_STATUS_ALLOC_FAILED)
                {
                    FIGARO_LOG_ERROR("Allocation failed")
                }
                if (status == SPARSE_STATUS_INVALID_VALUE)
                {
                    FIGARO_LOG_ERROR("Invalid value")
                }
                if (status == SPARSE_STATUS_INTERNAL_ERROR)
                {
                    FIGARO_LOG_ERROR("Interanl error")
                }

                if (status == SPARSE_STATUS_NOT_SUPPORTED)
                {
                    FIGARO_LOG_ERROR("Not supported")
                }


                return false;
            }
            return true;
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
