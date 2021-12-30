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
        std::vector<std::vector<std::string> > m_vvChildParJoinAttributeNames;
        std::vector<std::string> m_vChildrenNames;

    public:
        ASTNodeJoin(ASTNodeRelation* pRelation, std::vector<ASTNodeAbsRelation*> vPChild):
            m_pCenRelation(pRelation), m_vPChild(vPChild){}
        virtual ~ASTNodeJoin() override;

        ASTNodeRelation* getRelation(void) override
        {
            return m_pCenRelation;
        }
        ASTNodeRelation* getCentralRelation(void)
        {
            return m_pCenRelation;
        }

        const std::vector<std::string>& getChildrenNames()
        {
            if (m_vChildrenNames.size() == 0)
            {
                for (const auto& child: getChildren())
                {
                    m_vChildrenNames.push_back(child->getRelation()->getRelationName());
                }
            }
            return m_vChildrenNames;
        }

        const std::vector<std::string>& getAttributeNames(void) const override
        {
            return m_pCenRelation->getAttributeNames();
        }

        const std::vector<std::string>& getJoinAttributeNames(void) override;

        const std::vector<std::string>&
        getParJoinAttributeNames(void) override
        {
            return getCentralRelation()->getParJoinAttributeNames();
        }

        const std::vector<std::vector<std::string> >& getChildrenParentJoinAttributeNames()
        {
            return m_vvChildParJoinAttributeNames;
        }

        std::vector<ASTNodeAbsRelation*>& getChildren(void)
        {
            return m_vPChild;
        }

        /**
         * Intersects the attributes of the current node, its parrent and
         * its children and based on that updates @p m_vJoinAttributeNames
         */
        void checkAndUpdateJoinAttributes(void) override;

        void updateParJoinAttrs(void) override;

        void checkAndUpdateChildrenParJoinAttributes(void);


        ASTVisitorAbsResult* accept(ASTVisitor* pVisitor) override;


        virtual ASTNodeJoin* copy() override
        {
            std::vector<ASTNodeAbsRelation*> vChild;

            for (auto& child: m_vPChild)
            {
                vChild.push_back(child->copy());
            }
            ASTNodeJoin* pNewNode =  new ASTNodeJoin(m_pCenRelation->copy(), vChild);
            for (auto& child: pNewNode->getChildren())
            {
                child->setParent(pNewNode);
            }
            return pNewNode;
        }
    };
}


#endif