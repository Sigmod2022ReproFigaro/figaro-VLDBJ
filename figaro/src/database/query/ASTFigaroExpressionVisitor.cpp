#include "database/query/ASTFigaroExpressionVisitor.h"

namespace Figaro
{

    static std::string getFormateJoinAttributeNames(std::vector<std::string> vJoinAttributeNames)
    {
        std::string formatedStr = "";

        for (uint32_t idx = 0; idx < vJoinAttributeNames.size(); idx++)
        {
            if (idx > 0)
            {
                formatedStr += ",";
            }
            formatedStr += vJoinAttributeNames[idx];
        }
        return formatedStr;
    }

    std::vector<std::string> ASTFigaroExpressionVisitor::setIntersection(const std::vector<std::string>& vStr1, const std::vector<std::string>& vStr2)
    {
        std::map<std::string, bool> sStrAppears; 
        std::vector<std::string> vIntersection;
        FIGARO_LOG_DBG("set1", vStr1, "set2", vStr2)
        for (const auto& str: vStr1)
        {
            sStrAppears[str] = false;
        }
        for (const auto& str: vStr2)
        {
            if (sStrAppears.find(str) != sStrAppears.end())
            {
                sStrAppears[str] = true;
            }
        }
        for (const auto&[key, exists]: sStrAppears)
        {
            if (exists)
            {
                vIntersection.push_back(key);
            }
        }
        return vIntersection;
    }

    std::string ASTFigaroExpressionVisitor::l2TailnormExpression(ASTNodeRelation* pElement)
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

    void ASTFigaroExpressionVisitor::visitNodeRelation(ASTNodeRelation* pElement)
    {
        const auto& relationName = pElement->getRelationName();
        const auto& formJoinAttrNames = getFormateJoinAttributeNames(pElement->getJoinAttributeNames());
        std::string strSqrt = l2TailnormExpression(pElement);
        FIGARO_LOG_DBG("\\vcat_{", formJoinAttrNames, "} T(", relationName, "^{", formJoinAttrNames, "})", strSqrt);
    }

    void ASTFigaroExpressionVisitor::visitNodeJoin(ASTNodeJoin* pElement)
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

    void ASTFigaroExpressionVisitor::visitNodeQRGivens(ASTNodeQRGivens* pElement)
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