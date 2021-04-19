#include "database/query/ASTComputeDownCountsVisitor.h"

namespace Figaro
{
    std::string ASTComputeDownCountsVisitor::l2TailnormExpression(ASTNodeRelation* pElement)
    {
        std::string strSqrt = "\\sqrt{";
        for (const auto& pASTNodeRel: m_vpASTNodeRelation)
        {
            if (pASTNodeRel->getRelationName() == pElement->getRelationName())
            {
                continue;
            }
            strSqrt += pASTNodeRel->getRelationName();
            const auto& attrInter = getFormateJoinAttributeNames(setIntersection(pElement->getJoinAttributeNames(), pASTNodeRel->getJoinAttributeNames()));
            strSqrt += "^{" + attrInter + "}" + " \\join ";
        }
        for (uint32_t idx = 0; idx < 6; idx ++)
        {
            strSqrt.pop_back();
        }
        strSqrt += "}";
        return strSqrt;
    }

    void ASTComputeDownCountsVisitor::visitNodeRelation(ASTNodeRelation* pElement)
    {
        bool isRootNode;
        std::vector<std::string> childrenNames;
        std::vector<std::vector<std::string> > vvChildrenParentJoinAttributeNames;
        const auto& relationName = pElement->getRelationName();
        const auto& formJoinAttrNames = getFormateJoinAttributeNames(pElement->getJoinAttributeNames());

        m_pDatabase->sortRelation(pElement->getRelationName(),
                                  pElement->getJoinAttributeNames());
        isRootNode = pElement->getParent() == nullptr;
        m_pDatabase->computeDownCounts(relationName, childrenNames,
            pElement->getJoinAttributeNames(), pElement->getParJoinAttributeNames(),
            vvChildrenParentJoinAttributeNames, isRootNode);

        // TODO: Add compute up counts
    }

    void ASTComputeDownCountsVisitor::visitNodeJoin(ASTNodeJoin* pElement)
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
        m_pDatabase->sortRelation(pElement->getCentralRelation()->getRelationName(),
                                  pElement->getJoinAttributeNames());
        isRootNode = pElement->getParent() == nullptr;
        m_pDatabase->computeDownCounts(relationName, pElement->getChildrenNames(),
            pElement->getJoinAttributeNames(), pElement->getParJoinAttributeNames(),
            pElement->getChildrenParentJoinAttributeNames(), isRootNode);

        // TODO: Add compute up counts
    }

    void ASTComputeDownCountsVisitor::visitNodeQRGivens(ASTNodeQRGivens* pElement)
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