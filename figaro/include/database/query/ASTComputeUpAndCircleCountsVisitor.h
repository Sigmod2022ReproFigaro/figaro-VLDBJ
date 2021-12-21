#ifndef _FIGARO_AST_COMPUTE_UP_AND_CIRCLE_VISITOR_H_
#define _FIGARO_AST_COMPUTE_UP_AND_CIRCLE_VISITOR_H_

#include "ASTQRVisitor.h"

namespace Figaro
{
    class ASTComputeUpAndCircleCountsVisitor: public ASTQRVisitor
    {
    public:
        ASTComputeUpAndCircleCountsVisitor(
            Database* pDatabase): ASTQRVisitor(pDatabase) {}
        ASTVisitorAbsResult* visitNodeRelation(ASTNodeRelation* pElement) override;
        ASTVisitorAbsResult* visitNodeJoin(ASTNodeJoin* pElement) override;
        ASTVisitorAbsResult* visitNodeQRGivens(ASTNodeQRGivens* pElement) override;

        virtual ~ASTComputeUpAndCircleCountsVisitor() override {}
    };
}

#endif