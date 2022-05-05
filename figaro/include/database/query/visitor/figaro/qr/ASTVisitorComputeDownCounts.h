#ifndef _FIGARO_AST_VISITOR_FIGARO_COMPUTE_DOWN_COUNTS_H_
#define _FIGARO_AST_VISITOR_FIGARO_COMPUTE_DOWN_COUNTS_H_

#include "ASTVisitorQRFigaroAbs.h"

namespace Figaro
{
    class ASTVisitorComputeDownCounts: public ASTVisitorQRFigaroAbs
    {
    public:
        ASTVisitorComputeDownCounts(
            Database* pDatabase): ASTVisitorQRFigaroAbs(pDatabase) {}
        ASTVisitorResultAbs* visitNodeRelation(ASTNodeRelation* pElement) override;
        ASTVisitorResultAbs* visitNodeJoin(ASTNodeJoin* pElement) override;
        ASTVisitorResultAbs* visitNodeQRFigaro(ASTNodeQRFigaro* pElement) override;

        virtual ~ASTVisitorComputeDownCounts() override {}
    };
}

#endif