#include "database/query/node/ASTNodeLUFigaro.h"
#include "database/query/visitor/ASTVisitor.h"
#include "database/Database.h"

namespace Figaro
{

    ASTVisitorResultAbs* ASTNodeLUFigaro::accept(ASTVisitor *pVisitor)
    {
        return pVisitor->visitNodeLUFigaro(this);
    }
}