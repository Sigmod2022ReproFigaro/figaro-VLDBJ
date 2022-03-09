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
            FIGARO_LOG_ASSERT(1!=1)
            return nullptr;
        }

        virtual ASTVisitorAbsResult* visitNodeAssign(ASTNodeAssign* pElement)  override
        {
            FIGARO_LOG_ASSERT(1!=1)
            return nullptr;
        }

        virtual ASTVisitorAbsResult* visitNodeEvalJoin(ASTNodeEvalJoin* pElement)  override
        {
            FIGARO_LOG_ASSERT(1!=1)
            return nullptr;
        }

        virtual ASTVisitorAbsResult* visitNodeRightMultiply(ASTNodeRightMultiply* pElement)  override
        {
            FIGARO_LOG_ASSERT(1!=1)
            return nullptr;
        }

        virtual ASTVisitorAbsResult* visitNodeInverse(ASTNodeInverse* pElement) override
        {
            FIGARO_LOG_ASSERT(1!=1)
            return nullptr;
        }

        virtual ASTVisitorAbsResult* visitNodeLinReg(ASTNodeLinReg* pElement) override
        {
            FIGARO_LOG_ASSERT(1!=1)
            return nullptr;
        }


        virtual ~ASTVisitorQRGivensAbs() override {}


    };
}

#endif