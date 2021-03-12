#include "database/query/ASTFigaroSecondPassVisitor.h"

namespace Figaro
{

    // TODO: Add for arbitrary number of join attributes.
    void aggregateAwayRelation(ASTNodeRelation* pRel, ASTNodeJoin* pParent)
    {
        // Aggregate away pElement->getParent() and 
        // Aggregate away each element in postorder of parent
        //ASTNodeJoin* pParent = (ASTNodeJoin*)pRel->getParent();
        pRel->moveFromNumToDenum(pRel);
        pRel->moveFromNumToDenum(pParent->getCentralRelation());
        for (const auto& pCurRel: pParent->getRelationPostorder())
        {
            
            pCurRel->moveFromNumToDenum(pRel);
            pCurRel->moveFromNumToDenum(pParent->getCentralRelation());
        }
        pParent->getCentralRelation()->moveFromNumToDenum(pRel);
        pParent->getCentralRelation()->moveFromNumToDenum(pParent->getCentralRelation());

        // Iterate over all relations and remove aggregated away attributes.
    }

    std::string ASTFigaroSecondPassVisitor::strCountsHeadGenearlized(ASTNodeRelation* pRel)
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
        const auto& relationName = pElement->getRelationName();
        const auto& formJoinAttrNames = getFormateJoinAttributeNames(pElement->getJoinAttributeNames());
        if (nullptr != pElement->getParent())
        {
            aggregateAwayRelation(pElement, (ASTNodeJoin*)pElement->getParent());
        }
        for (const auto& pCurRel: m_vpASTNodeRelation)
        {
            std::string strHeadCounts = strCountsHeadGenearlized(pCurRel);
            FIGARO_LOG_DBG("Relation", pCurRel->getRelationName(), "HeadCounts", strHeadCounts)
        }
        pElement->getRelationPostorder().push_back(pElement);
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
            vRelPostorder.insert(vRelPostorder.begin(), 
                pChild->getRelationPostorder().begin(), pChild->getRelationPostorder().end());
        }
        FIGARO_LOG_DBG("Finished visiting children")

        const auto& relationName = pElement->getCentralRelation()->getRelationName();
        const auto& formJoinAttrNames = getFormateJoinAttributeNames(pElement->getJoinAttributeNames());


        if (nullptr != pElement->getParent())
        {
            aggregateAwayRelation(pElement->getCentralRelation(), (ASTNodeJoin*) pElement->getParent());
        }
        for (const auto& pCurRel: m_vpASTNodeRelation)
        {
            std::string strHeadCounts = strCountsHeadGenearlized(pCurRel);
            FIGARO_LOG_DBG("Relation", pCurRel->getRelationName(), "HeadCounts", strHeadCounts)
        }

        vRelPostorder.push_back(pElement->getCentralRelation());
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