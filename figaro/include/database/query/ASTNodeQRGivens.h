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
        std::vector<std::string> m_vRelationOrder;
    public:
        ASTNodeQRGivens(ASTNode *pOperand, const std::vector<std::string>& vRelationOrder): m_pOperand(pOperand), m_vRelationOrder(vRelationOrder) {};
        virtual ~ASTNodeQRGivens() override {}
        ASTNode* getOperand(void) 
        { 
            return m_pOperand;
        };

        const std::vector<std::string>& getRelationOrder(void)
        {
            return m_vRelationOrder;
        }
        void accept(ASTVisitor *pVisitor) override;
    };
    
    
} // namespace Figaro


#endif 