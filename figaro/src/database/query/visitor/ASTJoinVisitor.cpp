#include "database/query/visitor/ASTJoinVisitor.h"
#include "database/query/visitor/ASTJoinAttributesComputeVisitor.h"
#include <fstream>

namespace Figaro
{
    ASTVisitorJoinResult* ASTJoinVisitor::visitNodeRelation(ASTNodeRelation* pElement)
    {
        FIGARO_LOG_INFO("VISITING RELATION NODE")
        return new ASTVisitorJoinResult(pElement->getRelationName());
    }

    ASTVisitorJoinResult* ASTJoinVisitor::visitNodeJoin(ASTNodeJoin* pElement)
    {
        std::vector<std::string> vJoinResults;
        for (const auto& pChild: pElement->getChildren())
        {
            ASTVisitorJoinResult* pJoinResult = (ASTVisitorJoinResult*)(pChild->accept(this));
            vJoinResults.push_back(pJoinResult->getJoinRelName());
            delete pJoinResult;
        }
        std::string newRelName = m_pDatabase->joinRelations(
            pElement->getCentralRelation()->getRelationName(),
            vJoinResults,
            pElement->getJoinAttributeNames(),
            pElement->getParJoinAttributeNames(),
            pElement->getChildrenParentJoinAttributeNames(),
            false
            );
        return new ASTVisitorJoinResult(newRelName);
    }

    ASTVisitorAbsResult* ASTJoinVisitor::visitNodeAssign(ASTNodeAssign* pElement)
    {
        ASTJoinAttributesComputeVisitor joinAttrVisitor(m_pDatabase, false, Figaro::MemoryLayout::ROW_MAJOR);
        pElement->getOperand()->accept(&joinAttrVisitor);
        ASTVisitorJoinResult* pJoinResult = (ASTVisitorJoinResult*)pElement->getOperand()->accept(this);
        std::string newRelName = pElement->getRelationName();
        m_pDatabase->renameRelation(
            pJoinResult->getJoinRelName(), pElement->getRelationName());
        m_pDatabase->persistRelation(pElement->getRelationName());
        /*
        std::ofstream fileDumpR("/local/scratch/Figaro/tests_path/figaro-code/dumps/postprocess/lapack/row_major/only_r/DBRetailer10/JoinLocationRoot48/join.csv", std::ofstream::out);
        MatrixEigenT mOut;
        Relation::copyMatrixDTToMatrixEigen(m_pDatabase->m_relations.at(newRelName).m_data, mOut);
        Figaro::outputMatrixTToCSV(fileDumpR, mOut, ',', 2);
        ASTNodeJoin* pJoin = (ASTNodeJoin*)pElement->getOperand();
        */

        delete pJoinResult;
        return new ASTVisitorJoinResult(newRelName);
    }

    ASTVisitorJoinResult* ASTJoinVisitor::visitNodeEvalJoin(ASTNodeEvalJoin* pElement)
    {
        FIGARO_LOG_INFO("VISITING EVAL NODE")
        ASTVisitorJoinResult* pJoinResult = (ASTVisitorJoinResult*)pElement->getOperand()->accept(this);
        std::string newRelName = pJoinResult->getJoinRelName();
        delete pJoinResult;
        return new ASTVisitorJoinResult(newRelName);
        ;
    }
}