#ifndef _FIGARO_AST_VISITOR_FIGARO_FIRST_PASS_H_
#define _FIGARO_AST_VISITOR_FIGARO_FIRST_PASS_H_

#include "ASTVisitorQRFigaroAbs.h"
#include "./result/ASTVisitorResultFirstPass.h"

namespace Figaro
{
    class ASTVisitorQRFigaroFirstPass: public ASTVisitorQFigaroAbs
    {
    public:
        ASTVisitorQRFigaroFirstPass(
            Database* pDatabase): ASTVisitorQFigaroAbs(pDatabase) {}
        ASTVisitorResultFirstPass* visitNodeRelation(ASTNodeRelation* pElement) override;
        ASTVisitorResultFirstPass* visitNodeJoin(ASTNodeJoin* pElement) override;
        ASTVisitorResultFirstPass* visitNodeQRFigaro(ASTNodeQRFigaro* pElement) override;

        virtual ~ASTVisitorQRFigaroFirstPass() override {}
    };
}

#endif