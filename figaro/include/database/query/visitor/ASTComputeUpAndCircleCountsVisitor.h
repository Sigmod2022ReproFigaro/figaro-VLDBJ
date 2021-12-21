#ifndef _FIGARO_AST_COMPUTE_UP_AND_CIRCLE_VISITOR_H_
#define _FIGARO_AST_COMPUTE_UP_AND_CIRCLE_VISITOR_H_

#include "ASTVisitorQRGivensAbs.h"

namespace Figaro
{
    class ASTComputeUpAndCircleCountsVisitor: public ASTVisitorQRGivensAbs
    {
    public:
        ASTComputeUpAndCircleCountsVisitor(
            Database* pDatabase): ASTVisitorQRGivensAbs(pDatabase) {}
        ASTVisitorAbsResult* visitNodeRelation(ASTNodeRelation* pElement) override;
        ASTVisitorAbsResult* visitNodeJoin(ASTNodeJoin* pElement) override;
        ASTVisitorAbsResult* visitNodeQRGivens(ASTNodeQRGivens* pElement) override;

        virtual ~ASTComputeUpAndCircleCountsVisitor() override {}
    };
}

#endif