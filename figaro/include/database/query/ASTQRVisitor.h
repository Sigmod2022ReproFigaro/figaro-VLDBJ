#ifndef _FIGARO_AST_QR_VISITOR_H_
#define _FIGARO_AST_QR_VISITOR_H_

#include "ASTVisitor.h"
#include "ASTVisitorAbsResult.h"

namespace Figaro
{
    class ASTQRVisitor: public ASTVisitor
    {
    public:
        ASTQRVisitor(Database* pDatabase):
            ASTVisitor(pDatabase)
         {}
        virtual ASTVisitorAbsResult* visitNodePostProcQR(ASTNodePostProcQR* pElement) override {}

        virtual ASTVisitorAbsResult* visitNodeAssign(ASTNodeAssign* pElement)  override
        {
            // This line should not happen.
            FIGARO_LOG_ASSERT(false);
            return nullptr;
        }

        virtual ~ASTQRVisitor() override {}


    };
}

#endif