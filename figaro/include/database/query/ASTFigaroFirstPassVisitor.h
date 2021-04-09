#ifndef _FIGARO_AST_FIGARO_FIRST_PASS_VISITOR_H_
#define _FIGARO_AST_FIGARO_FIRST_PASS_VISITOR_H_

#include "ASTQRVisitor.h"

namespace Figaro
{
    class ASTFigaroFirstPassVisitor: public ASTQRVisitor
    {
        std::string l2TailnormExpression(ASTNodeRelation* pElement);

    public:
        ASTFigaroFirstPassVisitor(
            Database* pDatabase,
            const std::map<std::string, ASTNodeRelation*>& mRelNameASTNodeRel):
                ASTQRVisitor(pDatabase, mRelNameASTNodeRel) {}
        void visitNodeRelation(ASTNodeRelation* pElement) override;
        void visitNodeJoin(ASTNodeJoin* pElement) override;
        void visitNodeQRGivens(ASTNodeQRGivens* pElement) override;

        virtual ~ASTFigaroFirstPassVisitor() override {}
    };
}

#endif