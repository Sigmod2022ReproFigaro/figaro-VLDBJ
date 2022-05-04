#include "database/query/node/ASTNodeQRPostProc.h"
#include "database/query/visitor/ASTVisitor.h"
#include "database/Database.h"

namespace Figaro
{

    ASTVisitorResultAbs* ASTNodeQRPostProc::accept(ASTVisitor *pVisitor)
    {
        return pVisitor->visitNodeQRPostProc(this);
    }
}