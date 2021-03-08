#ifndef _FIGARO_AST_FIGARO_EXPRESSION_VISITOR_H_
#define _FIGARO_AST_FIGARO_EXPRESSION_VISITOR_H_

#include "ASTVisitor.h"

namespace Figaro
{
    class ASTFigaroExpressionVisitor: public ASTVisitor
    {
        std::vector<ASTNodeRelation*> m_vpASTNodeRelation;  
        const std::map<std::string, ASTNodeRelation*>& m_mRelNameASTNodeRel;
        static std::vector<std::string> setIntersection(const std::vector<std::string>& vStr1, const std::vector<std::string>& vStr2);
        std::string l2TailnormExpression(ASTNodeRelation* pElement);

    public:
        ASTFigaroExpressionVisitor(Database* pDatabase, const std::map<std::string, ASTNodeRelation*>& mRelNameASTNodeRel): ASTVisitor(pDatabase), m_mRelNameASTNodeRel(mRelNameASTNodeRel) {}
        void visitNodeRelation(ASTNodeRelation* pElement) override;
        void visitNodeJoin(ASTNodeJoin* pElement) override;
        void visitNodeQRGivens(ASTNodeQRGivens* pElement) override;

        virtual ~ASTFigaroExpressionVisitor() override {}


    };
}

#endif 