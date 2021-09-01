#ifndef _FIGARO_AST_FIGARO_SECOND_PASS_VISITOR_H_
#define _FIGARO_AST_FIGARO_SECOND_PASS_VISITOR_H_

#include "ASTQRVisitor.h"

namespace Figaro
{
    class ASTFigaroSecondPassVisitor: public ASTQRVisitor
    {
        std::string strCountsHeadGeneralized(ASTNodeRelation* pRel);
        bool m_postProcess = false;
        MatrixEigenT* m_pResult = nullptr;
        MatrixD::QRGivensHintType m_qrHintType;
    public:
        ASTFigaroSecondPassVisitor(
            Database* pDatabase,
            bool postProcess, MatrixD::QRGivensHintType qrHintType, MatrixEigenT* pResult,
                bool saveResult):
                ASTQRVisitor(pDatabase),
                m_postProcess(postProcess), m_pResult(pResult),
                 m_qrHintType(qrHintType) {
                     if (!saveResult)
                     {
                         m_pResult = nullptr;
                     }
                 }
        void visitNodeRelation(ASTNodeRelation* pElement) override;
        void visitNodeJoin(ASTNodeJoin* pElement) override;
        void visitNodeQRGivens(ASTNodeQRGivens* pElement) override;
        virtual ~ASTFigaroSecondPassVisitor() override {}


    };
}

#endif