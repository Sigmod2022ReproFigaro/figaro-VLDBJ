#ifndef _AST_NODE_SVD_DEC_ALG_H_
#define _AST_NODE_SVD_DEC_ALG_H_

#include "utils/Utils.h"
#include "ASTNode.h"
#include "utils/Types.h"

namespace Figaro
{
    class ASTVisitor;

    class ASTNodeSVDAlgDec: public ASTNode
    {
        friend class ASTVisitor;
        ASTNode* m_pOperand;
        std::vector<std::string> m_vRelationOrder;
        std::vector<std::string> m_vDropAttributes;
        uint32_t m_numThreads;
        bool m_computeUAndV;
        SVDHintType m_SVDAlg;
    public:
        ASTNodeSVDAlgDec(ASTNode *pOperand, const std::vector<std::string>& vRelationOrder, const std::vector<std::string>& vDropAttributes,
        uint32_t numThreads, bool computeUAndV, Figaro::SVDHintType algSvd):
            m_pOperand(pOperand), m_vRelationOrder(vRelationOrder),
            m_vDropAttributes(vDropAttributes),
            m_numThreads(numThreads), m_SVDAlg(algSvd),
            m_computeUAndV(computeUAndV) {};
        virtual ~ASTNodeSVDAlgDec() override { delete m_pOperand; }
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

        Figaro::SVDHintType getSVDAlgorithm(void) const
        {
            return m_SVDAlg;
        }

        bool isComputeUAndV(void) const
        {
            return m_computeUAndV;
        }

        ASTVisitorResultAbs* accept(ASTVisitor *pVisitor) override;

        virtual ASTNodeSVDAlgDec* copy() override
        {
            return new ASTNodeSVDAlgDec(m_pOperand->copy(),
                m_vRelationOrder, m_vDropAttributes, m_numThreads, m_computeUAndV, m_SVDAlg);
        }

    };


} // namespace Figaro


#endif