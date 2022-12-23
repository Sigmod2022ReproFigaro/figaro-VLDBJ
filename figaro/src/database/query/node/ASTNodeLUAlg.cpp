#include "database/query/node/ASTNodeLUAlg.h"
#include "database/query/visitor/ASTVisitor.h"
#include "database/Database.h"

namespace Figaro
{

    ASTVisitorResultAbs* ASTNodeLUAlg::accept(ASTVisitor *pVisitor)
    {
        return pVisitor->visitNodeLUDecAlg(this);
    }
}