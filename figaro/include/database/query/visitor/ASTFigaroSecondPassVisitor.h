#ifndef _FIGARO_AST_FIGARO_SECOND_PASS_VISITOR_H_
#define _FIGARO_AST_FIGARO_SECOND_PASS_VISITOR_H_

#include "ASTVisitorQRFigaroAbs.h"
#include "./result/ASTVisitorResultFirstPass.h"
#include "ASTVisitorSecondPassResult.h"
#include "./result/ASTVisitorResultQR.h"

namespace Figaro
{
    class ASTFigaroSecondPassVisitor: public ASTVisitorQFigaroAbs
    {
        std::string strCountsHeadGeneralized(ASTNodeRelation* pRel);
        bool m_saveResult;
        Figaro::QRHintType m_qrHintType;
        std::string m_joinRelName;
        std::unordered_map<std::string, ASTVisitorResultFirstPass::FirstPassRelNames> m_htTmpRelsNames;
        std::vector<std::string> m_vSubTreeRelNames;

        std::vector<std::string> getChildrenHeadNames(const std::vector<std::string>& vChildrenNames) const;

    public:
        ASTFigaroSecondPassVisitor(
            Database* pDatabase, Figaro::QRHintType qrHintType, bool saveResult,
            const std::string& joinRelName,
            const std::unordered_map<std::string, ASTVisitorResultFirstPass::FirstPassRelNames>
            htTmpRelsNames):
                ASTVisitorQFigaroAbs(pDatabase),
                m_saveResult(saveResult),
                 m_qrHintType(qrHintType),
                 m_joinRelName(joinRelName),
                 m_htTmpRelsNames(htTmpRelsNames) {
                 }
        ASTVisitorSecondPassResult* visitNodeRelation(ASTNodeRelation* pElement) override;
        ASTVisitorSecondPassResult* visitNodeJoin(ASTNodeJoin* pElement) override;
        ASTVisitorResultQR* visitNodeQRFigaro(ASTNodeQRFigaro* pElement) override;
        virtual ~ASTFigaroSecondPassVisitor() override {}


    };
}

#endif