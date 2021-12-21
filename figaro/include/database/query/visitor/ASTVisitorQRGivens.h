#ifndef _FIGARO_AST_VISITOR_QR_GIVENS_H_
#define _FIGARO_AST_VISITOR_QR_GIVENS_H_

#include "ASTVisitorQRGivensAbs.h"

namespace Figaro
{
    class ASTQRGivensVisitor: public ASTVisitorQRGivensAbs
    {
        MemoryLayout m_memoryLayout;
        QRGivensHintType m_qrHintType;
        Database* m_pDatabase;
        MatrixEigenT* m_pMatR;
        bool m_saveResult;
    public:
        ASTQRGivensVisitor(
            Database* pDatabase,
            Figaro::MemoryLayout memoryLayout,
            Figaro::QRGivensHintType qrHintType,
            MatrixEigenT* pMatR,
            bool saveResult
            ): ASTVisitorQRGivensAbs(pDatabase), m_memoryLayout(memoryLayout),
                m_pDatabase(pDatabase), m_qrHintType(qrHintType), m_pMatR(pMatR)
            {
                if (!saveResult)
                {
                    m_pMatR = nullptr;
                }
            }
        ASTVisitorAbsResult* visitNodeRelation(ASTNodeRelation* pElement) override
        { return nullptr; }
        ASTVisitorAbsResult* visitNodeJoin(ASTNodeJoin* pElement) override { return nullptr; }
        ASTVisitorAbsResult* visitNodeQRGivens(ASTNodeQRGivens* pElement) override;
        ASTVisitorAbsResult* visitNodePostProcQR(ASTNodePostProcQR* pElement) override;

        virtual ~ASTQRGivensVisitor() override {}
    };
}

#endif