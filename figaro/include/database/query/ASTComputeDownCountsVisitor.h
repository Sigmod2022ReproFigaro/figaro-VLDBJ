#ifndef _FIGARO_AST_FIGARO_COMPUTE_DOWN_COUNTS_VISITOR_H_
#define _FIGARO_AST_FIGARO_COMPUTE_DOWN_COUNTS_VISITOR_H_

#include "ASTQRVisitor.h"

namespace Figaro
{
    class ASTComputeDownCountsVisitor: public ASTQRVisitor
    {
        std::string l2TailnormExpression(ASTNodeRelation* pElement);

    public:
        ASTComputeDownCountsVisitor(
            Database* pDatabase,
            const std::map<std::string, ASTNodeRelation*>& mRelNameASTNodeRel):
                ASTQRVisitor(pDatabase, mRelNameASTNodeRel) {}
        void visitNodeRelation(ASTNodeRelation* pElement) override;
        void visitNodeJoin(ASTNodeJoin* pElement) override;
        void visitNodeQRGivens(ASTNodeQRGivens* pElement) override;

        virtual ~ASTComputeDownCountsVisitor() override {}
    };
}

#endif