#include "database/query/Query.h"
#include "database/query/ASTJoinAttributesComputeVisitor.h"
#include "database/query/ASTComputeUpAndCircleCountsVisitor.h"
#include "database/query/ASTComputeDownCountsVisitor.h"
#include "database/query/ASTFigaroFirstPassVisitor.h"
#include "database/query/ASTFigaroSecondPassVisitor.h"
#include "utils/Performance.h"
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
        if (operatorName == "GIV_QR")
        {
            const json& operand = jsonQueryConfig["operands"][0];
            std::vector<std::string> vRelationOrder;

            for (const auto& relName: jsonQueryConfig["relation_order"])
            {
                vRelationOrder.push_back(relName);
            }

            ASTNode* pCreatedOperandNode = createASTFromJson(operand);
            pCreatedNode = new ASTNodeQRGivens(pCreatedOperandNode, vRelationOrder);
            FIGARO_LOG_DBG("GIV_QR")
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
            FIGARO_LOG_DBG("JOIN")
        }
        else if (operatorName == "relation")
        {
            const std::string& relationName = jsonQueryConfig["relation"];
            pCreatedNode = new ASTNodeRelation(relationName, m_pDatabase->getRelationAttributeNames(relationName));
            m_mRelNameASTNodeRel[relationName] = (ASTNodeRelation*)pCreatedNode;
            FIGARO_LOG_DBG("RELATION", relationName)

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

    ErrorCode Query::loadQuery(const std::string& queryConfigPath)
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

        errorCode = createAST(jsonQueryConfig["query"]["evaluation_hint"]);
        return errorCode;
    }

     void Query::evaluateQuery(bool evalCounts, bool evalFirstFigaroPass,
        bool evalSecondFigaroPass, bool evalPostProcess)
     {
         // Create visitor
        ASTJoinAttributesComputeVisitor joinAttrVisitor(m_pDatabase, m_mRelNameASTNodeRel);
        ASTFigaroFirstPassVisitor figaroFirstPassVisitor(m_pDatabase, m_mRelNameASTNodeRel);
        ASTFigaroSecondPassVisitor figaroSecondPassVisitor(m_pDatabase,
                                m_mRelNameASTNodeRel, evalPostProcess);
        ASTComputeDownCountsVisitor computeDownVisitor(m_pDatabase, m_mRelNameASTNodeRel);
        ASTComputeUpAndCircleCountsVisitor computeUpAndCircleVisitor(m_pDatabase, m_mRelNameASTNodeRel);

        m_pASTRoot->accept(&joinAttrVisitor);
        MICRO_BENCH_INIT(downCnt)
        MICRO_BENCH_INIT(upCnt)
        if (evalCounts)
        {
            MICRO_BENCH_START(downCnt)
            m_pASTRoot->accept(&computeDownVisitor);
            MICRO_BENCH_STOP(downCnt)
            MICRO_BENCH_START(upCnt)
            m_pASTRoot->accept(&computeUpAndCircleVisitor);
            MICRO_BENCH_STOP(upCnt)
        }
        FIGARO_LOG_BENCH("Figaro", "query evaluation down",  MICRO_BENCH_GET_TIMER_LAP(downCnt));
        FIGARO_LOG_BENCH("Figaro", "query evaluation up",  MICRO_BENCH_GET_TIMER_LAP(upCnt));
        if (evalFirstFigaroPass)
        {
            m_pASTRoot->accept(&figaroFirstPassVisitor);
        }
        if (evalSecondFigaroPass)
        {
            m_pASTRoot->accept(&figaroSecondPassVisitor);
        }
     }
}