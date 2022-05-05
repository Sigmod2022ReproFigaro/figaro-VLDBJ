#ifndef _FIGARO_AST_VISITOR_RIGHT_MULTIPLY_H_
#define _FIGARO_AST_VISITOR_RIGHT_MULTIPLY_H_

#include "ASTVisitor.h"
#include "./result/ASTVisitorResultJoin.h"

namespace Figaro
{
    class ASTVisitorRightMultiply: public ASTVisitor
    {
        std::string m_relName;
        uint32_t startRowIdx = 0;
        bool m_useLFTJoin;
        uint32_t m_joinSize;
        std::vector<std::string> m_vRelNames;
        std::vector<std::string> m_vParRelNames;
        std::vector<std::string> m_vPreOrderRelNames;
        std::vector<std::string> m_vPreOrderParRelNames;
        std::vector<std::vector<std::string> > m_vvJoinAttrNames;
        std::vector<std::vector<std::string> > m_vvParJoinAttrNames;
    public:
        ASTVisitorRightMultiply(
            Database* pDatabase,
            const std::string& relName,
            bool useLFTJoin = false,
            const std::vector<std::string>& vPreOrderRelNames = {},
            const std::vector<std::string>& vPreOrderParRelNames = {},
            const std::vector<std::vector<std::string> >& vvJoinAttrNames = {},
            const std::vector<std::vector<std::string> >& vvParJoinAttrNames = {},
            uint32_t joinSize = 0): ASTVisitor(pDatabase), m_relName(relName),
                m_useLFTJoin(useLFTJoin), m_vPreOrderRelNames(vPreOrderRelNames),
                m_vPreOrderParRelNames(vPreOrderParRelNames),
                m_vvJoinAttrNames(vvJoinAttrNames),
                m_vvParJoinAttrNames(vvParJoinAttrNames),
                m_joinSize(joinSize) {}
        virtual ASTVisitorResultJoin* visitNodeRelation(ASTNodeRelation* pElement) override;
        virtual ASTVisitorResultJoin* visitNodeJoin(ASTNodeJoin* pElement) override;
        virtual  ASTVisitorResultAbs* visitNodeRightMultiply(ASTNodeRightMultiply* pElement) override;

        virtual ASTVisitorResultAbs* visitNodeQRFigaro([[maybe_unused]] ASTNodeQRFigaro* pElement) override {
            FIGARO_LOG_ASSERT(1!=1)
            return nullptr;
            }

        virtual ASTVisitorResultAbs* visitNodeLUFigaro([[maybe_unused]] ASTNodeLUFigaro* pElement) override {
            FIGARO_LOG_ASSERT(1!=1)
            return nullptr;
            }

        virtual ASTVisitorResultAbs* visitNodeQRPostProc([[maybe_unused]] ASTNodeQRPostProc* pElement) override {
            FIGARO_LOG_ASSERT(1!=1)
            return nullptr;
        }

        virtual ASTVisitorResultAbs* visitNodeAssign([[maybe_unused]] ASTNodeAssign* pElement)  override
        {
            FIGARO_LOG_ASSERT(1!=1)
            return nullptr;
        }

        virtual ASTVisitorResultAbs* visitNodeEvalJoin([[maybe_unused]] ASTNodeEvalJoin* pElement)  override
        {
            FIGARO_LOG_ASSERT(1!=1)
            return nullptr;
        }

        virtual ASTVisitorResultAbs* visitNodeInverse([[maybe_unused]]  ASTNodeInverse* pElement)  override
        {
            FIGARO_LOG_ASSERT(1!=1)
            return nullptr;
        }

         virtual ASTVisitorResultAbs* visitNodeLinReg([[maybe_unused]] ASTNodeLinReg* pElement)  override
        {
            FIGARO_LOG_ASSERT(1!=1)
            return nullptr;
        }

        virtual ~ASTVisitorRightMultiply() override {}
    };
}

#endif