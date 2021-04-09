#include "database/query/ASTFigaroFirstPassVisitor.h"

namespace Figaro
{
    std::string ASTFigaroFirstPassVisitor::l2TailnormExpression(ASTNodeRelation* pElement)
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

    void ASTFigaroFirstPassVisitor::visitNodeRelation(ASTNodeRelation* pElement)
    {
        const auto& relationName = pElement->getRelationName();
        const auto& formJoinAttrNames = getFormateJoinAttributeNames(pElement->getJoinAttributeNames());
        std::string strSqrt = l2TailnormExpression(pElement);
        FIGARO_LOG_DBG("\\vcat_{", formJoinAttrNames, "} T(", relationName, "^{", formJoinAttrNames, "})", strSqrt);
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
        const auto& strSqrt = l2TailnormExpression(pElement->getCentralRelation());
        FIGARO_LOG_DBG("\\vcat_{", formJoinAttrNames, "} T(", relationName, "^{", formJoinAttrNames, "})",
        strSqrt);
    }

    void ASTFigaroFirstPassVisitor::visitNodeQRGivens(ASTNodeQRGivens* pElement)
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