#include "database/query/visitor/ASTRightMultiplyVisitor.h"
#include "database/query/visitor/ASTVisitorJoinResult.h"

namespace Figaro
{
    ASTVisitorJoinResult* ASTRightMultiplyVisitor::visitNodeRelation(ASTNodeRelation* pElement)
    {
        FIGARO_LOG_INFO("Right multiply visiting NODE RELATION", pElement->getRelationName())
        uint32_t numNonJoinAttrs;
        numNonJoinAttrs = pElement->getAttributeNames().size() - pElement->getJoinAttributeNames().size();

        std::string mulRelName = m_pDatabase->multiply(pElement->getRelationName(), m_relName,
        pElement->getJoinAttributeNames(), {}, startRowIdx);

        startRowIdx += numNonJoinAttrs;
        FIGARO_LOG_INFO("Finished visiting NODE RELATION", pElement->getRelationName(), mulRelName)
        FIGARO_LOG_INFO("JOIN_ATTRS", pElement->getJoinAttributeNames())
        return new ASTVisitorJoinResult(mulRelName);
    }

    ASTVisitorJoinResult* ASTRightMultiplyVisitor::visitNodeJoin(ASTNodeJoin* pElement)
    {
        std::vector<std::string> vRelNames;
        std::string centralRelName;
        FIGARO_LOG_INFO("VISITING JOIN")
        ASTVisitorJoinResult* pJoinResult =
            (ASTVisitorJoinResult*)pElement->getCentralRelation()->accept(this);
        centralRelName = pJoinResult->getJoinRelName();
        FIGARO_LOG_INFO("central rel name", centralRelName)
        delete pJoinResult;

        for (const auto& pChild: pElement->getChildren())
        {
            ASTVisitorJoinResult* pJoinResult = (ASTVisitorJoinResult*)(pChild->accept(this));
            vRelNames.push_back(pJoinResult->getJoinRelName());
            delete pJoinResult;
        }

        FIGARO_LOG_INFO("central rel name", centralRelName)
        FIGARO_LOG_INFO("central rel name", pElement->getJoinAttributeNames())
        FIGARO_LOG_INFO("central rel name", pElement->getChildrenParentJoinAttributeNames())



        std::string joinRelName = m_pDatabase->joinRelationsAndAddColumns(
            centralRelName,
            vRelNames,
            pElement->getJoinAttributeNames(),
            pElement->getParJoinAttributeNames(),
            pElement->getChildrenParentJoinAttributeNames(), false);

        return new ASTVisitorJoinResult(joinRelName);
    }

    ASTVisitorAbsResult* ASTRightMultiplyVisitor::visitNodeRightMultiply(ASTNodeRightMultiply* pElement)
    {
        FIGARO_LOG_INFO("VISITING RIGHT MULTIPLY")
        ASTNodeRelation* pRightRelationNode = (ASTNodeRelation*)pElement->getRightOperand();
        FIGARO_LOG_INFO("VISITING RIGHT OPERAND")
        m_relName = pRightRelationNode->getRelationName();
        FIGARO_LOG_INFO("VISITING LEFT")
        ASTVisitorAbsResult* pResult = pElement->getLeftOperand()->accept(this);
        return pResult;
    }
}