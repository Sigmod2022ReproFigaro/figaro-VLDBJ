#ifndef _FIGARO_AST_NODE_JOIN_H_
#define _FIGARO_AST_NODE_JOIN_H_

#include "ASTNodeAbsRelation.h"

namespace Figaro
{
    class ASTVisitor;
    class ASTNodeJoin : public ASTNodeAbsRelation
    {
        friend class ASTVisitor;
        ASTNode* m_pLeftChild;
        ASTNode* m_pRightChild;
    public:
        ASTNode* getLeftChild(void) const
        {
            return m_pLeftChild;
        }

        ASTNode* getRightChild(void) const
        {
            return m_pRightChild;
        }

        ASTNodeJoin(ASTNode* pLeftChild, ASTNode* pRightChild): 
            m_pLeftChild(pLeftChild), m_pRightChild(pRightChild){}
        virtual ~ASTNodeJoin() override {}
        
        void accept(ASTVisitor* pVisitor) override;
    };
}


#endif