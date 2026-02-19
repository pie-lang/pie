#include <stdio.h>
#include <cstdlib>
#include <cstring>

#include "compiler/scanner.h"
#include "compiler/parser.h"
#include "compiler/parser.tab.hpp"

namespace pie { namespace compiler {

Parser::Parser(Scanner &s) : scanner(s), function(nullptr)
{
    module = new ModuleNode();
}

void Parser::parseFatal(std::string msg)
{
    fprintf(stderr, "Parse error at line %d: %s\n", scanner.m_line, msg.c_str());
}

int Parser::scan(void *token_ptr)
{
    YYSTYPE *token = (YYSTYPE *)token_ptr;
    int tok = scanner.scan();

    // Fill in the semantic value based on token type
    if (tok == T_IDENTIFIER) {
        token->str = new std::string(scanner.tokenText(), scanner.tokenLength());
    } else if (tok == T_NUMBER) {
        token->num = atoll(scanner.tokenText());
    } else if (tok == T_DOUBLE) {
        token->dbl = atof(scanner.tokenText());
    } else if (tok == T_STRING) {
        // For strings, use the accumulated string buffer content
        token->str = new std::string(scanner.stringValue());
    } else if (tok == T_ACC_PUBLIC) {
        token->visibility = 1;
    }

    return tok;
}

// AST building helpers
Node *Parser::makeInt(int64_t value)
{
    return new IntNode(value);
}

Node *Parser::makeDouble(double value)
{
    return new DoubleNode(value);
}

Node *Parser::makeString(const std::string &str)
{
    return new StringNode(str);
}

Node *Parser::makeIdentifier(const std::string &name)
{
    return new IdentifierNode(name);
}

Node *Parser::makeBinaryOp(BinaryOp op, Node *lhs, Node *rhs)
{
    return new BinaryOpNode(op, lhs, rhs);
}

Node *Parser::makeUnaryOp(UnaryOp op, Node *expr)
{
    return new UnaryOpNode(op, expr);
}

Node *Parser::makeFunctionCall(const std::string &name, std::vector<Node*> &args)
{
    FunctionCallNode *call = new FunctionCallNode(name);
    for (Node *arg : args) {
        call->push(arg);
    }
    return call;
}

Node *Parser::makeLet(const std::string &name, TypeNode *type, Node *value)
{
    return new LetNode(name, type, value);
}

Node *Parser::makeAssign(Node *var, Node *value)
{
    return new AssignNode(var, value);
}

Node *Parser::makeReturn(Node *expr)
{
    return new ReturnNode(expr);
}

Node *Parser::makeIf(Node *cond, BlockNode *then_block, Node *else_block)
{
    return new IfNode(cond, then_block, else_block);
}

BlockNode *Parser::makeBlock()
{
    return new BlockNode();
}

TypeNode *Parser::makeType(const std::string &name, bool isArray)
{
    return new TypeNode(name, isArray);
}

Node *Parser::makeStructLiteral(const std::string &typeName,
                                std::vector<std::pair<std::string, Node*>> *fields)
{
    StructLiteralNode *lit = new StructLiteralNode();
    lit->struct_name = typeName;
    if (fields) {
        lit->field_inits = *fields;
        delete fields;
    }
    return lit;
}

Node *Parser::makeFieldAccess(Node *object, const std::string &field)
{
    return new FieldAccessNode(object, field);
}

}}
