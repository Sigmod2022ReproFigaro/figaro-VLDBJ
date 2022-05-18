#include "database/query/node/ASTNodeLULapack.h"
#include "database/query/visitor/ASTVisitor.h"
#include "database/Database.h"

namespace Figaro
{

    ASTVisitorResultAbs* ASTNodeLULapack::accept(ASTVisitor *pVisitor)
    {
        return pVisitor->visitNodeLULapack(this);
    }
}