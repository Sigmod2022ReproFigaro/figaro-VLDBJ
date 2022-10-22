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

        FIGARO_MIC_BEN_INIT(rIndicesComp)
        FIGARO_MIC_BEN_INIT(rDownCntComp)
        FIGARO_MIC_BEN_INIT(rUpCntCompt)
        FIGARO_MIC_BEN_INIT(rFirstPassComp)
        //FIGARO_MIC_BEN_INIT(rSecondPassComp)
        FIGARO_BENCH_INIT(qComp)

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
        FIGARO_BENCH_INIT(rComp)
        FIGARO_BENCH_START(rComp)

        FIGARO_MIC_BEN_START(rIndicesComp)
        pElement->accept(&buildIndicesVisitor);
        FIGARO_MIC_BEN_STOP(rIndicesComp)

        FIGARO_MIC_BEN_START(rDownCntComp)
        pElement->accept(&computeDownVisitor);
        FIGARO_MIC_BEN_STOP(rDownCntComp)

         FIGARO_MIC_BEN_START(rUpCntCompt)
        pElement->accept(&computeUpAndCircleVisitor);
        FIGARO_MIC_BEN_STOP(rUpCntCompt)

        FIGARO_LOG_MIC_BEN("Figaro", "query evaluation indices building",  FIGARO_MIC_BEN_GET_TIMER_LAP(rIndicesComp));
        FIGARO_LOG_MIC_BEN("Figaro", "query evaluation down",  FIGARO_MIC_BEN_GET_TIMER_LAP(rDownCntComp));
        FIGARO_LOG_MIC_BEN("Figaro", "query evaluation up",  FIGARO_MIC_BEN_GET_TIMER_LAP(rUpCntCompt));

        if (isFlagOn("headsAndTails"))
        {

            FIGARO_MIC_BEN_START(rFirstPassComp)
            ASTVisitorResultFirstPass* pResult =
            (ASTVisitorResultFirstPass*)pElement->accept(&figaroFirstPassVisitor);
            FIGARO_MIC_BEN_STOP(rFirstPassComp)
            FIGARO_LOG_MIC_BEN("Figaro", "first pass",  FIGARO_MIC_BEN_GET_TIMER_LAP(rFirstPassComp));
            ASTVisitorResultQR* pQRrResult = nullptr;
            if (isFlagOn("generalizedHeadsAndTails"))
            {
                bool evalPostProcessing = isFlagOn("postProcessing");
                //FIGARO_MIC_BEN_START(rSecondPassComp)
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
                //FIGARO_MIC_BEN_STOP(rSecondPassComp)
                //FIGARO_LOG_MIC_BEN("Figaro", "second pass",  FIGARO_MIC_BEN_GET_TIMER_LAP(rSecondPassComp));
            }
            else
            {
                delete pResult;
            }
            FIGARO_BENCH_STOP(rComp)
            FIGARO_LOG_BENCH("Figaro", "Computation of R",  FIGARO_BENCH_GET_TIMER(rComp));

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
            FIGARO_LOG_INFO("COMPUTING Q")
            FIGARO_BENCH_START(qComp)
            ASTNodeRelation* astRNOde =
                new ASTNodeRelation(rName,
                m_pDatabase->getRelationAttributeNames(rName));
            ASTNodeInverse* astRInvNode = new ASTNodeInverse(astRNOde);
            ASTNodeRightMultiply astRightMulNode(pElement->getOperand()->copy(), astRInvNode,
                true);
            // Add relation.
            ASTVisitorResultJoin* pQResult =  (ASTVisitorResultJoin*)astRightMulNode.accept(this);
            qName = pQResult->getJoinRelName();
            delete pQResult;
            FIGARO_BENCH_STOP(qComp)
            FIGARO_LOG_BENCH("Figaro", "Computation of Q",  FIGARO_BENCH_GET_TIMER_LAP(qComp));
        }
        FIGARO_LOG_INFO("FINISHED")
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

        //double condA = m_pDatabase->estimateConditionNumber(pElement->getRelationOrder().at(0), {});
        //double normA = m_pDatabase->getNorm(pElement->getRelationOrder().at(0), {});
        //FIGARO_LOG_MIC_BEN("Norm A", normA);
        //FIGARO_LOG_MIC_BEN("Condition number A", condA);

        FIGARO_BENCH_INIT(qrPostprocEval)
        FIGARO_BENCH_START(qrPostprocEval)
        auto [rName, qName] =
            m_pDatabase->evalQRPostprocessing(pElement->getRelationOrder().at(0),
            m_qrHintType, m_memoryLayout, pElement->isComputeQ(), m_saveResult);
        FIGARO_BENCH_STOP(qrPostprocEval)
        FIGARO_LOG_BENCH("Figaro", "QR Postproc eval", FIGARO_BENCH_GET_TIMER_LAP(qrPostprocEval))


        //double condNumberQ = m_pDatabase->estimateConditionNumber(qName, {});
        //double condNumberR = m_pDatabase->estimateConditionNumber(rName, {});
        //FIGARO_LOG_BENCH("Condition number Q", condNumberQ);
        //FIGARO_LOG_BENCH("Condition number R", condNumberR);
        if (pElement->isComputeQ())
        {
            // changeMemoryLayout update by keeping the old data in order for this to work
            //double resError = m_pDatabase->checkResidualErrorOfQR(
            //pElement->getRelationOrder().at(0), qName, rName);
            //FIGARO_LOG_BENCH("resError", resError)
        }
        return new ASTVisitorResultQR(rName, qName);
        //return new ASTVisitorResultQR("", "");
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


        FIGARO_MIC_BEN_INIT(uIndicesComp)
        FIGARO_MIC_BEN_INIT(uFirstPassComp)
        FIGARO_MIC_BEN_INIT(uSecondPassComp)
        FIGARO_BENCH_INIT(uComp)

        FIGARO_LOG_INFO("JOIN EVALUATION")
        FIGARO_LOG_INFO("ONE HOT ENCODING")
        m_pDatabase->oneHotEncodeRelations();
        FIGARO_BENCH_INIT(lComp)

        /************* U COMPUTATION START ***********/
        FIGARO_BENCH_START(uComp)

        FIGARO_MIC_BEN_START(uIndicesComp)
        pElement->accept(&buildIndicesVisitor);
        FIGARO_MIC_BEN_STOP(uIndicesComp)

        if (isFlagOn("headsAndTails"))
        {
            FIGARO_MIC_BEN_START(uFirstPassComp)
            ASTVisitorResultFirstPass* pResult =
            (ASTVisitorResultFirstPass*)pElement->accept(&figaroFirstPassVisitor);
            FIGARO_MIC_BEN_STOP(uFirstPassComp)
            FIGARO_LOG_MIC_BEN("Figaro", "first pass",  FIGARO_MIC_BEN_GET_TIMER_LAP(uFirstPassComp));
            ASTVisitorResultQR* pQRrResult = nullptr;
            if (isFlagOn("generalizedHeadsAndTails"))
            {
                bool evalPostProcessing = isFlagOn("postProcessing");
                FIGARO_MIC_BEN_START(uSecondPassComp)
                ASTVisitorLUFigaroSecondPass figaroSecondPassVisitor(m_pDatabase, m_qrHintType, m_saveResult, joinRelName, pResult->getHtNamesTmpRels(), evalPostProcessing);
                delete pResult;
                ASTVisitorResultQR* pQRrResult = (ASTVisitorResultQR*)pElement->accept(&figaroSecondPassVisitor);
                uName = pQRrResult->getRRelationName();
                if (evalPostProcessing)
                {
                    m_pDatabase->persistRelation(uName);
                }
                FIGARO_MIC_BEN_STOP(uSecondPassComp)
                FIGARO_LOG_MIC_BEN("Figaro", "second pass",  FIGARO_MIC_BEN_GET_TIMER_LAP(uSecondPassComp));
            }
            else
            {
                delete pResult;
            }
            FIGARO_BENCH_STOP(uComp)
            FIGARO_LOG_BENCH("Figaro", "Computation of U",  FIGARO_BENCH_GET_TIMER_LAP(uComp));

            /************* U COMPUTATION END ***********/

            if (m_saveMemory)
            {
                m_pDatabase->destroyAuxRelations();
            }

            delete pQRrResult;

            if (isFlagOn("computeL"))
            {
                FIGARO_BENCH_START(lComp)
                ASTNodeRelation* astRNOde =
                    new ASTNodeRelation(uName,
                    m_pDatabase->getRelationAttributeNames(uName));
                ASTNodeInverse* astRInvNode = new ASTNodeInverse(astRNOde);
                ASTNodeRightMultiply astRightMulNode(pElement->getOperand()->copy(), astRInvNode, false);
                // Add relation.
                ASTVisitorResultJoin* pLUResult =  (ASTVisitorResultJoin*)astRightMulNode.accept(this);
                lName = pLUResult->getJoinRelName();
                delete pLUResult;
                std::string permName = m_pDatabase->extractLUPermutationMatrix(lName);
                //m_pDatabase->applyPermutationInPlace(uName, permName);


                FIGARO_BENCH_STOP(lComp)
                FIGARO_LOG_BENCH("Figaro", "Computation of L",  FIGARO_BENCH_GET_TIMER_LAP(lComp));
            }
        }


        return new ASTVisitorResultQR(lName, uName);
     }


    ASTVisitorResultQR* ASTVisitorQueryEval::visitNodeLULapack(ASTNodeLULapack* pElement)
    {
        FIGARO_LOG_INFO("VISITING LU LAPACK NODE")
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

        FIGARO_BENCH_INIT(luLapackEval)
        FIGARO_BENCH_START(luLapackEval)
        auto [lName, uName] =
            m_pDatabase->evalLULapack(pElement->getRelationOrder().at(0),
             m_memoryLayout, m_saveResult);
        FIGARO_BENCH_STOP(luLapackEval)
        FIGARO_LOG_BENCH("Figaro", "LU LAPACK eval", FIGARO_BENCH_GET_TIMER_LAP(luLapackEval))
        return new ASTVisitorResultQR(lName, uName);
    }

    ASTVisitorResultQR* ASTVisitorQueryEval::visitNodeLUThin(ASTNodeLUThin* pElement)
    {
        FIGARO_LOG_INFO("VISITING LU THIN NODE")
        ASTVisitorComputeJoinAttributes joinAttrVisitor(m_pDatabase, false, m_memoryLayout);

        omp_set_num_threads(pElement->getNumThreads());
        m_pDatabase->dropAttributesFromRelations(
            pElement->getDropAttributes());
        pElement->accept(&joinAttrVisitor);
        m_pDatabase->oneHotEncodeRelations();

        FIGARO_BENCH_INIT(luThinEval)
        FIGARO_BENCH_START(luThinEval)
        auto [lName, uName] =
            m_pDatabase->evalLULapack(pElement->getRelationOrder().at(0),
             m_memoryLayout, m_saveResult);
        FIGARO_BENCH_STOP(luThinEval)
        FIGARO_LOG_BENCH("Figaro", "LU Thin eval", FIGARO_BENCH_GET_TIMER_LAP(luThinEval))
        return new ASTVisitorResultQR(lName, uName);
    }

    ASTVisitorResultJoin* ASTVisitorQueryEval::visitNodeLinReg(ASTNodeLinReg* pElement)
    {
        FIGARO_LOG_INFO("VISITING LIN REG NODE")
        std::string rRelName;
        std::string qRelName;

        ASTVisitorComputeJoinAttributes joinAttrVisitor(m_pDatabase, false, m_memoryLayout);
        ASTVisitorJoin astVisitorJoin(m_pDatabase);

        omp_set_num_threads(pElement->getNumThreads());
        if (pElement->isFigaro())
        {
            // computation of R
            ASTNodeQRFigaro astQRGivens(
                pElement->getOperand()->copy(),
                pElement->getRelationOrder(),
                pElement->getDropAttributes(),
                pElement->getNumThreads(), true);
            ASTVisitorResultQR* pQrResult = (ASTVisitorResultQR*)astQRGivens.accept(this);
            rRelName = pQrResult->getRRelationName();
            qRelName = pQrResult->getQRelationName();
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
        ASTVisitorJoin astVisitorJoin(m_pDatabase);

        omp_set_num_threads(pElement->getNumThreads());
        m_pDatabase->dropAttributesFromRelations(
            pElement->getDropAttributes());
        pElement->accept(&joinAttrVisitor);
        //m_pDatabase->oneHotEncodeRelations();

        ASTVisitorResultJoin* pJoinResult = (ASTVisitorResultJoin*)pElement->accept(&astVisitorJoin);
        std::string newRelName = pJoinResult->getJoinRelName();
        delete pJoinResult;

        return new ASTVisitorResultJoin(newRelName);
    }

    ASTVisitorResultJoin* ASTVisitorQueryEval::visitNodeRightMultiply(ASTNodeRightMultiply* pElement)
    {
        FIGARO_LOG_INFO("VISITING EVAL RIGHT MULTIPLY NODE")
        ASTVisitorComputeJoinAttributes joinAttrVisitor(m_pDatabase, false, m_memoryLayout);

        //FIGARO_MIC_BEN_INIT(joinTime)
        //FIGARO_MIC_BEN_START(joinTime)
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
        //FIGARO_MIC_BEN_STOP(joinTime)
        //FIGARO_LOG_BENCH("Join time", FIGARO_MIC_BEN_GET_TIMER_LAP(joinTime))

        ASTVisitorResultJoin* pMatrix =
            (ASTVisitorResultJoin*)pElement->getRightOperand()->accept(this);

        uint32_t joinSize = 0;
        if (pElement->isLFTJoin())
        {
            joinSize = m_pDatabase->getDownCountSum(joinAttrVisitor.getPreOrderRelNames()[0]);
        }
        ASTVisitorRightMultiply astRMVisitor(m_pDatabase, pMatrix->getJoinRelName(),
            pElement->isLFTJoin(), joinAttrVisitor.getPreOrderRelNames(),
            joinAttrVisitor.getPreOrderParRelNames(),
            joinAttrVisitor.getPreOrderVVJoinAttrNames(),
            joinAttrVisitor.getPreOrderVVParJoinAttrNames(), joinSize);

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