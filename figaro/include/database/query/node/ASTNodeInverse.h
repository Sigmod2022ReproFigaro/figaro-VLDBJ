#ifndef _AST_NODE_INVERSE_H_
#define _AST_NODE_INVERSE_H_

#include "utils/Utils.h"
#include "ASTNode.h"

namespace Figaro
{
    class ASTVisitor;

    class ASTNodeInverse: public ASTNode
    {
        friend class ASTVisitor;
        ASTNode* m_pOperand;
    public:
        ASTNodeInverse(ASTNode *pOperand):
            m_pOperand(pOperand) {};
        virtual ~ASTNodeInverse() override { delete m_pOperand; }
        ASTNode* getOperand(void)
        {
            return m_pOperand;
        };

        virtual ASTVisitorAbsResult* accept(ASTVisitor *pVisitor) override;

        virtual ASTNode* copy()
        {
            return new ASTNodeInverse(m_pOperand->copy());
        }
    };


} // namespace Figaro


#endif