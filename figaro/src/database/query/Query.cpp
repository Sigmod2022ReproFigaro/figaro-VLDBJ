#include "database/query/Query.h"
#include "database/query/visitor/ASTVisitorQueryEval.h"
#include "database/query/visitor/ASTJoinVisitor.h"
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
        // TODO: Rename GIV_QR with FIGARO_QR
        if ((operatorName == "GIV_QR") || (operatorName == "POSTPROCESS_QR")
        || (operatorName == "eval_join") || (operatorName == "LIN_REG"))
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
                computeQ = m_computeAll;
            }

            if (jsonQueryConfig.contains("label_name"))
            {
                labelName = jsonQueryConfig["label_name"];
            }
            if (jsonQueryConfig.contains("figaro"))
            {
                isFigaro = jsonQueryConfig["figaro"];
            }


            ASTNode* pCreatedOperandNode = createASTFromJson(operand);
            if (operatorName == "GIV_QR")
            {
                pCreatedNode = new ASTNodeQRGivens(
                pCreatedOperandNode, vRelationOrder, vDropAttrNames, numThreads, computeQ);
                FIGARO_LOG_INFO("CREATE GIV_QR NODE")
            }
            else if (operatorName == "POSTPROCESS_QR")
            {
                pCreatedNode = new ASTNodePostProcQR(
                pCreatedOperandNode, vRelationOrder, vDropAttrNames, numThreads, computeQ);
                FIGARO_LOG_INFO("CREATE POSTPROCESS_QR NODE")
            }
            else if (operatorName == "LIN_REG")
            {
                pCreatedNode = new ASTNodeLinReg(
                pCreatedOperandNode, vRelationOrder, vDropAttrNames, numThreads,
                labelName, isFigaro);
                FIGARO_LOG_INFO("CREATE LIN_REG NODE")
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

    ErrorCode Query::loadQuery(const std::string& queryConfigPath, bool computeAll)
    {
        std::ifstream inputFileStream(queryConfigPath);
        json jsonQueryConfig;
        ErrorCode errorCode;
        if (inputFileStream.fail())
        {
            FIGARO_LOG_ERROR("Query configuration path incorrect", queryConfigPath);
            return ErrorCode::WRONG_PATH;
        }
        inputFileStream >> jsonQueryConfig;
        FIGARO_LOG_INFO("Database Configuration", jsonQueryConfig);
        m_computeAll = computeAll;
        errorCode = createAST(jsonQueryConfig["query"]["evaluation_hint"]);
        return errorCode;
    }

     void Query::evaluateQuery(bool evalCounts, bool evalFirstFigaroPass,
        bool evalSecondFigaroPass, bool evalPostProcess, Figaro::QRHintType qrHintType,
        Figaro::MemoryLayout memoryLayout, bool saveResult)
    {
        MICRO_BENCH_INIT(joinEval)
        MICRO_BENCH_START(joinEval)
        ASTVisitorQueryEval queryEvalVisitor(m_pDatabase, memoryLayout, qrHintType, saveResult);
        m_pResult = m_pASTRoot->accept(&queryEvalVisitor);
        MICRO_BENCH_STOP(joinEval)
        FIGARO_LOG_BENCH("Query eval", MICRO_BENCH_GET_TIMER_LAP(joinEval))
     }
}