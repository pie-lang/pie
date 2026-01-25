#ifndef __PIE_BACKEND_PRINT__
#define __PIE_BACKEND_PRINT__

#include <string>
#include <sstream>

#include "compiler/ast.h"

namespace pie { namespace compiler {

class PrintVisitor : public Visitor
{
public:
    PrintVisitor() : indent_level(0) {}

    std::string output() const { return out.str(); }

    void visit(Node *node) override;

    #define AST_NODE DECLARE_VISIT
    AST_NODES
    #undef AST_NODE

private:
    std::stringstream out;
    int indent_level;

    void indent();
    void newline();
    std::string binaryOpToString(BinaryOp op);
    std::string unaryOpToString(UnaryOp op);
};

}}

#endif
