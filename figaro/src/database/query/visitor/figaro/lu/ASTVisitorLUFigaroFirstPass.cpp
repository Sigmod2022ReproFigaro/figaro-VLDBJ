#include "database/query/visitor/figaro/lu/ASTVisitorLUFigaroFirstPass.h"

namespace Figaro
{
    ASTVisitorResultFirstPass* ASTVisitorLUFigaroFirstPass::visitNodeRelation(ASTNodeRelation* pElement)
    {
        std::vector<std::string> childrenNames;
        std::vector<std::vector<std::string> > vvChildrenParentJoinAttributeNames;
        const auto& relationName = pElement->getRelationName();
        const auto& formJoinAttrNames = getFormateJoinAttributeNames(pElement->getJoinAttributeNames());
        FIGARO_LOG_INFO("formJoinAttrNames", formJoinAttrNames)


        auto [headsName, tailsName] =  m_pDatabase->computeLUHeadsAndTails(relationName, pElement->getJoinAttributeNames(),
        true);

        std::unordered_map<std::string, ASTVisitorResultFirstPass::FirstPassRelNames> namesTmpRels =
        {{relationName,
        ASTVisitorResultFirstPass::FirstPassRelNames(headsName, tailsName)}};
        return new ASTVisitorResultFirstPass(namesTmpRels);
    }

    ASTVisitorResultFirstPass* ASTVisitorLUFigaroFirstPass::visitNodeJoin(ASTNodeJoin* pElement)
    {
        FIGARO_LOG_INFO("Join");
        FIGARO_LOG_INFO("Central");

        const auto& relationName = pElement->getCentralRelation()->getRelationName();
        const auto& formJoinAttrNames = getFormateJoinAttributeNames(pElement->getJoinAttributeNames());
        FIGARO_LOG_INFO("formJoinAttrNames", formJoinAttrNames)
        auto [headsName, tailsName] =  m_pDatabase->computeLUHeadsAndTails(relationName, pElement->getJoinAttributeNames(),
             false);


        std::unordered_map<std::string, ASTVisitorResultFirstPass::FirstPassRelNames> namesTmpRels =
        {{relationName,
        ASTVisitorResultFirstPass::FirstPassRelNames(headsName, tailsName)}};

        for (const auto& pChild: pElement->getChildren())
        {
            FIGARO_LOG_INFO("Child");
            ASTVisitorResultFirstPass* pResult = (ASTVisitorResultFirstPass*)pChild->accept(this);

            namesTmpRels.insert(
                pResult->getHtNamesTmpRels().begin(),
                pResult->getHtNamesTmpRels().end());
            delete pResult;
        }
        return new ASTVisitorResultFirstPass(namesTmpRels);
    }

    ASTVisitorResultFirstPass* ASTVisitorLUFigaroFirstPass::visitNodeLUFigaro(ASTNodeLUFigaro* pElement)
    {
        FIGARO_LOG_INFO("********************");
        FIGARO_LOG_INFO("LU Figaro");
        FIGARO_LOG_INFO("Relation order", pElement->getRelationOrder())
        ASTVisitorResultFirstPass* pResult = (ASTVisitorResultFirstPass*)pElement->getOperand()->accept(this);
        return pResult;
    }
}