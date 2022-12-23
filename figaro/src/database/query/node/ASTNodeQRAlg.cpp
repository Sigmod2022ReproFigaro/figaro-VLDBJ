#include "database/query/node/ASTNodeQRAlg.h"
#include "database/query/visitor/ASTVisitor.h"
#include "database/Database.h"

namespace Figaro
{

    ASTVisitorResultAbs* ASTNodeQRAlg::accept(ASTVisitor *pVisitor)
    {
        return pVisitor->visitNodeQRDecAlg(this);
    }
}