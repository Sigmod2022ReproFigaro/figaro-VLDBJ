#ifndef _FIGARO_AST_VISITOR_H_
#define _FIGARO_AST_VISITOR_H_

#include "ASTNode.h"
#include "ASTNodeJoin.h"
#include "ASTNodeQRGivens.h"
#include "ASTNodeRelation.h"
#include "database/Database.h"

namespace Figaro
{
    class ASTVisitor
    {
        Database* m_pDatabase;
    public:
        ASTVisitor(Database* pDatabase): m_pDatabase(pDatabase) {}
        void visitNodeRelation(ASTNodeRelation* pElement);
        void visitNodeJoin(ASTNodeJoin* pElement);
        void visitNodeQRGivens(ASTNodeQRGivens* pElement);
    };
}

#endif 