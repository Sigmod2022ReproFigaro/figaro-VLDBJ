#include "database/query/ASTNodeAssign.h"
#include "database/query/ASTVisitor.h"
#include "database/Database.h"

namespace Figaro
{

    ASTVisitorAbsResult* ASTNodeAssign::accept(ASTVisitor *pVisitor)
    {
        return pVisitor->visitNodeAssign(this);
    }
}