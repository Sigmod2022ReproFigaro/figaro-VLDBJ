#include "database/query/ASTScaledCartesianProductVisitor.h"

namespace Figaro
{
    void ASTScaledCartesianProductVisitor::visitNodeRelation(ASTNodeRelation* pElement)
    {
        FIGARO_LOG_DBG("Relation", pElement->getRelationName());
    }

    void ASTScaledCartesianProductVisitor::visitNodeJoin(ASTNodeJoin* pElement)
    {
        FIGARO_LOG_DBG("Join");
        FIGARO_LOG_DBG("Left");
        pElement->getLeftChild()->accept(this);
        FIGARO_LOG_DBG("Right");
        pElement->getRightChild()->accept(this);
    }

    void ASTScaledCartesianProductVisitor::visitNodeQRGivens(ASTNodeQRGivens* pElement)
    {
        FIGARO_LOG_DBG("QR Givens");
        pElement->getOperand()->accept(this);
    }

}