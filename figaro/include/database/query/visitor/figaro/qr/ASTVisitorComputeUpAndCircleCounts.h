#ifndef _FIGARO_AST_VISITOR_COMPUTE_UP_AND_CIRCLE_H_
#define _FIGARO_AST_VISITOR_COMPUTE_UP_AND_CIRCLE_H_

#include "ASTVisitorQRFigaroAbs.h"

namespace Figaro
{
    class ASTVisitorComputeUpAndCircleCounts: public ASTVisitorQRFigaroAbs
    {
    public:
        ASTVisitorComputeUpAndCircleCounts(
            Database* pDatabase): ASTVisitorQRFigaroAbs(pDatabase) {}
        ASTVisitorResultAbs* visitNodeRelation(ASTNodeRelation* pElement) override;
        ASTVisitorResultAbs* visitNodeJoin(ASTNodeJoin* pElement) override;
        ASTVisitorResultAbs* visitNodeQRFigaro(ASTNodeQRFigaro* pElement) override;

        virtual ~ASTVisitorComputeUpAndCircleCounts() override {}
    };
}

#endif