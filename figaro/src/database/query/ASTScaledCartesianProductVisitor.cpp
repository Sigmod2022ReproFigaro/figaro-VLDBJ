#include "database/query/ASTScaledCartesianProductVisitor.h"

namespace Figaro
{

    void ASTScaledCartesianProductVisitor::visitNodeRelation(ASTNodeRelation* pElement)
    {
        pElement->checkAndUpdateJoinAttributes();
        FIGARO_LOG_DBG("Relation", pElement->getRelationName());
        FIGARO_LOG_DBG("Join attribute names", pElement->getJoinAttributeNames());
    }

    void ASTScaledCartesianProductVisitor::visitNodeJoin(ASTNodeJoin* pElement)
    {
        std::map<std::string, bool> mIsJoinAttr;
        FIGARO_LOG_DBG("Join");
        FIGARO_LOG_DBG("Central");
        for (const auto& pChild: pElement->getChildren())
        {
            FIGARO_LOG_DBG("Child");
            pChild->accept(this);
        }
        pElement->checkAndUpdateJoinAttributes();
        FIGARO_LOG_DBG("Relation", pElement->getCentralRelation()->getRelationName());
        FIGARO_LOG_DBG("Join attribute names", pElement->getJoinAttributeNames());
    }

    void ASTScaledCartesianProductVisitor::visitNodeQRGivens(ASTNodeQRGivens* pElement)
    {
        FIGARO_LOG_DBG("QR Givens");
        pElement->getOperand()->accept(this);
    }

}