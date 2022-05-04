#ifndef _FIGARO_VISITOR_QR_RESULT_H_
#define _FIGARO_VISITOR_QR_RESULT_H_

#include "ASTVisitorResultAbs.h"
#include "database/Relation.h"

namespace Figaro
{
    class ASTVisitorResultQR: public ASTVisitorResultAbs
    {
        std::string m_rRelationName;
        std::string m_qRelationName;
    public:
        /**
         * @brief Construct a new ASTVisitorResultJoin object
         *
         * @param rRelName  if it is empty, it means this factor has not been computed.
         * @param qRelName if it is empty, it means this factor has not been computed
         */
        ASTVisitorResultQR(
            const std::string& rRelName,
            const std::string& qRelName) :
            ASTVisitorResultAbs(ASTVisitorResultAbs::ResultType::QR_RESULT),
            m_rRelationName(rRelName),
            m_qRelationName(qRelName) { }
        std::string getRRelationName(void) const { return m_rRelationName; }
        std::string getQRelationName(void) const { return m_qRelationName; }
        ~ASTVisitorResultQR() {}
    };

}

#endif