#ifndef _FIGARO_VISITOR_SVD_RESULT_H_
#define _FIGARO_VISITOR_SVD_RESULT_H_

#include "ASTVisitorResultAbs.h"
#include "database/Relation.h"

namespace Figaro
{
    class ASTVisitorResultSVD: public ASTVisitorResultAbs
    {
        std::string m_uRelationName;
        std::string m_sRelationName;
        std::string m_vRelationName;
    public:
        /**
         * @brief Construct a new ASTVisitorResultSVD object
         *
         */
        ASTVisitorResultSVD(
            const std::string& uRelName,
            const std::string& sRelName,
            const std::string& vRelName) :
            ASTVisitorResultAbs(ASTVisitorResultAbs::ResultType::SVD_RESULT),
            m_uRelationName(uRelName),
            m_sRelationName(sRelName),
            m_vRelationName(vRelName) { }
        std::string getURelationName(void) const { return m_uRelationName; }
        std::string getSRelationName(void) const { return m_sRelationName; }
        std::string getVRelationName(void) const { return m_vRelationName; }
        ~ASTVisitorResultSVD() {}
    };

}

#endif
