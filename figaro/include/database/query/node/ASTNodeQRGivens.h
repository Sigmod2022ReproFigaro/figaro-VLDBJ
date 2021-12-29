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
        uint32_t m_numThreads;
        bool m_computeQ;
    public:
        ASTNodeQRGivens(ASTNode *pOperand, const std::vector<std::string>& vRelationOrder, const std::vector<std::string>& vDropAttributes,
        uint32_t numThreads, bool computeQ
        ): m_pOperand(pOperand), m_vRelationOrder(vRelationOrder),
        m_vDropAttributes(vDropAttributes),
        m_numThreads(numThreads),
        m_computeQ(computeQ) {};
        virtual ~ASTNodeQRGivens() override { delete m_pOperand; }
        ASTNode* getOperand(void)
        {
            return m_pOperand;
        };

        uint32_t getNumThreads(void){ return m_numThreads; }

        const std::vector<std::string>& getRelationOrder(void)
        {
            return m_vRelationOrder;
        }

        const std::vector<std::string>& getDropAttributes(void)
        {
            return m_vDropAttributes;
        }


        bool isComputeQ(void) const { return m_computeQ; }

        ASTVisitorAbsResult* accept(ASTVisitor *pVisitor) override;

        virtual ASTNode* copy() override
        {
            return new ASTNodeQRGivens(m_pOperand->copy(),
                m_vDropAttributes, m_vRelationOrder, m_numThreads,
                m_computeQ);
        }
    };


} // namespace Figaro


#endif