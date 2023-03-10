#ifndef _AST_NODE_SVD_FIGARO_H_
#define _AST_NODE_SVD_FIGARO_H_

#include "utils/Utils.h"
#include "ASTNode.h"
#include "utils/Types.h"

namespace Figaro
{
    class ASTVisitor;

    class ASTNodeSVDFigaro: public ASTNode
    {
        friend class ASTVisitor;
        ASTNode* m_pOperand;
        std::vector<std::string> m_vRelationOrder;
        std::vector<std::string> m_vDropAttributes;
        uint32_t m_numThreads;
        bool m_computeU;
        SVDHintType m_svdHelpAlgorithm;
        QRHintType m_qrHelpAlgorithm;
    public:
        ASTNodeSVDFigaro(ASTNode *pOperand, const std::vector<std::string>& vRelationOrder, const std::vector<std::string>& vDropAttributes,
        uint32_t numThreads, bool computeU,
        SVDHintType svdAlgorithm,
        QRHintType qrAlgorithm
        ): m_pOperand(pOperand), m_vRelationOrder(vRelationOrder),
        m_vDropAttributes(vDropAttributes),
        m_numThreads(numThreads),
        m_computeU(computeU), m_svdHelpAlgorithm(svdAlgorithm),
        m_qrHelpAlgorithm(qrAlgorithm) {};
        virtual ~ASTNodeSVDFigaro() override { delete m_pOperand; }
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

        SVDHintType getHelpSVDAlg(void) const
        {
            return m_svdHelpAlgorithm;
        }

        QRHintType getHelpQRAlg(void) const
        {
            return m_qrHelpAlgorithm;
        }

        bool isComputeU(void) const { return m_computeU; }

        ASTVisitorResultAbs* accept(ASTVisitor *pVisitor) override;

        virtual ASTNode* copy() override
        {
            return new ASTNodeSVDFigaro(m_pOperand->copy(),
                m_vDropAttributes, m_vRelationOrder, m_numThreads,
                m_computeU, m_svdHelpAlgorithm, m_qrHelpAlgorithm);
        }
    };


} // namespace Figaro


#endif