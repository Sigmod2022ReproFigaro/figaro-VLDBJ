#ifndef _FIGARO_VISITOR_RESULT_SECOND_PASS_H_
#define _FIGARO_VISITOR_RESULT_SECOND_PASS_H_

#include "ASTVisitorResultAbs.h"
#include "database/Relation.h"

namespace Figaro
{
    class ASTVisitorResultSecondPass: public ASTVisitorResultAbs
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
        ASTVisitorResultSecondPass(
            const std::string& genHeadsName,
            const std::unordered_map<std::string, SecondPassRelNames>& htNamesTmpRels,
            const std::vector<std::string>& vSubTreeRelNames) :
            ASTVisitorResultAbs(ASTVisitorResultAbs::ResultType::SECOND_PASS_RESULT),
            m_genHeadsName(genHeadsName),
            m_htNamesTmpRels(htNamesTmpRels),
            m_vSubTreeRelNames(vSubTreeRelNames) { }

        const std::string& getGenHeadsName() const { return m_genHeadsName; }

        const std::vector<std::string>& getSubTreeRelNames() const { return m_vSubTreeRelNames; }

        const std::unordered_map<std::string, SecondPassRelNames>&
        getHtNamesTmpRels(void) const { return m_htNamesTmpRels; }
        ~ASTVisitorResultSecondPass() {}
    };

}

#endif