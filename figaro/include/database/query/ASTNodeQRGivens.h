#ifndef _AST_NODE_QR_GIVENS_H_
#define _AST_NODE_QR_GIVENS_H_

#include "utils/Utils.h"
#include "ASTNode.h"

namespace Figaro
{
    class ASTVisitor;

    class ASTNodeQRGivens: public ASTNode
    {
        friend class ASTVisitor;
        ASTNode* m_pOperand;
    public:
        ASTNodeQRGivens(ASTNode *pOperand): m_pOperand(pOperand) {};
        virtual ~ASTNodeQRGivens() override {}
        ASTNode* getOperand(void) 
        { 
            return m_pOperand;
        };
        void accept(ASTVisitor *pVisitor) override;
    };
    
    
} // namespace Figaro


#endif 