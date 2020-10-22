#include "database/query/Query.h"

namespace Figaro
{
    void Query::destroyAST(ASTNode* pASTRoot)
    {
        if (nullptr == pASTRoot)
        {
            return;
        }
    }

    void Query::loadQuery(const std::string& queryConfigPath)
    {

    }

     void Query::evaluateQuery(void)
     {

     }
}