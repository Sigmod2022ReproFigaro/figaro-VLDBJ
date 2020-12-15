#include "database/query/ASTNodeJoin.h"
#include "database/query/ASTVisitor.h"
#include "database/Database.h"

namespace Figaro 
{

    void ASTNodeJoin::accept(ASTVisitor *pVisitor) 
    {
        pVisitor->visitNodeJoin(this);
    }
}