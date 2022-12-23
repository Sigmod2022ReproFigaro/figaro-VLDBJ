#include "database/query/node/ASTNodeSVDFigaro.h"
#include "database/query/visitor/ASTVisitor.h"
#include "database/Database.h"

namespace Figaro
{

    ASTVisitorResultAbs* ASTNodeSVDFigaro::accept(ASTVisitor *pVisitor)
    {
        return pVisitor->visitNodeSVDFigaro(this);
    }
}