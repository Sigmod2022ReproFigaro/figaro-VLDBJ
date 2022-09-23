#include "database/query/node/ASTNodeLUThin.h"
#include "database/query/visitor/ASTVisitor.h"
#include "database/Database.h"

namespace Figaro
{

    ASTVisitorResultAbs* ASTNodeLUThin::accept(ASTVisitor *pVisitor)
    {
        return pVisitor->visitNodeLUThin(this);
    }
}