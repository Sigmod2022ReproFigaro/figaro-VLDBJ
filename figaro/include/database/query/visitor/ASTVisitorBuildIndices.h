#ifndef _FIGARO_AST_FIGARO_BUILD_INDICES_VISITOR_H_
#define _FIGARO_AST_FIGARO_BUILD_INDICES_VISITOR_H_

#include "ASTVisitorQRFigaroAbs.h"

namespace Figaro
{
    class ASTBuildIndicesVisitor: public ASTVisitorQFigaroAbs
    {
    public:
        ASTBuildIndicesVisitor(
            Database* pDatabase): ASTVisitorQFigaroAbs(pDatabase) {}
        ASTVisitorResultAbs* visitNodeRelation(ASTNodeRelation* pElement) override;
        ASTVisitorResultAbs* visitNodeJoin(ASTNodeJoin* pElement) override;
        ASTVisitorResultAbs* visitNodeQRFigaro(ASTNodeQRFigaro* pElement) override;

        virtual ~ASTBuildIndicesVisitor() override {}
    };
}

#endif