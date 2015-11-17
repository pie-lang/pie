#ifndef __PIE_AST_VISITOR__
#define __PIE_AST_VISITOR__

#include "compiler/ast/node.h"

#define DECLARE_VISITOR_INTERFACE(node)	virtual void visit(node *node) = 0;
#define DECLARE_VISITOR(node)			virtual void visit(node *node);

#define DEFINE_NODE(node)				void visit(Visitor *visitor) {		\
											visitor->visit(this);			\
										}
namespace pie { namespace compiler {



}}


#endif
