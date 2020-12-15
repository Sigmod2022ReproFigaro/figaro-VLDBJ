#ifndef _FIGARO_AST_SCALED_CARTESIAN_PRODUCT_VISITOR_H_
#define _FIGARO_AST_SCALED_CARTESIAN_PRODUCT_VISITOR_H_

#include "ASTVisitor.h"

namespace Figaro
{
    class ASTScaledCartesianProductVisitor: public ASTVisitor
    {
    public:
        ASTScaledCartesianProductVisitor(Database* pDatabase): ASTVisitor(pDatabase) {}
        void visitNodeRelation(ASTNodeRelation* pElement) override;
        void visitNodeJoin(ASTNodeJoin* pElement) override;
        void visitNodeQRGivens(ASTNodeQRGivens* pElement) override;

        virtual ~ASTScaledCartesianProductVisitor() override {}
    };
}

#endif 