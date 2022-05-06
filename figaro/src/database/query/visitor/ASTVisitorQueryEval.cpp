#include "database/query/visitor/ASTVisitorQueryEval.h"

#include "database/query/visitor/ASTVisitorComputeJoinAttributes.h"
#include "database/query/visitor/ASTVisitorBuildIndices.h"
#include "database/query/visitor/figaro/qr/ASTVisitorComputeDownCounts.h"
#include "database/query/visitor/figaro/qr/ASTVisitorComputeUpAndCircleCounts.h"
#include "database/query/visitor/figaro/qr/ASTVisitorQRFigaroFirstPass.h"
#include "database/query/visitor/figaro/qr/ASTVisitorQRFigaroSecondPass.h"
#include "database/query/visitor/figaro/lu/ASTVisitorLUFigaroFirstPass.h"
#include "database/query/visitor/figaro/lu/ASTVisitorLUFigaroSecondPass.h"
#include "database/query/visitor/ASTVisitorJoin.h"
#include "database/query/visitor/ASTVisitorRightMultiply.h"
#include "database/query/node/ASTNodeInverse.h"
#include "omp.h"

namespace Figaro
{
    ASTVisitorResultQR* ASTVisitorQueryEval::visitNodeQRFigaro(ASTNodeQRFigaro* pElement)
    {
        ASTVisitorComputeJoinAttributes joinAttrVisitor(m_pDatabase, true, m_memoryLayout);
        ASTBuildIndicesVisitor buildIndicesVisitor(m_pDatabase);
        ASTVisitorComputeDownCounts computeDownVisitor(m_pDatabase);
        ASTVisitorComputeUpAndCircleCounts computeUpAndCircleVisitor(m_pDatabase);
        ASTVisitorQRFigaroFirstPass figaroFirstPassVisitor(m_pDatabase);

        std::string joinRelName = "";
        std::string rName = "";
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
            ASTVisitorJoin ASTVisitorJoin(m_pDatabase);
            ASTVisitorResultJoin* pResult = (ASTVisitorResultJoin*)pElement->getOperand()->accept(&ASTVisitorJoin);
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

        if (isFlagOn("headsAndTails"))
        {

            //MICRO_BENCH_START(rFirstPassComp)
            ASTVisitorResultFirstPass* pResult =
            (ASTVisitorResultFirstPass*)pElement->accept(&figaroFirstPassVisitor);
            //MICRO_BENCH_STOP(rFirstPassComp)
            //FIGARO_LOG_BENCH("Figaro", "first pass",  MICRO_BENCH_GET_TIMER_LAP(rFirstPassComp));
            ASTVisitorResultQR* pQRrResult = nullptr;
            if (isFlagOn("generalizedHeadsAndTails"))
            {
                bool evalPostProcessing = isFlagOn("postProcessing");
                //MICRO_BENCH_START(rSecondPassComp)
                ASTVisitorQRFigaroSecondPass figaroSecondPassVisitor(m_pDatabase, m_qrHintType,
                    m_saveResult, joinRelName, pResult->getHtNamesTmpRels(),
                    evalPostProcessing);
                delete pResult;
                pQRrResult = (ASTVisitorResultQR*)pElement->accept(&figaroSecondPassVisitor);
                rName = pQRrResult->getRRelationName();
                if (evalPostProcessing)
                {
                    m_pDatabase->persistRelation(rName);
                }
                //MICRO_BENCH_STOP(rSecondPassComp)
                //FIGARO_LOG_BENCH("Figaro", "second pass",  MICRO_BENCH_GET_TIMER_LAP(rSecondPassComp));
            }
            else
            {
                delete pResult;
            }
            MICRO_BENCH_STOP(rComp)
            FIGARO_LOG_BENCH("Figaro", "Computation of R",  MICRO_BENCH_GET_TIMER_LAP(rComp));

            /************* R COMPUTATION END ***********/

            if (m_saveMemory)
            {
                m_pDatabase->destroyAuxRelations();
            }

            delete pQRrResult;
        }

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
            ASTVisitorResultJoin* pQResult =  (ASTVisitorResultJoin*)astRightMulNode.accept(this);
            qName = pQResult->getJoinRelName();
            delete pQResult;
            MICRO_BENCH_STOP(qComp)
            FIGARO_LOG_BENCH("Figaro", "Computation of Q",  MICRO_BENCH_GET_TIMER_LAP(qComp));
        }
        /************* Q COMPUTATION END ***********/


