#ifndef _FIGARO_AST_FIGARO_BUILD_INDICES_VISITOR_H_
#define _FIGARO_AST_FIGARO_BUILD_INDICES_VISITOR_H_

#include "./figaro/qr/ASTVisitorQRFigaroAbs.h"

namespace Figaro
{
    class ASTBuildIndicesVisitor: public ASTVisitorQRFigaroAbs
    {
    public:
        ASTBuildIndicesVisitor(
            Database* pDatabase): ASTVisitorQRFigaroAbs(pDatabase) {}
        ASTVisitorResultAbs* visitNodeRelation(ASTNodeRelation* pElement) override;
        ASTVisitorResultAbs* visitNodeJoin(ASTNodeJoin* pElement) override;
        ASTVisitorResultAbs* visitNodeQRFigaro(ASTNodeQRFigaro* pElement) override;
        ASTVisitorResultAbs* visitNodeLUFigaro(ASTNodeLUFigaro* pElement) override;

        virtual ~ASTBuildIndicesVisitor() override {}
    };
}

#endif