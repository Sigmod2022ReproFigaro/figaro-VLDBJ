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
    public:
        enum class OpType
        {
            DECOMP_QR, DECOMP_LU, DECOMP_SVD
        };
    private:
        ASTNode* m_pASTRoot = nullptr;
        Database* m_pDatabase = nullptr;
        ASTVisitorResultAbs* m_pResult = nullptr;
        bool m_computeAll = false;
        OpType m_opType;
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
        ErrorCode loadQuery(const std::string& queryConfigPath, bool computeAll = false);

        /** Evaluates expressions from abstract syntax tree.
         */
        void evaluateQuery(bool saveMemory = true,
            const std::map<std::string, bool>& mFlags = {},
            Figaro::QRHintType qrHintType = Figaro::QRHintType::THIN_DIAG,
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

        ASTVisitorResultAbs* getResult(void)
        {
            return m_pResult;
        }

        Query::OpType getOpType(void) const
        {
            return m_opType;
        }
    };
}


#endif