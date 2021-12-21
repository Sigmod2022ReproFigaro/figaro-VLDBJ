#ifndef _FIGARO_AST_JOIN_ATTRIBUTES_COMPUTE_VISITOR_H_
#define _FIGARO_AST_JOIN_ATTRIBUTES_COMPUTE_VISITOR_H_

#include "ASTVisitorQRGivensAbs.h"

namespace Figaro
{
    class ASTJoinAttributesComputeVisitor: public ASTVisitorQRGivensAbs
    {
        void initializeEnumAndDenomRelations(ASTNodeRelation* pRel);
        bool m_sortValues;
        Figaro::MemoryLayout m_memoryLayout;
    public:
        ASTJoinAttributesComputeVisitor(
            Database* pDatabase, bool sortValues,
            Figaro::MemoryLayout memoryLayout): ASTVisitorQRGivensAbs(pDatabase), m_sortValues(sortValues), m_memoryLayout(memoryLayout) {}
        ASTVisitorAbsResult* visitNodeRelation(ASTNodeRelation* pElement) override;
        ASTVisitorAbsResult* visitNodeJoin(ASTNodeJoin* pElement) override;
        ASTVisitorAbsResult* visitNodeQRGivens(ASTNodeQRGivens* pElement) override;
        ASTVisitorAbsResult* visitNodePostProcQR(ASTNodePostProcQR* pElement) override;

        virtual ~ASTJoinAttributesComputeVisitor() override {}


    };
}

#endif