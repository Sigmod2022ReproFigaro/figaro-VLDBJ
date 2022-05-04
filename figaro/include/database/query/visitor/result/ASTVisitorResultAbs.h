#ifndef _FIGARO_VISITOR_RESULT_ABS_H_
#define _FIGARO_VISITOR_RESULT_ABS_H_

namespace Figaro
{
    class ASTVisitorResultAbs
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
        ASTVisitorResultAbs(ResultType resultType): m_resultType(resultType) {}
        ResultType getResultType() const {
            return  m_resultType;
        };
    };
}

#endif