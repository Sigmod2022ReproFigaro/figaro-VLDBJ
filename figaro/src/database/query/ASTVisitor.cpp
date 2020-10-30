#include "database/query/ASTVisitor.h"

namespace Figaro
{
    void ASTVisitor::visitNodeRelation(ASTNodeRelation *pNodeRelation)
    {
        MatrixT* pHead = pNodeRelation->computeHead(m_pDatabase);
        MatrixT* pTail = pNodeRelation->computeTail(m_pDatabase);
    }

    void ASTVisitor::visitNodeJoin(ASTNodeJoin *pElement)
    {
        
    }
    
    void ASTVisitor::visitNodeQRGivens(ASTNodeQRGivens *pElement)
    {

    }
}