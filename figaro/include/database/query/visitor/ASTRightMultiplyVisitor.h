#ifndef _FIGARO_AST_RIGHT_MULTIPLY_VISITOR_H_
#define _FIGARO_AST_RIGHT_MULTIPLY_VISITOR_H_

#include "ASTVisitor.h"
#include "ASTVisitorJoinResult.h"

namespace Figaro
{
    class ASTRightMultiplyVisitor: public ASTVisitor
    {
        std::string m_relName;
        uint32_t startRowIdx = 0;
        bool m_useLFTJoin;
    public:
        ASTRightMultiplyVisitor(
            Database* pDatabase,
            const std::string& relName,
            bool useLFTJoin): ASTVisitor(pDatabase), m_relName(relName), 
                m_useLFTJoin(useLFTJoin) {}
        virtual ASTVisitorJoinResult* visitNodeRelation(ASTNodeRelation* pElement) override;
        virtual ASTVisitorJoinResult* visitNodeJoin(ASTNodeJoin* pElement) override;
        virtual  ASTVisitorAbsResult* visitNodeRightMultiply(ASTNodeRightMultiply* pElement) override;

        virtual ASTVisitorAbsResult* visitNodeQRGivens(ASTNodeQRGivens* pElement) override {
            FIGARO_LOG_ASSERT(1!=1)
            return nullptr;
            }
        virtual ASTVisitorAbsResult* visitNodePostProcQR(ASTNodePostProcQR* pElement) override {
            FIGARO_LOG_ASSERT(1!=1)
            return nullptr;
        }

        virtual ASTVisitorAbsResult* visitNodeAssign(ASTNodeAssign* pElement)  override
        {
            FIGARO_LOG_ASSERT(1!=1)
            return nullptr;
        }

        virtual ASTVisitorAbsResult* visitNodeEvalJoin(ASTNodeEvalJoin* pElement)  override
        {
            FIGARO_LOG_ASSERT(1!=1)
            return nullptr;
        }

        virtual ASTVisitorAbsResult* visitNodeInverse(ASTNodeInverse* pElement)  override
        {
            FIGARO_LOG_ASSERT(1!=1)
            return nullptr;
        }

        virtual ~ASTRightMultiplyVisitor() override {}
    };
}

#endif