#include "database/query/ASTNodePostProcQR.h"
#include "database/query/ASTVisitor.h"
#include "database/Database.h"

namespace Figaro
{

    void ASTNodePostProcQR::accept(ASTVisitor *pVisitor)
    {
        pVisitor->visitNodePostProcQR(this);
    }
}