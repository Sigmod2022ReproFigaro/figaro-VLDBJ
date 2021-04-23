#include "database/query/ASTFigaroSecondPassVisitor.h"

namespace Figaro
{
    void ASTFigaroSecondPassVisitor::visitNodeRelation(ASTNodeRelation* pElement)
    {
        FIGARO_LOG_DBG("Finished visiting relation", pElement->getRelationName())
    }

    void ASTFigaroSecondPassVisitor::visitNodeJoin(ASTNodeJoin* pElement)
    {
        FIGARO_LOG_DBG("Join");
        FIGARO_LOG_DBG("Central");
        for (const auto& pChild: pElement->getChildren())
        {
            FIGARO_LOG_DBG("Child");
            pChild->accept(this);
        }
        const auto& relationName = pElement->getCentralRelation()->getRelationName();
        FIGARO_LOG_DBG("vpChildRels", pElement->getChildrenNames())
        m_pDatabase-> aggregateAwayChildrenRelations(
            relationName,
            pElement->getChildrenNames(),
            pElement->getJoinAttributeNames(),
            pElement->getChildrenParentJoinAttributeNames());

        m_pDatabase->computeAndScaleGeneralizedHeadAndTail(
            relationName,
            pElement->getJoinAttributeNames(),
            pElement->getParJoinAttributeNames());
    }

    void ASTFigaroSecondPassVisitor::visitNodeQRGivens(ASTNodeQRGivens* pElement)
    {
         FIGARO_LOG_DBG("********************");
        FIGARO_LOG_DBG("QR Givens");
        FIGARO_LOG_DBG("Relation order", pElement->getRelationOrder())
        for (const auto& relName: pElement->getRelationOrder())
        {
            m_vpASTNodeRelation.push_back(m_mRelNameASTNodeRel.at(relName));
        }
        pElement->getOperand()->accept(this);
        FIGARO_LOG_DBG("FInished")

    }

}