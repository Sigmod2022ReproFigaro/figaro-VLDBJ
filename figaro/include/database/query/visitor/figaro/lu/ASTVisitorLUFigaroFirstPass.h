#ifndef _FIGARO_AST_VISITOR_LU_FIGARO_FIRST_PASS_H_
#define _FIGARO_AST_VISITOR_LU_FIGARO_FIRST_PASS_H_

#include "ASTVisitorLUFigaroAbs.h"
#include "../../result/ASTVisitorResultFirstPass.h"

namespace Figaro
{
    class ASTVisitorLUFigaroFirstPass: public ASTVisitorLUFigaroAbs
    {
    public:
        ASTVisitorLUFigaroFirstPass(
            Database* pDatabase): ASTVisitorLUFigaroAbs(pDatabase) {}
        ASTVisitorResultFirstPass* visitNodeRelation(ASTNodeRelation* pElement) override;
        ASTVisitorResultFirstPass* visitNodeJoin(ASTNodeJoin* pElement) override;
        ASTVisitorResultFirstPass* visitNodeLUFigaro(ASTNodeLUFigaro* pElement) override;

        virtual ~ASTVisitorLUFigaroFirstPass() override {}
    };
}

#endif