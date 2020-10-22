#ifndef _FIGARO_AST_VISITOR_H_
#define _FIGARO_AST_VISITOR_H_

#include "ASTNode.h"
#include "ASTNodeJoin.h"
#include "ASTNodeQRGivens.h"
#include "ASTNodeRelation.h"

namespace Figaro
{
    class ASTVisitor
    {
    public:
        ASTVisitor(/* args */) {}
        void visitNodeRelation(ASTNodeRelation *pElement);
        void visitNodeJoin(ASTNodeJoin *pElement);
        void visitNodeQRGivens(ASTNodeQRGivens *pElement);
    };

    
}

#endif 