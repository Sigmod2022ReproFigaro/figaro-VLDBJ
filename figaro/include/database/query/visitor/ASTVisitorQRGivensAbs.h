#ifndef _FIGARO_AST_VISITOR_QR_GIVENS_ABS_H_
#define _FIGARO_AST_VISITOR_QR_GIVENS_ABS_H_

#include "ASTVisitor.h"
#include "ASTVisitorAbsResult.h"

namespace Figaro
{
    class ASTVisitorQRGivensAbs: public ASTVisitor
    {
    public:
        ASTVisitorQRGivensAbs(Database* pDatabase):
            ASTVisitor(pDatabase)
         {}
        virtual ASTVisitorAbsResult* visitNodePostProcQR(ASTNodePostProcQR* pElement) override {
            return nullptr;
        }

        virtual ASTVisitorAbsResult* visitNodeAssign(ASTNodeAssign* pElement)  override
        {
            // This line should not happen.
            return nullptr;
        }

        virtual ASTVisitorAbsResult* visitNodeEvalJoin(ASTNodeEvalJoin* pElement)  override
        {
            // This line should not happen.
            return nullptr;
        }

        virtual ~ASTVisitorQRGivensAbs() override {}


    };
}

#endif