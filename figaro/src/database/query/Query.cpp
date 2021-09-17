#include "database/query/Query.h"
#include "database/query/ASTJoinAttributesComputeVisitor.h"
#include "database/query/ASTComputeUpAndCircleCountsVisitor.h"
#include "database/query/ASTComputeDownCountsVisitor.h"
#include "database/query/ASTFigaroFirstPassVisitor.h"
#include "database/query/ASTFigaroSecondPassVisitor.h"
#include "database/query/ASTPostProcQRVisitor.h"
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
        if ((operatorName == "GIV_QR") || (operatorName == "POSTPROCESS_QR"))
        {
            const json& operand = jsonQueryConfig["operands"][0];
            std::vector<std::string> vRelationOrder;
            std::vector<std::string> vDropAttrNames;
            uint32_t numThreads;

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

            ASTNode* pCreatedOperandNode = createASTFromJson(operand);
            if (operatorName == "GIV_QR")
            {
                pCreatedNode = new ASTNodeQRGivens(
                pCreatedOperandNode, vRelationOrder, vDropAttrNames, numThreads);
                FIGARO_LOG_INFO("CREATE GIV_QR NODE")
            }
            else
            {
                pCreatedNode = new ASTNodePostProcQR(
                pCreatedOperandNode, vRelationOrder, vDropAttrNames, numThreads);
                FIGARO_LOG_INFO("CREATE POSTPROCESS_QR NODE")
            }
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

    void Query::evaluateQueryFigaro(bool evalCounts, bool evalFirstFigaroPass,
        bool evalSecondFigaroPass, bool evalPostProcess, uint32_t numReps, Figaro::MatrixD::QRGivensHintType qrHintType, bool saveResult)
    {
         // Create visitor
        ASTJoinAttributesComputeVisitor joinAttrVisitor(m_pDatabase, true);
        ASTFigaroFirstPassVisitor figaroFirstPassVisitor(m_pDatabase);
        ASTFigaroSecondPassVisitor figaroSecondPassVisitor(m_pDatabase, evalPostProcess,qrHintType, &m_matResult, saveResult);
        ASTComputeDownCountsVisitor computeDownVisitor(m_pDatabase);
        ASTComputeUpAndCircleCountsVisitor computeUpAndCircleVisitor(m_pDatabase);
        //MICRO_BENCH_INIT(attrComp)
        //MICRO_BENCH_INIT(downCnt)
        //MICRO_BENCH_INIT(upCnt)
        //MICRO_BENCH_INIT(firstPass)
        //MICRO_BENCH_INIT(secondPass)

        //MICRO_BENCH_START(attrComp)
        m_pASTRoot->accept(&joinAttrVisitor);
        //MICRO_BENCH_STOP(attrComp)
        //FIGARO_LOG_BENCH("Figaro", "attribute comp",  MICRO_BENCH_GET_TIMER_LAP(attrComp));

        // If postprocess, apply computeQRGivens to the corresponding database
        for (uint32_t rep = 0; rep < numReps; rep++)
        {
            m_pDatabase->resetComputations();
            MICRO_BENCH_INIT(main)
            MICRO_BENCH_START(main)
            if (evalCounts)
            {
                //MICRO_BENCH_START(downCnt)
                m_pASTRoot->accept(&computeDownVisitor);
                //MICRO_BENCH_STOP(downCnt)
                //MICRO_BENCH_START(upCnt)
                m_pASTRoot->accept(&computeUpAndCircleVisitor);
                //MICRO_BENCH_STOP(upCnt)
            }
            //FIGARO_LOG_BENCH("Figaro", "query evaluation down",  MICRO_BENCH_GET_TIMER_LAP(downCnt));
            //FIGARO_LOG_BENCH("Figaro", "query evaluation up",  MICRO_BENCH_GET_TIMER_LAP(upCnt));

            //MICRO_BENCH_START(firstPass)
            if (evalFirstFigaroPass)
            {
                m_pASTRoot->accept(&figaroFirstPassVisitor);
            }
            //MICRO_BENCH_STOP(firstPass)
            //FIGARO_LOG_BENCH("Figaro", "first pass",  MICRO_BENCH_GET_TIMER_LAP(firstPass));

            //MICRO_BENCH_START(secondPass)
            if (evalSecondFigaroPass)
            {
                m_pASTRoot->accept(&figaroSecondPassVisitor);
            }
            //MICRO_BENCH_STOP(secondPass)
            //FIGARO_LOG_BENCH("Figaro", "second pass",  MICRO_BENCH_GET_TIMER_LAP(secondPass));
            MICRO_BENCH_STOP(main)
            FIGARO_LOG_BENCH("Figaro", "query evaluation",  MICRO_BENCH_GET_TIMER_LAP(main));
        }
    }

    void Query::evaluateQueryPostprocess(uint32_t numReps,
            Figaro::MatrixD::QRGivensHintType qrHintType, bool saveResult)
    {
        ASTJoinAttributesComputeVisitor joinAttrVisitor(m_pDatabase, false);
        ASTPostProcQRVisitor postpProcQRVisitor(m_pDatabase, qrHintType,
                                                &m_matResult, saveResult);

        m_pASTRoot->accept(&joinAttrVisitor);
        FIGARO_LOG_INFO("Finished joinAttrVisitor")
        for (uint32_t rep = 0; rep < numReps; rep++)
        {
            MICRO_BENCH_INIT(main)
            MICRO_BENCH_START(main)
            m_pASTRoot->accept(&postpProcQRVisitor);
            MICRO_BENCH_STOP(main)
            FIGARO_LOG_BENCH("Figaro", "query evaluation",  MICRO_BENCH_GET_TIMER_LAP(main));
        }
    }

     void Query::evaluateQuery(bool isPureFigaro, bool evalCounts, bool evalFirstFigaroPass,
        bool evalSecondFigaroPass, bool evalPostProcess, uint32_t numReps, Figaro::MatrixD::QRGivensHintType qrHintType, bool saveResult)
     {
         if (isPureFigaro)
         {
             evaluateQueryFigaro(evalCounts, evalFirstFigaroPass, evalSecondFigaroPass, evalPostProcess, numReps, qrHintType, saveResult);
         }
         else
         {
             evaluateQueryPostprocess(numReps, qrHintType, saveResult);
         }
     }
}