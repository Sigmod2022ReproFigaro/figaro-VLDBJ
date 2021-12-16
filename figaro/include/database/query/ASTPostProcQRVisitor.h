#ifndef _FIGARO_AST_POST_PROC_QR_VISITOR_H_
#define _FIGARO_AST_POST_PROC_QR_VISITOR_H_

#include "ASTVisitor.h"

namespace Figaro
{
    class ASTPostProcQRVisitor: public ASTVisitor
    {
        MatrixEigenT* m_pResult = nullptr;
        Figaro::QRGivensHintType m_qrHintType;
        Figaro::MemoryLayout m_memoryLayout;
        void initializeEnumAndDenomRelations(ASTNodeRelation* pRel);
    public:
        ASTPostProcQRVisitor(
            Database* pDatabase, Figaro::QRGivensHintType qrHintType,
            Figaro::MemoryLayout memoryLayout,
            MatrixEigenT* pResult, bool saveResult): ASTVisitor(pDatabase), m_pResult(pResult),
                 m_qrHintType(qrHintType), m_memoryLayout(memoryLayout) {
                     if (!saveResult)
                     {
                         m_pResult = nullptr;
                     }
                 }
        void visitNodeRelation(ASTNodeRelation* pElement) override
        {}
        void visitNodeJoin(ASTNodeJoin* pElement) override
        {}
        void visitNodeQRGivens(ASTNodeQRGivens* pElement) override
        {}
        void visitNodePostProcQR(ASTNodePostProcQR* pElement) override;

        virtual ~ASTPostProcQRVisitor() override {}


    };
}

#endif