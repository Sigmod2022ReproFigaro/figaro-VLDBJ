#include "database/query/node/ASTNodeEvalJoin.h"
#include "database/query/visitor/ASTVisitor.h"
#include "database/Database.h"

namespace Figaro
{

    ASTVisitorAbsResult* ASTNodeEvalJoin::accept(ASTVisitor *pVisitor)
    {
        return pVisitor->visitNodeEvalJoin(this);
    }
}