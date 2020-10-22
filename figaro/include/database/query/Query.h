#ifndef _FIGARO_QUERY_H_
#define _FIGARO_QUERY_H_

#include "utils/Utils.h"
#include "ASTNode.h"
#include "database/Database.h"

namespace Figaro
{
    class Query 
    {
        ASTNode* m_pASTRoot = nullptr;
        Database* m_pDatabase = nullptr; 
        static void destroyAST(ASTNode* pASTRoot);
    public:
        Query(Database* pDatabase): m_pDatabase(pDatabase) {} 
        ~Query() { destroyAST(m_pASTRoot); }

        /** Parses query and creates abstract syntax tree from 
         * configuration specified at the path @p queryConfigPath. 
         */
        void loadQuery(const std::string& queryConfigPath); 
        
        /** Evaulates expressions from abstract syntax tree. 
         */
        void evaluateQuery(void); 

        
        void setDatabase(Database* pDatabase) 
        {
             m_pDatabase = pDatabase;     
        }

        Database* getDatabase(Database* pDatabase) 
        {
             return pDatabase;
        }
    };
}


#endif 