#ifndef _AST_NODE_QR_POSTPROCESS_H_
#define _AST_NODE_QR_POSTPROCESS_H_

#include "utils/Utils.h"
#include "ASTNode.h"
#include "utils/Types.h"

namespace Figaro
{
    class ASTVisitor;

    class ASTNodeQRAlg: public ASTNode
    {
        friend class ASTVisitor;
        ASTNode* m_pOperand;
        std::vector<std::string> m_vRelationOrder;
        std::vector<std::string> m_vDropAttributes;
        uint32_t m_numThreads;
        bool m_computeQ;
        QRHintType m_qrAlgorithm;
    public:
        ASTNodeQRAlg(ASTNode *pOperand, const std::vector<std::string>& vRelationOrder, const std::vector<std::string>& vDropAttributes,
        uint32_t numThreads, bool computeQ, QRHintType qrAlg): m_pOperand(pOperand), m_vRelationOrder(vRelationOrder),
        m_vDropAttributes(vDropAttributes),
        m_numThreads(numThreads), m_computeQ(computeQ), m_qrAlgorithm(qrAlg) {};
        virtual ~ASTNodeQRAlg() override { delete m_pOperand; }
        ASTNode* getOperand(void)
        {
            return m_pOperand;
        };

        uint32_t getNumThreads(void){ return m_numThreads; }

        bool isComputeQ(void) const { return m_computeQ; }

        QRHintType getQrAlgorithm(void) const { return m_qrAlgorithm; }

        const std::vector<std::string>& getRelationOrder(void)
        {
            return m_vRelationOrder;
        }

        const std::vector<std::string>& getDropAttributes(void)
        {
            return m_vDropAttributes;
        }

        ASTVisitorResultAbs* accept(ASTVisitor *pVisitor) override;

        virtual ASTNodeQRAlg* copy() override
        {
            return new ASTNodeQRAlg(m_pOperand->copy(),
                m_vRelationOrder, m_vDropAttributes, m_numThreads,
                m_computeQ, m_qrAlgorithm);
        }
    };
} // namespace Figaro


#endif