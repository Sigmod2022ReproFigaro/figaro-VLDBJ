#include "database/query/node/ASTNodeQRGivens.h"
#include "database/query/visitor/ASTVisitor.h"
#include "database/Database.h"

namespace Figaro
{

    ASTVisitorAbsResult* ASTNodeQRGivens::accept(ASTVisitor *pVisitor)
    {
        return pVisitor->visitNodeQRGivens(this);
    }
}