#ifndef _FIGARO_AST_JOIN_ATTRIBUTES_COMPUTE_VISITOR_H_
#define _FIGARO_AST_JOIN_ATTRIBUTES_COMPUTE_VISITOR_H_

#include "ASTVisitorQRGivensAbs.h"

namespace Figaro
{
    class ASTJoinAttributesComputeVisitor: public ASTVisitorQRGivensAbs
    {
        void initializeEnumAndDenomRelations(ASTNodeRelation* pRel);
        bool m_sortValues;
        Figaro::MemoryLayout m_memoryLayout;
        std::vector<ASTNodeAbsRelation*> m_vPreOrderASTNodeAbsRels;
    public:
        ASTJoinAttributesComputeVisitor(
            Database* pDatabase, bool sortValues,
            Figaro::MemoryLayout memoryLayout): ASTVisitorQRGivensAbs(pDatabase), m_sortValues(sortValues), m_memoryLayout(memoryLayout) {}
        ASTVisitorAbsResult* visitNodeRelation(ASTNodeRelation* pElement) override;
        ASTVisitorAbsResult* visitNodeJoin(ASTNodeJoin* pElement) override;
        ASTVisitorAbsResult* visitNodeQRGivens(ASTNodeQRGivens* pElement) override;
        ASTVisitorAbsResult* visitNodePostProcQR(ASTNodePostProcQR* pElement) override;
        ASTVisitorAbsResult* visitNodeEvalJoin(ASTNodeEvalJoin* pElement) override;
        
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

        virtual ~ASTJoinAttributesComputeVisitor() override {}


    };
}

#endif