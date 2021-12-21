#ifndef _FIGARO_AST_FIGARO_SECOND_PASS_VISITOR_H_
#define _FIGARO_AST_FIGARO_SECOND_PASS_VISITOR_H_

#include "ASTVisitorQRGivensAbs.h"

namespace Figaro
{
    class ASTFigaroSecondPassVisitor: public ASTVisitorQRGivensAbs
    {
        std::string strCountsHeadGeneralized(ASTNodeRelation* pRel);
        MatrixEigenT* m_pResult = nullptr;
        Figaro::QRGivensHintType m_qrHintType;
    public:
        ASTFigaroSecondPassVisitor(
            Database* pDatabase, Figaro::QRGivensHintType qrHintType, MatrixEigenT* pResult):
                ASTVisitorQRGivensAbs(pDatabase),
                m_pResult(pResult),
                 m_qrHintType(qrHintType) {
                 }
        ASTVisitorAbsResult* visitNodeRelation(ASTNodeRelation* pElement) override;
        ASTVisitorAbsResult* visitNodeJoin(ASTNodeJoin* pElement) override;
        ASTVisitorAbsResult* visitNodeQRGivens(ASTNodeQRGivens* pElement) override;
        virtual ~ASTFigaroSecondPassVisitor() override {}


    };
}

#endif