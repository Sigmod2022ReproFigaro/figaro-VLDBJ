#include "database/query/node/ASTNodeQRFigaro.h"
#include "database/query/visitor/ASTVisitor.h"
#include "database/Database.h"

namespace Figaro
{

    ASTVisitorAbsResult* ASTNodeQRFigaro::accept(ASTVisitor *pVisitor)
    {
        return pVisitor->visitNodeQRGivens(this);
    }
}