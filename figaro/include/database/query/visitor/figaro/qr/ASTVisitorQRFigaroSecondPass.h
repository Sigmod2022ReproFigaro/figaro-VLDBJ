#ifndef _FIGARO_AST_VISITOR_QR_FIGARO_SECOND_PASS_H_
#define _FIGARO_AST_VISITOR_QR_FIGARO_SECOND_PASS_H_

#include "ASTVisitorQRFigaroAbs.h"
#include "../../result/ASTVisitorResultSecondPass.h"
#include "../../result/ASTVisitorResultFirstPass.h"
#include "../../result/ASTVisitorResultQR.h"

namespace Figaro
{
    class ASTVisitorQRFigaroSecondPass: public ASTVisitorQRFigaroAbs
    {
        std::string strCountsHeadGeneralized(ASTNodeRelation* pRel);
        bool m_saveResult;
        Figaro::QRHintType m_qrHintType;
        std::string m_joinRelName;
        std::unordered_map<std::string, ASTVisitorResultFirstPass::FirstPassRelNames> m_htTmpRelsNames;
        std::vector<std::string> m_vSubTreeRelNames;

        std::vector<std::string> getChildrenHeadNames(const std::vector<std::string>& vChildrenNames) const;
        bool m_evalPostProcessing;

    public:
        ASTVisitorQRFigaroSecondPass(
            Database* pDatabase, Figaro::QRHintType qrHintType, bool saveResult,
            const std::string& joinRelName,
            const std::unordered_map<std::string, ASTVisitorResultFirstPass::FirstPassRelNames>
            htTmpRelsNames,
            bool evalPostProcessing):
                ASTVisitorQRFigaroAbs(pDatabase),
                m_saveResult(saveResult),
                 m_qrHintType(qrHintType),
                 m_joinRelName(joinRelName),
                 m_htTmpRelsNames(htTmpRelsNames),
                 m_evalPostProcessing(evalPostProcessing) {
                 }
        ASTVisitorResultSecondPass* visitNodeRelation(ASTNodeRelation* pElement) override;
        ASTVisitorResultSecondPass* visitNodeJoin(ASTNodeJoin* pElement) override;
        ASTVisitorResultQR* visitNodeQRFigaro(ASTNodeQRFigaro* pElement) override;
        virtual ~ASTVisitorQRFigaroSecondPass() override {}


    };
}

#endif