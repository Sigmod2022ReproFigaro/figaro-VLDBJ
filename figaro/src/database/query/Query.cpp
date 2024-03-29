#include "database/query/Query.h"
#include "database/query/visitor/ASTVisitorQueryEval.h"
#include "database/query/visitor/ASTVisitorJoin.h"
#include "utils/Performance.h"
#include "database/storage/Matrix.h"
#include <fstream>

namespace Figaro
{
    void Query::destroyAST(ASTNode* pASTRoot)
    {
        if (nullptr == pASTRoot)
        {
            return;
        }
        delete pASTRoot;

    }

    ASTNode* Query::createASTFromJson(const json& jsonQueryConfig)
    {
        std::string operatorName = jsonQueryConfig["operator"];
        ASTNode* pCreatedNode = nullptr;
    // TODO: Replace with factory pattern.
        // TODO: Rename QR_FIGARO with QR_FIGARO
        if ((operatorName == "QR_FIGARO") || (operatorName == "QR_HOUSEHOLDER")
        || (operatorName == "QR_GIV_THIN_DIAG") ||
        (operatorName == "QR_SPARSE") ||
        (operatorName == "eval_join")
        || (operatorName == "LIN_REG") || (operatorName == "LIN_REG_FIGARO")
        || (operatorName == "LU_FIGARO") || (operatorName == "LU_LAPACK") ||(operatorName == "LU_THIN")
        || (operatorName == "SVD_FIGARO") || (operatorName == "SVD_DIV_AND_CONQ")
        || (operatorName == "SVD_QR_ITER") || (operatorName == "SVD_QR")
        || (operatorName == "SVD_POWER_ITER") || (operatorName == "SVD_EIGEN_DECOMP")
        || (operatorName == ("SVD_EIGEN_DECOMP_DIV_AND_CONQ")
        || (operatorName == "SVD_EIGEN_DECOMP_QR_ITER") ||
        (operatorName == "SVD_EIGEN_DECOMP_RRR"))
        || (operatorName == "PCA_FIGARO") || (operatorName == "PCA_DIV_AND_CONQ")
        || (operatorName == "PCA_QR_ITER") || (operatorName == "PCA_QR")
        || (operatorName == "PCA_POWER_ITER") || (operatorName == "PCA_EIGEN_DECOMP")
        || (operatorName == "PCA_EIGEN_DECOMP_DIV_AND_CONQ")
        || (operatorName == "PCA_EIGEN_DECOMP_QR_ITER") ||
        (operatorName == "PCA_EIGEN_DECOMP_RRR") ||
        (operatorName == "LLS_QR"))
        {
            const json& operand = jsonQueryConfig["operands"][0];
            std::vector<std::string> vRelationOrder;
            std::vector<std::string> vDropAttrNames;
            uint32_t numThreads;
            std::string labelName;
            bool computeQ = false;
            bool isFigaro = false;

            for (const auto& relName: jsonQueryConfig["relation_order"])
            {
                vRelationOrder.push_back(relName);
            }

            if (jsonQueryConfig.contains("skip_attributes"))
            {
                for (const auto& attrName: jsonQueryConfig["skip_attributes"])
                {
                    vDropAttrNames.push_back(attrName);
                }
            }

            if (jsonQueryConfig.contains("num_threads"))
            {
                numThreads = jsonQueryConfig["num_threads"];
            }
            else
            {
                numThreads = getNumberOfThreads();
            }

            if (jsonQueryConfig.contains("compute_all"))
            {
                computeQ = jsonQueryConfig["compute_all"];
            }
            else
            {
                if (m_mOps.contains("compute_all") && (m_mOps["compute_all"] == "true"))
                {
                    computeQ = true;
                }
                else
                {
                    computeQ = false;
                }
            }
            m_computeAll = computeQ;

            if (jsonQueryConfig.contains("label_name"))
            {
                labelName = jsonQueryConfig["label_name"];
            }
            if (jsonQueryConfig.contains("figaro"))
            {
                isFigaro = jsonQueryConfig["figaro"];
            }


            ASTNode* pCreatedOperandNode = createASTFromJson(operand);
            if (operatorName == "LU_FIGARO")
            {
                m_opType = Query::OpType::DECOMP_LU;
                pCreatedNode = new ASTNodeLUFigaro(
                pCreatedOperandNode, vRelationOrder, vDropAttrNames, numThreads, m_computeAll,
                    Figaro::LUHintType::PART_PIVOT_LAPACK);
                FIGARO_LOG_INFO("CREATE LU_FIGARO NODE")
            }
            else if (operatorName == "QR_FIGARO")
            {
                m_opType = Query::OpType::DECOMP_QR;
                Figaro::QRHintType decompAlg = Figaro::QRHintType::GIV_THIN_DIAG;
                if (m_mOps.contains("decomp_alg"))
                {
                    std::string strDecAlg = m_mOps["decomp_alg"];
                    if (strDecAlg == "giv_thin_diag")
                    {
                        decompAlg = Figaro::QRHintType::GIV_THIN_DIAG;
                    }
                    else if (strDecAlg == "householder")
                    {
                        decompAlg = Figaro::QRHintType::HOUSEHOLDER;
                    }
                }
                pCreatedNode = new ASTNodeQRFigaro(
                pCreatedOperandNode, vRelationOrder, vDropAttrNames, numThreads, m_computeAll,
                    decompAlg);
                FIGARO_LOG_INFO("CREATE QR_FIGARO NODE")
            }
            else if (operatorName == "SVD_FIGARO")
            {
                m_opType = Query::OpType::DECOMP_SVD;
                Figaro::SVDHintType decompAlg = Figaro::SVDHintType::DIV_AND_CONQ;
                Figaro::QRHintType rFigAlg = Figaro::QRHintType::HOUSEHOLDER;
                if (m_mOps.contains("decomp_alg"))
                {
                    std::string strDecAlg = m_mOps["decomp_alg"];
                    if (strDecAlg == "divide_and_conquer")
                    {
                        decompAlg = Figaro::SVDHintType::DIV_AND_CONQ;
                    }
                    else if (strDecAlg == "power_iter")
                    {
                        decompAlg = Figaro::SVDHintType::POWER_ITER;
                    }
                }
                if (m_mOps.contains("decomp_alg"))
                {
                    std::string strrFigAlg = m_mOps["sub_method"];
                    if (strrFigAlg == "giv_thin_diag")
                    {
                        rFigAlg = Figaro::QRHintType::GIV_THIN_DIAG;
                    }
                    else if (strrFigAlg == "householder")
                    {
                        rFigAlg = Figaro::QRHintType::HOUSEHOLDER;
                    }
                }
                pCreatedNode = new ASTNodeSVDFigaro(
                pCreatedOperandNode, vRelationOrder, vDropAttrNames, numThreads, m_computeAll,
                    decompAlg, rFigAlg);
                FIGARO_LOG_INFO("CREATE SVD_FIGARO NODE")
            }
             else if (operatorName == "PCA_FIGARO")
            {
                m_opType = Query::OpType::DECOMP_PCA;
                Figaro::QRHintType rFigAlg = Figaro::QRHintType::HOUSEHOLDER;
                Figaro::PCAHintType decompAlg = Figaro::PCAHintType::DIV_AND_CONQ;
                if (m_mOps.contains("decomp_alg"))
                {
                    std::string strDecAlg = m_mOps["decomp_alg"];
                    if (strDecAlg == "divide_and_conquer")
                    {
                        decompAlg = Figaro::PCAHintType::DIV_AND_CONQ;
                    }
                    else if (strDecAlg == "power_iter")
                    {
                        decompAlg = Figaro::PCAHintType::POWER_ITER;
                    }
                }
                if (m_mOps.contains("decomp_alg"))
                {
                    std::string strrFigAlg = m_mOps["sub_method"];
                    if (strrFigAlg == "giv_thin_diag")
                    {
                        rFigAlg = Figaro::QRHintType::GIV_THIN_DIAG;
                    }
                    else if (strrFigAlg == "householder")
                    {
                        rFigAlg = Figaro::QRHintType::HOUSEHOLDER;
                    }
                }
                pCreatedNode = new ASTNodePCAFigaro(
                pCreatedOperandNode, vRelationOrder, vDropAttrNames, numThreads, m_computeAll,
                    decompAlg, rFigAlg);
                FIGARO_LOG_INFO("CREATE PCA_FIGARO NODE")
            }
            else if (operatorName == "QR_HOUSEHOLDER")
            {
                m_opType = Query::OpType::DECOMP_QR;
                pCreatedNode = new ASTNodeQRAlg(
                pCreatedOperandNode, vRelationOrder, vDropAttrNames, numThreads, m_computeAll,
                Figaro::QRHintType::HOUSEHOLDER);
                FIGARO_LOG_INFO("CREATE QR_HOUSEHOLDER NODE")
            }
            else if (operatorName == "QR_SPARSE")
            {
                m_opType = Query::OpType::DECOMP_QR;
                pCreatedNode = new ASTNodeQRAlg(
                pCreatedOperandNode, vRelationOrder, vDropAttrNames, numThreads, m_computeAll,
                Figaro::QRHintType::QR_SPARSE);
                FIGARO_LOG_INFO("CREATE QR_SPARSE NODE")
            }
            else if (operatorName == "QR_GIV_THIN_DIAG")
            {
                m_opType = Query::OpType::DECOMP_QR;
                pCreatedNode = new ASTNodeQRAlg(
                pCreatedOperandNode, vRelationOrder, vDropAttrNames, numThreads, m_computeAll,
                 Figaro::QRHintType::GIV_THIN_DIAG);
                FIGARO_LOG_INFO("CREATE QR_HOUSEHOLDER NODE")
            }
            else if (operatorName == "LU_LAPACK")
            {
                m_opType = Query::OpType::DECOMP_LU;
                pCreatedNode = new ASTNodeLUAlg(
                pCreatedOperandNode, vRelationOrder, vDropAttrNames, numThreads,
                    LUHintType::PART_PIVOT_LAPACK);
                FIGARO_LOG_INFO("CREATE LU_LAPACK NODE")
            }
            else if (operatorName == "LU_THIN")
            {
                m_opType = Query::OpType::DECOMP_LU;
                pCreatedNode = new ASTNodeLUThin(
                pCreatedOperandNode, vRelationOrder, vDropAttrNames, numThreads);
                FIGARO_LOG_INFO("CREATE LU_THIN NODE")
            }
            else if (operatorName == "SVD_DIV_AND_CONQ")
            {
                m_opType = Query::OpType::DECOMP_SVD;
                pCreatedNode = new ASTNodeSVDAlgDec(
                pCreatedOperandNode, vRelationOrder, vDropAttrNames, numThreads, m_computeAll,
                Figaro::SVDHintType::DIV_AND_CONQ);
                FIGARO_LOG_INFO("CREATE SVD_DIV_AND_CONQ NODE")
            }
            else if (operatorName == "SVD_QR_ITER")
            {
                m_opType = Query::OpType::DECOMP_SVD;
                pCreatedNode = new ASTNodeSVDAlgDec(
                pCreatedOperandNode, vRelationOrder, vDropAttrNames, numThreads, m_computeAll,
                Figaro::SVDHintType::QR_ITER);
                FIGARO_LOG_INFO("CREATE SVD_QR_ITER NODE")
            }
            else if (operatorName == "SVD_POWER_ITER")
            {
                m_opType = Query::OpType::DECOMP_SVD;
                pCreatedNode = new ASTNodeSVDAlgDec(
                pCreatedOperandNode, vRelationOrder, vDropAttrNames, numThreads, m_computeAll,
                Figaro::SVDHintType::POWER_ITER);
                FIGARO_LOG_INFO("CREATE SVD_POWER_ITER NODE")
            }
            else if (operatorName == "SVD_EIGEN__DECOMP")
            {
                m_opType = Query::OpType::DECOMP_SVD;
                pCreatedNode = new ASTNodeSVDAlgDec(
                pCreatedOperandNode, vRelationOrder, vDropAttrNames, numThreads, m_computeAll,
                Figaro::SVDHintType::EIGEN_DECOMP);
                FIGARO_LOG_INFO("CREATE SVD_EIGEN__DECOMP NODE")
            }
            else if (operatorName == "SVD_EIGEN_DECOMP_DIV_AND_CONQ")
            {
                m_opType = Query::OpType::DECOMP_SVD;
                pCreatedNode = new ASTNodeSVDAlgDec(
                pCreatedOperandNode, vRelationOrder, vDropAttrNames, numThreads, m_computeAll,
                Figaro::SVDHintType::EIGEN_DECOMP_DIV_AND_CONQ);
                FIGARO_LOG_INFO("CREATE SVD_EIGEN_DECOMP_DIV_AND_CONQ NODE")
            }
            else if (operatorName == "SVD_EIGEN_DECOMP_QR_ITER")
            {
                m_opType = Query::OpType::DECOMP_SVD;
                pCreatedNode = new ASTNodeSVDAlgDec(
                pCreatedOperandNode, vRelationOrder, vDropAttrNames, numThreads, m_computeAll,
                Figaro::SVDHintType::EIGEN_DECOMP_QR_ITER);
                FIGARO_LOG_INFO("CREATE SVD_EIGEN_DECOMP_QR_ITER NODE")
            }
            else if (operatorName == "SVD_EIGEN_DECOMP_RRR")
            {
                m_opType = Query::OpType::DECOMP_SVD;
                pCreatedNode = new ASTNodeSVDAlgDec(
                pCreatedOperandNode, vRelationOrder, vDropAttrNames, numThreads, m_computeAll,
                Figaro::SVDHintType::EIGEN_DECOMP_RRR);
                FIGARO_LOG_INFO("CREATE SVD_EIGEN_DECOMP_RRR NODE")
            }
            else if (operatorName == "PCA_DIV_AND_CONQ")
            {
                m_opType = Query::OpType::DECOMP_PCA;
                pCreatedNode = new ASTNodePCAAlgDec(
                pCreatedOperandNode, vRelationOrder, vDropAttrNames, numThreads, m_computeAll,
                Figaro::PCAHintType::DIV_AND_CONQ);
                FIGARO_LOG_INFO("CREATE PCA_DIV_AND_CONQ NODE")
            }
            else if (operatorName == "PCA_QR_ITER")
            {
                m_opType = Query::OpType::DECOMP_PCA;
                pCreatedNode = new ASTNodePCAAlgDec(
                pCreatedOperandNode, vRelationOrder, vDropAttrNames, numThreads, m_computeAll,
                Figaro::PCAHintType::QR_ITER);
                FIGARO_LOG_INFO("CREATE PCA_QR_ITER NODE")
            }
            else if (operatorName == "PCA_POWER_ITER")
            {
                m_opType = Query::OpType::DECOMP_PCA;
                pCreatedNode = new ASTNodePCAAlgDec(
                pCreatedOperandNode, vRelationOrder, vDropAttrNames, numThreads, m_computeAll,
                Figaro::PCAHintType::POWER_ITER);
                FIGARO_LOG_INFO("CREATE PCA_POWER_ITER NODE")
            }
            else if (operatorName == "PCA_EIGEN_DECOMP")
            {
                m_opType = Query::OpType::DECOMP_PCA;
                pCreatedNode = new ASTNodePCAAlgDec(
                pCreatedOperandNode, vRelationOrder, vDropAttrNames, numThreads, m_computeAll,
                Figaro::PCAHintType::EIGEN_DECOMP);
                FIGARO_LOG_INFO("CREATE PCA_EIGEN_DECOMP NODE")
            }
            else if (operatorName == "PCA_EIGEN_DECOMP_DIV_AND_CONQ")
            {
                m_opType = Query::OpType::DECOMP_PCA;
                pCreatedNode = new ASTNodePCAAlgDec(
                pCreatedOperandNode, vRelationOrder, vDropAttrNames, numThreads, m_computeAll,
                Figaro::PCAHintType::EIGEN_DECOMP_DIV_AND_CONQ);
                FIGARO_LOG_INFO("CREATE PCA_EIGEN_DECOMP_DIV_AND_CONQ NODE")
            }
            else if (operatorName == "PCA_EIGEN_DECOMP_QR_ITER")
            {
                m_opType = Query::OpType::DECOMP_PCA;
                pCreatedNode = new ASTNodePCAAlgDec(
                pCreatedOperandNode, vRelationOrder, vDropAttrNames, numThreads, m_computeAll,
                Figaro::PCAHintType::EIGEN_DECOMP_QR_ITER);
                FIGARO_LOG_INFO("CREATE PCA_EIGEN_DECOMP_QR_ITER NODE")
            }
            else if (operatorName == "PCA_EIGEN_DECOMP_RRR")
            {
                m_opType = Query::OpType::DECOMP_PCA;
                pCreatedNode = new ASTNodePCAAlgDec(
                pCreatedOperandNode, vRelationOrder, vDropAttrNames, numThreads, m_computeAll,
                Figaro::PCAHintType::EIGEN_DECOMP_RRR);
                FIGARO_LOG_INFO("CREATE PCA_EIGEN_DECOMP_RRR NODE")
            }
            else if (operatorName == "PCA_QR")
            {
                m_opType = Query::OpType::DECOMP_PCA;
                pCreatedNode = new ASTNodePCAAlgDec(
                pCreatedOperandNode, vRelationOrder, vDropAttrNames, numThreads, m_computeAll,
                Figaro::PCAHintType::QR);
                FIGARO_LOG_INFO("CREATE PCA_QR NODE")
            }
            else if (operatorName == "LIN_REG")
            {
                pCreatedNode = new ASTNodeLinReg(
                pCreatedOperandNode, vRelationOrder, vDropAttrNames, numThreads,
                labelName, isFigaro);
                FIGARO_LOG_INFO("CREATE LIN_REG NODE")
            }
            else if (operatorName == "LIN_REG_FIGARO")
            {
                m_opType = Query::OpType::DECOMP_LLS;
                pCreatedNode = new ASTNodeLeastSquares(
                pCreatedOperandNode, vRelationOrder, vDropAttrNames, numThreads,
                labelName, isFigaro);
                FIGARO_LOG_INFO("CREATE LIN_REG_FIGARO NODE")
            }
            else if (operatorName == "LLS_QR")
            {
                m_opType = Query::OpType::DECOMP_LLS;
                pCreatedNode = new ASTNodeLeastSquares(
                pCreatedOperandNode, vRelationOrder, vDropAttrNames, numThreads,
                labelName, isFigaro);
                FIGARO_LOG_INFO("CREATE LLS_QR NODE")
            }
            else
            {
                pCreatedNode = new ASTNodeEvalJoin(
                pCreatedOperandNode, vRelationOrder, vDropAttrNames, numThreads);
                FIGARO_LOG_INFO("CREATE EVAL_JOIN NODE")
            }
        }
        else if (operatorName == "assign")
        {
            const json& operand = jsonQueryConfig["operands"][0];
            std::string relationName = "";
            if (jsonQueryConfig.contains("name"))
            {
                relationName = jsonQueryConfig["name"];
            }
            ASTNode* pCreatedOperandNode = createASTFromJson(operand);
            pCreatedNode = new ASTNodeAssign(pCreatedOperandNode, relationName);
            FIGARO_LOG_INFO("CREATE ASSIGN NODE")
            return pCreatedNode;
        }
        else if (operatorName == "natural_join")
        {
            const json& operandCentral = jsonQueryConfig["central_relation"];
            ASTNodeRelation* pCreatedCentralOperand =
            (ASTNodeRelation*)createASTFromJson(operandCentral);
            std::vector<ASTNodeAbsRelation*> vpCreatedChildOperands;
            for (const auto& operandChild: jsonQueryConfig["children"])
            {
                ASTNodeAbsRelation* pCreatedOperandChild =
                (ASTNodeAbsRelation*)createASTFromJson(operandChild);
                vpCreatedChildOperands.push_back(pCreatedOperandChild);
            }
            pCreatedNode = new ASTNodeJoin(pCreatedCentralOperand, vpCreatedChildOperands);
            for (ASTNodeAbsRelation* pChild: vpCreatedChildOperands)
            {
                pChild->setParent((ASTNodeAbsRelation*)pCreatedNode);
            }
            FIGARO_LOG_INFO("CREATE JOIN NODE")
        }
        else if (operatorName == "relation")
        {
            const std::string& relationName = jsonQueryConfig["relation"];
            std::vector<std::string> vAttrNames;
            if (jsonQueryConfig.contains("attributes_order"))
            {
                for (const auto& attrName: jsonQueryConfig["attributes_order"])
                {
                    vAttrNames.push_back(attrName);
                }
            }
            else
            {
                vAttrNames = m_pDatabase->getRelationAttributeNames(relationName);
            }
            pCreatedNode = new ASTNodeRelation(relationName, vAttrNames);
            m_mRelNameASTNodeRel[relationName] = (ASTNodeRelation*)pCreatedNode;
            m_pDatabase->updateSchemaOfRelation(relationName, vAttrNames);
            FIGARO_LOG_INFO("CREATE RELATION NODE", relationName)
        }

        return pCreatedNode;
    }

