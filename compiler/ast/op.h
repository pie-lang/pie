#ifndef __PIE_AST_OP__
#define __PIE_AST_OP__

#include "compiler/ast/node.h"

namespace pie { namespace compiler {

// Binary operator types
enum class BinaryOp {
	Add,      // +
	Sub,      // -
	Mul,      // *
	Div,      // /
	Mod,      // %
	Lt,       // <
	Gt,       // >
	Le,       // <=
	Ge,       // >=
	Eq,       // ==
	Ne,       // !=
	And,      // &&
	Or,       // ||
	Assign,   // =
	AddAssign,// +=
	SubAssign,// -=
	Dot       // .
};

// Unary operator types
enum class UnaryOp {
	Neg,      // -
	Not,      // !
	Inc,      // ++
	Dec       // --
};

class BinaryOpNode : public Node
{
public:
	Node *lhs;
	Node *rhs;
	BinaryOp op;

	BinaryOpNode(BinaryOp op, Node *lhs, Node *rhs) : op(op), lhs(lhs), rhs(rhs)
	{
		push(lhs);
		push(rhs);
	}

	DEFINE_VISIT(BinaryOpNode);
};

class UnaryOpNode : public Node
{
public:
	Node *expr;
	UnaryOp op;

	UnaryOpNode(UnaryOp op, Node *expr) : op(op), expr(expr)
	{
		push(expr);
	}

	DEFINE_VISIT(UnaryOpNode);
};

}}

#endif
