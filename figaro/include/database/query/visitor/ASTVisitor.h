#ifndef _FIGARO_AST_VISITOR_H_
#define _FIGARO_AST_VISITOR_H_

#include "database/query/node/ASTNode.h"
#include "database/query/node/ASTNodeJoin.h"
#include "database/query/node/ASTNodeQRFigaro.h"
#include "database/query/node/ASTNodeQRPostProc.h"
#include "database/query/node/ASTNodeAssign.h"
#include "database/query/node/ASTNodeRelation.h"
#include "database/query/node/ASTNodeEvalJoin.h"
#include "database/query/node/ASTNodeRightMultiply.h"
#include "database/query/node/ASTNodeInverse.h"
#include "database/query/node/ASTNodeLinReg.h"
#include "database/query/node/ASTNodeLUFigaro.h"
#include "./result/ASTVisitorResultAbs.h"
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
        virtual ASTVisitorResultAbs* visitNodeRelation(ASTNodeRelation* pElement) = 0;
        virtual ASTVisitorResultAbs* visitNodeJoin(ASTNodeJoin* pElement) = 0;
        virtual ASTVisitorResultAbs* visitNodeQRFigaro(ASTNodeQRFigaro* pElement) = 0;
        virtual ASTVisitorResultAbs* visitNodeQRPostProc(ASTNodeQRPostProc* pElement) = 0;
        virtual ASTVisitorResultAbs* visitNodeLUFigaro(ASTNodeLUFigaro* pElement) = 0;
        virtual ASTVisitorResultAbs* visitNodeAssign(ASTNodeAssign* pElement) = 0;
        virtual ASTVisitorResultAbs* visitNodeEvalJoin(ASTNodeEvalJoin* pElement) = 0;
        virtual ASTVisitorResultAbs* visitNodeRightMultiply(ASTNodeRightMultiply* pElement) = 0;
        virtual ASTVisitorResultAbs* visitNodeInverse(ASTNodeInverse* pElement) = 0;
        virtual ASTVisitorResultAbs* visitNodeLinReg(ASTNodeLinReg* pElement) = 0;

        virtual ~ASTVisitor() {}
    };
}

#endif