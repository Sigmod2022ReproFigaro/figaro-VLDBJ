#include "database/query/visitor/ASTVisitorQueryEval.h"

#include "database/query/visitor/ASTJoinAttributesComputeVisitor.h"
#include "database/query/visitor/ASTComputeDownCountsVisitor.h"
#include "database/query/visitor/ASTComputeUpAndCircleCountsVisitor.h"
#include "database/query/visitor/ASTFigaroFirstPassVisitor.h"
#include "database/query/visitor/ASTFigaroSecondPassVisitor.h"
#include "database/query/visitor/ASTJoinVisitor.h"
#include "database/query/visitor/ASTRightMultiplyVisitor.h"
#include "database/query/node/ASTNodeInverse.h"
#include "omp.h"

namespace Figaro
{
    ASTVisitorQRResult* ASTVisitorQueryEval::visitNodeQRGivens(ASTNodeQRGivens* pElement)
    {
        ASTJoinAttributesComputeVisitor joinAttrVisitor(m_pDatabase, true, m_memoryLayout);
        ASTComputeDownCountsVisitor computeDownVisitor(m_pDatabase);
        ASTComputeUpAndCircleCountsVisitor computeUpAndCircleVisitor(m_pDatabase);
        ASTFigaroFirstPassVisitor figaroFirstPassVisitor(m_pDatabase);

        std::string joinRelName = "";
        std::string qName = "";

        FIGARO_LOG_INFO("VISITING QR GIVENS NODE")

        FIGARO_LOG_INFO("HELPER PREPROCESSING")
        omp_set_num_threads(pElement->getNumThreads());
        // Project away operator.
        m_pDatabase->dropAttributesFromRelations(
            pElement->getDropAttributes());
        pElement->accept(&joinAttrVisitor);

        MICRO_BENCH_INIT(downCnt)
        MICRO_BENCH_INIT(upCnt)
        MICRO_BENCH_INIT(firstPass)
        MICRO_BENCH_INIT(secondPass)
        MICRO_BENCH_INIT(qComp)

        FIGARO_LOG_INFO("JOIN EVALUATION")
        if (pElement->isComputeQ())
        {
            /*
            ASTJoinVisitor astJoinVisitor(m_pDatabase);
            ASTVisitorJoinResult* pResult = (ASTVisitorJoinResult*)pElement->getOperand()->accept(&astJoinVisitor);
            joinRelName = pResult->getJoinRelName();
            delete pResult;
            */
        }
        FIGARO_LOG_INFO("ONE HOT ENCODING")
        m_pDatabase->oneHotEncodeRelations();

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
        ASTVisitorFirstPassResult* pResult =
        (ASTVisitorFirstPassResult*)pElement->accept(&figaroFirstPassVisitor);
        MICRO_BENCH_STOP(firstPass)
        FIGARO_LOG_BENCH("Figaro", "first pass",  MICRO_BENCH_GET_TIMER_LAP(firstPass));

        MICRO_BENCH_START(secondPass)
        ASTFigaroSecondPassVisitor figaroSecondPassVisitor(m_pDatabase, m_qrHintType, m_saveResult, joinRelName, pResult->getHtNamesTmpRels());
        delete pResult;
        ASTVisitorQRResult* pQRrResult = (ASTVisitorQRResult*)pElement->accept(&figaroSecondPassVisitor);
        std::string rName = pQRrResult->getRRelationName();
        delete pQRrResult;
        MICRO_BENCH_STOP(secondPass)
        FIGARO_LOG_BENCH("Figaro", "second pass",  MICRO_BENCH_GET_TIMER_LAP(secondPass));

        if (pElement->isComputeQ())
        {
            MICRO_BENCH_START(qComp)
            ASTNodeRelation* astRNOde =
                new ASTNodeRelation(rName,
                m_pDatabase->getRelationAttributeNames(rName));
            ASTNodeInverse* astRInvNode = new ASTNodeInverse(astRNOde);
            ASTNodeRightMultiply astRightMulNode(pElement->getOperand()->copy(), astRInvNode);
            // Add relation.
            ASTVisitorJoinResult* pQResult =  (ASTVisitorJoinResult*)astRightMulNode.accept(this);
            qName = pQResult->getJoinRelName();
            delete pQResult;
            MICRO_BENCH_STOP(qComp)
            FIGARO_LOG_BENCH("Figaro", "Computation of Q",  MICRO_BENCH_GET_TIMER_LAP(qComp));
        }
        MICRO_BENCH_STOP(main)
        FIGARO_LOG_BENCH("Figaro", "query evaluation",  MICRO_BENCH_GET_TIMER_LAP(main));

        return new ASTVisitorQRResult(rName, qName);
    }

