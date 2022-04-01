#ifndef _FIGARO_AST_FIGARO_SECOND_PASS_VISITOR_H_
#define _FIGARO_AST_FIGARO_SECOND_PASS_VISITOR_H_

#include "ASTVisitorQRGivensAbs.h"
#include "ASTVisitorFirstPassResult.h"
#include "ASTVisitorSecondPassResult.h"
#include "ASTVisitorQRResult.h"

namespace Figaro
{
    class ASTFigaroSecondPassVisitor: public ASTVisitorQRGivensAbs
    {
        std::string strCountsHeadGeneralized(ASTNodeRelation* pRel);
        bool m_saveResult;
        Figaro::QRHintType m_qrHintType;
        std::string m_joinRelName;
        std::unordered_map<std::string, ASTVisitorFirstPassResult::FirstPassRelNames> m_htTmpRelsNames;
        std::vector<std::string> m_vSubTreeRelNames;

        std::vector<std::string> getChildrenHeadNames(const std::vector<std::string>& vChildrenNames) const;

    public:
        ASTFigaroSecondPassVisitor(
            Database* pDatabase, Figaro::QRHintType qrHintType, bool saveResult,
            const std::string& joinRelName,
            const std::unordered_map<std::string, ASTVisitorFirstPassResult::FirstPassRelNames>
            htTmpRelsNames):
                ASTVisitorQRGivensAbs(pDatabase),
                m_saveResult(saveResult),
                 m_qrHintType(qrHintType),
                 m_joinRelName(joinRelName),
                 m_htTmpRelsNames(htTmpRelsNames) {
                 }
        ASTVisitorSecondPassResult* visitNodeRelation(ASTNodeRelation* pElement) override;
        ASTVisitorSecondPassResult* visitNodeJoin(ASTNodeJoin* pElement) override;
        ASTVisitorQRResult* visitNodeQRGivens(ASTNodeQRGivens* pElement) override;
        virtual ~ASTFigaroSecondPassVisitor() override {}


    };
}

#endif