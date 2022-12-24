#ifndef _AST_NODE_SVD_SVT_INV_H_
#define _AST_NODE_SVD_SVT_INV_H_

#include "utils/Utils.h"
#include "ASTNode.h"

namespace Figaro
{
    class ASTVisitor;

    class ASTNodeSVDSVTInverse: public ASTNode
    {
        friend class ASTVisitor;
        ASTNode* m_pOpSig;
        ASTNode* m_pOpV;
    public:
        ASTNodeSVDSVTInverse(ASTNode *pOpSig, ASTNode *pOpV):
            m_pOpSig(pOpSig), m_pOpV(pOpV) {};
        virtual ~ASTNodeSVDSVTInverse() override { delete m_pOpSig; delete m_pOpV; }
        ASTNode* getOpSig(void)
        {
            return m_pOpSig;
        };

        ASTNode* getOpV(void)
        {
            return m_pOpV;
        };

        virtual ASTVisitorResultAbs* accept(ASTVisitor *pVisitor) override;

        virtual ASTNode* copy()
        {
            return new ASTNodeSVDSVTInverse(m_pOpSig->copy(), m_pOpV->copy());
        }
    };


} // namespace Figaro


#endif