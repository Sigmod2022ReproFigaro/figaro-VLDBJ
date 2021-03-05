#include "database/query/Query.h"
#include "database/query/ASTScaledCartesianProductVisitor.h"
#include <fstream>

namespace Figaro
{
    void Query::destroyAST(ASTNode* pASTRoot)
    {
        if (nullptr == pASTRoot)
        {
            return;
        }
    }

    ASTNode* Query::createASTFromJson(const json& jsonQueryConfig)
    {
        std::string operatorName = jsonQueryConfig["operator"];
        ASTNode* pCreatedNode = nullptr;
        // TODO: Replace with factory pattern.
        if (operatorName == "GIV_QR")
        {
            const json& operand = jsonQueryConfig["operands"][0];
            ASTNode* pCreatedOperandNode = createASTFromJson(operand);
            pCreatedNode = new ASTNodeQRGivens(pCreatedOperandNode);
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

     void Query::evaluateQuery(void)
     {
         // Create visitor
        ASTScaledCartesianProductVisitor visitor(m_pDatabase);
        // TODO: Visitor to fill out everything in the root. 
        m_pASTRoot->accept(&visitor);
     }
}