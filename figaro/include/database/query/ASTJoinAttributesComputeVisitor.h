#ifndef _FIGARO_AST_JOIN_ATTRIBUTES_COMPUTE_VISITOR_H_
#define _FIGARO_AST_JOIN_ATTRIBUTES_COMPUTE_VISITOR_H_

#include "ASTQRVisitor.h"

namespace Figaro
{
    class ASTJoinAttributesComputeVisitor: public ASTQRVisitor
    {
        void initializeEnumAndDenomRelations(ASTNodeRelation* pRel);
    public:
        ASTJoinAttributesComputeVisitor(
            Database* pDatabase, 
            const std::map<std::string, ASTNodeRelation*>& mRelNameASTNodeRel): 
                ASTQRVisitor(pDatabase, mRelNameASTNodeRel) {}
        void visitNodeRelation(ASTNodeRelation* pElement) override;
        void visitNodeJoin(ASTNodeJoin* pElement) override;
        void visitNodeQRGivens(ASTNodeQRGivens* pElement) override;

        virtual ~ASTJoinAttributesComputeVisitor() override {}


    };
}

#endif 