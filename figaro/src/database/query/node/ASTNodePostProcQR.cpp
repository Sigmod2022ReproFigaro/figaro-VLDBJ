#include "database/query/node/ASTNodePostProcQR.h"
#include "database/query/visitor/ASTVisitor.h"
#include "database/Database.h"

namespace Figaro
{

    ASTVisitorAbsResult* ASTNodePostProcQR::accept(ASTVisitor *pVisitor)
    {
        return pVisitor->visitNodePostProcQR(this);
    }
}