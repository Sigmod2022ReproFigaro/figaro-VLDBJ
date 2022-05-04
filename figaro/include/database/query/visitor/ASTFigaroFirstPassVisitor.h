#ifndef _FIGARO_AST_FIGARO_FIRST_PASS_VISITOR_H_
#define _FIGARO_AST_FIGARO_FIRST_PASS_VISITOR_H_

#include "ASTVisitorQRFigaroAbs.h"
#include "./result/ASTVisitorResultFirstPass.h"

namespace Figaro
{
    class ASTFigaroFirstPassVisitor: public ASTVisitorQFigaroAbs
    {
    public:
        ASTFigaroFirstPassVisitor(
            Database* pDatabase): ASTVisitorQFigaroAbs(pDatabase) {}
        ASTVisitorResultFirstPass* visitNodeRelation(ASTNodeRelation* pElement) override;
        ASTVisitorResultFirstPass* visitNodeJoin(ASTNodeJoin* pElement) override;
        ASTVisitorResultFirstPass* visitNodeQRFigaro(ASTNodeQRFigaro* pElement) override;

        virtual ~ASTFigaroFirstPassVisitor() override {}
    };
}

#endif