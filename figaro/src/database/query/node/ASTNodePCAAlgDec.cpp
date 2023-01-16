#include "database/query/node/ASTNodePCAAlgDec.h"
#include "database/query/visitor/ASTVisitor.h"
#include "database/Database.h"

namespace Figaro
{

    ASTVisitorResultAbs* ASTNodePCAAlgDec::accept(ASTVisitor *pVisitor)
    {
        return pVisitor->visitNodePCADecAlg(this);
    }
}