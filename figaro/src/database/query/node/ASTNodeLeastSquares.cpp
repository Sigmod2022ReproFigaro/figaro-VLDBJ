#include "database/query/node/ASTNodeLeastSquares.h"
#include "database/query/visitor/ASTVisitor.h"
#include "database/Database.h"

namespace Figaro
{

    ASTVisitorResultAbs* ASTNodeLeastSquares::accept(ASTVisitor *pVisitor)
    {
        return pVisitor->visitNodeLeastSquares(this);
    }
}