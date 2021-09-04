#ifndef _FIGARO_AST_QR_VISITOR_H_
#define _FIGARO_AST_QR_VISITOR_H_

#include "ASTVisitor.h"

namespace Figaro
{
    class ASTQRVisitor: public ASTVisitor
    {
    public:
        ASTQRVisitor(Database* pDatabase):
            ASTVisitor(pDatabase)
         {}
        virtual void visitNodePostProcQR(ASTNodePostProcQR* pElement) override {}
        virtual ~ASTQRVisitor() override {}


    };
}

#endif