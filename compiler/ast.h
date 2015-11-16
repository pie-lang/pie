#ifndef __PIE_AST__
#define __PIE_AST__

#include <vector>

#define NEW_NODE(type, ...) new type ## Node(__VA_ARGS__)

#define DECLARE_NODE(node)	virtual void visit(node *node) = 0;
#define DEFINE_NODE(node)	void visit(Visitor *visitor) {		\
								visitor->visit(this);			\
							}

#define AST_NODES 				\
	AST_NODE(ModuleNode)		\
	AST_NODE(ImportNode)		\
	AST_NODE(FunctionNode)		\
	AST_NODE(ClosureNode)		\
	AST_NODE(FunctionCallNode)	\
	AST_NODE(TypeNode)			\
	AST_NODE(IntNode)			\
	AST_NODE(BinarNode)			\
	AST_NODE(UnaryNode)			\


#define AST_PUSH_CHILD(node)	this->children.push_back(node)

namespace pie { namespace compiler {

// pre-declare all nodes here
#define AST_NODE(node) class node;
AST_NODES
#undef AST_NODE

class Visitor;

class Node {
public:
	Node(): visited(0), extended_value(0) {}

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
	int extended_value;

	std::vector<Node *> children;
};

class Visitor {
public:
	virtual void visit(Node *node) = 0;

	// declare visit handlers
	#define AST_NODE DECLARE_NODE
	AST_NODES
	#undef AST_NODE
};

}}

#endif
