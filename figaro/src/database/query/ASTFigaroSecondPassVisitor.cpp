#include "database/query/ASTFigaroSecondPassVisitor.h"

namespace Figaro
{

    // TODO: Add for arbitrary number of join attributes.
    void aggregateAwayRelation(ASTNodeAbsRelation* pAbsRel)
    {
        ASTNodeAbsRelation* pParent = pAbsRel->getParent();
        ASTNodeRelation* pRelInst = pAbsRel->getRelation();
        for (const auto& pCurRel: pAbsRel->getRelationPostorder())
        {

            pCurRel->moveFromNumerToDenum(pRelInst);
            pCurRel->moveFromNumerToDenum(pParent->getRelation());
            pParent->getRelation()->moveFromNumerToDenum(pCurRel);
            for (const auto& pCurRelIn: pParent->getRelationPostorder())
            {
                pCurRel->moveFromNumerToDenum(pCurRelIn);
            }
        }
        for (const auto& pCurRel: pParent->getRelationPostorder())
        {

            pCurRel->moveFromNumerToDenum(pRelInst);
            pRelInst->moveFromNumerToDenum(pCurRel);
            pCurRel->moveFromNumerToDenum(pParent->getRelation());
            for (const auto& pCurRelIn: pAbsRel->getRelationPostorder())
            {
                pCurRel->moveFromNumerToDenum(pCurRelIn);
            }
        }
        pRelInst->moveFromNumerToDenum(pRelInst);
        pRelInst->moveFromNumerToDenum(pParent->getRelation());
        pParent->getRelation()->moveFromNumerToDenum(pRelInst);
        pParent->getRelation()->moveFromNumerToDenum(pParent->getRelation());

        // Iterate over all relations and remove aggregated away attributes.
    }

    std::string ASTFigaroSecondPassVisitor::strCountsHeadGeneralized(ASTNodeRelation* pRel)
    {
        std::string str = "";
        str += "\\sqrt{|";
        for (const auto& pCurRel: pRel->getNumerRelations())
        {
            std::string joinAttributes = getFormateJoinAttributeNames(pCurRel->getJoinAttributeNames());
            str +=  pCurRel->getRelationName() + "^{" + joinAttributes + "}" + "\\join";
        }
        str += "| / |";

        for (const auto& pCurRel: pRel->getDenomRelations())
        {
             std::string joinAttributes = getFormateJoinAttributeNames(pCurRel->getJoinAttributeNames());
            str +=  pCurRel->getRelationName() + "^{" + joinAttributes + "}" + "\\join";
        }
        str += "|}";
        return str;
    }

    void ASTFigaroSecondPassVisitor::visitNodeRelation(ASTNodeRelation* pElement)
    {
        FIGARO_LOG_DBG("Finished visiting relation", pElement->getRelationName())
    }

    void ASTFigaroSecondPassVisitor::visitNodeJoin(ASTNodeJoin* pElement)
    {
        FIGARO_LOG_DBG("Join");
        FIGARO_LOG_DBG("Central");
        std::vector<ASTNodeRelation*>&  vRelPostorder = pElement->getRelationPostorder();
        for (const auto& pChild: pElement->getChildren())
        {
            FIGARO_LOG_DBG("Child");
            pChild->accept(this);
        }
        const auto& relationName = pElement->getCentralRelation()->getRelationName();
        m_pDatabase-> aggregateAwayChildrenRelations(
            relationName,
            pElement->getChildrenNames(),
            pElement->getJoinAttributeNames(),
            pElement->getParJoinAttributeNames(),
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