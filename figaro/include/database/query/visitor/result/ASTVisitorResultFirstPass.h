#ifndef _FIGARO_VISITOR_RESULT_FIRST_PASS_H_
#define _FIGARO_VISITOR_RESULT_FIRST_PASS_H_

#include "ASTVisitorResultAbs.h"
#include "database/Relation.h"

namespace Figaro
{
    class ASTVisitorResultFirstPass: public ASTVisitorResultAbs
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
         * @brief Construct a new ASTVisitorResultJoin object
         *
         * @param rRelName  if it is empty, it means this factor has not been computed.
         * @param qRelName if it is empty, it means this factor has not been computed
         */
    public:
        ASTVisitorResultFirstPass(
            const std::unordered_map<std::string, FirstPassRelNames>& htNamesTmpRels) :
            ASTVisitorResultAbs(ASTVisitorResultAbs::ResultType::FIRST_PASS_RESULT),
            m_htNamesTmpRels(htNamesTmpRels) { }
        const std::unordered_map<std::string, FirstPassRelNames>&
        getHtNamesTmpRels(void) const { return m_htNamesTmpRels; }
        ~ASTVisitorResultFirstPass() {}
    };

}

#endif