#include "database/query/node/ASTNodeAssign.h"
#include "database/query/visitor/ASTVisitor.h"
#include "database/Database.h"

namespace Figaro
{

    ASTVisitorResultAbs* ASTNodeAssign::accept(ASTVisitor *pVisitor)
    {
        return pVisitor->visitNodeAssign(this);
    }
}