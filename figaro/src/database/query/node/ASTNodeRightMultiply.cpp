#include "database/query/node/ASTNodeRightMultiply.h"
#include "database/query/visitor/ASTVisitor.h"
#include "database/Database.h"

namespace Figaro
{

    ASTVisitorResultAbs* ASTNodeRightMultiply::accept(ASTVisitor *pVisitor)
    {
        return pVisitor->visitNodeRightMultiply(this);
    }
}