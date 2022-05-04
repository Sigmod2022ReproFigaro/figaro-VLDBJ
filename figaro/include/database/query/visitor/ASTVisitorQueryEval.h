#ifndef _FIGARO_AST_VISITOR_QR_GIVENS_H_
#define _FIGARO_AST_VISITOR_QR_GIVENS_H_

#include "ASTVisitorQRFigaroAbs.h"
#include "./result/ASTVisitorResultJoin.h"
#include "./result/ASTVisitorResultQR.h"

namespace Figaro
{
    class ASTVisitorQueryEval: public ASTVisitor
    {
        MemoryLayout m_memoryLayout;
        QRHintType m_qrHintType;
        Database* m_pDatabase;
        bool m_saveResult;
    public:
        ASTVisitorQueryEval(
            Database* pDatabase,
            Figaro::MemoryLayout memoryLayout,
            Figaro::QRHintType qrHintType,
            bool saveResult
            ): ASTVisitor(pDatabase), m_memoryLayout(memoryLayout),
                m_pDatabase(pDatabase), m_qrHintType(qrHintType),
                m_saveResult(saveResult){}
        ASTVisitorResultJoin* visitNodeRelation(ASTNodeRelation* pElement) override;
        ASTVisitorResultAbs* visitNodeJoin(ASTNodeJoin* pElement) override { return nullptr; }
        ASTVisitorResultQR* visitNodeQRFigaro(ASTNodeQRFigaro* pElement) override;
        ASTVisitorResultQR* visitNodeQRPostProc(ASTNodeQRPostProc* pElement) override;
        ASTVisitorResultQR* visitNodeLUFigaro(ASTNodeLUFigaro* pElement) override;
        ASTVisitorResultJoin* visitNodeLinReg(ASTNodeLinReg* pElement) override;
        ASTVisitorResultAbs* visitNodeAssign(ASTNodeAssign* pElement) override { return nullptr; }
        ASTVisitorResultJoin* visitNodeEvalJoin(ASTNodeEvalJoin* pElement) override;
        ASTVisitorResultJoin* visitNodeRightMultiply(ASTNodeRightMultiply* pElement) override;
        ASTVisitorResultJoin* visitNodeInverse(ASTNodeInverse* pElement) override;

        virtual ~ASTVisitorQueryEval() override {}
    };
}

#endif