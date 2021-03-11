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
        const auto& itRelation = std::find(m_vpASTNodeRelNum.begin(), m_vpASTNodeRelNum.end(), pRelation);
        m_vpASTNodeRelNum.erase(itRelation);
        m_vpASTNodeRelDenom.push_back(pRelation);
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