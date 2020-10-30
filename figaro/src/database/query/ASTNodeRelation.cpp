#include "database/query/ASTNodeRelation.h"
#include "database/query/ASTVisitor.h"
#include "database/Database.h"

namespace Figaro 
{

    void ASTNodeRelation::accept(ASTVisitor *pVisitor) 
    {
        pVisitor->visitNodeRelation(this);
    }

    MatrixT* ASTNodeRelation::computeHead(Database* pDatabase) const
    {
        return pDatabase->computeHead(m_relationName);
    }

    MatrixT* ASTNodeRelation::computeTail(Database* pDatabase) const
    {
        return pDatabase->computeTail(m_relationName);
    }

    void ASTNodeRelation::computeHeadSingleThreaded(
        const std::vector<std::string>& joinAttributes) const
    {
        for (const auto& joinAttribute: joinAttributes)
        {
            
        }
    }
}