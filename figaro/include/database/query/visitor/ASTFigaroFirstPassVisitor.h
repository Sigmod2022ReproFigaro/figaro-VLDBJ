#ifndef _FIGARO_AST_FIGARO_FIRST_PASS_VISITOR_H_
#define _FIGARO_AST_FIGARO_FIRST_PASS_VISITOR_H_

#include "ASTVisitorQRGivensAbs.h"
#include "ASTVisitorFirstPassResult.h"

namespace Figaro
{
    class ASTFigaroFirstPassVisitor: public ASTVisitorQRGivensAbs
    {
    public:
        ASTFigaroFirstPassVisitor(
            Database* pDatabase): ASTVisitorQRGivensAbs(pDatabase) {}
        ASTVisitorFirstPassResult* visitNodeRelation(ASTNodeRelation* pElement) override;
        ASTVisitorFirstPassResult* visitNodeJoin(ASTNodeJoin* pElement) override;
        ASTVisitorFirstPassResult* visitNodeQRGivens(ASTNodeQRGivens* pElement) override;

        virtual ~ASTFigaroFirstPassVisitor() override {}
    };
}

#endif