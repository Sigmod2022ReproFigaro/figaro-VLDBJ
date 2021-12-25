#ifndef _FIGARO_VISITOR_QR_RESULT_RESULT_H_
#define _FIGARO_VISITOR_QR_RESULT_RESULT_H_

#include "ASTVisitorAbsResult.h"
#include "database/Relation.h"

namespace Figaro
{
    class ASTVisitorQRResult: public ASTVisitorAbsResult
    {
        std::string m_rRelationName;
        std::string m_qRelationName;
    public:
        /**
         * @brief Construct a new ASTVisitorJoinResult object
         *
         * @param rRelName  if it is empty, it means this factor has not been computed.
         * @param qRelName if it is empty, it means this factor has not been computed
         */
        ASTVisitorQRResult(
            const std::string& rRelName,
            const std::string& qRelName) :
            ASTVisitorAbsResult(ASTVisitorAbsResult::ResultType::QR_RESULT),
            m_rRelationName(rRelName),
            m_qRelationName(qRelName) { }
        std::string getRRelationName(void) const { return m_rRelationName; }
        std::string getQRelationName(void) const { return m_qRelationName; }
        ~ASTVisitorQRResult() {}
    };

}

#endif