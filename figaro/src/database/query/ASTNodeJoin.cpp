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

    const std::vector<std::string>& ASTNodeJoin::getJoinAttributeNames(void)
    {
        return m_pCenRelation->getJoinAttributeNames();
    }

    void ASTNodeJoin::accept(ASTVisitor *pVisitor) 
    {
        pVisitor->visitNodeJoin(this);
    }
}