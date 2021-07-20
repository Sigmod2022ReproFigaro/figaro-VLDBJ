#include "database/query/ASTJoinAttributesComputeVisitor.h"
#include "omp.h"

namespace Figaro
{
    void ASTJoinAttributesComputeVisitor::visitNodeRelation(ASTNodeRelation* pElement)
    {
        pElement->checkAndUpdateJoinAttributes();
         m_pDatabase->sortRelation(pElement->getRelation()->getRelationName(),
                                  pElement->getJoinAttributeNames());
        const auto& relationName = pElement->getRelationName();
        const auto& formJoinAttrNames = getFormateJoinAttributeNames(pElement->getJoinAttributeNames());
        FIGARO_LOG_DBG("relation", relationName, "joinAttributeNames", formJoinAttrNames);
    }

    void ASTJoinAttributesComputeVisitor::visitNodeJoin(ASTNodeJoin* pElement)
    {
        FIGARO_LOG_DBG("Join");
        FIGARO_LOG_DBG("Central");
        for (const auto& pChild: pElement->getChildren())
        {
            FIGARO_LOG_DBG("Child");
            pChild->accept(this);
        }
        pElement->checkAndUpdateJoinAttributes();
        m_pDatabase->sortRelation(pElement->getCentralRelation()->getRelationName(),
                                  pElement->getJoinAttributeNames());

        pElement->checkAndUpdateChildrenParJoinAttributes();

        const auto& relationName = pElement->getCentralRelation()->getRelationName();
        const auto& formJoinAttrNames = getFormateJoinAttributeNames(pElement->getJoinAttributeNames());
        FIGARO_LOG_DBG("relation", relationName, "joinAttributeNames", formJoinAttrNames);
    }

    void ASTJoinAttributesComputeVisitor::visitNodeQRGivens(ASTNodeQRGivens* pElement)
    {
        FIGARO_LOG_DBG("QR Givens");
        FIGARO_LOG_DBG("Relation order", pElement->getRelationOrder())
        FIGARO_LOG_DBG("Skipped attributes", pElement->getDropAttributes())
        FIGARO_LOG_DBG("Number of threads", pElement->getNumThreads())
        omp_set_num_threads(pElement->getNumThreads());
        m_pDatabase->dropAttributesFromRelations(
            pElement->getDropAttributes());
        m_pDatabase->oneHotEncodeRelations();
        pElement->getOperand()->accept(this);
    }

}