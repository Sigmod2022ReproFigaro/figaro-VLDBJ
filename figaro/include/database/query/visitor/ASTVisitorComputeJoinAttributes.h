#ifndef _FIGARO_AST_VISITOR_COMPUTE_JOIN_ATTRIBUTES_H_
#define _FIGARO_AST_VISITOR_COMPUTE_JOIN_ATTRIBUTES_H_

#include "./figaro/qr/ASTVisitorQRFigaroAbs.h"

namespace Figaro
{
    class ASTVisitorComputeJoinAttributes: public ASTVisitorQRFigaroAbs
    {
        void initializeEnumAndDenomRelations(ASTNodeRelation* pRel);
        bool m_sortValues;
        Figaro::MemoryLayout m_memoryLayout;
        std::vector<ASTNodeAbsRelation*> m_vPreOrderASTNodeAbsRels;
    public:
        ASTVisitorComputeJoinAttributes(
            Database* pDatabase, bool sortValues,
            Figaro::MemoryLayout memoryLayout): ASTVisitorQRFigaroAbs(pDatabase), m_sortValues(sortValues), m_memoryLayout(memoryLayout) {}
        ASTVisitorResultAbs* visitNodeRelation(ASTNodeRelation* pElement) override;
        ASTVisitorResultAbs* visitNodeJoin(ASTNodeJoin* pElement) override;
        ASTVisitorResultAbs* visitNodeQRFigaro(ASTNodeQRFigaro* pElement) override;
        virtual ASTVisitorResultAbs* visitNodeLUFigaro(ASTNodeLUFigaro* pElement) override;
        ASTVisitorResultAbs* visitNodeQRDecAlg(ASTNodeQRAlg* pElement) override;
        ASTVisitorResultAbs* visitNodeEvalJoin(ASTNodeEvalJoin* pElement) override;

        std::vector<std::string> getPreOrderRelNames(void) const
        {
            std::vector<std::string> vPreOrderRelNames;
            for (const auto& astNodeAbsRel: m_vPreOrderASTNodeAbsRels)
            {
                vPreOrderRelNames.push_back(astNodeAbsRel->getRelation()->getRelationName());
            }
            return vPreOrderRelNames;
        }

        std::vector<std::string> getPreOrderParRelNames(void) const
        {
            std::vector<std::string> vPreOrderParRelNames;
            for (const auto& astNodeAbsRel: m_vPreOrderASTNodeAbsRels)
            {
                std::string parRelName = "";
                if (astNodeAbsRel->getParent() != nullptr)
                {
                    parRelName = astNodeAbsRel->getParent()->getRelation()->getRelationName();
                }
                vPreOrderParRelNames.push_back(parRelName);
            }
            return vPreOrderParRelNames;
        }

        const std::vector<std::vector<std::string> > getPreOrderVVJoinAttrNames(void) const
        {
            std::vector<std::vector<std::string> > vvJoinAttrNames;
            for (const auto& astNodeAbsRel: m_vPreOrderASTNodeAbsRels)
            {
                vvJoinAttrNames.push_back(astNodeAbsRel->getRelation()->getJoinAttributeNames());
            }
            return vvJoinAttrNames;
        }

        const std::vector<std::vector<std::string> > getPreOrderVVParJoinAttrNames(void) const
        {
            std::vector<std::vector<std::string> > vvParJoinAttrNames;
            for (const auto& astNodeAbsRel: m_vPreOrderASTNodeAbsRels)
            {
                std::vector vParJoinAttrNames = astNodeAbsRel->getRelation()->getParJoinAttributeNames();
                vvParJoinAttrNames.push_back(vParJoinAttrNames);
            }
            return vvParJoinAttrNames;
        }

        virtual ~ASTVisitorComputeJoinAttributes() override {}


    };
}

#endif