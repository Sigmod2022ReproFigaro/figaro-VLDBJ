#include "database/query/ASTJoinAttributesComputeVisitor.h"

namespace Figaro
{

    void ASTJoinAttributesComputeVisitor::initializeEnumAndDenomRelations(ASTNodeRelation* pRelation)
    {
        std::vector<ASTNodeRelation*>& numRelations =  pRelation->getNumerRelations();
        std::vector<ASTNodeRelation*>& denomRelations =  pRelation->getDenomRelations();

        denomRelations.push_back(pRelation);
        for (const auto& pCurRelation: m_vpASTNodeRelation)
        {
            if (pCurRelation != pRelation)
            {
                numRelations.push_back(pCurRelation);
            }
        }
    }

    void ASTJoinAttributesComputeVisitor::visitNodeRelation(ASTNodeRelation* pElement)
    {
        pElement->checkAndUpdateJoinAttributes();
        const auto& relationName = pElement->getRelationName();
        const auto& formJoinAttrNames = getFormateJoinAttributeNames(pElement->getJoinAttributeNames());
        FIGARO_LOG_DBG("relation", relationName, "joinAttributeNames", formJoinAttrNames);
        //initializeEnumAndDenomRelations(pElement);
    }

    void ASTJoinAttributesComputeVisitor::visitNodeJoin(ASTNodeJoin* pElement)
    {
        FIGARO_LOG_DBG("Join");
        FIGARO_LOG_DBG("Central");
        for (const auto& pChild: pElement->getChildren())
        {
            FIGARO_LOG_DBG("Child");
            pChild->accept(this);
        }
        pElement->checkAndUpdateJoinAttributes();
        pElement->checkAndUpdateChildrenParJoinAttributes();

        //initializeEnumAndDenomRelations(pElement->getCentralRelation());
        const auto& relationName = pElement->getCentralRelation()->getRelationName();
        const auto& formJoinAttrNames = getFormateJoinAttributeNames(pElement->getJoinAttributeNames());
        FIGARO_LOG_DBG("relation", relationName, "joinAttributeNames", formJoinAttrNames);
    }

    void ASTJoinAttributesComputeVisitor::visitNodeQRGivens(ASTNodeQRGivens* pElement)
    {
        FIGARO_LOG_DBG("QR Givens");
        FIGARO_LOG_DBG("Relation order", pElement->getRelationOrder())
        for (const auto& relName: pElement->getRelationOrder())
        {
            m_vpASTNodeRelation.push_back(m_mRelNameASTNodeRel.at(relName));
        }
        pElement->getOperand()->accept(this);

    }

}