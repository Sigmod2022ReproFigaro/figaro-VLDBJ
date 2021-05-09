#ifndef _FIGARO_QUERY_H_
#define _FIGARO_QUERY_H_

#include "utils/Utils.h"
#include "ASTNode.h"
#include "ASTNodeRelation.h"
#include "database/Database.h"

namespace Figaro
{
    class Query
    {
        ASTNode* m_pASTRoot = nullptr;
        Database* m_pDatabase = nullptr;
        MatrixEigenT m_matResult;
        std::map<std::string, ASTNodeRelation*> m_mRelNameASTNodeRel;
        static void destroyAST(ASTNode* pASTRoot);
        ASTNode* createASTFromJson(const json& jsonQueryConfig);
        ErrorCode createAST(const json& jsonQueryConfig);

    public:
        Query(Database* pDatabase): m_pDatabase(pDatabase){}
        ~Query() { destroyAST(m_pASTRoot); }

        /** Parses query and creates abstract syntax tree from
         * configuration specified at the path @p queryConfigPath.
         */
        ErrorCode loadQuery(const std::string& queryConfigPath);

        /** Evaluates expressions from abstract syntax tree.
         */
        void evaluateQuery(bool evalCounts = true, bool evalFirstFigaroPass = true,
            bool evalSecondFigaroPass = true, bool evalPostProcess = true, uint32_t numResp = 1);

        void setDatabase(Database* pDatabase)
        {
             m_pDatabase = pDatabase;
        }

        Database* getDatabase(Database* pDatabase)
        {
             return pDatabase;
        }

        MatrixEigenT& getResult(void)
        {
            return m_matResult;
        }
    };
}


#endif