#include "database/query/node/ASTNodeRightMultiply.h"
#include "database/query/visitor/ASTVisitor.h"
#include "database/Database.h"

namespace Figaro
{

    ASTVisitorAbsResult* ASTNodeRightMultiply::accept(ASTVisitor *pVisitor)
    {
        FIGARO_LOG_INFO("VISITING RIGHT MULTIPLY")
        return pVisitor->visitNodeRightMultiply(this);
        //FIGARO_LOG_INFO("ENd")
    }
}