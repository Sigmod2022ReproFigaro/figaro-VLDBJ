#ifndef _AST_NODE_RIGHT_MULTIPLY_H_
#define _AST_NODE_RIGHT_MULTIPLY_H_

#include "utils/Utils.h"
#include "ASTNode.h"

namespace Figaro
{
    class ASTVisitor;

    class ASTNodeRightMultiply: public ASTNode
    {
        friend class ASTVisitor;
        ASTNode* m_pLeftOperand;
        ASTNode* m_pRightOperand;
    public:
        ASTNodeRightMultiply(ASTNode *pLeftOperand, ASTNode *pRightOperand):  m_pLeftOperand(pLeftOperand), m_pRightOperand(pRightOperand) {};
        virtual ~ASTNodeRightMultiply() override {
            delete m_pLeftOperand;
            delete m_pRightOperand; }
        ASTNode* getLeftOperand(void)
        {
            return m_pLeftOperand;
        };

         ASTNode* getRightOperand(void)
        {
            return m_pRightOperand;
        };

        ASTVisitorAbsResult* accept(ASTVisitor *pVisitor) override;

        virtual ASTNodeRightMultiply* copy() override
        {
            return new ASTNodeRightMultiply(
                m_pLeftOperand->copy(), m_pRightOperand-> copy());
        }
    };


} // namespace Figaro


#endif