#include "database/query/node/ASTNodeLinReg.h"
#include "database/query/visitor/ASTVisitor.h"
#include "database/Database.h"

namespace Figaro
{

    ASTVisitorAbsResult* ASTNodeLinReg::accept(ASTVisitor *pVisitor)
    {
        return pVisitor->visitNodeLinReg(this);
    }
}