        return new ASTVisitorResultQR(rName, qName);
    }

    ASTVisitorResultQR* ASTVisitorQueryEval::visitNodeQRPostProc(ASTNodeQRPostProc* pElement)
    {
        FIGARO_LOG_INFO("VISITING POST PROC QR NODE")
        ASTVisitorComputeJoinAttributes joinAttrVisitor(m_pDatabase, false, m_memoryLayout);

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
            m_pDatabase->evalQRPostprocessing(pElement->getRelationOrder().at(0),
            m_qrHintType, m_memoryLayout, pElement->isComputeQ(), m_saveResult);
        MICRO_BENCH_STOP(qrPostprocEval)
        FIGARO_LOG_BENCH("Figaro", "Postproc eval", MICRO_BENCH_GET_TIMER_LAP(qrPostprocEval))
        return new ASTVisitorResultQR(rName, qName);
    }


     ASTVisitorResultQR* ASTVisitorQueryEval::visitNodeLUFigaro(ASTNodeLUFigaro* pElement)
     {
        ASTVisitorComputeJoinAttributes joinAttrVisitor(m_pDatabase, true, m_memoryLayout);
        ASTBuildIndicesVisitor buildIndicesVisitor(m_pDatabase);
        ASTVisitorLUFigaroFirstPass figaroFirstPassVisitor(m_pDatabase);

        std::string joinRelName = "";
        std::string lName = "";
        std::string uName = "";

        FIGARO_LOG_INFO("VISITING LU FIGARO NODE")

        FIGARO_LOG_INFO("HELPER PREPROCESSING")
        omp_set_num_threads(pElement->getNumThreads());
        // Project away operator.
        m_pDatabase->dropAttributesFromRelations(
            pElement->getDropAttributes());
        pElement->accept(&joinAttrVisitor);


        MICRO_BENCH_INIT(lIndicesComp)
        MICRO_BENCH_INIT(lFirstPassComp)
        MICRO_BENCH_INIT(lSecondPassComp)
        MICRO_BENCH_INIT(lComp)

        FIGARO_LOG_INFO("JOIN EVALUATION")
        FIGARO_LOG_INFO("ONE HOT ENCODING")
        m_pDatabase->oneHotEncodeRelations();

        /************* U COMPUTATION START ***********/
        MICRO_BENCH_INIT(uComp)
        MICRO_BENCH_START(uComp)

        //MICRO_BENCH_START(lIndicesComp)
        pElement->accept(&buildIndicesVisitor);
        //MICRO_BENCH_STOP(lIndicesComp)

        if (isFlagOn("headsAndTails"))
        {
            //MICRO_BENCH_START(lFirstPassComp)
            ASTVisitorResultFirstPass* pResult =
            (ASTVisitorResultFirstPass*)pElement->accept(&figaroFirstPassVisitor);
            //MICRO_BENCH_STOP(lFirstPassComp)
            //FIGARO_LOG_BENCH("Figaro", "first pass",  MICRO_BENCH_GET_TIMER_LAP(lFirstPassComp));
            ASTVisitorResultQR* pQRrResult = nullptr;
            if (isFlagOn("generalizedHeadsAndTails"))
            {
                bool evalPostProcessing = isFlagOn("postProcessing");
                //MICRO_BENCH_START(lSecondPassComp)
                ASTVisitorLUFigaroSecondPass figaroSecondPassVisitor(m_pDatabase, m_qrHintType, m_saveResult, joinRelName, pResult->getHtNamesTmpRels(), evalPostProcessing);
                delete pResult;
                ASTVisitorResultQR* pQRrResult = (ASTVisitorResultQR*)pElement->accept(&figaroSecondPassVisitor);
                std::string lName = pQRrResult->getRRelationName();
                if (evalPostProcessing)
                {
                    m_pDatabase->persistRelation(lName);
                }
                //MICRO_BENCH_STOP(lSecondPassComp)
                //FIGARO_LOG_BENCH("Figaro", "second pass",  MICRO_BENCH_GET_TIMER_LAP(lSecondPassComp));
            }
            else
            {
                delete pResult;
            }
            MICRO_BENCH_STOP(uComp)
            FIGARO_LOG_BENCH("Figaro", "Computation of U",  MICRO_BENCH_GET_TIMER_LAP(uComp));

            /************* U COMPUTATION END ***********/

            if (m_saveMemory)
            {
                m_pDatabase->destroyAuxRelations();
            }

            delete pQRrResult;

            if (isFlagOn("computeL"))
            {

                MICRO_BENCH_START(lComp)
                ASTNodeRelation* astRNOde =
                    new ASTNodeRelation(lName,
                    m_pDatabase->getRelationAttributeNames(lName));
                ASTNodeInverse* astRInvNode = new ASTNodeInverse(astRNOde);
                ASTNodeRightMultiply astRightMulNode(pElement->getOperand()->copy(), astRInvNode);
                // Add relation.
                ASTVisitorResultJoin* pQResult =  (ASTVisitorResultJoin*)astRightMulNode.accept(this);
                uName = pQResult->getJoinRelName();
                delete pQResult;
                MICRO_BENCH_STOP(lComp)
                FIGARO_LOG_BENCH("Figaro", "Computation of L",  MICRO_BENCH_GET_TIMER_LAP(lComp));
            }
        }



        return new ASTVisitorResultQR(lName, uName);
     }

    ASTVisitorResultJoin* ASTVisitorQueryEval::visitNodeLinReg(ASTNodeLinReg* pElement)
    {
        FIGARO_LOG_INFO("VISITING LIN REG NODE")
        std::string rRelName;

        ASTVisitorComputeJoinAttributes joinAttrVisitor(m_pDatabase, false, m_memoryLayout);
        ASTVisitorJoin ASTVisitorJoin(m_pDatabase);

        omp_set_num_threads(pElement->getNumThreads());
        if (pElement->isFigaro())
        {
            // computation of R
            ASTNodeQRFigaro astQRGivens(
                pElement->getOperand()->copy(),
                pElement->getRelationOrder(),
                pElement->getDropAttributes(),
                pElement->getNumThreads(), false);
            ASTVisitorResultQR* pQrResult = (ASTVisitorResultQR*)astQRGivens.accept(this);
            rRelName = pQrResult->getRRelationName();
            delete pQrResult;
        }
        else
        {
            ASTNodeQRPostProc astNodePostProc(
                pElement->getOperand()->copy(),
                pElement->getRelationOrder(),
                pElement->getDropAttributes(),
                pElement->getNumThreads(), false
            );
            m_saveResult = true;
            ASTVisitorResultQR* pQrResult = (ASTVisitorResultQR*)astNodePostProc.accept(this);
            rRelName = pQrResult->getRRelationName();
            delete pQrResult;
        }

        std::string linRegVec = m_pDatabase->linearRegression(rRelName,
            pElement->getLabelName());
        return new ASTVisitorResultJoin(linRegVec);
    }

    ASTVisitorResultJoin* ASTVisitorQueryEval::visitNodeEvalJoin(ASTNodeEvalJoin* pElement)
    {
        FIGARO_LOG_INFO("VISITING EVAL JOIN NODE")
        ASTVisitorComputeJoinAttributes joinAttrVisitor(m_pDatabase, false, m_memoryLayout);
        ASTVisitorJoin ASTVisitorJoin(m_pDatabase);

        omp_set_num_threads(pElement->getNumThreads());
        m_pDatabase->dropAttributesFromRelations(
            pElement->getDropAttributes());
        pElement->accept(&joinAttrVisitor);
        //m_pDatabase->oneHotEncodeRelations();

        ASTVisitorResultJoin* pJoinResult = (ASTVisitorResultJoin*)pElement->accept(&ASTVisitorJoin);
        std::string newRelName = pJoinResult->getJoinRelName();
        delete pJoinResult;

        return new ASTVisitorResultJoin(newRelName);
    }

    ASTVisitorResultJoin* ASTVisitorQueryEval::visitNodeRightMultiply(ASTNodeRightMultiply* pElement)
    {
        FIGARO_LOG_INFO("VISITING EVAL RIGHT MULTIPLY NODE")
        ASTVisitorComputeJoinAttributes joinAttrVisitor(m_pDatabase, false, m_memoryLayout);

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
        ASTVisitorJoin ASTVisitorJoin(m_pDatabase, true,
            joinAttrVisitor.getPreOrderRelNames(),
            joinAttrVisitor.getPreOrderParRelNames(),
            joinAttrVisitor.getPreOrderVVJoinAttrNames(),
            joinAttrVisitor.getPreOrderVVParJoinAttrNames());
        FIGARO_LOG_INFO("Works up to here")
        ASTVisitorResultJoin* pJoinResult =
            (ASTVisitorResultJoin*)pNodeEvalJoin->accept(&ASTVisitorJoin);
        delete pJoinResult;
        */
        //MICRO_BENCH_STOP(joinTime)
        //FIGARO_LOG_BENCH("Join time", MICRO_BENCH_GET_TIMER_LAP(joinTime))

        ASTVisitorResultJoin* pMatrix =
            (ASTVisitorResultJoin*)pElement->getRightOperand()->accept(this);

        uint32_t joinSize = m_pDatabase->getDownCountSum(joinAttrVisitor.getPreOrderRelNames()[0]);
        ASTVisitorRightMultiply astRMVisitor(m_pDatabase, pMatrix->getJoinRelName(),
            true, joinAttrVisitor.getPreOrderRelNames(), joinAttrVisitor.getPreOrderParRelNames(),
            joinAttrVisitor.getPreOrderVVJoinAttrNames(), joinAttrVisitor.getPreOrderVVParJoinAttrNames(), joinSize);

        ASTVisitorResultJoin* pMatMulResult = (ASTVisitorResultJoin*)pElement->accept(&astRMVisitor);
        std::string newRelName = pMatMulResult->getJoinRelName();
        delete pMatMulResult;

        return new ASTVisitorResultJoin(newRelName);
    }

    ASTVisitorResultJoin* ASTVisitorQueryEval::visitNodeInverse(ASTNodeInverse* pElement)
    {
        FIGARO_LOG_INFO("VISITING INVERSE NODE")
        ASTVisitorResultJoin* pJoinResult = (ASTVisitorResultJoin*)pElement->getOperand()->accept(this);
        std::string invName = m_pDatabase->inverse(pJoinResult->getJoinRelName(), {});
        delete pJoinResult;

        return new ASTVisitorResultJoin(invName);
    }

    ASTVisitorResultJoin* ASTVisitorQueryEval::visitNodeRelation(ASTNodeRelation* pElement)
    {
        return new ASTVisitorResultJoin(pElement->getRelationName());
    }

}