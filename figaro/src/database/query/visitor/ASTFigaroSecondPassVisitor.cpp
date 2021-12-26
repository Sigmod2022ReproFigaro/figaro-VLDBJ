#include "database/query/visitor/ASTFigaroSecondPassVisitor.h"
#include "utils/Performance.h"

namespace Figaro
{
    ASTVisitorAbsResult* ASTFigaroSecondPassVisitor::visitNodeRelation(ASTNodeRelation* pElement)
    {
        FIGARO_LOG_DBG("Finished visiting relation", pElement->getRelationName())
        return nullptr;
    }

    ASTVisitorAbsResult* ASTFigaroSecondPassVisitor::visitNodeJoin(ASTNodeJoin* pElement)
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
        // TODO: Pass temporary relations to functions
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
        return nullptr;

    }

    ASTVisitorQRResult* ASTFigaroSecondPassVisitor::visitNodeQRGivens(ASTNodeQRGivens* pElement)
    {
         FIGARO_LOG_DBG("********************");
        FIGARO_LOG_DBG("QR Givens");
        FIGARO_LOG_DBG("Relation order", pElement->getRelationOrder())
        MICRO_BENCH_INIT(mainAlgorithm)
        MICRO_BENCH_START(mainAlgorithm)
        pElement->getOperand()->accept(this);
        MICRO_BENCH_STOP(mainAlgorithm)
        FIGARO_LOG_BENCH("Figaro", "Main second pass algorithm",  MICRO_BENCH_GET_TIMER_LAP(mainAlgorithm));
        MICRO_BENCH_INIT(postprocess)
        MICRO_BENCH_START(postprocess)
        auto [rName, qName] =
            m_pDatabase->computePostprocessing(pElement->getRelationOrder(), m_qrHintType, m_saveResult, m_joinRelName);
        MICRO_BENCH_STOP(postprocess)
        FIGARO_LOG_BENCH("Figaro", "Post processing",  MICRO_BENCH_GET_TIMER_LAP(postprocess));
        FIGARO_LOG_DBG("FInished")
        return new ASTVisitorQRResult(rName, qName);

    }

}