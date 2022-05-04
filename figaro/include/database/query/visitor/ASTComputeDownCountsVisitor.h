#ifndef _FIGARO_AST_FIGARO_COMPUTE_DOWN_COUNTS_VISITOR_H_
#define _FIGARO_AST_FIGARO_COMPUTE_DOWN_COUNTS_VISITOR_H_

#include "ASTVisitorQRFigaroAbs.h"

namespace Figaro
{
    class ASTComputeDownCountsVisitor: public ASTVisitorQFigaroAbs
    {
    public:
        ASTComputeDownCountsVisitor(
            Database* pDatabase): ASTVisitorQFigaroAbs(pDatabase) {}
        ASTVisitorResultAbs* visitNodeRelation(ASTNodeRelation* pElement) override;
        ASTVisitorResultAbs* visitNodeJoin(ASTNodeJoin* pElement) override;
        ASTVisitorResultAbs* visitNodeQRFigaro(ASTNodeQRFigaro* pElement) override;

        virtual ~ASTComputeDownCountsVisitor() override {}
    };
}

#endif