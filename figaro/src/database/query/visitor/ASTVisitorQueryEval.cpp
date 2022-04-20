#include "database/query/visitor/ASTVisitorQueryEval.h"

#include "database/query/visitor/ASTJoinAttributesComputeVisitor.h"
#include "database/query/visitor/ASTBuildIndicesVisitor.h"
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
        ASTBuildIndicesVisitor buildIndicesVisitor(m_pDatabase);
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

        MICRO_BENCH_INIT(rIndicesComp)
        MICRO_BENCH_INIT(rDownCntComp)
        MICRO_BENCH_INIT(rUpCntCompt)
        MICRO_BENCH_INIT(rFirstPassComp)
        MICRO_BENCH_INIT(rSecondPassComp)
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

        /************* R COMPUTATION START ***********/
        MICRO_BENCH_INIT(rComp)
        MICRO_BENCH_START(rComp)

        //MICRO_BENCH_START(rIndicesComp)
        pElement->accept(&buildIndicesVisitor);
        //MICRO_BENCH_STOP(rIndicesComp)

        //MICRO_BENCH_START(rDownCntComp)
        pElement->accept(&computeDownVisitor);
        //MICRO_BENCH_STOP(rDownCntComp)

       // MICRO_BENCH_START(rUpCntCompt)
        pElement->accept(&computeUpAndCircleVisitor);
        //MICRO_BENCH_STOP(rUpCntCompt)

        //FIGARO_LOG_BENCH("Figaro", "query evaluation indices building",  MICRO_BENCH_GET_TIMER_LAP(rIndicesComp));
        //FIGARO_LOG_BENCH("Figaro", "query evaluation down",  MICRO_BENCH_GET_TIMER_LAP(rDownCntComp));
        //FIGARO_LOG_BENCH("Figaro", "query evaluation up",  MICRO_BENCH_GET_TIMER_LAP(rUpCntCompt));

        //MICRO_BENCH_START(rFirstPassComp)
        ASTVisitorFirstPassResult* pResult =
        (ASTVisitorFirstPassResult*)pElement->accept(&figaroFirstPassVisitor);
        //MICRO_BENCH_STOP(rFirstPassComp)
        //FIGARO_LOG_BENCH("Figaro", "first pass",  MICRO_BENCH_GET_TIMER_LAP(rFirstPassComp));

        //MICRO_BENCH_START(rSecondPassComp)
        ASTFigaroSecondPassVisitor figaroSecondPassVisitor(m_pDatabase, m_qrHintType, m_saveResult, joinRelName, pResult->getHtNamesTmpRels());
        delete pResult;
        ASTVisitorQRResult* pQRrResult = (ASTVisitorQRResult*)pElement->accept(&figaroSecondPassVisitor);
        std::string rName = pQRrResult->getRRelationName();
        m_pDatabase->persistRelation(rName);
        //MICRO_BENCH_STOP(rSecondPassComp)
        //FIGARO_LOG_BENCH("Figaro", "second pass",  MICRO_BENCH_GET_TIMER_LAP(rSecondPassComp));

        MICRO_BENCH_STOP(rComp)
        FIGARO_LOG_BENCH("Figaro", "Computation of R",  MICRO_BENCH_GET_TIMER_LAP(rComp));

        /************* R COMPUTATION END ***********/

        m_pDatabase->destroyAuxRelations();

        delete pQRrResult;

        /************* Q COMPUTATION START ***********/
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
        /************* Q COMPUTATION END ***********/


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
        FIGARO_LOG_BENCH("Figaro", "Postproc eval", MICRO_BENCH_GET_TIMER_LAP(qrPostprocEval))
        return new ASTVisitorQRResult(rName, qName);
    }


    ASTVisitorJoinResult* ASTVisitorQueryEval::visitNodeLinReg(ASTNodeLinReg* pElement)
    {
        FIGARO_LOG_INFO("VISITING LIN REG NODE")
        std::string rRelName;

        ASTJoinAttributesComputeVisitor joinAttrVisitor(m_pDatabase, false, m_memoryLayout);
        ASTJoinVisitor astJoinVisitor(m_pDatabase);

        omp_set_num_threads(pElement->getNumThreads());
        if (pElement->isFigaro())
        {
            // computation of R
            ASTNodeQRGivens astQRGivens(
                pElement->getOperand()->copy(),
                pElement->getRelationOrder(),
                pElement->getDropAttributes(),
                pElement->getNumThreads(), false);
            ASTVisitorQRResult* pQrResult = (ASTVisitorQRResult*)astQRGivens.accept(this);
            rRelName = pQrResult->getRRelationName();
            delete pQrResult;
        }
        else
        {
            ASTNodePostProcQR astNodePostProc(
                pElement->getOperand()->copy(),
                pElement->getRelationOrder(),
                pElement->getDropAttributes(),
                pElement->getNumThreads(), false
            );
            m_saveResult = true;
            ASTVisitorQRResult* pQrResult = (ASTVisitorQRResult*)astNodePostProc.accept(this);
            rRelName = pQrResult->getRRelationName();
            delete pQrResult;
        }

        std::string linRegVec = m_pDatabase->linearRegression(rRelName,
            pElement->getLabelName());
        return new ASTVisitorJoinResult(linRegVec);
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

        //MICRO_BENCH_INIT(joinTime)
        //MICRO_BENCH_START(joinTime)
        pElement->getLeftOperand()->accept(&joinAttrVisitor);
        /*
        ASTNodeEvalJoin* pNodeEvalJoin = new ASTNodeEvalJoin(pElement->getLeftOperand()->copy(), {}, {}, 0);

        pNodeEvalJoin->accept(&joinAttrVisitor);

        FIGARO_LOG_INFO("preOrderRelNames", joinAttrVisitor.getPreOrderRelNames())
        FIGARO_LOG_INFO("preOrderParRelNames", joinAttrVisitor.getPreOrderParRelNames())
        FIGARO_LOG_INFO("vvJoinAttrNames", joinAttrVisitor.getPreOrderVVJoinAttrNames())
        FIGARO_LOG_INFO("vvParJoinAttrNames", joinAttrVisitor.getPreOrderVVParJoinAttrNames())
        ASTJoinVisitor astJoinVisitor(m_pDatabase, true,
            joinAttrVisitor.getPreOrderRelNames(),
            joinAttrVisitor.getPreOrderParRelNames(),
            joinAttrVisitor.getPreOrderVVJoinAttrNames(),
            joinAttrVisitor.getPreOrderVVParJoinAttrNames());
        FIGARO_LOG_INFO("Works up to here")
        ASTVisitorJoinResult* pJoinResult =
            (ASTVisitorJoinResult*)pNodeEvalJoin->accept(&astJoinVisitor);
        delete pJoinResult;
        */
        //MICRO_BENCH_STOP(joinTime)
        //FIGARO_LOG_BENCH("Join time", MICRO_BENCH_GET_TIMER_LAP(joinTime))

        ASTVisitorJoinResult* pMatrix =
            (ASTVisitorJoinResult*)pElement->getRightOperand()->accept(this);

        uint32_t joinSize = m_pDatabase->getDownCountSum(joinAttrVisitor.getPreOrderRelNames()[0]);
        ASTRightMultiplyVisitor astRMVisitor(m_pDatabase, pMatrix->getJoinRelName(),
            true, joinAttrVisitor.getPreOrderRelNames(), joinAttrVisitor.getPreOrderParRelNames(),
            joinAttrVisitor.getPreOrderVVJoinAttrNames(), joinAttrVisitor.getPreOrderVVParJoinAttrNames(), joinSize);

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