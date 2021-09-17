#ifndef _FIGARO_AST_JOIN_ATTRIBUTES_COMPUTE_VISITOR_H_
#define _FIGARO_AST_JOIN_ATTRIBUTES_COMPUTE_VISITOR_H_

#include "ASTQRVisitor.h"

namespace Figaro
{
    class ASTJoinAttributesComputeVisitor: public ASTQRVisitor
    {
        void initializeEnumAndDenomRelations(ASTNodeRelation* pRel);
        bool m_sortValues;
    public:
        ASTJoinAttributesComputeVisitor(
            Database* pDatabase, bool sortValues): ASTQRVisitor(pDatabase), m_sortValues(sortValues) {}
        void visitNodeRelation(ASTNodeRelation* pElement) override;
        void visitNodeJoin(ASTNodeJoin* pElement) override;
        void visitNodeQRGivens(ASTNodeQRGivens* pElement) override;
        void visitNodePostProcQR(ASTNodePostProcQR* pElement) override;

        virtual ~ASTJoinAttributesComputeVisitor() override {}


    };
}

#endif