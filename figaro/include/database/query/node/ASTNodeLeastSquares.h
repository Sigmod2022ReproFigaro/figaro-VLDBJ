#ifndef _AST_NODE_LEAST_SQUARES_H_
#define _AST_NODE_LEAST_SQUARES_H_

#include "utils/Utils.h"
#include "ASTNode.h"

namespace Figaro
{
    class ASTVisitor;

    class ASTNodeLeastSquares: public ASTNode
    {
        friend class ASTVisitor;
        ASTNode* m_pOperand;
        std::vector<std::string> m_vRelationOrder;
        std::vector<std::string> m_vDropAttributes;
        uint32_t m_numThreads;
        std::string m_labelName;
        bool m_isFigaro;
    public:
        ASTNodeLeastSquares(ASTNode *pOperand, const std::vector<std::string>& vRelationOrder, const std::vector<std::string>& vDropAttributes,
        uint32_t numThreads,
        const std::string& labelName, bool isFigaro): m_pOperand(pOperand), m_vRelationOrder(vRelationOrder),
        m_vDropAttributes(vDropAttributes),
        m_numThreads(numThreads), m_labelName(labelName),
        m_isFigaro(isFigaro) {};
        virtual ~ASTNodeLeastSquares() override { delete m_pOperand; }
        ASTNode* getOperand(void)
        {
            return m_pOperand;
        };

        uint32_t getNumThreads(void){ return m_numThreads; }
        std::string getLabelName(void){ return m_labelName; }


        const std::vector<std::string>& getRelationOrder(void)
        {
            return m_vRelationOrder;
        }

        const std::vector<std::string>& getDropAttributes(void)
        {
            return m_vDropAttributes;
        }

        bool isFigaro(void) const
        {
            return m_isFigaro;
        }

        ASTVisitorResultAbs* accept(ASTVisitor *pVisitor) override;

        virtual ASTNodeLeastSquares* copy() override
        {
            return new ASTNodeLeastSquares(m_pOperand->copy(),
                m_vRelationOrder, m_vDropAttributes, m_numThreads, m_labelName,
                m_isFigaro);
        }
    };


} // namespace Figaro


#endif