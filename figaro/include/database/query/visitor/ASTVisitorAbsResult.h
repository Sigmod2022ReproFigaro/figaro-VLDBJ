#ifndef _FIGARO_VISITOR_ABS_RESULT_H_
#define _FIGARO_VISITOR_ABS_RESULT_H_

namespace Figaro
{
    class ASTVisitorAbsResult
    {
        public:
        enum class ResultType
        {
            JOIN_RESULT = 0,
            QR_RESULT = 1,
            FIRST_PASS_RESULT = 2,
            SECOND_PASS_RESULT = 2,
        };
    private:
        ResultType m_resultType;
    public:
        ASTVisitorAbsResult(ResultType resultType): m_resultType(resultType) {}
        ResultType getResultType() const {
            return  m_resultType;
        };
    };
}

#endif