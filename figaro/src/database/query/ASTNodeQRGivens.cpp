#include "database/query/ASTNodeQRGivens.h"
#include "database/query/ASTVisitor.h"
#include "database/Database.h"

namespace Figaro
{

    ASTVisitorAbsResult* ASTNodeQRGivens::accept(ASTVisitor *pVisitor)
    {
        return pVisitor->visitNodeQRGivens(this);
    }
}