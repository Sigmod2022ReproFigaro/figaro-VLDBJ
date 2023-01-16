#include "database/query/node/ASTNodePCAFigaro.h"
#include "database/query/visitor/ASTVisitor.h"
#include "database/Database.h"

namespace Figaro
{

    ASTVisitorResultAbs* ASTNodePCAFigaro::accept(ASTVisitor *pVisitor)
    {
        return pVisitor->visitNodePCAFigaro(this);
    }
}