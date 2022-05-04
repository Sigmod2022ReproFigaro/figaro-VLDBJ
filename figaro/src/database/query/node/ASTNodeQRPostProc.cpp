#include "database/query/node/ASTNodeQRPostProc.h"
#include "database/query/visitor/ASTVisitor.h"
#include "database/Database.h"

namespace Figaro
{

    ASTVisitorAbsResult* ASTNodeQRPostProc::accept(ASTVisitor *pVisitor)
    {
        return pVisitor->visitNodePostProcQR(this);
    }
}