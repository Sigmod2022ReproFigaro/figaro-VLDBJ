#include "database/query/visitor/ASTComputeUpAndCircleCountsVisitor.h"

namespace Figaro
{
    ASTVisitorAbsResult* ASTComputeUpAndCircleCountsVisitor::visitNodeRelation(ASTNodeRelation* pElement)
    {
        bool isRootNode;
        const auto& relationName = pElement->getRelationName();
        std::vector<std::string> childrenNames;
        std::vector<std::vector<std::string> > vvChildrenParentJoinAttributeNames;
        isRootNode = pElement->getParent() == nullptr;
        m_pDatabase->computeUpAndCircleCounts(relationName, childrenNames,
                pElement->getParJoinAttributeNames(),
                vvChildrenParentJoinAttributeNames, isRootNode);
        return nullptr;
    }

    ASTVisitorAbsResult* ASTComputeUpAndCircleCountsVisitor::visitNodeJoin(ASTNodeJoin* pElement)
    {
        bool isRootNode;
        const auto& relationName = pElement->getCentralRelation()->getRelationName();
        isRootNode = pElement->getParent() == nullptr;
        m_pDatabase->computeUpAndCircleCounts(relationName, pElement->getChildrenNames(),
                pElement->getParJoinAttributeNames(),
                pElement->getChildrenParentJoinAttributeNames(), isRootNode);

        FIGARO_LOG_DBG("Join");
        FIGARO_LOG_DBG("Central");
        for (const auto& pChild: pElement->getChildren())
        {
            FIGARO_LOG_DBG("Child");
            pChild->accept(this);
        }
        return nullptr;
    }

    ASTVisitorAbsResult* ASTComputeUpAndCircleCountsVisitor::visitNodeQRGivens(ASTNodeQRGivens* pElement)
    {
         FIGARO_LOG_DBG("********************");
        FIGARO_LOG_DBG("QR Givens");
        FIGARO_LOG_DBG("Relation order", pElement->getRelationOrder())
        pElement->getOperand()->accept(this);
        return nullptr;
    }

}