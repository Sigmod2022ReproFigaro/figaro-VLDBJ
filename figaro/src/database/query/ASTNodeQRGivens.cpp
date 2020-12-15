#include "database/query/ASTNodeQRGivens.h"
#include "database/query/ASTVisitor.h"
#include "database/Database.h"

namespace Figaro 
{

    void ASTNodeQRGivens::accept(ASTVisitor *pVisitor) 
    {
        pVisitor->visitNodeQRGivens(this);
    }
}