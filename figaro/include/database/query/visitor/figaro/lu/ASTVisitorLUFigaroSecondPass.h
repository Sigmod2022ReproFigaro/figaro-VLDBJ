#ifndef _FIGARO_AST_VISITOR_LU_FIGARO_SECOND_PASS_H_
#define _FIGARO_AST_VISITOR_LU_FIGARO_SECOND_PASS_H_

#include "ASTVisitorLUFigaroAbs.h"
#include "../../result/ASTVisitorResultSecondPass.h"
#include "../../result/ASTVisitorResultFirstPass.h"
#include "../../result/ASTVisitorResultQR.h"

namespace Figaro
{
    class ASTVisitorLUFigaroSecondPass: public ASTVisitorLUFigaroAbs
    {
        std::string strCountsHeadGeneralized(ASTNodeRelation* pRel);
        bool m_saveResult;
        Figaro::LUHintType m_luHintType;
        std::string m_joinRelName;
        std::unordered_map<std::string, ASTVisitorResultFirstPass::FirstPassRelNames> m_htTmpRelsNames;
        std::vector<std::string> m_vSubTreeRelNames;

        std::vector<std::string> getChildrenHeadNames(const std::vector<std::string>& vChildrenNames) const;
        bool m_evalPostProcessing;
    public:
        ASTVisitorLUFigaroSecondPass(
            Database* pDatabase, Figaro::LUHintType luHintType, bool saveResult,
            const std::string& joinRelName,
            const std::unordered_map<std::string, ASTVisitorResultFirstPass::FirstPassRelNames>
            htTmpRelsNames,
            bool evalPostProcessing):
                ASTVisitorLUFigaroAbs(pDatabase),
                m_saveResult(saveResult),
                 m_luHintType(luHintType),
                 m_joinRelName(joinRelName),
                 m_htTmpRelsNames(htTmpRelsNames),
                 m_evalPostProcessing(evalPostProcessing) {
                 }
        ASTVisitorResultSecondPass* visitNodeRelation(ASTNodeRelation* pElement) override;
        ASTVisitorResultSecondPass* visitNodeJoin(ASTNodeJoin* pElement) override;
        ASTVisitorResultQR* visitNodeLUFigaro(ASTNodeLUFigaro* pElement) override;
        virtual ~ASTVisitorLUFigaroSecondPass() override {}


    };
}

#endif