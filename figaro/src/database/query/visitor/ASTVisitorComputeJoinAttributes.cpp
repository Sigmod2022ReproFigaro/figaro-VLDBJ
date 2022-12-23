#include "database/query/visitor/ASTVisitorComputeJoinAttributes.h"
#include "omp.h"

namespace Figaro
{
    ASTVisitorResultAbs* ASTVisitorComputeJoinAttributes::visitNodeRelation(ASTNodeRelation* pElement)
    {
        std::string parName = "";
        m_vPreOrderASTNodeAbsRels.push_back(pElement);
        pElement->checkAndUpdateJoinAttributes();
        if (m_sortValues)
        {
            m_pDatabase->sortRelation(pElement->getRelation()->getRelationName(),
                                  pElement->getJoinAttributeNames());
        }
        const auto& relationName = pElement->getRelationName();
        const auto& formJoinAttrNames = getFormateJoinAttributeNames(pElement->getJoinAttributeNames());
        FIGARO_LOG_DBG("relation", relationName, "joinAttributeNames", formJoinAttrNames);
        return nullptr;
    }

    ASTVisitorResultAbs* ASTVisitorComputeJoinAttributes::visitNodeJoin(ASTNodeJoin* pElement)
    {
        FIGARO_LOG_DBG("Join");
        FIGARO_LOG_DBG("Central");
        m_vPreOrderASTNodeAbsRels.push_back(pElement);
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
        return nullptr;
    }

    ASTVisitorResultAbs* ASTVisitorComputeJoinAttributes::visitNodeQRFigaro(ASTNodeQRFigaro* pElement)
    {
        FIGARO_LOG_INFO("QR Figaro");
        FIGARO_LOG_INFO("Relation order", pElement->getRelationOrder())
        FIGARO_LOG_INFO("Skipped attributes", pElement->getDropAttributes())
        FIGARO_LOG_INFO("Number of threads", pElement->getNumThreads())
        pElement->getOperand()->accept(this);
        return nullptr;
    }

    ASTVisitorResultAbs* ASTVisitorComputeJoinAttributes::visitNodeQRPostProc(ASTNodeQRAlg* pElement)
    {
        FIGARO_LOG_INFO("QR Postprocess");
        FIGARO_LOG_INFO("Relation order", pElement->getRelationOrder())

        pElement->getOperand()->accept(this);
        return nullptr;
    }

    ASTVisitorResultAbs* ASTVisitorComputeJoinAttributes::visitNodeEvalJoin(ASTNodeEvalJoin* pElement)
    {
        FIGARO_LOG_INFO("Eval join");
        m_vPreOrderASTNodeAbsRels.clear();
        pElement->getOperand()->accept(this);
        return nullptr;
    }

    ASTVisitorResultAbs* ASTVisitorComputeJoinAttributes::visitNodeLUFigaro(ASTNodeLUFigaro* pElement)
    {
        FIGARO_LOG_INFO("LU Figaro");
        FIGARO_LOG_INFO("Relation order", pElement->getRelationOrder())
        FIGARO_LOG_INFO("Skipped attributes", pElement->getDropAttributes())
        FIGARO_LOG_INFO("Number of threads", pElement->getNumThreads())
        pElement->getOperand()->accept(this);
        return nullptr;
    }


}