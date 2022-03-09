#ifndef _FIGARO_AST_JOIN_VISITOR_H_
#define _FIGARO_AST_JOIN_VISITOR_H_

#include "ASTVisitor.h"
#include "ASTVisitorJoinResult.h"

namespace Figaro
{
    class ASTJoinVisitor: public ASTVisitor
    {
        uint32_t m_joinSize;
        bool m_isLeapFrog = false;
        std::vector<std::string> m_vPreOrderRelNames;
        std::vector<std::string> m_vPreOrderParRelNames;
        std::vector<std::vector<std::string> > m_vvJoinAttrNames;
        std::vector<std::vector<std::string> > m_vvParJoinAttrNames;
    public:
        ASTJoinVisitor(Database* pDatabase, bool isLeapFrog = false,
            const std::vector<std::string>& vPreOrderRelNames = {},
            const std::vector<std::string>& vPreOrderParRelNames = {},
            const std::vector<std::vector<std::string> >& vvJoinAttrNames = {},
            const std::vector<std::vector<std::string> >& vvParJoinAttrNames = {},
            uint32_t joinSize = 0
            ):
            ASTVisitor(pDatabase), m_isLeapFrog(isLeapFrog),
            m_vPreOrderRelNames(vPreOrderRelNames),
            m_vPreOrderParRelNames(vPreOrderParRelNames),
            m_vvJoinAttrNames(vvJoinAttrNames),
            m_vvParJoinAttrNames(vvParJoinAttrNames),
            m_joinSize(joinSize)
         {}
        virtual ASTVisitorJoinResult* visitNodeRelation(ASTNodeRelation* pElement) override;
        virtual ASTVisitorJoinResult* visitNodeJoin(ASTNodeJoin* pElement) override;
        virtual ASTVisitorJoinResult* visitNodeQRGivens(ASTNodeQRGivens* pElement) override {
            FIGARO_LOG_ASSERT(1!=1)
            return nullptr;}
        virtual ASTVisitorJoinResult* visitNodePostProcQR(ASTNodePostProcQR* pElement) override {
            FIGARO_LOG_ASSERT(1!=1)
            return nullptr;
        }

        virtual ASTVisitorAbsResult* visitNodeRightMultiply(ASTNodeRightMultiply* pElement) override
        {
            FIGARO_LOG_ASSERT(1!=1)
            return nullptr;
        }

        virtual ASTVisitorAbsResult* visitNodeInverse(ASTNodeInverse* pElement) override
        {
            FIGARO_LOG_ASSERT(1!=1)
            return nullptr;
        }

        virtual ASTVisitorAbsResult* visitNodeLinReg(ASTNodeLinReg* pElement) override
        {
            FIGARO_LOG_ASSERT(1!=1)
            return nullptr;
        }

        virtual ~ASTJoinVisitor() override {}
        virtual ASTVisitorAbsResult* visitNodeAssign(ASTNodeAssign* pElement)  override;
        virtual ASTVisitorJoinResult* visitNodeEvalJoin(ASTNodeEvalJoin* pElement)  override;



    };
}

#endif