#ifndef _FIGARO_AST_QR_VISITOR_H_
#define _FIGARO_AST_QR_VISITOR_H_

#include "ASTVisitor.h"

namespace Figaro
{
    class ASTQRVisitor: public ASTVisitor
    {
    protected:
        std::vector<ASTNodeRelation*> m_vpASTNodeRelation;  
        const std::map<std::string, ASTNodeRelation*>& m_mRelNameASTNodeRel;

        static std::string getFormateJoinAttributeNames(
            const std::vector<std::string>& vJoinAttributeNames);
    public:
        ASTQRVisitor(Database* pDatabase, 
            const std::map<std::string, ASTNodeRelation*>& mRelNameASTNodeRel): 
            ASTVisitor(pDatabase), m_mRelNameASTNodeRel(mRelNameASTNodeRel)
         {}
        virtual ~ASTQRVisitor() override {}


    };
}

#endif 