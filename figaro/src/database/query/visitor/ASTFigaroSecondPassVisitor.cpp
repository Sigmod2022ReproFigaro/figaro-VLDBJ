#include "database/query/visitor/ASTFigaroSecondPassVisitor.h"
#include "utils/Performance.h"

namespace Figaro
{
    ASTVisitorSecondPassResult* ASTFigaroSecondPassVisitor::visitNodeRelation(ASTNodeRelation* pElement)
    {
        const auto& relationName = pElement->getRelationName();
        FIGARO_LOG_INFO("Finished visiting relation", relationName)
        std::string relHeadsName =
        m_htTmpRelsNames.at(relationName).m_headsName;
        std::string relGenTailsName = m_pDatabase->createDummyGenTailRelation(relationName);return new ASTVisitorSecondPassResult(relHeadsName,
            {{relationName, relGenTailsName}},
            {relationName});
    }

    std::vector<std::string>
    ASTFigaroSecondPassVisitor::getChildrenHeadNames(
        const std::vector<std::string>& vChildrenNames) const
    {
        std::vector<std::string> vChildrenHeadNames;
        for (const auto& childName: vChildrenNames)
        {
            auto& firstPassRelNames = m_htTmpRelsNames.at(childName);
            vChildrenHeadNames.push_back(firstPassRelNames.m_headsName);
        }
        return vChildrenHeadNames;
    }

    ASTVisitorSecondPassResult* ASTFigaroSecondPassVisitor::visitNodeJoin(ASTNodeJoin* pElement)
    {
        std::vector<std::string> vChildrenNames;
        const auto& relationName = pElement->getCentralRelation()->getRelationName();
        FIGARO_LOG_INFO("vpChildRels", pElement->getChildrenNames())
        vChildrenNames = pElement->getChildrenNames();

        std::unordered_map<std::string, ASTVisitorSecondPassResult::SecondPassRelNames> namesTmpRels;
        std::vector<std::string> vChildGenHeadNames;
        std::vector<std::string> vSubTreeRelNames;
        std::vector<std::vector<std::string> > vvSubTreeRelNames;

        vSubTreeRelNames.push_back(relationName);

        for (const auto& pChild: pElement->getChildren())
        {
            FIGARO_LOG_INFO("Child");
            ASTVisitorSecondPassResult* pResult =
            (ASTVisitorSecondPassResult*)pChild->accept(this);
            namesTmpRels.insert(pResult->getHtNamesTmpRels().begin(),
                pResult->getHtNamesTmpRels().end());
            vChildGenHeadNames.push_back(pResult->getGenHeadsName());
            vSubTreeRelNames.insert(
                vSubTreeRelNames.end(),
                pResult->getSubTreeRelNames().begin(),
                pResult->getSubTreeRelNames().end());
            vvSubTreeRelNames.push_back(pResult->getSubTreeRelNames());
            delete pResult;
        }
        // TODO: return new relation
        std::string aggrAwayRelName =
            m_pDatabase-> aggregateAwayChildrenRelations(
            relationName,
            m_htTmpRelsNames.at(relationName).m_headsName,
            vChildrenNames,
            vChildGenHeadNames,
            pElement->getJoinAttributeNames(),
            pElement->getChildrenParentJoinAttributeNames(),
            vSubTreeRelNames,
            vvSubTreeRelNames);

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
        // TODO: pass new relation
        auto [genHeadRelName, genTailRelName] = m_pDatabase->computeAndScaleGeneralizedHeadAndTail(
            relationName,
            aggrAwayRelName,
            pElement->getJoinAttributeNames(),
            parJoinAttributeNames,
            isRootNode, vSubTreeRelNames.size());
        namesTmpRels.insert({{relationName, ASTVisitorSecondPassResult::SecondPassRelNames(genTailRelName)}});

        // multiply r^-1 with each of the relations using new API
        // join relations.

        return  new ASTVisitorSecondPassResult(genHeadRelName, namesTmpRels, vSubTreeRelNames);
    }

    ASTVisitorQRResult* ASTFigaroSecondPassVisitor::visitNodeQRGivens(ASTNodeQRGivens* pElement)
    {
        std::vector<std::string> vGenTailRelNames;
        std::vector<std::string> vTailRelNames;
        std::vector<std::string> vRelOrder = pElement->getRelationOrder();
        FIGARO_LOG_INFO("********************");
        FIGARO_LOG_INFO("QR Givens");
        FIGARO_LOG_INFO("Relation order", pElement->getRelationOrder())
        //MICRO_BENCH_INIT(mainAlgorithm)
        //MICRO_BENCH_START(mainAlgorithm)
         ASTVisitorSecondPassResult* pResult =
            (ASTVisitorSecondPassResult*)pElement->getOperand()->accept(this);
        //MICRO_BENCH_STOP(mainAlgorithm)
        //FIGARO_LOG_BENCH("Figaro", "Main second pass algorithm",  MICRO_BENCH_GET_TIMER_LAP(mainAlgorithm));

        for (const auto& relName: vRelOrder)
        {
            vTailRelNames.push_back(m_htTmpRelsNames.at(relName).m_tailsName);
        }

        for (const auto& relName: vRelOrder)
        {
            vGenTailRelNames.push_back(pResult->getHtNamesTmpRels().at(relName).m_genTailsName);
        }

        //MICRO_BENCH_INIT(postprocess)
        //MICRO_BENCH_START(postprocess)
        auto [rName, qName] =
            m_pDatabase->computePostprocessing
            (pElement->getRelationOrder(),
            pResult->getGenHeadsName(),
            vTailRelNames,
            vGenTailRelNames,
            m_qrHintType, m_saveResult, m_joinRelName);
        //MICRO_BENCH_STOP(postprocess)
        //FIGARO_LOG_BENCH("Figaro", "Post processing",  MICRO_BENCH_GET_TIMER_LAP(postprocess));
        FIGARO_LOG_INFO("FInished")
        delete pResult;
        return new ASTVisitorQRResult(rName, qName);
    }

}