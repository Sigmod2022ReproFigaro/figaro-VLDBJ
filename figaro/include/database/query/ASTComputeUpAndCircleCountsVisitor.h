#ifndef _FIGARO_AST_COMPUTE_UP_AND_CIRCLE_VISITOR_H_
#define _FIGARO_AST_COMPUTE_UP_AND_CIRCLE_VISITOR_H_

#include "ASTQRVisitor.h"

namespace Figaro
{
    class ASTComputeUpAndCircleCountsVisitor: public ASTQRVisitor
    {
    public:
        ASTComputeUpAndCircleCountsVisitor(
            Database* pDatabase,
            const std::map<std::string, ASTNodeRelation*>& mRelNameASTNodeRel):
                ASTQRVisitor(pDatabase, mRelNameASTNodeRel) {}
        void visitNodeRelation(ASTNodeRelation* pElement) override;
        void visitNodeJoin(ASTNodeJoin* pElement) override;
        void visitNodeQRGivens(ASTNodeQRGivens* pElement) override;

        virtual ~ASTComputeUpAndCircleCountsVisitor() override {}
    };
}

#endif