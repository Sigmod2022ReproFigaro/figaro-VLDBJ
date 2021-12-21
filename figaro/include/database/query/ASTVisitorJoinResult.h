#ifndef _FIGARO_VISITOR_JOIN_RESULT_H_
#define _FIGARO_VISITOR_JOIN_RESULT_H_

#include "database/query/ASTVisitorAbsResult.h"
#include "database/Relation.h"

namespace Figaro
{
    class ASTVisitorJoinResult: public ASTVisitorAbsResult
    {
        std::string m_relationName;
    public:
        ASTVisitorJoinResult(const std::string& relationName) :
            m_relationName(relationName) { }
        std::string getJoinRelName(void) const { return m_relationName; }
        ~ASTVisitorJoinResult() {}
    };

}

#endif