#ifndef _FIGARO_AST_VISITOR_QR_GIVENS_H_
#define _FIGARO_AST_VISITOR_QR_GIVENS_H_

#include "ASTVisitorQRGivensAbs.h"
#include "ASTVisitorJoinResult.h"
#include "ASTVisitorQRResult.h"

namespace Figaro
{
    class ASTVisitorQueryEval: public ASTVisitor
    {
        MemoryLayout m_memoryLayout;
        QRGivensHintType m_qrHintType;
        Database* m_pDatabase;
        bool m_saveResult;
    public:
        ASTVisitorQueryEval(
            Database* pDatabase,
            Figaro::MemoryLayout memoryLayout,
            Figaro::QRGivensHintType qrHintType,
            bool saveResult
            ): ASTVisitor(pDatabase), m_memoryLayout(memoryLayout),
                m_pDatabase(pDatabase), m_qrHintType(qrHintType),
                m_saveResult(saveResult){}
        ASTVisitorAbsResult* visitNodeRelation(ASTNodeRelation* pElement) override
        { return nullptr; }
        ASTVisitorAbsResult* visitNodeJoin(ASTNodeJoin* pElement) override { return nullptr; }
        ASTVisitorQRResult* visitNodeQRGivens(ASTNodeQRGivens* pElement) override;
        ASTVisitorQRResult* visitNodePostProcQR(ASTNodePostProcQR* pElement) override;
        ASTVisitorAbsResult* visitNodeAssign(ASTNodeAssign* pElement) override { return nullptr; }
        ASTVisitorJoinResult* visitNodeEvalJoin(ASTNodeEvalJoin* pElement) override;
        ASTVisitorAbsResult* visitNodeRightMultiply(ASTNodeRightMultiply* pElement) override;

        virtual ~ASTVisitorQueryEval() override {}
    };
}

#endif