#ifndef _FIGARO_VISITOR_SECOND_PASS_RESULT_H_
#define _FIGARO_VISITOR_SECOND_PASS_RESULT_H_

#include "ASTVisitorAbsResult.h"
#include "database/Relation.h"

namespace Figaro
{
    class ASTVisitorSecondPassResult: public ASTVisitorAbsResult
    {
    public:
        struct SecondPassRelNames
        {
            std::string m_genTailsName;
            SecondPassRelNames(
                const std::string& genTailsName):
                m_genTailsName(genTailsName)
            {}
        };
    private:
        std::unordered_map<std::string, SecondPassRelNames> m_htNamesTmpRels;
        std::string m_genHeadsName;
        std::vector<std::string> m_vSubTreeRelNames;
    public:
        ASTVisitorSecondPassResult(
            const std::string& genHeadsName,
            const std::unordered_map<std::string, SecondPassRelNames>& htNamesTmpRels,
            const std::vector<std::string>& vSubTreeRelNames) :
            ASTVisitorAbsResult(ASTVisitorAbsResult::ResultType::SECOND_PASS_RESULT),
            m_genHeadsName(genHeadsName),
            m_htNamesTmpRels(htNamesTmpRels),
            m_vSubTreeRelNames(vSubTreeRelNames) { }

        const std::string& getGenHeadsName() const { return m_genHeadsName; }

        const std::vector<std::string>& getSubTreeRelNames() const { return m_vSubTreeRelNames; }

        const std::unordered_map<std::string, SecondPassRelNames>&
        getHtNamesTmpRels(void) const { return m_htNamesTmpRels; }
        ~ASTVisitorSecondPassResult() {}
    };

}

#endif