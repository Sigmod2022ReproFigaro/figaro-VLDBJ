#include "database/query/node/ASTNodeSVDAlgDec.h"
#include "database/query/visitor/ASTVisitor.h"
#include "database/Database.h"

namespace Figaro
{

    ASTVisitorResultAbs* ASTNodeSVDAlgDec::accept(ASTVisitor *pVisitor)
    {
        return pVisitor->visitNodeSVDDecAlg(this);
    }
}