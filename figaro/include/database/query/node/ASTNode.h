#ifndef _FIGARO_AST_NODE_H_
#define _FIGARO_AST_NODE_H_

#include "utils/Utils.h"
#include "database/query/visitor/result/ASTVisitorResultAbs.h"

namespace Figaro
{
    class ASTVisitor;
    class ASTNode
    {
        friend class ASTVisitor;
    public:
        virtual ASTVisitorResultAbs* accept(ASTVisitor* pVisitor) = 0;
        virtual ~ASTNode() = 0;
        virtual ASTNode* copy() = 0;
    };

    inline ASTNode::~ASTNode() {}
}

#endif