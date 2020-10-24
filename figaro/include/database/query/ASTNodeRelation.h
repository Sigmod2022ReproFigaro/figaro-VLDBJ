#ifndef _FIGARO_AST_NODE_RELATION_H_
#define _FIGARO_AST_NODE_RELATION_H_

#include "ASTNode.h"

namespace Figaro
{
    class ASTVisitor;

    class ASTNodeRelation: public ASTNode
    {
        friend class ASTVisitor;
        std::string m_relationName;   
    public:
        // TODO: Add ranges and other options. 
        ASTNodeRelation(const std::string& relationName): m_relationName(relationName) {};
        ASTNodeRelation(const json& jsonRelationOperands);
        virtual ~ASTNodeRelation() override;
        
        void accept(ASTVisitor *pVisitor) override;
    };
} // namespace Figaro


#endif