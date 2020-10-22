#ifndef _FIGARO_AST_NODE_JOIN_H_
#define _FIGARO_AST_NODE_JOIN_H_

#include "ASTNode.h"

namespace Figaro
{
    class ASTVisitor;
    class ASTNodeJoin : public ASTNode
    {
        friend class ASTVisitor;
        ASTNode* m_pLeftChild;
        ASTNode* m_pRightChild;
    public:
        ASTNodeJoin(ASTNode* pLeftChild, ASTNode* pRightChild): 
            m_pLeftChild(pLeftChild), m_pRightChild(pRightChild){}
        ASTNodeJoin(const json& jsonJoinOperands);
        virtual ~ASTNodeJoin() override;
        
        void accept(ASTVisitor *pVisitor) override;
    };
}


#endif