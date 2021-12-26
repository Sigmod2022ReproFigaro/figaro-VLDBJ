#ifndef _FIGARO_VISITOR_FIRST_PASS_RESULT_H_
#define _FIGARO_VISITOR_FIRST_PASS_RESULT_H_

#include "ASTVisitorAbsResult.h"
#include "database/Relation.h"

namespace Figaro
{
    class ASTVisitorFirstPassResult: public ASTVisitorAbsResult
    {
    public:
        struct FirstPassRelNames
        {
            std::string m_headsName;
            std::string m_tailsName;
            FirstPassRelNames(
                const std::string& headsName,
                const std::string& tailsName):
                m_headsName(headsName),
                m_tailsName(tailsName)
            {}
        };
    private:
        std::unordered_map<std::string, FirstPassRelNames> m_htNamesTmpRels;
        /**
         * @brief Construct a new ASTVisitorJoinResult object
         *
         * @param rRelName  if it is empty, it means this factor has not been computed.
         * @param qRelName if it is empty, it means this factor has not been computed
         */
    public:
        ASTVisitorFirstPassResult(
            const std::unordered_map<std::string, FirstPassRelNames>& htNamesTmpRels) :
            ASTVisitorAbsResult(ASTVisitorAbsResult::ResultType::FIRST_PASS_RESULT),
            m_htNamesTmpRels(htNamesTmpRels) { }
        const std::unordered_map<std::string, FirstPassRelNames>&
        getHtNamesTmpRels(void) const { return m_htNamesTmpRels; }
        ~ASTVisitorFirstPassResult() {}
    };

}

#endif