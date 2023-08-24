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
        int64_t* m_pRows = nullptr;
        int64_t* m_pColInds = nullptr;
        double* m_pVals = nullptr;

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

                long long int numRowsCopy;
                long long int numColsCopy;
                sparse_index_base_t base;
                long long int* pRowsCopy;
                long long int* pRowsCopyEnd;
                long long int* pColIndsCopy;
                double* pValsCopy;
                long long int* pRowsEnd;

                mkl_sparse_d_export_csr(m_pMatrix, &base,
                    &numRowsCopy,  &numColsCopy, &pRowsCopy, &pRowsCopyEnd,
                    &pColIndsCopy, &pValsCopy);
                m_pRows = (int64_t*)pRowsCopy;
                m_pColInds = (int64_t*)pColIndsCopy;
                m_pVals = (double*)pValsCopy;
            }
            else
            {
                mkl_sparse_d_create_csr(&m_pMatrix,
                    sparse_index_base_t::SPARSE_INDEX_BASE_ZERO,
                    numRows,  numCols, (long long int*)pRows, (long long int*)(pRows + 1),
                    (long long int*)pColInds, pVals);
                m_pRows = pRows;
                m_pColInds = pColInds;
                m_pVals = pVals;
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
            m_pRows = other.m_pRows;
            m_pColInds = other.m_pColInds;
            m_pVals = other.m_pVals;

            other.m_pMatrix = nullptr;
            other.m_numCols = 0;
            other.m_numRows = 0;
            other.m_pRows = nullptr;
            other.m_pColInds = nullptr;
            other.m_pVals = nullptr;
        }

        MatrixSparse& operator=(MatrixSparse&& other)
        {
            if (this != &other)
            {
                destroyData();
                m_pMatrix = other.m_pMatrix;
                m_numRows = other.m_numRows;
                m_numCols = other.m_numCols;
                m_pRows = other.m_pRows;
                m_pColInds = other.m_pColInds;
                m_pVals = other.m_pVals;

                other.m_pMatrix = nullptr;
                other.m_numCols = 0;
                other.m_numRows = 0;
                other.m_pRows = nullptr;
                other.m_pColInds = nullptr;
                other.m_pVals = nullptr;
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

        template <typename SolMemType, MemoryLayout SolMemLayout>
        Matrix<SolMemType, SolMemLayout>
        multiply(
            const Matrix<SolMemType, SolMemLayout>& first,
            const Matrix<SolMemType, SolMemLayout>& second,
            uint32_t numJoinAttr1, uint32_t numJoinAttr2,
            uint32_t startRowIdx = 0) const
        {
            Matrix<SolMemType, SolMemLayout> matC{getNumRows(),
                second.getNumCols() - numJoinAttr2 + numJoinAttr1};
            matC.setToZeros();

            #pragma omp parallel for schedule(static)
            for (uint32_t rowIdx = 0; rowIdx < m_numRows; rowIdx++)
            {
                int64_t offsetStartIdx = m_pRows[rowIdx];
                int64_t offestEndIdx = m_pRows[rowIdx+1];
                for (uint32_t offsetIdx = offsetStartIdx;
                    offsetIdx < offestEndIdx; offsetIdx++)
                {
                    int64_t colIdx = m_pColInds[offsetIdx];
                    if (colIdx < numJoinAttr1)
                    {
                        continue;
                    }

                    double val = m_pVals[offsetIdx];
                    for (uint32_t colIdxOut = 0;
                        colIdxOut < second.getNumCols() - numJoinAttr2; colIdxOut++)
                    {
                        int64_t rowIdxIn = startRowIdx + colIdx - numJoinAttr1;
                        int64_t colIdxIn = colIdxOut + numJoinAttr2;

                        matC(rowIdx, colIdxOut + numJoinAttr1) += val * second(rowIdxIn, colIdxIn);
                    }
                }
            }
            if constexpr (SolMemLayout == MemoryLayout::ROW_MAJOR)
            {
                for (uint32_t rowIdx = 0; rowIdx < matC.getNumRows(); rowIdx++)
                {
                    for (uint32_t colIdx = 0; colIdx < numJoinAttr1; colIdx++)
                    {
                        matC(rowIdx, colIdx) = first(rowIdx, colIdx);
                    }
                }
            }
            else
            {
                for (uint32_t colIdx = 0; colIdx < numJoinAttr1; colIdx++)
                {
                    for (uint32_t rowIdx = 0; rowIdx < matC.getNumRows(); rowIdx++)
                    {
                        matC(rowIdx, colIdx) = first(rowIdx, colIdx);
                    }
                }
            }
            return matC;
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

            FIGARO_MIC_BEN_INIT(llSparseQR)
            FIGARO_MIC_BEN_START(llSparseQR)
            status = mkl_sparse_d_qr_solve(SPARSE_OPERATION_NON_TRANSPOSE, m_pMatrix, nullptr,
                                    sparseLayout, matB.getNumCols(), pMatX, ldX, pMatB, ldB);
            FIGARO_MIC_BEN_STOP(llSparseQR)
            FIGARO_LOG_MIC_BEN("LLS Sparse QR", FIGARO_MIC_BEN_GET_TIMER_LAP(llSparseQR));
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
