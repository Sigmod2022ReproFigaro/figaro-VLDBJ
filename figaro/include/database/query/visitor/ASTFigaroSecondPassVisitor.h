#ifndef _FIGARO_AST_FIGARO_SECOND_PASS_VISITOR_H_
#define _FIGARO_AST_FIGARO_SECOND_PASS_VISITOR_H_

#include "ASTVisitorQRGivensAbs.h"
#include "ASTVisitorQRResult.h"

namespace Figaro
{
    class ASTFigaroSecondPassVisitor: public ASTVisitorQRGivensAbs
    {
        std::string strCountsHeadGeneralized(ASTNodeRelation* pRel);
        bool m_saveResult;
        Figaro::QRGivensHintType m_qrHintType;
        std::string m_joinRelName;
    public:
        ASTFigaroSecondPassVisitor(
            Database* pDatabase, Figaro::QRGivensHintType qrHintType, bool saveResult,
            const std::string& joinRelName):
                ASTVisitorQRGivensAbs(pDatabase),
                m_saveResult(saveResult),
                 m_qrHintType(qrHintType),
                 m_joinRelName(joinRelName) {
                 }
        ASTVisitorAbsResult* visitNodeRelation(ASTNodeRelation* pElement) override;
        ASTVisitorAbsResult* visitNodeJoin(ASTNodeJoin* pElement) override;
        ASTVisitorQRResult* visitNodeQRGivens(ASTNodeQRGivens* pElement) override;
        virtual ~ASTFigaroSecondPassVisitor() override {}


    };
}

#endif