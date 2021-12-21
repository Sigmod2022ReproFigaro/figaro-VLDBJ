#include "database/query/node/ASTNodeJoin.h"
#include "database/query/visitor/ASTVisitor.h"
#include "database/Database.h"

namespace Figaro
{

    ASTNodeJoin::~ASTNodeJoin()
    {
        for (uint32_t idx = 0; idx < m_vPChild.size(); idx++)
        {
            delete m_vPChild[idx];
        }
        delete m_pCenRelation;
    }

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
        FIGARO_LOG_DBG("Relation parent join attributes of children", getRelation()->getRelationName(), m_vvChildParJoinAttributeNames)
    }

    const std::vector<std::string>& ASTNodeJoin::getJoinAttributeNames(void)
    {
        return m_pCenRelation->getJoinAttributeNames();
    }

    ASTVisitorAbsResult* ASTNodeJoin::accept(ASTVisitor *pVisitor)
    {
        return pVisitor->visitNodeJoin(this);
    }
}