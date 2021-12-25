#include "database/query/visitor/ASTVisitorQueryEval.h"

#include "database/query/visitor/ASTJoinAttributesComputeVisitor.h"
#include "database/query/visitor/ASTComputeDownCountsVisitor.h"
#include "database/query/visitor/ASTComputeUpAndCircleCountsVisitor.h"
#include "database/query/visitor/ASTFigaroFirstPassVisitor.h"
#include "database/query/visitor/ASTFigaroSecondPassVisitor.h"
#include "database/query/visitor/ASTJoinVisitor.h"
#include "omp.h"

namespace Figaro
{
    ASTVisitorAbsResult* ASTVisitorQueryEval::visitNodeQRGivens(ASTNodeQRGivens* pElement)
    {
        ASTJoinAttributesComputeVisitor joinAttrVisitor(m_pDatabase, true, m_memoryLayout);
        ASTComputeDownCountsVisitor computeDownVisitor(m_pDatabase);
        ASTComputeUpAndCircleCountsVisitor computeUpAndCircleVisitor(m_pDatabase);
        ASTFigaroFirstPassVisitor figaroFirstPassVisitor(m_pDatabase);
        ASTFigaroSecondPassVisitor figaroSecondPassVisitor(m_pDatabase, m_qrHintType, m_pMatR);

        omp_set_num_threads(pElement->getNumThreads());
        m_pDatabase->dropAttributesFromRelations(
            pElement->getDropAttributes());
        pElement->accept(&joinAttrVisitor);
        m_pDatabase->oneHotEncodeRelations();

        MICRO_BENCH_INIT(downCnt)
        MICRO_BENCH_INIT(upCnt)
        MICRO_BENCH_INIT(firstPass)
        MICRO_BENCH_INIT(secondPass)
        m_pDatabase->resetComputations();
        MICRO_BENCH_INIT(main)
        MICRO_BENCH_START(main)
        MICRO_BENCH_START(downCnt)
        pElement->accept(&computeDownVisitor);
        MICRO_BENCH_STOP(downCnt)
        MICRO_BENCH_START(upCnt)
        pElement->accept(&computeUpAndCircleVisitor);
        MICRO_BENCH_STOP(upCnt)
        FIGARO_LOG_BENCH("Figaro", "query evaluation down",  MICRO_BENCH_GET_TIMER_LAP(downCnt));
        FIGARO_LOG_BENCH("Figaro", "query evaluation up",  MICRO_BENCH_GET_TIMER_LAP(upCnt));

        MICRO_BENCH_START(firstPass)
        pElement->accept(&figaroFirstPassVisitor);
        MICRO_BENCH_STOP(firstPass)
        FIGARO_LOG_BENCH("Figaro", "first pass",  MICRO_BENCH_GET_TIMER_LAP(firstPass));

        MICRO_BENCH_START(secondPass)
        pElement->accept(&figaroSecondPassVisitor);
        MICRO_BENCH_STOP(secondPass)
        FIGARO_LOG_BENCH("Figaro", "second pass",  MICRO_BENCH_GET_TIMER_LAP(secondPass));
        MICRO_BENCH_STOP(main)
        FIGARO_LOG_BENCH("Figaro", "query evaluation",  MICRO_BENCH_GET_TIMER_LAP(main));
        return nullptr;
    }

    ASTVisitorAbsResult* ASTVisitorQueryEval::visitNodePostProcQR(ASTNodePostProcQR* pElement)
    {
        ASTJoinAttributesComputeVisitor joinAttrVisitor(m_pDatabase, false, m_memoryLayout);

        omp_set_num_threads(pElement->getNumThreads());
        m_pDatabase->dropAttributesFromRelations(
            pElement->getDropAttributes());
        pElement->accept(&joinAttrVisitor);
        if (m_memoryLayout == Figaro::MemoryLayout::COL_MAJOR)
        {
            m_pDatabase->changeMemoryLayout();
        }
        m_pDatabase->oneHotEncodeRelations();

        m_pDatabase->evalPostprocessing(pElement->getRelationOrder().at(0),
            m_qrHintType, m_memoryLayout, pElement->isComputeQ(), m_pMatR);
        return nullptr;
    }

    ASTVisitorJoinResult* ASTVisitorQueryEval::visitNodeEvalJoin(ASTNodeEvalJoin* pElement)
    {
        ASTJoinAttributesComputeVisitor joinAttrVisitor(m_pDatabase, false, m_memoryLayout);
        ASTJoinVisitor astJoinVisitor(m_pDatabase);

        omp_set_num_threads(pElement->getNumThreads());
        m_pDatabase->dropAttributesFromRelations(
            pElement->getDropAttributes());
        pElement->accept(&joinAttrVisitor);
        //m_pDatabase->oneHotEncodeRelations();

        ASTVisitorJoinResult* pJoinResult = (ASTVisitorJoinResult*)pElement->accept(&astJoinVisitor);
        std::string newRelName = pJoinResult->getJoinRelName();
        delete pJoinResult;

        return new ASTVisitorJoinResult(newRelName);
    }
}