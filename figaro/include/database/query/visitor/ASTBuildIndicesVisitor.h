#ifndef _FIGARO_AST_FIGARO_BUILD_INDICES_VISITOR_H_
#define _FIGARO_AST_FIGARO_BUILD_INDICES_VISITOR_H_

#include "ASTVisitorQRGivensAbs.h"

namespace Figaro
{
    class ASTBuildIndicesVisitor: public ASTVisitorQRGivensAbs
    {
    public:
        ASTBuildIndicesVisitor(
            Database* pDatabase): ASTVisitorQRGivensAbs(pDatabase) {}
        ASTVisitorAbsResult* visitNodeRelation(ASTNodeRelation* pElement) override;
        ASTVisitorAbsResult* visitNodeJoin(ASTNodeJoin* pElement) override;
        ASTVisitorAbsResult* visitNodeQRGivens(ASTNodeQRGivens* pElement) override;

        virtual ~ASTBuildIndicesVisitor() override {}
    };
}

#endif