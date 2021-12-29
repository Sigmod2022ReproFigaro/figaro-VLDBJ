#ifndef _FIGARO_AST_NODE_RELATION_H_
#define _FIGARO_AST_NODE_RELATION_H_

#include "ASTNodeAbsRelation.h"

namespace Figaro
{
    class ASTVisitor;
    class Database;

    class ASTNodeRelation: public ASTNodeAbsRelation
    {
        friend class ASTVisitor;
        std::string m_relationName;
        std::vector<std::string> m_vAttributeNames;
        std::map<std::string, bool> m_mIsJoinAttr;
        std::vector<std::string> m_vJoinAttributeNames;
        std::vector<std::string> m_vParJoinAttributeNames;
    public:
        // TODO: Add ranges and other options.
        ASTNodeRelation(const std::string& relationName,
                        const std::vector<std::string>& vAttributeNames): m_relationName(relationName), m_vAttributeNames(vAttributeNames)
        {
            for (const auto& attrName: m_vAttributeNames)
            {
                m_mIsJoinAttr[attrName] = false;
            }
        };
        virtual ~ASTNodeRelation() override {}

        ASTNodeRelation* getRelation(void) override
        {
            return this;
        }

        const std::string& getRelationName() const
        {
            return m_relationName;
        }

        void setJoinAttribute(const std::string& attrName);

        /**
         * Intersects the sets of attributes of the current node and
         * @p pNodeAbsRelation and
         * based on that updates @p m_vJoinAttributeNames
         */
        void checkAndUpdateJoinAttributes(ASTNodeAbsRelation* pNodeAbsRelation);

        /**
         * Intersects the attributes of the current node and its parrent and
         * based on that updates @p m_vJoinAttributeNames
         */
        void checkAndUpdateJoinAttributes(void) override;

        void updateParJoinAttrs(ASTNodeAbsRelation* pParent);

        void updateParJoinAttrs(void) override;

        const std::vector<std::string>& getAttributeNames(void) const override
        {
            return m_vAttributeNames;
        }


        const std::vector<std::string>& getParJoinAttributeNames(void) override
        {
            return m_vParJoinAttributeNames;
        }

        const std::vector<std::string>& getJoinAttributeNames(void) override;

        ASTVisitorAbsResult* accept(ASTVisitor *pVisitor) override;

        virtual ASTNodeRelation* copy() override
        {
            return new ASTNodeRelation(m_relationName, m_vAttributeNames);
        }
    };
} // namespace Figaro


#endif