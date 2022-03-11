#ifndef _FIGARO_AST_VISITOR_H_
#define _FIGARO_AST_VISITOR_H_

#include "database/query/node/ASTNode.h"
#include "database/query/node/ASTNodeJoin.h"
#include "database/query/node/ASTNodeQRGivens.h"
#include "database/query/node/ASTNodePostProcQR.h"
#include "database/query/node/ASTNodeAssign.h"
#include "database/query/node/ASTNodeRelation.h"
#include "database/query/node/ASTNodeEvalJoin.h"
#include "database/query/node/ASTNodeRightMultiply.h"
#include "database/query/node/ASTNodeInverse.h"
#include "database/query/node/ASTNodeLinReg.h"
#include "ASTVisitorAbsResult.h"
#include "database/Database.h"

namespace Figaro
{
    class ASTVisitor
    {
    protected:
        Database* m_pDatabase;
        static std::string getFormateJoinAttributeNames(
            const std::vector<std::string>& vJoinAttributeNames);
    public:
        ASTVisitor(Database* pDatabase): m_pDatabase(pDatabase) {}
        virtual ASTVisitorAbsResult* visitNodeRelation(ASTNodeRelation* pElement) = 0;
        virtual ASTVisitorAbsResult* visitNodeJoin(ASTNodeJoin* pElement) = 0;
        virtual ASTVisitorAbsResult* visitNodeQRGivens(ASTNodeQRGivens* pElement) = 0;
        virtual ASTVisitorAbsResult* visitNodePostProcQR(ASTNodePostProcQR* pElement) = 0;
        virtual ASTVisitorAbsResult* visitNodeAssign(ASTNodeAssign* pElement) = 0;
        virtual ASTVisitorAbsResult* visitNodeEvalJoin(ASTNodeEvalJoin* pElement) = 0;
        virtual ASTVisitorAbsResult* visitNodeRightMultiply(ASTNodeRightMultiply* pElement) = 0;
        virtual ASTVisitorAbsResult* visitNodeInverse(ASTNodeInverse* pElement) = 0;
        virtual ASTVisitorAbsResult* visitNodeLinReg(ASTNodeLinReg* pElement) = 0;

        virtual ~ASTVisitor() {}
    };
}

#endif