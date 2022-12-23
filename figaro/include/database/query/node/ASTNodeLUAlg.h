#ifndef _AST_NODE_LU_LAPACK_H_
#define _AST_NODE_LU_LAPACK_H_

#include "utils/Utils.h"
#include "ASTNode.h"
#include "utils/Types.h"

namespace Figaro
{
    class ASTVisitor;

    class ASTNodeLUAlg: public ASTNode
    {
        friend class ASTVisitor;
        ASTNode* m_pOperand;
        std::vector<std::string> m_vRelationOrder;
        std::vector<std::string> m_vDropAttributes;
        uint32_t m_numThreads;
        LUHintType m_LUAlg;
    public:
        ASTNodeLUAlg(ASTNode *pOperand, const std::vector<std::string>& vRelationOrder, const std::vector<std::string>& vDropAttributes,
        uint32_t numThreads, LUHintType luAlg): m_pOperand(pOperand), m_vRelationOrder(vRelationOrder),
        m_vDropAttributes(vDropAttributes),
        m_numThreads(numThreads), m_LUAlg(luAlg) {};
        virtual ~ASTNodeLUAlg() override { delete m_pOperand; }
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

        LUHintType getLUAlg(void) const
        {
            return m_LUAlg;
        }

        ASTVisitorResultAbs* accept(ASTVisitor *pVisitor) override;

        virtual ASTNodeLUAlg* copy() override
        {
            return new ASTNodeLUAlg(m_pOperand->copy(),
                m_vRelationOrder, m_vDropAttributes, m_numThreads, m_LUAlg);
        }
    };


} // namespace Figaro


#endif