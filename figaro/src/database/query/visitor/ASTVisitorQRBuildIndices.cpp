#include "database/query/visitor/ASTVisitorBuildIndices.h"

namespace Figaro
{
    ASTVisitorResultAbs* ASTBuildIndicesVisitor::visitNodeRelation(ASTNodeRelation* pElement)
    {
        bool isRootNode;
        const auto& relationName = pElement->getRelationName();

        isRootNode = pElement->getParent() == nullptr;
        m_pDatabase->buildIndices(relationName,
            pElement->getJoinAttributeNames(), pElement->getParJoinAttributeNames(),
             isRootNode);

        return nullptr;
    }

    ASTVisitorResultAbs* ASTBuildIndicesVisitor::visitNodeJoin(ASTNodeJoin* pElement)
    {
        bool isRootNode;
        FIGARO_LOG_DBG("Join");
        FIGARO_LOG_DBG("Central");
        for (const auto& pChild: pElement->getChildren())
        {
            FIGARO_LOG_DBG("Child");
            pChild->accept(this);
        }
        const auto& relationName = pElement->getCentralRelation()->getRelationName();
        isRootNode = pElement->getParent() == nullptr;
        m_pDatabase->buildIndices(relationName,
            pElement->getJoinAttributeNames(), pElement->getParJoinAttributeNames(), isRootNode);

        return nullptr;
    }

    ASTVisitorResultAbs* ASTBuildIndicesVisitor::visitNodeQRFigaro(ASTNodeQRFigaro* pElement)
    {
         FIGARO_LOG_DBG("********************");
        FIGARO_LOG_DBG("QR Givens");
        FIGARO_LOG_DBG("Relation order", pElement->getRelationOrder())
        pElement->getOperand()->accept(this);
        return nullptr;
    }

}