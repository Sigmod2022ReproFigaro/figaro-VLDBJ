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
        std::vector<std::string> m_vDropAttributes;
    public:
        ASTNodeQRGivens(ASTNode *pOperand, const std::vector<std::string>& vRelationOrder, const std::vector<std::string>& vDropAttributes
        ): m_pOperand(pOperand), m_vRelationOrder(vRelationOrder),
        m_vDropAttributes(vDropAttributes) {};
        virtual ~ASTNodeQRGivens() override { delete m_pOperand; }
        ASTNode* getOperand(void)
        {
            return m_pOperand;
        };

        const std::vector<std::string>& getRelationOrder(void)
        {
            return m_vRelationOrder;
        }

        const std::vector<std::string>& getDropAttributes(void)
        {
            return m_vDropAttributes;
        }

        void accept(ASTVisitor *pVisitor) override;
    };


} // namespace Figaro


#endif