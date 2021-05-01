#include "database/query/ASTFigaroSecondPassVisitor.h"
#include "utils/Performance.h"

namespace Figaro
{
    void ASTFigaroSecondPassVisitor::visitNodeRelation(ASTNodeRelation* pElement)
    {
        FIGARO_LOG_DBG("Finished visiting relation", pElement->getRelationName())
    }

    void ASTFigaroSecondPassVisitor::visitNodeJoin(ASTNodeJoin* pElement)
    {
        FIGARO_LOG_DBG("Join");
        FIGARO_LOG_DBG("Central");
        for (const auto& pChild: pElement->getChildren())
        {
            FIGARO_LOG_DBG("Child");
            pChild->accept(this);
        }
        const auto& relationName = pElement->getCentralRelation()->getRelationName();
        FIGARO_LOG_DBG("vpChildRels", pElement->getChildrenNames())
        m_pDatabase-> aggregateAwayChildrenRelations(
            relationName,
            pElement->getChildrenNames(),
            pElement->getJoinAttributeNames(),
            pElement->getChildrenParentJoinAttributeNames());

        bool isRootNode = pElement->getParent() == nullptr;
        std::vector<std::string> parJoinAttributeNames;
        if (isRootNode)
        {
            parJoinAttributeNames = pElement->getJoinAttributeNames();
        }
        else
        {
            parJoinAttributeNames = pElement->getParJoinAttributeNames();
        }

        m_pDatabase->computeAndScaleGeneralizedHeadAndTail(
            relationName,
            pElement->getJoinAttributeNames(),
            parJoinAttributeNames,
            isRootNode);

    }

    void ASTFigaroSecondPassVisitor::visitNodeQRGivens(ASTNodeQRGivens* pElement)
    {
         FIGARO_LOG_DBG("********************");
        FIGARO_LOG_DBG("QR Givens");
        FIGARO_LOG_DBG("Relation order", pElement->getRelationOrder())
        pElement->getOperand()->accept(this);
        if (m_postProcess)
        {
            MICRO_BENCH_INIT(postprocess)
            MICRO_BENCH_START(postprocess)
            m_pDatabase->computeQROfConcatenatedGeneralizedHeadAndTails(pElement->getRelationOrder(), m_pResult);
            MICRO_BENCH_STOP(postprocess)
            FIGARO_LOG_BENCH("Figaro", "Post processing",  MICRO_BENCH_GET_TIMER_LAP(postprocess));
        }
        FIGARO_LOG_DBG("FInished")

    }

}