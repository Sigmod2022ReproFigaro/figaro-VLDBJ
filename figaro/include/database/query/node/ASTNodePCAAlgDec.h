#ifndef _AST_NODE_PCA_DEC_ALG_H_
#define _AST_NODE_PCA_DEC_ALG_H_

#include "utils/Utils.h"
#include "ASTNode.h"
#include "utils/Types.h"

namespace Figaro
{
    class ASTVisitor;

    class ASTNodePCAAlgDec: public ASTNode
    {
        friend class ASTVisitor;
        ASTNode* m_pOperand;
        std::vector<std::string> m_vRelationOrder;
        std::vector<std::string> m_vDropAttributes;
        uint32_t m_numThreads;
        bool m_computeUAndV;
        PCAHintType m_PCAAlg;
    public:
        ASTNodePCAAlgDec(ASTNode *pOperand, const std::vector<std::string>& vRelationOrder, const std::vector<std::string>& vDropAttributes,
        uint32_t numThreads, bool computeUAndV, Figaro::PCAHintType algPCA):
            m_pOperand(pOperand), m_vRelationOrder(vRelationOrder),
            m_vDropAttributes(vDropAttributes),
            m_numThreads(numThreads), m_PCAAlg(algPCA),
            m_computeUAndV(computeUAndV) {};
        virtual ~ASTNodePCAAlgDec() override { delete m_pOperand; }
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

        Figaro::PCAHintType getPCAAlgorithm(void) const
        {
            return m_PCAAlg;
        }

        bool isComputeUAndV(void) const
        {
            return m_computeUAndV;
        }

        ASTVisitorResultAbs* accept(ASTVisitor *pVisitor) override;

        virtual ASTNodePCAAlgDec* copy() override
        {
            return new ASTNodePCAAlgDec(m_pOperand->copy(),
                m_vRelationOrder, m_vDropAttributes, m_numThreads, m_computeUAndV, m_PCAAlg);
        }

    };


} // namespace Figaro


#endif