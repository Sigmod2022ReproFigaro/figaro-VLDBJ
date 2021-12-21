#include "database/query/visitor/ASTJoinAttributesComputeVisitor.h"
#include "omp.h"

namespace Figaro
{
    ASTVisitorAbsResult* ASTJoinAttributesComputeVisitor::visitNodeRelation(ASTNodeRelation* pElement)
    {
        pElement->checkAndUpdateJoinAttributes();
        if (m_sortValues)
        {
            m_pDatabase->sortRelation(pElement->getRelation()->getRelationName(),
                                  pElement->getJoinAttributeNames());
        }
        const auto& relationName = pElement->getRelationName();
        const auto& formJoinAttrNames = getFormateJoinAttributeNames(pElement->getJoinAttributeNames());
        FIGARO_LOG_DBG("relation", relationName, "joinAttributeNames", formJoinAttrNames);
        FIGARO_LOG_DBG("HOHO")
        return nullptr;
    }

    ASTVisitorAbsResult* ASTJoinAttributesComputeVisitor::visitNodeJoin(ASTNodeJoin* pElement)
    {
        FIGARO_LOG_DBG("Join");
        FIGARO_LOG_DBG("Central");
        for (const auto& pChild: pElement->getChildren())
        {
            FIGARO_LOG_DBG("Child");
            pChild->accept(this);
        }
        pElement->checkAndUpdateJoinAttributes();
        if (m_sortValues)
        {
            m_pDatabase->sortRelation(pElement->getCentralRelation()->getRelationName(),
                                    pElement->getJoinAttributeNames());
        }

        pElement->checkAndUpdateChildrenParJoinAttributes();

        const auto& relationName = pElement->getCentralRelation()->getRelationName();
        const auto& formJoinAttrNames = getFormateJoinAttributeNames(pElement->getJoinAttributeNames());
        FIGARO_LOG_DBG("relation", relationName, "joinAttributeNames", formJoinAttrNames);
        FIGARO_LOG_DBG("HOHO")
        return nullptr;
    }

    ASTVisitorAbsResult* ASTJoinAttributesComputeVisitor::visitNodeQRGivens(ASTNodeQRGivens* pElement)
    {
        FIGARO_LOG_INFO("QR Givens");
        FIGARO_LOG_INFO("Relation order", pElement->getRelationOrder())
        FIGARO_LOG_INFO("Skipped attributes", pElement->getDropAttributes())
        FIGARO_LOG_INFO("Number of threads", pElement->getNumThreads())
        omp_set_num_threads(pElement->getNumThreads());
        m_pDatabase->dropAttributesFromRelations(
            pElement->getDropAttributes());
        m_pDatabase->oneHotEncodeRelations();
        pElement->getOperand()->accept(this);
        return nullptr;
    }

    ASTVisitorAbsResult* ASTJoinAttributesComputeVisitor::visitNodePostProcQR(ASTNodePostProcQR* pElement)
    {
        FIGARO_LOG_INFO("QR Postprocess");
        FIGARO_LOG_INFO("Relation order", pElement->getRelationOrder())
        FIGARO_LOG_INFO("Skipped attributes", pElement->getDropAttributes())
        FIGARO_LOG_INFO("Number of threads", pElement->getNumThreads())
        omp_set_num_threads(pElement->getNumThreads());
        m_pDatabase->dropAttributesFromRelations(
            pElement->getDropAttributes());
        m_pDatabase->oneHotEncodeRelations();
        FIGARO_LOG_DBG("Here")
        pElement->getOperand()->accept(this);
        FIGARO_LOG_DBG("HOHOH")
        if (m_memoryLayout == Figaro::MemoryLayout::COL_MAJOR)
        {
            FIGARO_LOG_DBG("HOHOH")
            m_pDatabase->changeMemoryLayout();
        }
        return nullptr;
    }

}