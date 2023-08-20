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
                if (pElement->getHelpQrAlg() == QRHintType::GIV_THIN_DIAG)
                {
                    FIGARO_LOG_BENCH("QR ALGORITHM", "THIN_DIAG")
                }
                else if (pElement->getHelpQrAlg() == QRHintType::HOUSEHOLDER)
                {
                    FIGARO_LOG_BENCH("QR ALGORITHM", "HOUSEHOLDER")
                }
                ASTVisitorQRFigaroSecondPass figaroSecondPassVisitor(m_pDatabase,
                 pElement->getHelpQrAlg(), m_saveResult, joinRelName,
                 pResult->getHtNamesTmpRels(),
                    evalPostProcessing);
                delete pResult;
                pQRrResult = (ASTVisitorResultQR*)pElement->accept(&figaroSecondPassVisitor);
                rName = pQRrResult->getRRelationName();
                if (evalPostProcessing)
                {
                    m_pDatabase->persistRelation(rName);
                }
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
        if (pElement->isComputeQ() || isFlagOn("computeQ"))
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

    ASTVisitorResultQR* ASTVisitorQueryEval::visitNodeQRDecAlg(ASTNodeQRAlg* pElement)
    {
        FIGARO_LOG_INFO("VISITING POST PROC QR NODE")
        ASTVisitorComputeJoinAttributes joinAttrVisitor(m_pDatabase, false, m_memoryLayout);

        omp_set_num_threads(pElement->getNumThreads());
        m_pDatabase->dropAttributesFromRelations(
            pElement->getDropAttributes());
        pElement->accept(&joinAttrVisitor);
        m_pDatabase->oneHotEncodeRelations();
        if (m_memoryLayout != Figaro::MemoryLayout::ROW_MAJOR)
        {
            m_pDatabase->changeMemoryLayout(m_memoryLayout);
        }

        //double condA = m_pDatabase->estimateConditionNumber(pElement->getRelationOrder().at(0), {});
        //double normA = m_pDatabase->getNorm(pElement->getRelationOrder().at(0), {});
        //FIGARO_LOG_MIC_BEN("Norm A", normA);
        //FIGARO_LOG_MIC_BEN("Condition number A", condA);
        FIGARO_BENCH_INIT(qrPostprocEval)
        FIGARO_BENCH_START(qrPostprocEval)
        auto [rName, qName] =
            m_pDatabase->evalQRDecAlg(pElement->getRelationOrder().at(0),
            pElement->getQrAlgorithm(), m_memoryLayout, pElement->isComputeQ(), m_saveResult);
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
                ASTVisitorLUFigaroSecondPass figaroSecondPassVisitor(m_pDatabase,
                 pElement->geHelpLUAlg(), m_saveResult, joinRelName,
                 pResult->getHtNamesTmpRels(), evalPostProcessing);
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
                // TODO: replace this with
                ASTNodeRightMultiply astRightMulNode(pElement->getOperand()->copy(), astRInvNode, false);
                // Add relation.
                ASTVisitorResultJoin* pLUResult =  (ASTVisitorResultJoin*)astRightMulNode.accept(this);
                lName = pLUResult->getJoinRelName();
                delete pLUResult;
                //std::string permName = m_pDatabase->extractLUPermutationMatrix(lName);
                //m_pDatabase->applyPermutationInPlace(uName, permName);


                FIGARO_BENCH_STOP(lComp)
                FIGARO_LOG_BENCH("Figaro", "Computation of L",  FIGARO_BENCH_GET_TIMER_LAP(lComp));
            }
        }


        return new ASTVisitorResultQR(lName, uName);
     }


    ASTVisitorResultQR* ASTVisitorQueryEval::visitNodeLUDecAlg(ASTNodeLUAlg* pElement)
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
            m_pDatabase->changeMemoryLayout(m_memoryLayout);
        }

        FIGARO_BENCH_INIT(luLapackEval)
        FIGARO_BENCH_START(luLapackEval)
        auto [lName, uName] =
            m_pDatabase->evalLUDecAlg(pElement->getRelationOrder().at(0),
             m_memoryLayout, m_saveResult);
        FIGARO_BENCH_STOP(luLapackEval)
        FIGARO_LOG_BENCH("Figaro", "LU LAPACK eval", FIGARO_BENCH_GET_TIMER_LAP(luLapackEval))
        return new ASTVisitorResultQR(lName, uName);
    }


    ASTVisitorResultSVD* ASTVisitorQueryEval::visitNodeSVDFigaro(ASTNodeSVDFigaro* pElement)
    {
        std::string rRelName;

        ASTVisitorComputeJoinAttributes joinAttrVisitor(m_pDatabase, false, m_memoryLayout);
        ASTVisitorJoin astVisitorJoin(m_pDatabase);
        omp_set_num_threads(pElement->getNumThreads());
        ASTNodeQRFigaro astQRGivens(
                pElement->getOperand()->copy(),
                pElement->getRelationOrder(),
                pElement->getDropAttributes(),
                pElement->getNumThreads(), false,
                pElement->getHelpQRAlg());
        ASTVisitorResultQR* pQrResult = (ASTVisitorResultQR*)astQRGivens.accept(this);
        rRelName = pQrResult->getRRelationName();
        delete pQrResult;

        uint32_t perSingVals = 100;
         if (m_mIntOpts.contains("numSingVals"))
        {
            perSingVals = m_mIntOpts["numSingVals"];
        }

        FIGARO_BENCH_INIT(vComp)
        FIGARO_BENCH_START(vComp)
        auto [uName, sName, vName] = m_pDatabase->evalSVDDecAlg(rRelName,
            pElement->getHelpSVDAlg(), Figaro::MemoryLayout::ROW_MAJOR,
            perSingVals, pElement->isComputeU(), true);
        FIGARO_BENCH_STOP(vComp)
        FIGARO_LOG_BENCH("Figaro", "Computation of S and V",  FIGARO_BENCH_GET_TIMER_LAP(vComp));


        if (pElement->isComputeU() || isFlagOn("computeUAndV"))
        {
            FIGARO_LOG_INFO("COMPUTING U")
            FIGARO_BENCH_INIT(uComp)
            FIGARO_BENCH_START(uComp)
            ASTNodeRelation* astVNOde =
                        new ASTNodeRelation(vName,
                        m_pDatabase->getRelationAttributeNames(vName));
            ASTNodeRelation* astSNOde =
                        new ASTNodeRelation(sName,
                        m_pDatabase->getRelationAttributeNames(sName));
            ASTNodeSVDSVTInverse* astSVDInvNode =
                new ASTNodeSVDSVTInverse(astSNOde, astVNOde);
            ASTNodeRightMultiply astRightMulNode(pElement->getOperand()->copy(), astSVDInvNode,
                true);
            // Add relation.
            ASTVisitorResultJoin* pUResult =  (ASTVisitorResultJoin*)astRightMulNode.accept(this);
            uName = pUResult->getJoinRelName();
            delete pUResult;
            FIGARO_BENCH_STOP(uComp)
            FIGARO_LOG_BENCH("Figaro", "Computation of U",  FIGARO_BENCH_GET_TIMER_LAP(uComp));
        }

        return new ASTVisitorResultSVD(uName, sName, vName);
    }

    ASTVisitorResultSVD* ASTVisitorQueryEval::visitNodeSVDDecAlg(ASTNodeSVDAlgDec* pElement)
    {
        FIGARO_LOG_INFO("VISITING SVD DEC ALG NODE")
        ASTVisitorComputeJoinAttributes joinAttrVisitor(m_pDatabase, false, m_memoryLayout);
        uint32_t perDims = 100;
        omp_set_num_threads(pElement->getNumThreads());
        m_pDatabase->dropAttributesFromRelations(
            pElement->getDropAttributes());
        pElement->accept(&joinAttrVisitor);
        m_pDatabase->oneHotEncodeRelations();
        if (m_memoryLayout != Figaro::MemoryLayout::ROW_MAJOR)
        {
            m_pDatabase->changeMemoryLayout(m_memoryLayout);
        }

        if (m_mIntOpts.contains("numSingVals"))
        {
            perDims = m_mIntOpts["numSingVals"];
        }

        FIGARO_BENCH_INIT(svdLapackEval)
        FIGARO_BENCH_START(svdLapackEval)
        auto [uName, sName, vName] =
            m_pDatabase->evalSVDDecAlg(pElement->getRelationOrder().at(0),
            pElement->getSVDAlgorithm(),
             m_memoryLayout, perDims,
             pElement->isComputeUAndV(), m_saveResult);
        FIGARO_BENCH_STOP(svdLapackEval)
        FIGARO_LOG_BENCH("Figaro", "SVD Algorithm evaluation", FIGARO_BENCH_GET_TIMER_LAP(svdLapackEval))
        return new ASTVisitorResultSVD(uName, sName, vName);
    }

    ASTVisitorResultSVD* ASTVisitorQueryEval::visitNodePCAFigaro(ASTNodePCAFigaro* pElement)
    {
        uint32_t perDims = 100;
        std::string uRelName;
        std::string sRelName;
        std::string vRelName;
        if (m_mIntOpts.contains("numSingVals"))
        {
            perDims = m_mIntOpts["numSingVals"];
        }

        ASTVisitorComputeJoinAttributes joinAttrVisitor(m_pDatabase, false, m_memoryLayout);
        ASTVisitorJoin astVisitorJoin(m_pDatabase);
        omp_set_num_threads(pElement->getNumThreads());
        ASTNodeSVDFigaro astSVDFigaro(
                pElement->getOperand()->copy(),
                pElement->getRelationOrder(),
                pElement->getDropAttributes(),
                pElement->getNumThreads(), true,
                convertPcaHintTypeToSvd(pElement->getHelpPCAAlg()),
                pElement->getHelpQRAlg());
        ASTVisitorResultSVD* pSvdResult = (ASTVisitorResultSVD*)astSVDFigaro.accept(this);
        uRelName = pSvdResult->getURelationName();
        sRelName = pSvdResult->getSRelationName();
        vRelName = pSvdResult->getVRelationName();
        delete pSvdResult;

        FIGARO_BENCH_INIT(pcaFigaroEval)
        FIGARO_BENCH_START(pcaFigaroEval)
        std::string uRedName = m_pDatabase->computeMatrixProductRecDiag(uRelName, sRelName);
        FIGARO_BENCH_STOP(pcaFigaroEval)
        FIGARO_LOG_BENCH("Figaro", "US product", FIGARO_BENCH_GET_TIMER_LAP(pcaFigaroEval))

        return new ASTVisitorResultSVD(uRedName, sRelName, vRelName);
    }

    ASTVisitorResultSVD* ASTVisitorQueryEval::visitNodePCADecAlg(ASTNodePCAAlgDec* pElement)
    {
        FIGARO_LOG_INFO("VISITING PCA DEC ALG NODE")
        ASTVisitorComputeJoinAttributes joinAttrVisitor(m_pDatabase, false, m_memoryLayout);
        uint32_t numDims = 1;

        omp_set_num_threads(pElement->getNumThreads());
        m_pDatabase->dropAttributesFromRelations(
            pElement->getDropAttributes());
        pElement->accept(&joinAttrVisitor);
        m_pDatabase->oneHotEncodeRelations();
        if (m_memoryLayout != Figaro::MemoryLayout::ROW_MAJOR)
        {
            m_pDatabase->changeMemoryLayout(m_memoryLayout);
        }
        if (m_mIntOpts.contains("numSingVals"))
        {
            numDims = m_mIntOpts["numSingVals"];
        }
        FIGARO_BENCH_INIT(pcaLapackEval)
        FIGARO_BENCH_START(pcaLapackEval)
        auto [uName, sName, vName] =
            m_pDatabase->evalPCADecAlg(pElement->getRelationOrder().at(0),
            pElement->getPCAAlgorithm(),
             m_memoryLayout,
             numDims, pElement->isComputeUAndV(), m_saveResult);
        FIGARO_BENCH_STOP(pcaLapackEval)
        FIGARO_LOG_BENCH("Figaro", "PCA Algorithm evaluation", FIGARO_BENCH_GET_TIMER_LAP(pcaLapackEval))
        return new ASTVisitorResultSVD(uName, sName, vName);
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
            m_pDatabase->evalLUDecAlg(pElement->getRelationOrder().at(0),
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
        std::string linRegVec;

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
                pElement->getNumThreads(), true,
                QRHintType::GIV_THIN_DIAG);
            ASTVisitorResultQR* pQrResult = (ASTVisitorResultQR*)astQRGivens.accept(this);
            rRelName = pQrResult->getRRelationName();
            qRelName = pQrResult->getQRelationName();
            delete pQrResult;
            linRegVec = m_pDatabase->linearRegression(rRelName,
            pElement->getLabelName());
        }
        else
        {
            ASTNodeQRAlg astNodeQRAlg(
                pElement->getOperand()->copy(),
                pElement->getRelationOrder(),
                pElement->getDropAttributes(),
                pElement->getNumThreads(), false,
                QRHintType::GIV_THIN_DIAG
            );
            m_saveResult = true;
            ASTVisitorResultQR* pQrResult = (ASTVisitorResultQR*)astNodeQRAlg.accept(this);
            rRelName = pQrResult->getRRelationName();
            delete pQrResult;
        }

        return new ASTVisitorResultJoin(linRegVec);
    }

        ASTVisitorResultJoin* ASTVisitorQueryEval::visitNodeLeastSquares(ASTNodeLeastSquares* pElement)
    {
        FIGARO_LOG_INFO("VISITING LIN REG NODE")
        std::string rRelName;
        std::string qRelName;
        std::string linRegVec;

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
                pElement->getNumThreads(), true,
                QRHintType::GIV_THIN_DIAG);
            ASTVisitorResultQR* pQrResult = (ASTVisitorResultQR*)astQRGivens.accept(this);
            rRelName = pQrResult->getRRelationName();
            qRelName = pQrResult->getQRelationName();

            delete pQrResult;
            uint32_t qNumRows = m_pDatabase->getNumberOfRows(qRelName);

            FIGARO_BENCH_INIT(llsFigaro)
            FIGARO_BENCH_START(llsFigaro)

            FIGARO_MIC_BEN_INIT(generateRel)
            FIGARO_MIC_BEN_START(generateRel)
            std::string bRel = m_pDatabase->generateRelation(qNumRows, 1, m_memoryLayout);
            FIGARO_MIC_BEN_STOP(generateRel)
            FIGARO_LOG_MIC_BEN("Figaro", "GEneration",  FIGARO_BENCH_GET_TIMER_LAP(generateRel));

            FIGARO_MIC_BEN_INIT(mulQ)
            FIGARO_MIC_BEN_START(mulQ)
            std::string strSol = m_pDatabase->multiplyTranspose(qRelName, bRel);
            std::string bSol = m_pDatabase->leastSquareQR(strSol, bRel);
            FIGARO_MIC_BEN_STOP(mulQ)
            FIGARO_LOG_MIC_BEN("Figaro", "MULQ",  FIGARO_BENCH_GET_TIMER_LAP(mulQ));
            FIGARO_BENCH_STOP(llsFigaro)
            FIGARO_LOG_BENCH("Figaro", "Computation of Least squares",  FIGARO_BENCH_GET_TIMER_LAP(llsFigaro));
        }
        else
        {
            FIGARO_LOG_INFO("VISITING LLQ QR DEC ALG NODE")
            ASTVisitorComputeJoinAttributes joinAttrVisitor(m_pDatabase, false, m_memoryLayout);
            omp_set_num_threads(pElement->getNumThreads());
            m_pDatabase->dropAttributesFromRelations(
                pElement->getDropAttributes());
            pElement->accept(&joinAttrVisitor);
            m_pDatabase->oneHotEncodeRelations();
            if (m_memoryLayout != Figaro::MemoryLayout::ROW_MAJOR)
            {
                m_pDatabase->changeMemoryLayout(m_memoryLayout);
            }
            FIGARO_BENCH_INIT(llsQRLapackEval)
            FIGARO_BENCH_START(llsQRLapackEval)
            std::string relName = pElement->getRelationOrder().at(0);
            uint32_t qNumRows = m_pDatabase->getNumberOfRows(relName);
            std::string bRelName = m_pDatabase->generateRelation(qNumRows, 1, m_memoryLayout);
            std::string labName = m_pDatabase->leastSquareQR(relName, bRelName);
            FIGARO_BENCH_STOP(llsQRLapackEval)
            FIGARO_LOG_BENCH("Figaro", "LL QR Algorithm evaluation", FIGARO_BENCH_GET_TIMER_LAP(llsQRLapackEval))
            }

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

        std::vector<uint32_t> vDownCountsSize;
        std::vector<uint32_t> vBlockSizes;
        if (pElement->isLFTJoin())
        {
            m_pDatabase->getDownCountSum(joinAttrVisitor.getPreOrderRelNames()[0], vDownCountsSize, vBlockSizes);
        }
        ASTVisitorRightMultiply astRMVisitor(m_pDatabase, pMatrix->getJoinRelName(),
            pElement->isLFTJoin(), joinAttrVisitor.getPreOrderRelNames(),
            joinAttrVisitor.getPreOrderParRelNames(),
            joinAttrVisitor.getPreOrderVVJoinAttrNames(),
            joinAttrVisitor.getPreOrderVVParJoinAttrNames(), vDownCountsSize, vBlockSizes);

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

    ASTVisitorResultJoin* ASTVisitorQueryEval::visitNodeSVDSVTInverse(ASTNodeSVDSVTInverse* pElement)
    {
        FIGARO_LOG_INFO("VISITING SVD INVERSE (SIGMA * V^T) NODE")
        ASTVisitorResultJoin* pSigmaRes = (ASTVisitorResultJoin*)pElement->getOpSig()->accept(this);
         ASTVisitorResultJoin* pVRes = (ASTVisitorResultJoin*)pElement->getOpV()->accept(this);

        uint32_t perSingVals = 100;
        if (m_mIntOpts.contains("numSingVals"))
        {
            perSingVals = m_mIntOpts["numSingVals"];
        }

        std::string svdSVTInvName = m_pDatabase->computeSVDSigmaVTranInverse(
            pSigmaRes->getJoinRelName(), pVRes->getJoinRelName(),
            perSingVals);

        delete pSigmaRes;
        delete pVRes;

        return new ASTVisitorResultJoin(svdSVTInvName);
    }

    ASTVisitorResultJoin* ASTVisitorQueryEval::visitNodeRelation(ASTNodeRelation* pElement)
    {
        return new ASTVisitorResultJoin(pElement->getRelationName());
    }

}