#ifndef _FIGARO_AST_COMPUTE_UP_AND_CIRCLE_VISITOR_H_
#define _FIGARO_AST_COMPUTE_UP_AND_CIRCLE_VISITOR_H_

#include "ASTVisitorQRFigaroAbs.h"

namespace Figaro
{
    class ASTComputeUpAndCircleCountsVisitor: public ASTVisitorQFigaroAbs
    {
    public:
        ASTComputeUpAndCircleCountsVisitor(
            Database* pDatabase): ASTVisitorQFigaroAbs(pDatabase) {}
        ASTVisitorResultAbs* visitNodeRelation(ASTNodeRelation* pElement) override;
        ASTVisitorResultAbs* visitNodeJoin(ASTNodeJoin* pElement) override;
        ASTVisitorResultAbs* visitNodeQRFigaro(ASTNodeQRFigaro* pElement) override;

        virtual ~ASTComputeUpAndCircleCountsVisitor() override {}
    };
}

#endif