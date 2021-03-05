#ifndef _FIGARO_AST_NODE_JOIN_H_
#define _FIGARO_AST_NODE_JOIN_H_

#include "ASTNodeAbsRelation.h"
#include "ASTNodeRelation.h"

namespace Figaro
{
    class ASTVisitor;
    class ASTNodeJoin : public ASTNodeAbsRelation
    {
        friend class ASTVisitor;
        ASTNodeRelation* m_pCenRelation;
        std::vector<ASTNodeAbsRelation*> m_vPChild;
    public:
        ASTNodeJoin(ASTNodeRelation* pRelation, std::vector<ASTNodeAbsRelation*> vPChild): 
            m_pCenRelation(pRelation), m_vPChild(vPChild){}
        virtual ~ASTNodeJoin() override {}

        ASTNodeRelation* getCentralRelation(void)
        {
            return m_pCenRelation;
        }

        const std::vector<std::string>& getAttributeNames(void) const override
        {
            return m_pCenRelation->getAttributeNames();
        }

        const std::vector<std::string>& getJoinAttributeNames(void) override;

        std::vector<ASTNodeAbsRelation*> getChildren(void)
        {
            return m_vPChild;
        }
        
        void checkAndUpdateJoinAttributes(void) override;

        void accept(ASTVisitor* pVisitor) override;
    };
}


#endif