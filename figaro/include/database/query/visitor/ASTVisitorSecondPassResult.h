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
        /**
         * @brief Construct a new ASTVisitorJoinResult object
         *
         * @param rRelName  if it is empty, it means this factor has not been computed.
         * @param qRelName if it is empty, it means this factor has not been computed
         */
    public:
        ASTVisitorSecondPassResult(
            const std::string& genHeadsName,
            const std::unordered_map<std::string, SecondPassRelNames>& htNamesTmpRels) :
            ASTVisitorAbsResult(ASTVisitorAbsResult::ResultType::SECOND_PASS_RESULT),
            m_genHeadsName(genHeadsName),
            m_htNamesTmpRels(htNamesTmpRels) { }

        const std::string& getGenHeadsName() const { return m_genHeadsName; }

        const std::unordered_map<std::string, SecondPassRelNames>&
        getHtNamesTmpRels(void) const { return m_htNamesTmpRels; }
        ~ASTVisitorSecondPassResult() {}
    };

}

#endif