#include "compiler/typecheck.h"

#include <sstream>
#include <cassert>

namespace pie { namespace compiler {

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

TypeChecker::TypeChecker()
    : result_type_(PieType::makeUnknown()),
      type_env_(&global_type_env_),
      current_return_type_(nullptr)
{
    registerBuiltins();
}

void TypeChecker::registerBuiltins()
{
    // Built-in functions are variadic or have flexible signatures.
    // We record only their return types; argument checking is skipped for them.
    global_type_env_.define("print",    PieType::makeVoid());
    global_type_env_.define("io.print", PieType::makeVoid());
    global_type_env_.define("exit",     PieType::makeVoid());
    global_type_env_.define("len",      PieType::makeInt());
    global_type_env_.define("type",     PieType::makeString());
}

void TypeChecker::error(const std::string &msg)
{
    errors_.emplace_back(msg);
}

PieTypePtr TypeChecker::resolveTypeNode(TypeNode *tn)
{
    if (!tn) return PieType::makeVoid();
    PieTypePtr t = PieType::fromName(tn->name, tn->is_array);
    if (t->isUnknown()) {
        error("Unknown type name: '" + tn->name + "'");
    }
    return t;
}

// Numeric widening only: int is assignable to double.
bool TypeChecker::assignable(const PieType &actual, const PieType &expected) const
{
    if (actual.equals(expected)) return true;
    // Allow int → double widening
    if (actual.kind == PieType::Kind::Int &&
        expected.kind == PieType::Kind::Double) return true;
    // Unknown propagates without error (error already reported earlier)
    if (actual.isUnknown() || expected.isUnknown()) return true;
    return false;
}

PieTypePtr TypeChecker::checkNode(Node *node)
{
    if (!node) {
        result_type_ = PieType::makeVoid();
        return result_type_;
    }
    node->visit(this);
    return result_type_;
}

// ---------------------------------------------------------------------------
// Entry point
// ---------------------------------------------------------------------------

bool TypeChecker::check(ModuleNode *module)
{
    visit(module);
    return errors_.empty();
}

// ---------------------------------------------------------------------------
// Visitor dispatch
// ---------------------------------------------------------------------------

void TypeChecker::visit(Node *node)
{
    if (node) node->visit(this);
}

// ---------------------------------------------------------------------------
// Module
// ---------------------------------------------------------------------------

void TypeChecker::visit(ModuleNode *node)
{
    // Pass 1: register all top-level function types so recursive / mutually
    // recursive calls can be resolved during Pass 2.
    for (FunctionNode *fn : node->functions) {
        std::vector<PieTypePtr> param_types;
        for (auto &param : fn->params) {
            param_types.push_back(resolveTypeNode(param.second));
        }
        PieTypePtr ret = resolveTypeNode(fn->return_type);
        global_type_env_.define(fn->name,
            PieType::makeFunction(std::move(param_types), ret));
    }

    // Pass 2: type-check each function body.
    for (FunctionNode *fn : node->functions) {
        checkNode(fn);
    }
}

// ---------------------------------------------------------------------------
// Import
// ---------------------------------------------------------------------------

void TypeChecker::visit(ImportNode *node)
{
    result_type_ = PieType::makeVoid();
}

// ---------------------------------------------------------------------------
// Function declaration
// ---------------------------------------------------------------------------

void TypeChecker::visit(FunctionNode *node)
{
    // Resolve the declared return type.
    PieTypePtr declared_ret = resolveTypeNode(node->return_type);

    // Create a new type scope for the function body.
    TypeEnv fn_env(&global_type_env_);
    TypeEnv *saved_env = type_env_;
    type_env_ = &fn_env;

    // Bind parameter types.
    for (auto &param : node->params) {
        fn_env.define(param.first, resolveTypeNode(param.second));
    }

    // Set the expected return type for return-statement checks.
    PieTypePtr saved_ret = current_return_type_;
    current_return_type_ = declared_ret;

    // Check all statements in the function body.
    for (Node *stmt : node->children) {
        checkNode(stmt);
    }

    // Restore
    current_return_type_ = saved_ret;
    type_env_ = saved_env;

    result_type_ = PieType::makeVoid();
}

// ---------------------------------------------------------------------------
// Closure (stub — closures not yet implemented in the evaluator)
// ---------------------------------------------------------------------------

void TypeChecker::visit(ClosureNode *node)
{
    result_type_ = PieType::makeUnknown();
}

// ---------------------------------------------------------------------------
// Function call
// ---------------------------------------------------------------------------

void TypeChecker::visit(FunctionCallNode *node)
{
    // Evaluate argument types.
    std::vector<PieTypePtr> arg_types;
    for (Node *arg : node->children) {
        arg_types.push_back(checkNode(arg));
    }

    // Look up the callee.
    PieTypePtr callee_type = type_env_->get(node->name);

    if (!callee_type) {
        error("Call to undefined function '" + node->name + "'");
        result_type_ = PieType::makeUnknown();
        return;
    }

    // Built-ins are stored as non-Function types (just their return type).
    // Skip argument checking for them.
    if (callee_type->kind != PieType::Kind::Function) {
        result_type_ = callee_type;
        return;
    }

    // Arity check.
    const auto &params = callee_type->param_types;
    if (arg_types.size() != params.size()) {
        std::ostringstream os;
        os << "Function '" << node->name << "' expects " << params.size()
           << " argument(s), got " << arg_types.size();
        error(os.str());
        result_type_ = callee_type->return_type
                     ? callee_type->return_type
                     : PieType::makeUnknown();
        return;
    }

    // Argument type checks.
    for (size_t i = 0; i < params.size(); i++) {
        if (!assignable(*arg_types[i], *params[i])) {
            std::ostringstream os;
            os << "Argument " << (i + 1) << " of '" << node->name
               << "': expected " << params[i]->toString()
               << ", got " << arg_types[i]->toString();
            error(os.str());
        }
    }

    result_type_ = callee_type->return_type
                 ? callee_type->return_type
                 : PieType::makeVoid();
}

// ---------------------------------------------------------------------------
// Assignment (var = expr)
// ---------------------------------------------------------------------------

void TypeChecker::visit(AssignNode *node)
{
    PieTypePtr rhs = checkNode(node->value);

    IdentifierNode *id = dynamic_cast<IdentifierNode *>(node->var);
    if (!id) {
        error("Invalid assignment target");
        result_type_ = rhs;
        return;
    }

    PieTypePtr lhs_type = type_env_->get(id->name);
    if (!lhs_type) {
        error("Assignment to undeclared variable '" + id->name + "'");
        result_type_ = rhs;
        return;
    }

    if (!assignable(*rhs, *lhs_type)) {
        error("Cannot assign " + rhs->toString() +
              " to variable '" + id->name + "' of type " + lhs_type->toString());
    }

    result_type_ = lhs_type;
}

// ---------------------------------------------------------------------------
// Let (variable declaration)
// ---------------------------------------------------------------------------

void TypeChecker::visit(LetNode *node)
{
    PieTypePtr declared = node->type ? resolveTypeNode(node->type) : nullptr;
    PieTypePtr inferred = node->value ? checkNode(node->value) : PieType::makeVoid();

    PieTypePtr final_type;

    if (declared) {
        // Annotation present: verify the initialiser matches.
        if (!assignable(*inferred, *declared)) {
            error("Cannot initialise '" + node->name +
                  "' (declared " + declared->toString() +
                  ") with a value of type " + inferred->toString());
        }
        final_type = declared;
    } else {
        // No annotation: infer from the initialiser.
        if (inferred->isUnknown()) {
            error("Cannot infer type of '" + node->name +
                  "': initialiser has unknown type");
        }
        final_type = inferred;
    }

    type_env_->define(node->name, final_type);
    result_type_ = final_type;
}

// ---------------------------------------------------------------------------
// Type annotation node (visited inline; not a standalone statement)
// ---------------------------------------------------------------------------

void TypeChecker::visit(TypeNode *node)
{
    result_type_ = resolveTypeNode(node);
}

// ---------------------------------------------------------------------------
// Primitives
// ---------------------------------------------------------------------------

void TypeChecker::visit(IntNode *node)
{
    result_type_ = PieType::makeInt();
}

void TypeChecker::visit(DoubleNode *node)
{
    result_type_ = PieType::makeDouble();
}

void TypeChecker::visit(StringNode *node)
{
    result_type_ = PieType::makeString();
}

void TypeChecker::visit(IdentifierNode *node)
{
    PieTypePtr t = type_env_->get(node->name);
    if (!t) {
        error("Undefined variable '" + node->name + "'");
        result_type_ = PieType::makeUnknown();
        return;
    }
    result_type_ = t;
}

// ---------------------------------------------------------------------------
// Binary operators
// ---------------------------------------------------------------------------

void TypeChecker::visit(BinaryOpNode *node)
{
    // Short-circuit operators: both sides must be bool.
    if (node->op == BinaryOp::And || node->op == BinaryOp::Or) {
        PieTypePtr lhs = checkNode(node->lhs);
        PieTypePtr rhs = checkNode(node->rhs);
        if (!lhs->isUnknown() && lhs->kind != PieType::Kind::Bool) {
            error("Left operand of " +
                  std::string(node->op == BinaryOp::And ? "&&" : "||") +
                  " must be bool, got " + lhs->toString());
        }
        if (!rhs->isUnknown() && rhs->kind != PieType::Kind::Bool) {
            error("Right operand of " +
                  std::string(node->op == BinaryOp::And ? "&&" : "||") +
                  " must be bool, got " + rhs->toString());
        }
        result_type_ = PieType::makeBool();
        return;
    }

    // Compound assignment operators: += and -=
    if (node->op == BinaryOp::AddAssign || node->op == BinaryOp::SubAssign) {
        IdentifierNode *id = dynamic_cast<IdentifierNode *>(node->lhs);
        if (!id) {
            error("Left side of compound assignment must be a variable");
            result_type_ = PieType::makeUnknown();
            return;
        }
        PieTypePtr var_type = type_env_->get(id->name);
        if (!var_type) {
            error("Undefined variable '" + id->name + "'");
            result_type_ = PieType::makeUnknown();
            return;
        }
        PieTypePtr rhs = checkNode(node->rhs);
        if (!var_type->isNumeric()) {
            error("Compound assignment requires a numeric variable, but '" +
                  id->name + "' is " + var_type->toString());
        } else if (!rhs->isUnknown() && !assignable(*rhs, *var_type)) {
            error("Cannot " +
                  std::string(node->op == BinaryOp::AddAssign ? "+=" : "-=") +
                  " a value of type " + rhs->toString() +
                  " to variable '" + id->name + "' of type " + var_type->toString());
        }
        result_type_ = var_type;
        return;
    }

    PieTypePtr lhs = checkNode(node->lhs);
    PieTypePtr rhs = checkNode(node->rhs);

    switch (node->op) {

        // Arithmetic: numeric × numeric → promoted; string + string → string
        case BinaryOp::Add: {
            if (lhs->kind == PieType::Kind::String &&
                rhs->kind == PieType::Kind::String) {
                result_type_ = PieType::makeString();
            } else {
                PieTypePtr promoted = PieType::numericPromotion(*lhs, *rhs);
                if (!promoted && !lhs->isUnknown() && !rhs->isUnknown()) {
                    error("Operator '+' requires two numeric or two string operands, got " +
                          lhs->toString() + " and " + rhs->toString());
                    result_type_ = PieType::makeUnknown();
                } else {
                    result_type_ = promoted ? promoted : PieType::makeUnknown();
                }
            }
            break;
        }
        case BinaryOp::Sub:
        case BinaryOp::Mul:
        case BinaryOp::Div: {
            static const char *names[] = {"-", "*", "/"};
            int idx = (node->op == BinaryOp::Sub) ? 0
                    : (node->op == BinaryOp::Mul) ? 1 : 2;
            PieTypePtr promoted = PieType::numericPromotion(*lhs, *rhs);
            if (!promoted && !lhs->isUnknown() && !rhs->isUnknown()) {
                error(std::string("Operator '") + names[idx] +
                      "' requires numeric operands, got " +
                      lhs->toString() + " and " + rhs->toString());
                result_type_ = PieType::makeUnknown();
            } else {
                result_type_ = promoted ? promoted : PieType::makeUnknown();
            }
            break;
        }
        case BinaryOp::Mod: {
            if (!lhs->isUnknown() && lhs->kind != PieType::Kind::Int) {
                error("Operator '%' requires int operands, got " + lhs->toString());
            }
            if (!rhs->isUnknown() && rhs->kind != PieType::Kind::Int) {
                error("Operator '%' requires int operands, got " + rhs->toString());
            }
            result_type_ = PieType::makeInt();
            break;
        }

        // Ordered comparison: both numeric → bool
        case BinaryOp::Lt:
        case BinaryOp::Gt:
        case BinaryOp::Le:
        case BinaryOp::Ge: {
            if (!lhs->isUnknown() && !lhs->isNumeric()) {
                error("Comparison operator requires numeric operands, got " +
                      lhs->toString());
            }
            if (!rhs->isUnknown() && !rhs->isNumeric()) {
                error("Comparison operator requires numeric operands, got " +
                      rhs->toString());
            }
            result_type_ = PieType::makeBool();
            break;
        }

        // Equality: same type (or both numeric) → bool
        case BinaryOp::Eq:
        case BinaryOp::Ne: {
            if (!lhs->isUnknown() && !rhs->isUnknown()) {
                bool ok = lhs->equals(*rhs) ||
                          (lhs->isNumeric() && rhs->isNumeric());
                if (!ok) {
                    error("Cannot compare " + lhs->toString() +
                          " and " + rhs->toString() + " for equality");
                }
            }
            result_type_ = PieType::makeBool();
            break;
        }

        case BinaryOp::Assign:
            // Bare '=' is handled by AssignNode; if it appears here just propagate.
            result_type_ = rhs;
            break;

        default:
            result_type_ = PieType::makeUnknown();
            break;
    }
}

// ---------------------------------------------------------------------------
// Unary operators
// ---------------------------------------------------------------------------

void TypeChecker::visit(UnaryOpNode *node)
{
    PieTypePtr operand = checkNode(node->expr);

    switch (node->op) {
        case UnaryOp::Neg:
            if (!operand->isUnknown() && !operand->isNumeric()) {
                error("Unary '-' requires a numeric operand, got " +
                      operand->toString());
                result_type_ = PieType::makeUnknown();
            } else {
                result_type_ = operand;
            }
            break;

        case UnaryOp::Not:
            if (!operand->isUnknown() && operand->kind != PieType::Kind::Bool) {
                error("Unary '!' requires a bool operand, got " +
                      operand->toString());
            }
            result_type_ = PieType::makeBool();
            break;

        case UnaryOp::Inc:
        case UnaryOp::Dec:
            if (!operand->isUnknown() && operand->kind != PieType::Kind::Int) {
                error("'++' / '--' require an int operand, got " +
                      operand->toString());
            }
            result_type_ = PieType::makeInt();
            break;

        default:
            result_type_ = PieType::makeUnknown();
            break;
    }
}

// ---------------------------------------------------------------------------
// Return statement
// ---------------------------------------------------------------------------

void TypeChecker::visit(ReturnNode *node)
{
    PieTypePtr ret = node->expr ? checkNode(node->expr) : PieType::makeVoid();

    if (current_return_type_) {
        if (!assignable(*ret, *current_return_type_)) {
            error("Return type mismatch: expected " +
                  current_return_type_->toString() +
                  ", got " + ret->toString());
        }
    }

    result_type_ = ret;
}

// ---------------------------------------------------------------------------
// If / else
// ---------------------------------------------------------------------------

void TypeChecker::visit(IfNode *node)
{
    PieTypePtr cond = checkNode(node->condition);
    if (!cond->isUnknown() && cond->kind != PieType::Kind::Bool) {
        error("If condition must be bool, got " + cond->toString());
    }
    if (node->then_block) checkNode(node->then_block);
    if (node->else_block) checkNode(node->else_block);
    result_type_ = PieType::makeVoid();
}

// ---------------------------------------------------------------------------
// Block
// ---------------------------------------------------------------------------

void TypeChecker::visit(BlockNode *node)
{
    TypeEnv block_env(type_env_);
    TypeEnv *saved = type_env_;
    type_env_ = &block_env;

    for (Node *stmt : node->children) {
        checkNode(stmt);
    }

    type_env_ = saved;
    result_type_ = PieType::makeVoid();
}

}}
