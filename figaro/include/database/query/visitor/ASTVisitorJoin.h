#ifndef _FIGARO_AST_VISITOR_JOIN_H_
#define _FIGARO_AST_VISITOR_JOIN_H_

#include "ASTVisitor.h"
#include "./result/ASTVisitorResultJoin.h"

namespace Figaro
{
    class ASTVisitorJoin: public ASTVisitor
    {
        uint32_t m_joinSize;
        bool m_isLeapFrog = false;
        std::vector<std::string> m_vPreOrderRelNames;
        std::vector<std::string> m_vPreOrderParRelNames;
        std::vector<std::vector<std::string> > m_vvJoinAttrNames;
        std::vector<std::vector<std::string> > m_vvParJoinAttrNames;
    public:
        ASTVisitorJoin(Database* pDatabase, bool isLeapFrog = false,
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
        virtual ASTVisitorResultJoin* visitNodeRelation([[maybe_unused]]ASTNodeRelation* pElement) override;

        virtual ASTVisitorResultJoin* visitNodeJoin(ASTNodeJoin* pElement) override;

        virtual ASTVisitorResultJoin* visitNodeQRFigaro([[maybe_unused]]ASTNodeQRFigaro* pElement) override
        {
            FIGARO_LOG_ASSERT(1!=1)
            return nullptr;
        }

        virtual ASTVisitorResultJoin* visitNodeQRPostProc([[maybe_unused]]ASTNodeQRPostProc* pElement) override
        {
            FIGARO_LOG_ASSERT(1!=1)
            return nullptr;
        }

        virtual ASTVisitorResultJoin* visitNodeLUFigaro([[maybe_unused]]ASTNodeLUFigaro* pElement) override {
            FIGARO_LOG_ASSERT(1!=1)
            return nullptr;
        }

        virtual ASTVisitorResultJoin* visitNodeLULapack([[maybe_unused]]ASTNodeLULapack* pElement) override
        {
            FIGARO_LOG_ASSERT(1!=1)
            return nullptr;
        }

        virtual ASTVisitorResultJoin* visitNodeLUThin([[maybe_unused]]ASTNodeLUThin* pElement) override
        {
            FIGARO_LOG_ASSERT(1!=1)
            return nullptr;
        }


        virtual ASTVisitorResultAbs* visitNodeRightMultiply([[maybe_unused]]ASTNodeRightMultiply* pElement) override
        {
            FIGARO_LOG_ASSERT(1!=1)
            return nullptr;
        }

        virtual ASTVisitorResultAbs* visitNodeInverse([[maybe_unused]]ASTNodeInverse* pElement) override
        {
            FIGARO_LOG_ASSERT(1!=1)
            return nullptr;
        }

        virtual ASTVisitorResultAbs* visitNodeLinReg([[maybe_unused]]ASTNodeLinReg* pElement) override
        {
            FIGARO_LOG_ASSERT(1!=1)
            return nullptr;
        }

        virtual ~ASTVisitorJoin() override {}
        virtual ASTVisitorResultAbs* visitNodeAssign([[maybe_unused]]ASTNodeAssign* pElement)  override;
        virtual ASTVisitorResultJoin* visitNodeEvalJoin(ASTNodeEvalJoin* pElement)  override;



    };
}

#endif