    ASTVisitorQRResult* ASTVisitorQueryEval::visitNodePostProcQR(ASTNodePostProcQR* pElement)
    {
        FIGARO_LOG_INFO("VISITING POST PROC QR NODE")
        ASTJoinAttributesComputeVisitor joinAttrVisitor(m_pDatabase, false, m_memoryLayout);

        omp_set_num_threads(pElement->getNumThreads());
        m_pDatabase->dropAttributesFromRelations(
            pElement->getDropAttributes());
        pElement->accept(&joinAttrVisitor);
        m_pDatabase->oneHotEncodeRelations();
        if (m_memoryLayout == Figaro::MemoryLayout::COL_MAJOR)
        {
            m_pDatabase->changeMemoryLayout();
        }

        MICRO_BENCH_INIT(qrPostprocEval)
        MICRO_BENCH_START(qrPostprocEval)
        auto [rName, qName] =
            m_pDatabase->evalPostprocessing(pElement->getRelationOrder().at(0),
            m_qrHintType, m_memoryLayout, pElement->isComputeQ(), m_saveResult);
        MICRO_BENCH_STOP(qrPostprocEval)
        FIGARO_LOG_BENCH("Postproc eval", MICRO_BENCH_GET_TIMER_LAP(qrPostprocEval))
        return new ASTVisitorQRResult(rName, qName);
    }

    ASTVisitorJoinResult* ASTVisitorQueryEval::visitNodeEvalJoin(ASTNodeEvalJoin* pElement)
    {
        FIGARO_LOG_INFO("VISITING EVAL JOIN NODE")
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

    ASTVisitorJoinResult* ASTVisitorQueryEval::visitNodeRightMultiply(ASTNodeRightMultiply* pElement)
    {
        FIGARO_LOG_INFO("VISITING EVAL RIGHT MULTIPLY NODE")
        ASTJoinAttributesComputeVisitor joinAttrVisitor(m_pDatabase, false, m_memoryLayout);
        //ASTJoinVisitor astJoinVisitor(m_pDatabase);

        MICRO_BENCH_INIT(test)
        MICRO_BENCH_START(test)
        pElement->getLeftOperand()->accept(&joinAttrVisitor);
        /*
        ASTVisitorJoinResult* pJoinResult =
            (ASTVisitorJoinResult*)pElement->getLeftOperand()->accept(&astJoinVisitor);
        delete pJoinResult;
        */
        MICRO_BENCH_STOP(test)
        FIGARO_LOG_BENCH("Timer lap", MICRO_BENCH_GET_TIMER_LAP(test))
        ASTVisitorJoinResult* pMatrix =
            (ASTVisitorJoinResult*)pElement->getRightOperand()->accept(this);

        ASTRightMultiplyVisitor astRMVisitor(m_pDatabase, pMatrix->getJoinRelName());
        ASTVisitorJoinResult* pMatMulResult = (ASTVisitorJoinResult*)pElement->accept(&astRMVisitor);
        std::string newRelName = pMatMulResult->getJoinRelName();
        delete pMatMulResult;

        return new ASTVisitorJoinResult(newRelName);
    }

    ASTVisitorJoinResult* ASTVisitorQueryEval::visitNodeInverse(ASTNodeInverse* pElement)
    {
        FIGARO_LOG_INFO("VISITING INVERSE NODE")
        ASTVisitorJoinResult* pJoinResult = (ASTVisitorJoinResult*)pElement->getOperand()->accept(this);
        std::string invName = m_pDatabase->inverse(pJoinResult->getJoinRelName(), {});
        delete pJoinResult;

        return new ASTVisitorJoinResult(invName);
    }

    ASTVisitorJoinResult* ASTVisitorQueryEval::visitNodeRelation(ASTNodeRelation* pElement)
    {
        return new ASTVisitorJoinResult(pElement->getRelationName());
    }








}