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
    public:
        // TODO: Add ranges and other options. 
        ASTNodeRelation(const std::string& relationName): m_relationName(relationName) {};
        virtual ~ASTNodeRelation() override {}

        const std::string& getRelationName() const
        {
            return m_relationName;
        }
        
        void accept(ASTVisitor *pVisitor) override;

        MatrixT* computeHead(Database* pDatabase) const;

        MatrixT* computeTail(Database* pDatabase) const;

        void computeHeadSingleThreaded(const std::vector<std::string>& ) const;
    };
} // namespace Figaro


#endif