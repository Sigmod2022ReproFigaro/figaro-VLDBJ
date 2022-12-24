#include "database/query/node/ASTNodeSVDSVTInv.h"
#include "database/query/visitor/ASTVisitor.h"
#include "database/Database.h"

namespace Figaro
{

    ASTVisitorResultAbs* ASTNodeSVDSVTInverse::accept(ASTVisitor *pVisitor)
    {
        return pVisitor->visitNodeSVDSVTInverse(this);
    }
}