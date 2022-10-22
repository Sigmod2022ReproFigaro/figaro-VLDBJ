#include "database/query/visitor/figaro/qr/ASTVisitorQRFigaroSecondPass.h"
#include "utils/Performance.h"

namespace Figaro
{
    ASTVisitorResultSecondPass* ASTVisitorQRFigaroSecondPass::visitNodeRelation(ASTNodeRelation* pElement)
    {
        const auto& relationName = pElement->getRelationName();
        FIGARO_LOG_INFO("Finished visiting relation", relationName)
        std::string relHeadsName =
        m_htTmpRelsNames.at(relationName).m_headsName;
        std::string relGenTailsName = m_pDatabase->createDummyGenTailRelation(relationName);return new ASTVisitorResultSecondPass(relHeadsName,
            {{relationName, relGenTailsName}},
            {relationName});
    }

    std::vector<std::string>
    ASTVisitorQRFigaroSecondPass::getChildrenHeadNames(
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

    ASTVisitorResultSecondPass* ASTVisitorQRFigaroSecondPass::visitNodeJoin(ASTNodeJoin* pElement)
    {
        std::vector<std::string> vChildrenNames;
        const auto& relationName = pElement->getCentralRelation()->getRelationName();
        FIGARO_LOG_INFO("vpChildRels", pElement->getChildrenNames())
        vChildrenNames = pElement->getChildrenNames();

        std::unordered_map<std::string, ASTVisitorResultSecondPass::SecondPassRelNames> namesTmpRels;
        std::vector<std::string> vChildGenHeadNames;
        std::vector<std::string> vSubTreeRelNames;
        std::vector<std::vector<std::string> > vvSubTreeRelNames;

        vSubTreeRelNames.push_back(relationName);

        for (const auto& pChild: pElement->getChildren())
        {
            FIGARO_LOG_INFO("Child");
            ASTVisitorResultSecondPass* pResult =
            (ASTVisitorResultSecondPass*)pChild->accept(this);
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
            m_pDatabase-> aggregateAwayQRChildrenRelations(
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
        auto [genHeadRelName, genTailRelName] = m_pDatabase->computeAndScaleQRGeneralizedHeadAndTail(
            relationName,
            aggrAwayRelName,
            pElement->getJoinAttributeNames(),
            parJoinAttributeNames,
            isRootNode, vSubTreeRelNames.size());
        namesTmpRels.insert({{relationName, ASTVisitorResultSecondPass::SecondPassRelNames(genTailRelName)}});

        // multiply r^-1 with each of the relations using new API
        // join relations.

        return  new ASTVisitorResultSecondPass(genHeadRelName, namesTmpRels, vSubTreeRelNames);
    }

    ASTVisitorResultQR* ASTVisitorQRFigaroSecondPass::visitNodeQRFigaro(ASTNodeQRFigaro* pElement)
    {
        std::string rNameOut = "";
        std::string qNameOut = "";
        std::vector<std::string> vGenTailRelNames;
        std::vector<std::string> vTailRelNames;
        std::vector<std::string> vRelOrder = pElement->getRelationOrder();
        FIGARO_LOG_INFO("********************");
        FIGARO_LOG_INFO("QR Givens");
        FIGARO_LOG_INFO("Relation order", pElement->getRelationOrder())
        FIGARO_MIC_BEN_INIT(mainAlgorithm)
        FIGARO_MIC_BEN_START(mainAlgorithm)
         ASTVisitorResultSecondPass* pResult =
            (ASTVisitorResultSecondPass*)pElement->getOperand()->accept(this);

        for (const auto& relName: vRelOrder)
        {
            vTailRelNames.push_back(m_htTmpRelsNames.at(relName).m_tailsName);
        }

        for (const auto& relName: vRelOrder)
        {
            vGenTailRelNames.push_back(pResult->getHtNamesTmpRels().at(relName).m_genTailsName);
        }
        FIGARO_MIC_BEN_STOP(mainAlgorithm)
        FIGARO_LOG_MIC_BEN("Figaro", "Main second pass algorithm",  FIGARO_MIC_BEN_GET_TIMER_LAP(mainAlgorithm));


        FIGARO_MIC_BEN_INIT(postprocess)
        FIGARO_MIC_BEN_START(postprocess)
        if (m_evalPostProcessing)
        {
            auto [rName, qName] =
                m_pDatabase->computeQRPostprocessing
                (pElement->getRelationOrder(),
                pResult->getGenHeadsName(),
                vTailRelNames,
                vGenTailRelNames,
                m_qrHintType, m_saveResult, m_joinRelName);
            FIGARO_LOG_INFO("Finished")
            rNameOut = rName;
            qNameOut = qName;
        }
        FIGARO_MIC_BEN_STOP(postprocess)
        FIGARO_LOG_MIC_BEN("Figaro", "Post processing",FIGARO_MIC_BEN_GET_TIMER_LAP(postprocess));
        delete pResult;
        return new ASTVisitorResultQR(rNameOut, qNameOut);
    }

}