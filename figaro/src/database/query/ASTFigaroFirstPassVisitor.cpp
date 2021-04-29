#include "database/query/ASTFigaroFirstPassVisitor.h"

namespace Figaro
{
    void ASTFigaroFirstPassVisitor::visitNodeRelation(ASTNodeRelation* pElement)
    {
        std::vector<std::string> childrenNames;
        std::vector<std::vector<std::string> > vvChildrenParentJoinAttributeNames;
        const auto& relationName = pElement->getRelationName();
        const auto& formJoinAttrNames = getFormateJoinAttributeNames(pElement->getJoinAttributeNames());
        FIGARO_LOG_DBG("formJoinAttrNames", formJoinAttrNames)

        m_pDatabase->computeHeadsAndTails(relationName, pElement->getJoinAttributeNames(),
        true);
    }

    void ASTFigaroFirstPassVisitor::visitNodeJoin(ASTNodeJoin* pElement)
    {
        FIGARO_LOG_DBG("Join");
        FIGARO_LOG_DBG("Central");
        for (const auto& pChild: pElement->getChildren())
        {
            FIGARO_LOG_DBG("Child");
            pChild->accept(this);
        }
        const auto& relationName = pElement->getCentralRelation()->getRelationName();
        const auto& formJoinAttrNames = getFormateJoinAttributeNames(pElement->getJoinAttributeNames());
        FIGARO_LOG_DBG("formJoinAttrNames", formJoinAttrNames)
        m_pDatabase->computeHeadsAndTails(relationName, pElement->getJoinAttributeNames(),
             false);
    }

    void ASTFigaroFirstPassVisitor::visitNodeQRGivens(ASTNodeQRGivens* pElement)
    {
        FIGARO_LOG_DBG("********************");
        FIGARO_LOG_DBG("QR Givens");
        FIGARO_LOG_DBG("Relation order", pElement->getRelationOrder())
        pElement->getOperand()->accept(this);
    }
}