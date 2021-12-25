#ifndef _FIGARO_AST_JOIN_VISITOR_H_
#define _FIGARO_AST_JOIN_VISITOR_H_

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
        virtual ASTVisitorJoinResult* visitNodeQRGivens(ASTNodeQRGivens* pElement) override {
            FIGARO_LOG_ASSERT(1!=1)
            return nullptr;}
        virtual ASTVisitorJoinResult* visitNodePostProcQR(ASTNodePostProcQR* pElement) override {
            FIGARO_LOG_ASSERT(1!=1)
            return nullptr;
        }
        virtual ~ASTJoinVisitor() override {}
        virtual ASTVisitorAbsResult* visitNodeAssign(ASTNodeAssign* pElement)  override;
        virtual ASTVisitorJoinResult* visitNodeEvalJoin(ASTNodeEvalJoin* pElement)  override;


    };
}

#endif