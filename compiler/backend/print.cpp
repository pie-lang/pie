#include "compiler/backend/print.h"

namespace pie { namespace compiler {

void PrintVisitor::indent()
{
    for (int i = 0; i < indent_level; i++) {
        out << "    ";
    }
}

void PrintVisitor::newline()
{
    out << "\n";
}

std::string PrintVisitor::binaryOpToString(BinaryOp op)
{
    switch (op) {
        case BinaryOp::Add: return "+";
        case BinaryOp::Sub: return "-";
        case BinaryOp::Mul: return "*";
        case BinaryOp::Div: return "/";
        case BinaryOp::Mod: return "%";
        case BinaryOp::Lt: return "<";
        case BinaryOp::Gt: return ">";
        case BinaryOp::Le: return "<=";
        case BinaryOp::Ge: return ">=";
        case BinaryOp::Eq: return "==";
        case BinaryOp::Ne: return "!=";
        case BinaryOp::And: return "&&";
        case BinaryOp::Or: return "||";
        case BinaryOp::Assign: return "=";
        case BinaryOp::AddAssign: return "+=";
        case BinaryOp::SubAssign: return "-=";
        case BinaryOp::Dot: return ".";
        default: return "?";
    }
}

std::string PrintVisitor::unaryOpToString(UnaryOp op)
{
    switch (op) {
        case UnaryOp::Neg: return "-";
        case UnaryOp::Not: return "!";
        case UnaryOp::Inc: return "++";
        case UnaryOp::Dec: return "--";
        default: return "?";
    }
}

void PrintVisitor::visit(Node *node)
{
    if (node) {
        node->visit(this);
    }
}

void PrintVisitor::visit(ModuleNode *node)
{
    out << "module " << node->name;
    newline();
    newline();

    // Print imports
    for (ImportNode *imp : node->imports) {
        imp->visit(this);
    }

    if (!node->imports.empty()) {
        newline();
    }

    // Print functions
    for (FunctionNode *fn : node->functions) {
        fn->visit(this);
        newline();
    }
}

void PrintVisitor::visit(ImportNode *node)
{
    if (node->access_level == 1) {
        out << "public ";
    }
    out << "import " << node->module_name;
    if (node->import_all) {
        out << ".*";
    }
    newline();
}

void PrintVisitor::visit(FunctionNode *node)
{
    if (node->access_level == 1) {
        out << "public ";
    }
    out << "fn " << node->name << "(";

    // Print parameters
    bool first = true;
    for (const auto &param : node->params) {
        if (!first) out << ", ";
        out << param.first;
        if (param.second) {
            out << ": ";
            param.second->visit(this);
        }
        first = false;
    }
    out << ")";

    // Print return type
    if (node->return_type) {
        out << " : ";
        node->return_type->visit(this);
    }

    out << " {";
    newline();

    // Print body statements
    indent_level++;
    for (Node *child : node->children) {
        indent();
        child->visit(this);
        newline();
    }
    indent_level--;

    out << "}";
    newline();
}

void PrintVisitor::visit(ClosureNode *node)
{
    out << "fn (";

    bool first = true;
    for (const auto &param : node->params) {
        if (!first) out << ", ";
        out << param.first;
        if (param.second) {
            out << ": ";
            param.second->visit(this);
        }
        first = false;
    }
    out << ")";

    if (node->return_type) {
        out << " : ";
        node->return_type->visit(this);
    }

    out << " { ... }";
}

void PrintVisitor::visit(FunctionCallNode *node)
{
    out << node->name << "(";

    bool first = true;
    for (Node *child : node->children) {
        if (!first) out << ", ";
        child->visit(this);
        first = false;
    }

    out << ")";
}

void PrintVisitor::visit(AssignNode *node)
{
    node->var->visit(this);
    out << " = ";
    node->value->visit(this);
}

void PrintVisitor::visit(LetNode *node)
{
    out << "let " << node->name;
    if (node->type) {
        out << ": ";
        node->type->visit(this);
    }
    if (node->value) {
        out << " = ";
        node->value->visit(this);
    }
}

void PrintVisitor::visit(TypeNode *node)
{
    out << node->name;
    if (node->is_array) {
        out << "[]";
    }
}

void PrintVisitor::visit(IntNode *node)
{
    out << node->value;
}

void PrintVisitor::visit(DoubleNode *node)
{
    out << node->value;
}

void PrintVisitor::visit(StringNode *node)
{
    out << "\"" << node->str << "\"";
}

void PrintVisitor::visit(IdentifierNode *node)
{
    out << node->name;
}

void PrintVisitor::visit(BinaryOpNode *node)
{
    out << "(";
    node->lhs->visit(this);
    out << " " << binaryOpToString(node->op) << " ";
    node->rhs->visit(this);
    out << ")";
}

void PrintVisitor::visit(UnaryOpNode *node)
{
    out << "(" << unaryOpToString(node->op);
    node->expr->visit(this);
    out << ")";
}

void PrintVisitor::visit(ReturnNode *node)
{
    out << "return";
    if (node->expr) {
        out << " ";
        node->expr->visit(this);
    }
}

void PrintVisitor::visit(IfNode *node)
{
    out << "if (";
    if (node->condition) {
        node->condition->visit(this);
    }
    out << ") ";

    if (node->then_block) {
        node->then_block->visit(this);
    }

    if (node->else_block) {
        out << " else ";
        node->else_block->visit(this);
    }
}

void PrintVisitor::visit(BlockNode *node)
{
    out << "{";
    newline();

    indent_level++;
    for (Node *child : node->children) {
        indent();
        child->visit(this);
        newline();
    }
    indent_level--;

    indent();
    out << "}";
}

}}
