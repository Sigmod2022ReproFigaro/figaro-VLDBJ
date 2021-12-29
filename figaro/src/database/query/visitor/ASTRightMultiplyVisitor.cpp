#include "database/query/visitor/ASTRightMultiplyVisitor.h"
#include "database/query/visitor/ASTVisitorJoinResult.h"

namespace Figaro
{
    ASTVisitorAbsResult* ASTRightMultiplyVisitor::visitNodeRelation(ASTNodeRelation* pElement)
    {
        //m_pDatabase multiply each of the relations with m_rel
        uint32_t numNonJoinAttrs;
        numNonJoinAttrs = pElement->getAttributeNames().size() - pElement->getJoinAttributeNames().size();

        std::string mulRelName = m_pDatabase->multiply(pElement->getRelationName(), m_relName,
        pElement->getParJoinAttributeNames(), {}, startRowIdx);

        startRowIdx += numNonJoinAttrs;
        return new ASTVisitorJoinResult(mulRelName);
    }

    ASTVisitorAbsResult* ASTRightMultiplyVisitor::visitNodeJoin(ASTNodeJoin* pElement)
    {
        std::vector<std::string> vRelNames;
        ASTVisitorJoinResult* pJoinResult = pElement->getCentralRelation()->accept(this);
        vRelNames.push_back(pJoinResult->getJoinRelName());
        delete pJoinResult;
        for (const auto& pChild: pElement->getChildren())
        {
            ASTVisitorJoinResult* pJoinResult = (ASTVisitorJoinResult*)(pChild->accept(this));
            vRelNames.push_back(pJoinResult->getJoinRelName());
            delete pJoinResult;
        }
        // TODO: Add join option that adds columns
        return new ASTVisitorJoinResult("");
    }

    ASTVisitorAbsResult* ASTRightMultiplyVisitor::visitNodeRightMultiply(ASTNodeRightMultiply* pElement)
    {
        FIGARO_LOG_INFO("VISITING RIGHT MULTIPLY")
        ASTNodeRelation* pRightRelationNode = (ASTNodeRelation*)pElement->getRightOperand();
        m_relName = pRightRelationNode->getRelationName();
        ASTVisitorAbsResult* pResult = pElement->getLeftOperand()->accept(this);
        return pResult;
    }
}