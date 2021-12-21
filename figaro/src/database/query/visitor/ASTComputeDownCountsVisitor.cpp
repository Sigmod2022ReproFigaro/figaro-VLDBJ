#include "database/query/visitor/ASTComputeDownCountsVisitor.h"

namespace Figaro
{
    ASTVisitorAbsResult* ASTComputeDownCountsVisitor::visitNodeRelation(ASTNodeRelation* pElement)
    {
        bool isRootNode;
        std::vector<std::string> childrenNames;
        std::vector<std::vector<std::string> > vvChildrenParentJoinAttributeNames;
        const auto& relationName = pElement->getRelationName();
        const auto& formJoinAttrNames = getFormateJoinAttributeNames(pElement->getJoinAttributeNames());

        //m_pDatabase->sortRelation(pElement->getRelationName(),
        //                          pElement->getJoinAttributeNames());
        isRootNode = pElement->getParent() == nullptr;
        m_pDatabase->computeDownCounts(relationName, childrenNames,
            pElement->getJoinAttributeNames(), pElement->getParJoinAttributeNames(),
            vvChildrenParentJoinAttributeNames, isRootNode);

        // TODO: Add compute up counts
        return nullptr;
    }

    ASTVisitorAbsResult* ASTComputeDownCountsVisitor::visitNodeJoin(ASTNodeJoin* pElement)
    {
        bool isRootNode;
        FIGARO_LOG_DBG("Join");
        FIGARO_LOG_DBG("Central");
        for (const auto& pChild: pElement->getChildren())
        {
            FIGARO_LOG_DBG("Child");
            pChild->accept(this);
        }
        const auto& relationName = pElement->getCentralRelation()->getRelationName();
        const auto& formJoinAttrNames = getFormateJoinAttributeNames(pElement->getJoinAttributeNames());
        //m_pDatabase->sortRelation(pElement->getCentralRelation()->getRelationName(),
        //                          pElement->getJoinAttributeNames());
        isRootNode = pElement->getParent() == nullptr;
        m_pDatabase->computeDownCounts(relationName, pElement->getChildrenNames(),
            pElement->getJoinAttributeNames(), pElement->getParJoinAttributeNames(),
            pElement->getChildrenParentJoinAttributeNames(), isRootNode);

        // TODO: Add compute up counts
        return nullptr;
    }

    ASTVisitorAbsResult* ASTComputeDownCountsVisitor::visitNodeQRGivens(ASTNodeQRGivens* pElement)
    {
         FIGARO_LOG_DBG("********************");
        FIGARO_LOG_DBG("QR Givens");
        FIGARO_LOG_DBG("Relation order", pElement->getRelationOrder())
        pElement->getOperand()->accept(this);
        return nullptr;
    }

}