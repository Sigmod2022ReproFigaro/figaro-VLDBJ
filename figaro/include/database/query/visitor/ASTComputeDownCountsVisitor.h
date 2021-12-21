#ifndef _FIGARO_AST_FIGARO_COMPUTE_DOWN_COUNTS_VISITOR_H_
#define _FIGARO_AST_FIGARO_COMPUTE_DOWN_COUNTS_VISITOR_H_

#include "ASTVisitorQRGivensAbs.h"

namespace Figaro
{
    class ASTComputeDownCountsVisitor: public ASTVisitorQRGivensAbs
    {
    public:
        ASTComputeDownCountsVisitor(
            Database* pDatabase): ASTVisitorQRGivensAbs(pDatabase) {}
        ASTVisitorAbsResult* visitNodeRelation(ASTNodeRelation* pElement) override;
        ASTVisitorAbsResult* visitNodeJoin(ASTNodeJoin* pElement) override;
        ASTVisitorAbsResult* visitNodeQRGivens(ASTNodeQRGivens* pElement) override;

        virtual ~ASTComputeDownCountsVisitor() override {}
    };
}

#endif