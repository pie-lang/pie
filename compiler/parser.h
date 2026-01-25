#ifndef __PIE_PARSER__
#define __PIE_PARSER__

#include <string>
#include <stack>
#include <vector>

#include "compiler/ast.h"

namespace pie { namespace compiler {

class Scanner;

struct Location {
	int line;
	int column;

	Location() : line(0), column(0) {}
};

class Parser {
public:
	Parser(Scanner &s);

	int scan(void *token);  // Takes YYSTYPE* from generated parser
	int parse();
	void parseFatal(std::string msg);

	// AST building helpers
	Node *makeInt(int64_t value);
	Node *makeDouble(double value);
	Node *makeString(const std::string &str);
	Node *makeIdentifier(const std::string &name);
	Node *makeBinaryOp(BinaryOp op, Node *lhs, Node *rhs);
	Node *makeUnaryOp(UnaryOp op, Node *expr);
	Node *makeFunctionCall(const std::string &name, std::vector<Node*> &args);
	Node *makeLet(const std::string &name, TypeNode *type, Node *value);
	Node *makeAssign(Node *var, Node *value);
	Node *makeReturn(Node *expr);
	Node *makeIf(Node *cond, BlockNode *then_block, Node *else_block);
	BlockNode *makeBlock();
	TypeNode *makeType(const std::string &name, bool isArray);

public:
	Scanner &scanner;

	ModuleNode *module;             // current parsed module
	FunctionNode *function;         // current parsed function
	std::stack<BlockNode*> blocks;  // block stack for nested blocks
};

}}

#endif
