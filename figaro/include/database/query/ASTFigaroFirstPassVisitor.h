#ifndef _FIGARO_AST_FIGARO_FIRST_PASS_VISITOR_H_
#define _FIGARO_AST_FIGARO_FIRST_PASS_VISITOR_H_

#include "ASTQRVisitor.h"

namespace Figaro
{
    class ASTFigaroFirstPassVisitor: public ASTQRVisitor
    {
    public:
        ASTFigaroFirstPassVisitor(
            Database* pDatabase): ASTQRVisitor(pDatabase) {}
        void visitNodeRelation(ASTNodeRelation* pElement) override;
        void visitNodeJoin(ASTNodeJoin* pElement) override;
        void visitNodeQRGivens(ASTNodeQRGivens* pElement) override;

        virtual ~ASTFigaroFirstPassVisitor() override {}
    };
}

#endif