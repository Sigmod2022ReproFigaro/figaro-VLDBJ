#include "database/query/ASTComputeUpAndCircleCountsVisitor.h"

namespace Figaro
{
    void ASTComputeUpAndCircleCountsVisitor::visitNodeRelation(ASTNodeRelation* pElement)
    {
        bool isRootNode;
        const auto& relationName = pElement->getRelationName();
        const auto& formJoinAttrNames = getFormateJoinAttributeNames(pElement->getJoinAttributeNames());
        std::vector<std::string> childrenNames;
        std::vector<std::vector<std::string> > vvChildrenParentJoinAttributeNames;
        m_pDatabase->sortRelation(pElement->getRelationName(),
                                  pElement->getJoinAttributeNames());
        isRootNode = pElement->getParent() == nullptr;
        m_pDatabase->computeUpAndCircleCounts(relationName, childrenNames,
                pElement->getParJoinAttributeNames(),
                vvChildrenParentJoinAttributeNames, isRootNode);
    }

    void ASTComputeUpAndCircleCountsVisitor::visitNodeJoin(ASTNodeJoin* pElement)
    {
        bool isRootNode;
        const auto& relationName = pElement->getCentralRelation()->getRelationName();
        const auto& formJoinAttrNames = getFormateJoinAttributeNames(pElement->getJoinAttributeNames());
        m_pDatabase->sortRelation(pElement->getCentralRelation()->getRelationName(),
                                  pElement->getJoinAttributeNames());
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
    }

    void ASTComputeUpAndCircleCountsVisitor::visitNodeQRGivens(ASTNodeQRGivens* pElement)
    {
         FIGARO_LOG_DBG("********************");
        FIGARO_LOG_DBG("QR Givens");
        FIGARO_LOG_DBG("Relation order", pElement->getRelationOrder())
        for (const auto& relName: pElement->getRelationOrder())
        {
            m_vpASTNodeRelation.push_back(m_mRelNameASTNodeRel.at(relName));
        }
        pElement->getOperand()->accept(this);
    }

}