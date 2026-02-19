#ifndef __PIE_TYPECHECK__
#define __PIE_TYPECHECK__

#include <string>
#include <vector>
#include <map>

#include "compiler/ast.h"
#include "compiler/types.h"

namespace pie { namespace compiler {

// ---------------------------------------------------------------------------
// Type environment (a scope chain mapping names to their resolved types)
// ---------------------------------------------------------------------------
class TypeEnv {
public:
    explicit TypeEnv(TypeEnv *parent = nullptr) : parent_(parent) {}

    void define(const std::string &name, PieTypePtr type)
    {
        vars_[name] = type;
    }

    // Returns nullptr if the name is not found anywhere in the chain.
    PieTypePtr get(const std::string &name) const
    {
        auto it = vars_.find(name);
        if (it != vars_.end()) return it->second;
        if (parent_) return parent_->get(name);
        return nullptr;
    }

    // Update an existing binding (searches the chain).
    void set(const std::string &name, PieTypePtr type)
    {
        auto it = vars_.find(name);
        if (it != vars_.end()) { it->second = type; return; }
        if (parent_) { parent_->set(name, type); }
    }

    TypeEnv *parent() const { return parent_; }

private:
    std::map<std::string, PieTypePtr> vars_;
    TypeEnv *parent_;
};

// ---------------------------------------------------------------------------
// A single type error
// ---------------------------------------------------------------------------
struct TypeError {
    std::string message;
    explicit TypeError(const std::string &msg) : message(msg) {}
};

// ---------------------------------------------------------------------------
// TypeChecker — walks the AST before evaluation and enforces static types
// ---------------------------------------------------------------------------
class TypeChecker : public Visitor {
public:
    TypeChecker();

    // Entry point. Returns true iff there are no type errors.
    bool check(ModuleNode *module);

    const std::vector<TypeError> &errors() const { return errors_; }

    // Visitor interface
    void visit(Node *node) override;

    #define AST_NODE DECLARE_VISIT
    AST_NODES
    #undef AST_NODE

private:
    // The type inferred for the most recently visited expression.
    PieTypePtr result_type_;

    // Current type environment (points into the scope chain).
    TypeEnv *type_env_;
    TypeEnv  global_type_env_;

    // Expected return type of the function currently being checked.
    // nullptr means we are at module level (not inside a function).
    PieTypePtr current_return_type_;

    std::vector<TypeError> errors_;

    // --- helpers ---

    // Visit a node and return the inferred type.
    PieTypePtr checkNode(Node *node);

    // Convert a TypeNode annotation to a resolved PieType.
    PieTypePtr resolveTypeNode(TypeNode *tn);

    // Record a type error.
    void error(const std::string &msg);

    // True if 'actual' is assignable to 'expected'.
    // int is assignable to double (numeric widening) but not vice-versa.
    bool assignable(const PieType &actual, const PieType &expected) const;

    // Register built-in function return types so callers can be type-checked.
    void registerBuiltins();
};

}}

#endif