    ErrorCode Query::createAST(const json& jsonQueryConfig)
    {
        ErrorCode errorCode = ErrorCode::NO_ERROR;

        ASTNode* pASTRoot = createASTFromJson(jsonQueryConfig);
        if (nullptr == pASTRoot)
        {
            errorCode = ErrorCode::WRONG_QUERY_CONFIG_SPECS;
        }
        else
        {
            m_pASTRoot = pASTRoot;
        }

        return errorCode;
    }

    ErrorCode Query::loadQuery(const std::string& queryConfigPath,
        std::map<std::string, std::string> mOps)
    {
        std::ifstream inputFileStream(queryConfigPath);
        json jsonQueryConfig;
        ErrorCode errorCode;
        m_mOps = mOps;
        if (inputFileStream.fail())
        {
            FIGARO_LOG_ERROR("Query configuration path incorrect", queryConfigPath);
            return ErrorCode::WRONG_PATH;
        }
        inputFileStream >> jsonQueryConfig;
        FIGARO_LOG_INFO("Database Configuration", jsonQueryConfig);
        errorCode = createAST(jsonQueryConfig["query"]["evaluation_hint"]);
        return errorCode;
    }

     void Query::evaluateQuery(bool saveMemory,
        const std::map<std::string, bool>& mFlags,
        const std::map<std::string, uint32_t>& mIntOpts,
        Figaro::MemoryLayout memoryLayout, bool saveResult
        )
    {
        FIGARO_BENCH_INIT(joinEval)
        FIGARO_BENCH_START(joinEval)
        ASTVisitorQueryEval queryEvalVisitor(m_pDatabase, memoryLayout,
            saveResult, saveMemory, mFlags, mIntOpts);
        m_pResult = m_pASTRoot->accept(&queryEvalVisitor);
        FIGARO_BENCH_STOP(joinEval)
        FIGARO_LOG_BENCH("Query eval", FIGARO_BENCH_GET_TIMER_LAP(joinEval))
     }
}