#include "database/query/ASTNodeJoin.h"
#include "database/query/ASTVisitor.h"
#include "database/Database.h"

namespace Figaro
{

    void ASTNodeJoin::checkAndUpdateJoinAttributes(void)
    {
        m_pCenRelation->checkAndUpdateJoinAttributes(getParent());

        for (const auto& pChild: getChildren())
        {
            m_pCenRelation->checkAndUpdateJoinAttributes(pChild);
        }
    }

    void ASTNodeJoin::updateParJoinAttrs(void)
    {
        m_pCenRelation->updateParJoinAttrs(getParent());
    }

    void ASTNodeJoin::checkAndUpdateChildrenParJoinAttributes(void)
    {
        for (const auto& pChild: getChildren())
        {
            pChild->updateParJoinAttrs();
            const auto& vChildParJoinAttrNames = pChild->getParJoinAttributeNames();
            FIGARO_LOG_DBG("childName", pChild->getRelation()->getRelationName(),
            "parAttrNames", vChildParJoinAttrNames)
            m_vvChildParJoinAttributeNames.push_back(vChildParJoinAttrNames);
        }
    }

    const std::vector<std::string>& ASTNodeJoin::getJoinAttributeNames(void)
    {
        return m_pCenRelation->getJoinAttributeNames();
    }

    void ASTNodeJoin::accept(ASTVisitor *pVisitor)
    {
        pVisitor->visitNodeJoin(this);
    }
}