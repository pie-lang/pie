#ifndef __PIE_AST_STRUCT__
#define __PIE_AST_STRUCT__

#include <string>
#include <vector>
#include <utility>

#include "compiler/ast/node.h"

namespace pie { namespace compiler {

class TypeNode;

// ---------------------------------------------------------------------------
// Struct type definition:  struct Point { x: int, y: int }
// ---------------------------------------------------------------------------
class StructDefNode : public Node
{
public:
    std::string name;
    std::vector<std::pair<std::string, TypeNode *>> fields;  // field_name → TypeNode

    StructDefNode() {}

    DEFINE_VISIT(StructDefNode);
};

// ---------------------------------------------------------------------------
// Struct literal:  Point { x: 1, y: 2 }
// The field_inits vector stores (field_name, value_expr) pairs.
// Value exprs are NOT added to children — they are visited explicitly.
// ---------------------------------------------------------------------------
class StructLiteralNode : public Node
{
public:
    std::string struct_name;
    std::vector<std::pair<std::string, Node *>> field_inits;

    StructLiteralNode() {}

    DEFINE_VISIT(StructLiteralNode);
};

// ---------------------------------------------------------------------------
// Field read access:  p.x
// ---------------------------------------------------------------------------
class FieldAccessNode : public Node
{
public:
    Node *object;
    std::string field;

    FieldAccessNode(Node *obj, const std::string &f) : object(obj), field(f)
    {
        if (obj) push(obj);
    }

    DEFINE_VISIT(FieldAccessNode);
};

}}

#endif
