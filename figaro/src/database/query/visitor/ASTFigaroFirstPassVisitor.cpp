#include "database/query/visitor/ASTFigaroFirstPassVisitor.h"

namespace Figaro
{
    ASTVisitorFirstPassResult* ASTFigaroFirstPassVisitor::visitNodeRelation(ASTNodeRelation* pElement)
    {
        std::vector<std::string> childrenNames;
        std::vector<std::vector<std::string> > vvChildrenParentJoinAttributeNames;
        const auto& relationName = pElement->getRelationName();
        const auto& formJoinAttrNames = getFormateJoinAttributeNames(pElement->getJoinAttributeNames());
        FIGARO_LOG_INFO("formJoinAttrNames", formJoinAttrNames)


        auto [headsName, tailsName] =  m_pDatabase->computeHeadsAndTails(relationName, pElement->getJoinAttributeNames(),
        true);

        std::unordered_map<std::string, ASTVisitorFirstPassResult::FirstPassRelNames> namesTmpRels =
        {{relationName,
        ASTVisitorFirstPassResult::FirstPassRelNames(headsName, tailsName)}};
        return new ASTVisitorFirstPassResult(namesTmpRels);
    }

    ASTVisitorFirstPassResult* ASTFigaroFirstPassVisitor::visitNodeJoin(ASTNodeJoin* pElement)
    {
        FIGARO_LOG_INFO("Join");
        FIGARO_LOG_INFO("Central");

        const auto& relationName = pElement->getCentralRelation()->getRelationName();
        const auto& formJoinAttrNames = getFormateJoinAttributeNames(pElement->getJoinAttributeNames());
        FIGARO_LOG_INFO("formJoinAttrNames", formJoinAttrNames)
        auto [headsName, tailsName] =  m_pDatabase->computeHeadsAndTails(relationName, pElement->getJoinAttributeNames(),
             false);


        std::unordered_map<std::string, ASTVisitorFirstPassResult::FirstPassRelNames> namesTmpRels =
        {{relationName,
        ASTVisitorFirstPassResult::FirstPassRelNames(headsName, tailsName)}};

        for (const auto& pChild: pElement->getChildren())
        {
            FIGARO_LOG_INFO("Child");
            ASTVisitorFirstPassResult* pResult = (ASTVisitorFirstPassResult*)pChild->accept(this);

            namesTmpRels.insert(
                pResult->getHtNamesTmpRels().begin(),
                pResult->getHtNamesTmpRels().end());
            delete pResult;
        }
        return new ASTVisitorFirstPassResult(namesTmpRels);
    }

    ASTVisitorFirstPassResult* ASTFigaroFirstPassVisitor::visitNodeQRGivens(ASTNodeQRGivens* pElement)
    {
        FIGARO_LOG_INFO("********************");
        FIGARO_LOG_INFO("QR Givens");
        FIGARO_LOG_INFO("Relation order", pElement->getRelationOrder())
        ASTVisitorFirstPassResult* pResult = (ASTVisitorFirstPassResult*)pElement->getOperand()->accept(this);
        return pResult;
    }
}