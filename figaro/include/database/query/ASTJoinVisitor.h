#ifndef _FIGARO_AST_QR_VISITOR_H_
#define _FIGARO_AST_QR_VISITOR_H_

#include "ASTVisitor.h"
#include "ASTVisitorJoinResult.h"

namespace Figaro
{
    class ASTJoinVisitor: public ASTVisitor
    {
    public:
        ASTJoinVisitor(Database* pDatabase):
            ASTVisitor(pDatabase)
         {}
        virtual ASTVisitorJoinResult* visitNodeRelation(ASTNodeRelation* pElement) override;
        virtual ASTVisitorJoinResult* visitNodeJoin(ASTNodeJoin* pElement) override;
        virtual ASTVisitorJoinResult* visitNodeQRGivens(ASTNodeQRGivens* pElement) override {}
        virtual ASTVisitorJoinResult* visitNodePostProcQR(ASTNodePostProcQR* pElement) override {}
        virtual ~ASTJoinVisitor() override {}
        virtual ASTVisitorAbsResult* visitNodeAssign(ASTNodeAssign* pElement)  override;


    };
}

#endif