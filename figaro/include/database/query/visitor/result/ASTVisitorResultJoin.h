#ifndef _FIGARO_VISITOR_RESULT_JOIN_H_
#define _FIGARO_VISITOR_RESULT_JOIN_H_

#include "ASTVisitorResultAbs.h"
#include "database/Relation.h"

namespace Figaro
{
    class ASTVisitorResultJoin: public ASTVisitorResultAbs
    {
        std::string m_relationName;
    public:
        ASTVisitorResultJoin(const std::string& relationName) :
            ASTVisitorResultAbs(ASTVisitorResultAbs::ResultType::JOIN_RESULT),
            m_relationName(relationName) { }
        std::string getJoinRelName(void) const { return m_relationName; }
        ~ASTVisitorResultJoin() {}
    };

}

#endif