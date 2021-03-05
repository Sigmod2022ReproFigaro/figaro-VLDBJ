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
    public:
        // TODO: Add ranges and other options. 
        ASTNodeRelation(const std::string& relationName, 
                        const std::vector<std::string>& vAttributeNames): m_relationName(relationName), m_vAttributeNames(vAttributeNames)
        {
            FIGARO_LOG_DBG("AttributeNames", vAttributeNames)
            for (const auto& attrName: m_vAttributeNames)
            {
                m_mIsJoinAttr[attrName] = false;
            }
        };
        virtual ~ASTNodeRelation() override {}

        const std::string& getRelationName() const
        {
            return m_relationName;
        }

        void setJoinAttribute(const std::string& attrName);

        void checkAndUpdateJoinAttributes(ASTNodeAbsRelation* pNodeAbsRelation);

        void checkAndUpdateJoinAttributes(void) override;        
        const std::vector<std::string>& getAttributeNames(void) const override
        {
            return m_vAttributeNames;
        }

        const std::vector<std::string>& getJoinAttributeNames(void) override;

        void accept(ASTVisitor *pVisitor) override;

        MatrixEigenT* computeHead(Database* pDatabase) const;

        MatrixEigenT* computeTail(Database* pDatabase) const;

        void computeHeadSingleThreaded(const std::vector<std::string>& ) const;
    };
} // namespace Figaro


#endif