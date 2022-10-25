#include "database/query/visitor/ASTVisitorRightMultiply.h"
#include "database/query/visitor/result/ASTVisitorResultJoin.h"

namespace Figaro
{
    ASTVisitorResultJoin* ASTVisitorRightMultiply::visitNodeRelation(ASTNodeRelation* pElement)
    {
        FIGARO_LOG_INFO("Right multiply visiting NODE RELATION", pElement->getRelationName())
        uint32_t numNonJoinAttrs;
        numNonJoinAttrs = m_pDatabase->
            getRelationAttributeNames(pElement->getRelationName()).size()
            - pElement->getJoinAttributeNames().size();
        //FIGARO_MIC_BEN_INIT(rightMultiply)
        //FIGARO_MIC_BEN_START(rightMultiply)
        std::string mulRelName = m_pDatabase->multiply(pElement->getRelationName(), m_relName,
        pElement->getJoinAttributeNames(), {}, startRowIdx);
        if (m_useLFTJoin)
        {
            m_vRelNames.push_back(mulRelName);
        }

        startRowIdx += numNonJoinAttrs;
        FIGARO_LOG_INFO("Finished visiting NODE RELATION", pElement->getRelationName(), mulRelName)
        FIGARO_LOG_INFO("JOIN_ATTRS", pElement->getJoinAttributeNames())
        //FIGARO_MIC_BEN_STOP(rightMultiply)
        //FIGARO_LOG_BENCH("rightMultiplyRelation", FIGARO_MIC_BEN_GET_TIMER_LAP(rightMultiply))
        return new ASTVisitorResultJoin(mulRelName);
    }

    ASTVisitorResultJoin* ASTVisitorRightMultiply::visitNodeJoin(ASTNodeJoin* pElement)
    {
        std::vector<std::string> vRelNames;
        std::string centralRelName;
        std::string joinRelName = "";
        FIGARO_LOG_INFO("VISITING JOIN")
        ASTVisitorResultJoin* pJoinResult =
            (ASTVisitorResultJoin*)pElement->getCentralRelation()->accept(this);
        centralRelName = pJoinResult->getJoinRelName();
        FIGARO_LOG_INFO("central rel name", centralRelName)
        delete pJoinResult;
        for (const auto& pChild: pElement->getChildren())
        {
            ASTVisitorResultJoin* pJoinResult = (ASTVisitorResultJoin*)(pChild->accept(this));
            vRelNames.push_back(pJoinResult->getJoinRelName());
            delete pJoinResult;
        }

        if (!m_useLFTJoin)
        {

            FIGARO_LOG_INFO("central rel name", centralRelName)
            FIGARO_LOG_INFO("central rel name", pElement->getJoinAttributeNames())
            FIGARO_LOG_INFO("central rel name", pElement->getChildrenParentJoinAttributeNames())

            FIGARO_MIC_BEN_INIT(joinRelationsAndAddColumns)
            FIGARO_MIC_BEN_START(joinRelationsAndAddColumns)
            joinRelName = m_pDatabase->joinRelationsAndAddColumns(
                centralRelName,
                vRelNames,
                pElement->getJoinAttributeNames(),
                pElement->getParJoinAttributeNames(),
                pElement->getChildrenParentJoinAttributeNames(), false);
            FIGARO_MIC_BEN_STOP(joinRelationsAndAddColumns)
            FIGARO_LOG_BENCH("Value" + centralRelName, FIGARO_MIC_BEN_GET_TIMER_LAP(joinRelationsAndAddColumns))
        }
        return new ASTVisitorResultJoin(joinRelName);
    }

    ASTVisitorResultAbs* ASTVisitorRightMultiply::visitNodeRightMultiply(ASTNodeRightMultiply* pElement)
    {
        FIGARO_LOG_INFO("VISITING RIGHT MULTIPLY")
        FIGARO_LOG_INFO("VISITING LEFT")
        ASTVisitorResultAbs* pResult;
        if (m_useLFTJoin)
        {
            FIGARO_LOG_INFO("USING LFT JOIN")
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
            for (const auto& out: m_vDownCountsSize)
            {
                FIGARO_LOG_INFO("Down count size", out)
            }
            std::string qRelName = m_pDatabase->joinRelationsAndAddColumns(
                m_vRelNames, m_vParRelNames,
                m_vvJoinAttrNames, m_vvParJoinAttrNames, m_vDownCountsSize);
            pResult = new ASTVisitorResultJoin(qRelName);
        }
        else
        {
            pResult = pElement->getLeftOperand()->accept(this);
        }
        return pResult;
    }
}