#include "database/query/ASTNodeRelation.h"
#include "database/query/ASTVisitor.h"
#include "database/Database.h"

namespace Figaro 
{

     void ASTNodeRelation::checkAndUpdateJoinAttributes(ASTNodeAbsRelation* pNodeAbsRelation)
    {
        if (nullptr != pNodeAbsRelation)
        {
            const std::vector<std::string> vAttrNames = pNodeAbsRelation->getAttributeNames();
            for (const auto& attrName: vAttrNames)
            {
                setJoinAttribute(attrName);
            }
        }
    }

    void ASTNodeRelation::setJoinAttribute(const std::string& attrName)
    {
        if (m_mIsJoinAttr.find(attrName) != m_mIsJoinAttr.end())
        {
            m_mIsJoinAttr[attrName] = true;
        }
    }

    void ASTNodeRelation::moveFromNumToDenum(ASTNodeRelation* pRelation)
    {
        const auto& itRelation = std::find(m_vpASTNodeRelNumer.begin(), m_vpASTNodeRelNumer.end(), pRelation);
        if (itRelation != m_vpASTNodeRelNumer.end())
        {
            FIGARO_LOG_DBG("itRelation", (*itRelation)->getRelationName())
            for (const auto& pCurRel: m_vpASTNodeRelNumer)
            {
                FIGARO_LOG_DBG("numer", pCurRel->getRelationName())
            }
            for (const auto& pCurRel: m_vpASTNodeRelDenom)
            {
                FIGARO_LOG_DBG("denom", pCurRel->getRelationName())
            }

            m_vpASTNodeRelNumer.erase(itRelation);
            FIGARO_LOG_DBG("Erasure passed")

            m_vpASTNodeRelDenom.push_back(pRelation);
            FIGARO_LOG_DBG("AddedRElation")
        }
         for (const auto& pCurRel: m_vpASTNodeRelNumer)
            {
                FIGARO_LOG_DBG("numer", pCurRel->getRelationName())
            }
            for (const auto& pCurRel: m_vpASTNodeRelDenom)
            {
                FIGARO_LOG_DBG("denom", pCurRel->getRelationName())
            }
    }

    void ASTNodeRelation::checkAndUpdateJoinAttributes(void) 
    {
        checkAndUpdateJoinAttributes(getParent());
    }

    const std::vector<std::string>& ASTNodeRelation::getJoinAttributeNames(void) 
    {
        if (m_vJoinAttributeNames.size() == 0)
        {
            for (auto const& [attrName, isJoinAttr]: m_mIsJoinAttr)
            {
                if (isJoinAttr)
                {
                    m_vJoinAttributeNames.push_back(attrName);
                }
            }
        }
        return m_vJoinAttributeNames;
    }

    void ASTNodeRelation::accept(ASTVisitor *pVisitor) 
    {
        pVisitor->visitNodeRelation(this);
    }

    MatrixEigenT* ASTNodeRelation::computeHead(Database* pDatabase) const
    {
        return pDatabase->computeHead(m_relationName);
    }

    MatrixEigenT* ASTNodeRelation::computeTail(Database* pDatabase) const
    {
        return pDatabase->computeTail(m_relationName);
    }

    void ASTNodeRelation::computeHeadSingleThreaded(
        const std::vector<std::string>& joinAttributes) const
    {
        for (const auto& joinAttribute: joinAttributes)
        {
            
        } 
    }
}