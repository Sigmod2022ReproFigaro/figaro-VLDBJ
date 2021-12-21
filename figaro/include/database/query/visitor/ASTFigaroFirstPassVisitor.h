#ifndef _FIGARO_AST_FIGARO_FIRST_PASS_VISITOR_H_
#define _FIGARO_AST_FIGARO_FIRST_PASS_VISITOR_H_

#include "ASTVisitorQRGivensAbs.h"

namespace Figaro
{
    class ASTFigaroFirstPassVisitor: public ASTVisitorQRGivensAbs
    {
    public:
        ASTFigaroFirstPassVisitor(
            Database* pDatabase): ASTVisitorQRGivensAbs(pDatabase) {}
        ASTVisitorAbsResult* visitNodeRelation(ASTNodeRelation* pElement) override;
        ASTVisitorAbsResult* visitNodeJoin(ASTNodeJoin* pElement) override;
        ASTVisitorAbsResult* visitNodeQRGivens(ASTNodeQRGivens* pElement) override;

        virtual ~ASTFigaroFirstPassVisitor() override {}
    };
}

#endif