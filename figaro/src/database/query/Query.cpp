#include "database/query/Query.h"
#include "database/query/ASTVisitor.h"

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
        // TODO: Create AST from config 
    }

     void Query::evaluateQuery(void)
     {
         // Create visitor
        ASTVisitor visitor(m_pDatabase);
        // TODO: Visitor to fill out everything in the root. 
        m_pASTRoot->accept(&visitor);
     }
}