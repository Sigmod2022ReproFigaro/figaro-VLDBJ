#include "database/query/node/ASTNodeInverse.h"
#include "database/query/visitor/ASTVisitor.h"
#include "database/Database.h"

namespace Figaro
{

    ASTVisitorResultAbs* ASTNodeInverse::accept(ASTVisitor *pVisitor)
    {
        return pVisitor->visitNodeInverse(this);
    }
}