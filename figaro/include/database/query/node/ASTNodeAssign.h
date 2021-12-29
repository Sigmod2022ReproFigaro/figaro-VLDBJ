#ifndef _AST_NODE_ASSIGN_H_
#define _AST_NODE_ASSIGN_H_

#include "utils/Utils.h"
#include "ASTNode.h"

namespace Figaro
{
    class ASTVisitor;

    class ASTNodeAssign: public ASTNode
    {
        friend class ASTVisitor;
        ASTNode* m_pOperand;
        std::string m_relationName;
    public:
        ASTNodeAssign(ASTNode *pOperand, std::string relName):
            m_pOperand(pOperand), m_relationName(relName) {};
        virtual ~ASTNodeAssign() override { delete m_pOperand; }
        ASTNode* getOperand(void)
        {
            return m_pOperand;
        };

        std::string getRelationName(void) const { return m_relationName; }

        virtual ASTVisitorAbsResult* accept(ASTVisitor *pVisitor) override;

        virtual ASTNode* copy()
        {
            return new ASTNodeAssign(m_pOperand->copy(), m_relationName);
        }
    };


} // namespace Figaro


#endif