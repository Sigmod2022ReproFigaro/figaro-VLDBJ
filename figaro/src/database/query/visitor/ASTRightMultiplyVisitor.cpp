#include "database/query/visitor/ASTRightMultiplyVisitor.h"
#include "database/query/visitor/ASTVisitorJoinResult.h"

namespace Figaro
{
    ASTVisitorJoinResult* ASTRightMultiplyVisitor::visitNodeRelation(ASTNodeRelation* pElement)
    {
        FIGARO_LOG_INFO("Right multiply visiting NODE RELATION", pElement->getRelationName())
        uint32_t numNonJoinAttrs;
        numNonJoinAttrs = m_pDatabase->
            getRelationAttributeNames(pElement->getRelationName()).size()
            - pElement->getJoinAttributeNames().size();
        //MICRO_BENCH_INIT(rightMultiply)
        //MICRO_BENCH_START(rightMultiply)
        std::string mulRelName = m_pDatabase->multiply(pElement->getRelationName(), m_relName,
        pElement->getJoinAttributeNames(), {}, startRowIdx);
        if (m_useLFTJoin)
        {
            m_vRelNames.push_back(mulRelName);
        }

        startRowIdx += numNonJoinAttrs;
        FIGARO_LOG_INFO("Finished visiting NODE RELATION", pElement->getRelationName(), mulRelName)
        FIGARO_LOG_INFO("JOIN_ATTRS", pElement->getJoinAttributeNames())
        //MICRO_BENCH_STOP(rightMultiply)
        //FIGARO_LOG_BENCH("rightMultiplyRelation", MICRO_BENCH_GET_TIMER_LAP(rightMultiply))
        return new ASTVisitorJoinResult(mulRelName);
    }

    ASTVisitorJoinResult* ASTRightMultiplyVisitor::visitNodeJoin(ASTNodeJoin* pElement)
    {
        std::vector<std::string> vRelNames;
        std::string centralRelName;
        std::string joinRelName = "";
        FIGARO_LOG_INFO("VISITING JOIN")
        ASTVisitorJoinResult* pJoinResult =
            (ASTVisitorJoinResult*)pElement->getCentralRelation()->accept(this);
        centralRelName = pJoinResult->getJoinRelName();
        FIGARO_LOG_INFO("central rel name", centralRelName)
        delete pJoinResult;
        for (const auto& pChild: pElement->getChildren())
        {
            ASTVisitorJoinResult* pJoinResult = (ASTVisitorJoinResult*)(pChild->accept(this));
            vRelNames.push_back(pJoinResult->getJoinRelName());
            delete pJoinResult;
        }

        if (!m_useLFTJoin)
        {

            FIGARO_LOG_INFO("central rel name", centralRelName)
            FIGARO_LOG_INFO("central rel name", pElement->getJoinAttributeNames())
            FIGARO_LOG_INFO("central rel name", pElement->getChildrenParentJoinAttributeNames())

            MICRO_BENCH_INIT(joinRelationsAndAddColumns)
            MICRO_BENCH_START(joinRelationsAndAddColumns)
            joinRelName = m_pDatabase->joinRelationsAndAddColumns(
                centralRelName,
                vRelNames,
                pElement->getJoinAttributeNames(),
                pElement->getParJoinAttributeNames(),
                pElement->getChildrenParentJoinAttributeNames(), false);
            MICRO_BENCH_STOP(joinRelationsAndAddColumns)
            FIGARO_LOG_BENCH("Value" + centralRelName, MICRO_BENCH_GET_TIMER_LAP(joinRelationsAndAddColumns))
        }
        return new ASTVisitorJoinResult(joinRelName);
    }

    ASTVisitorAbsResult* ASTRightMultiplyVisitor::visitNodeRightMultiply(ASTNodeRightMultiply* pElement)
    {
        FIGARO_LOG_INFO("VISITING RIGHT MULTIPLY")
        FIGARO_LOG_INFO("VISITING LEFT")
        ASTVisitorAbsResult* pResult;
        if (m_useLFTJoin)
        {
            pResult = pElement->getLeftOperand()->accept(this);
            std::unordered_map<std::string, uint32_t> htNameIdx;
            for (uint32_t idxRel = 0; idxRel < m_vPreOrderRelNames.size(); idxRel++)
            {
                htNameIdx[m_vPreOrderRelNames[idxRel]] = idxRel;
            }
            for (uint32_t idxRel = 0; idxRel < m_vPreOrderParRelNames.size(); idxRel++)
            {
                std::string parRelName = m_vPreOrderParRelNames[idxRel];
                uint32_t parIdxRel;
                std::string parRelNameAd;
                if (parRelName == "")
                {
                    parIdxRel = 0;
                    parRelNameAd = "";
                }
                else
                {
                    parIdxRel = htNameIdx[parRelName];
                    parRelNameAd = m_vRelNames[parIdxRel];
                }
                m_vParRelNames.push_back(parRelNameAd);
            }
            std::string qRelName = m_pDatabase->joinRelationsAndAddColumns(
                m_vRelNames, m_vParRelNames,
                m_vvJoinAttrNames, m_vvParJoinAttrNames, m_joinSize);
            pResult = new ASTVisitorJoinResult(qRelName);
        }
        else
        {
            pResult = pElement->getLeftOperand()->accept(this);
        }
        return pResult;
    }
}