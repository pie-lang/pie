#ifndef __PIE_AST_NODE__
#define __PIE_AST_NODE__

#include <vector>

#define AST_NODES 				\
	AST_NODE(ModuleNode)		\
	AST_NODE(ImportNode)		\
	AST_NODE(FunctionNode)		\
	AST_NODE(ClosureNode)		\
	AST_NODE(FunctionCallNode)	\
	AST_NODE(AssignNode)		\
	AST_NODE(LetNode)			\
	AST_NODE(TypeNode)			\
	AST_NODE(IntNode)			\
	AST_NODE(DoubleNode)		\
	AST_NODE(StringNode)		\
	AST_NODE(IdentifierNode)	\
	AST_NODE(BinaryOpNode)		\
	AST_NODE(UnaryOpNode)		\
	AST_NODE(ReturnNode)		\
	AST_NODE(IfNode)			\
	AST_NODE(BlockNode)

#define NEW_NODE(type, ...) new type ## Node(__VA_ARGS__)

#define NODE_PUSH_CHILD(node)	this->children.push_back(node)

#define DECLARE_VISIT_INTERFACE(node)	virtual void visit(node *node) = 0;
#define DECLARE_VISIT(node)				virtual void visit(node *node);

#define DEFINE_VISIT(node)				void visit(Visitor *visitor) {		\
											visitor->visit(this);			\
										}

namespace pie { namespace compiler {

class Visitor;


// pre-declare nodes
#define AST_NODE(node) class node;
AST_NODES
#undef AST_NODE

class Node {
public:
	Node(): visited(0), attr(0) {}

	void reset()
	{
		// TODO loop over all children to reset walk status
	}

	void push(Node *node)
	{
		children.push_back(node);
	}

	virtual void visit(Visitor *visitor) = 0;

public:
	bool visited;
	int attr;

	std::vector<Node *> children;
};

class Visitor {
public:
	virtual void visit(Node *node) = 0;

	// declare visit handlers
	#define AST_NODE DECLARE_VISIT_INTERFACE
	AST_NODES
	#undef AST_NODE
};

}}

#endif
