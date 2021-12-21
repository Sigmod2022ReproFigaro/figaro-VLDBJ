#ifndef _FIGARO_AST_VISITOR_H_
#define _FIGARO_AST_VISITOR_H_

#include "ASTNode.h"
#include "ASTNodeJoin.h"
#include "ASTNodeQRGivens.h"
#include "ASTNodePostProcQR.h"
#include "ASTNodeAssign.h"
#include "ASTNodeRelation.h"
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

        virtual ~ASTVisitor() {}
    };
}

#endif