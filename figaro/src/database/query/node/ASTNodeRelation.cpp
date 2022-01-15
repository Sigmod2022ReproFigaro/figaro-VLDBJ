#include "database/query/node/ASTNodeRelation.h"
#include "database/query/visitor/ASTVisitor.h"
#include "database/Database.h"
#include <set>
namespace Figaro
{

    void ASTNodeRelation::setJoinAttribute(const std::string& attrName)
    {
        if (m_mIsJoinAttr.contains(attrName))
        {
            m_mIsJoinAttr[attrName] = true;
        }
    }

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

    void ASTNodeRelation::checkAndUpdateJoinAttributes(void)
    {
        checkAndUpdateJoinAttributes(getParent());
    }

    void ASTNodeRelation::updateParJoinAttrs(ASTNodeAbsRelation* pParent)
    {
        if (m_vParJoinAttributeNames.size() != 0)
        {
            return;
        }
        if (nullptr != pParent)
        {
            const std::vector<std::string> vParJoinAttrs = pParent->getJoinAttributeNames();
            const auto& vParJoinAttrNames = setIntersection(getJoinAttributeNames(), vParJoinAttrs);
            std::set<std::string> sParJoinAttrNames (vParJoinAttrNames.begin(), vParJoinAttrNames.end());
            for (const auto& joinAttrName: getJoinAttributeNames())
            {
                if (sParJoinAttrNames.find(joinAttrName) != sParJoinAttrNames.end())
                {
                    m_vParJoinAttributeNames.push_back(joinAttrName);
                }
            }
        }
    }

    void ASTNodeRelation::updateParJoinAttrs(void)
    {
        updateParJoinAttrs(getParent());
    }

    const std::vector<std::string>& ASTNodeRelation::getJoinAttributeNames(void)
    {
        if (m_vJoinAttributeNames.size() == 0)
        {
            for (const auto& attrName: m_vAttributeNames)
            {
                if (m_mIsJoinAttr[attrName])
                {
                    m_vJoinAttributeNames.push_back(attrName);
                }
            }
        }
        return m_vJoinAttributeNames;
    }

    ASTVisitorAbsResult* ASTNodeRelation::accept(ASTVisitor *pVisitor)
    {
        return pVisitor->visitNodeRelation(this);
    }
}