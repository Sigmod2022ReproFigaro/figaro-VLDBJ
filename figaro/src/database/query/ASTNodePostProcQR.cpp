#include "database/query/ASTNodePostProcQR.h"
#include "database/query/ASTVisitor.h"
#include "database/Database.h"

namespace Figaro
{

    ASTVisitorAbsResult* ASTNodePostProcQR::accept(ASTVisitor *pVisitor)
    {
        return pVisitor->visitNodePostProcQR(this);
    }
}