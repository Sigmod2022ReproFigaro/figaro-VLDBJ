#ifndef _FIGARO_AST_JOIN_ATTRIBUTES_COMPUTE_VISITOR_H_
#define _FIGARO_AST_JOIN_ATTRIBUTES_COMPUTE_VISITOR_H_

#include "ASTVisitor.h"

namespace Figaro
{
    class ASTJoinAttributesComputeVisitor: public ASTVisitor
    {
        std::vector<ASTNodeRelation*> m_vpASTNodeRelation;  
    public:
        ASTJoinAttributesComputeVisitor(Database* pDatabase): ASTVisitor(pDatabase) {}
        void visitNodeRelation(ASTNodeRelation* pElement) override;
        void visitNodeJoin(ASTNodeJoin* pElement) override;
        void visitNodeQRGivens(ASTNodeQRGivens* pElement) override;

        virtual ~ASTJoinAttributesComputeVisitor() override {}


    };
}

#endif 