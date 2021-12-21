#ifndef _FIGARO_QUERY_H_
#define _FIGARO_QUERY_H_

#include "utils/Utils.h"
#include "database/query/node/ASTNode.h"
#include "database/query/node/ASTNodeRelation.h"
#include "database/Database.h"
#include "database/storage/Matrix.h"

namespace Figaro
{
    class Query
    {
        ASTNode* m_pASTRoot = nullptr;
        Database* m_pDatabase = nullptr;
        MatrixEigenT m_matResult;
        bool m_saveResult;
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
            bool evalSecondFigaroPass = true, bool evalPostProcess = true,
            Figaro::QRGivensHintType qrHintType = Figaro::QRGivensHintType::THIN_DIAG,
            Figaro::MemoryLayout  = Figaro::MemoryLayout::ROW_MAJOR,
            bool saveResult = false);

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