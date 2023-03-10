#ifndef _AST_NODE_QR_FIGARO_H_
#define _AST_NODE_QR_FIGARO_H_

#include "utils/Utils.h"
#include "ASTNode.h"
#include "utils/Types.h"

namespace Figaro
{
    class ASTVisitor;

    class ASTNodeQRFigaro: public ASTNode
    {
        friend class ASTVisitor;
        ASTNode* m_pOperand;
        std::vector<std::string> m_vRelationOrder;
        std::vector<std::string> m_vDropAttributes;
        uint32_t m_numThreads;
        bool m_computeQ;
        QRHintType m_qrHelpAlgorithm;
    public:
        ASTNodeQRFigaro(ASTNode *pOperand, const std::vector<std::string>& vRelationOrder, const std::vector<std::string>& vDropAttributes,
        uint32_t numThreads, bool computeQ,
        QRHintType qrAlgorithm
        ): m_pOperand(pOperand), m_vRelationOrder(vRelationOrder),
        m_vDropAttributes(vDropAttributes),
        m_numThreads(numThreads),
        m_computeQ(computeQ), m_qrHelpAlgorithm(qrAlgorithm) {};
        virtual ~ASTNodeQRFigaro() override { delete m_pOperand; }
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

        QRHintType getHelpQrAlg(void) const
        {
            return m_qrHelpAlgorithm;
        }

        bool isComputeQ(void) const { return m_computeQ; }

        ASTVisitorResultAbs* accept(ASTVisitor *pVisitor) override;

        virtual ASTNode* copy() override
        {
            return new ASTNodeQRFigaro(m_pOperand->copy(),
                m_vDropAttributes, m_vRelationOrder, m_numThreads,
                m_computeQ, m_qrHelpAlgorithm);
        }
    };


} // namespace Figaro


#endif