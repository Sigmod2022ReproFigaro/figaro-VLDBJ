#include "database/query/node/ASTNodeSVDLapack.h"
#include "database/query/visitor/ASTVisitor.h"
#include "database/Database.h"

namespace Figaro
{

    ASTVisitorResultAbs* ASTNodeSVDLapack::accept(ASTVisitor *pVisitor)
    {
        return pVisitor->visitNodeSVDLapack(this);
    }
}