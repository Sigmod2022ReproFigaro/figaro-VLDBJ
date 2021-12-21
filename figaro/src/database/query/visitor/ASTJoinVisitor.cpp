#include "database/query/visitor/ASTJoinVisitor.h"

namespace Figaro
{
    ASTVisitorJoinResult* ASTJoinVisitor::visitNodeRelation(ASTNodeRelation* pElement)
    {
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
        ASTVisitorJoinResult* pJoinResult = (ASTVisitorJoinResult*)pElement->getOperand()->accept(this);

        m_pDatabase->renameRelation(
            pJoinResult->getJoinRelName(), pElement->getRelationName());
        m_pDatabase->persistRelation(pElement->getRelationName());
        delete pJoinResult;
    }
}