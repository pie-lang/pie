#include "compiler/backend/eval.h"
#include <iostream>
#include <cstdlib>

namespace pie { namespace compiler {

EvalVisitor::EvalVisitor()
    : env(&global_env), current_module(nullptr)
{
    registerBuiltins();
}

void EvalVisitor::registerBuiltins()
{
    // print function
    global_env.define("print", Value::makeBuiltin([](std::vector<Value> &args) -> Value {
        for (size_t i = 0; i < args.size(); i++) {
            if (i > 0) std::cout << " ";
            std::cout << args[i].toString();
        }
        std::cout << std::endl;
        return Value::makeNil();
    }));

    // io.print function (same as print)
    global_env.define("io.print", Value::makeBuiltin([](std::vector<Value> &args) -> Value {
        for (size_t i = 0; i < args.size(); i++) {
            if (i > 0) std::cout << " ";
            std::cout << args[i].toString();
        }
        std::cout << std::endl;
        return Value::makeNil();
    }));

    // exit function
    global_env.define("exit", Value::makeBuiltin([](std::vector<Value> &args) -> Value {
        int code = 0;
        if (!args.empty()) {
            code = (int)args[0].toInt();
        }
        std::exit(code);
        return Value::makeNil();  // Never reached
    }));

    // len function for strings
    global_env.define("len", Value::makeBuiltin([](std::vector<Value> &args) -> Value {
        if (args.empty()) return Value::makeInt(0);
        if (args[0].type == Value::Type::String) {
            return Value::makeInt(args[0].string_val.length());
        }
        return Value::makeInt(0);
    }));

    // type function
    global_env.define("type", Value::makeBuiltin([](std::vector<Value> &args) -> Value {
        if (args.empty()) return Value::makeString("nil");
        switch (args[0].type) {
            case Value::Type::Nil: return Value::makeString("nil");
            case Value::Type::Int: return Value::makeString("int");
            case Value::Type::Double: return Value::makeString("double");
            case Value::Type::Bool: return Value::makeString("bool");
            case Value::Type::String: return Value::makeString("string");
            case Value::Type::Function: return Value::makeString("function");
            case Value::Type::BuiltinFunction: return Value::makeString("builtin");
            default: return Value::makeString("unknown");
        }
    }));
}

Value EvalVisitor::evaluate(Node *node)
{
    if (!node) return Value::makeNil();
    node->visit(this);
    return result;
}

Value EvalVisitor::run(ModuleNode *module)
{
    current_module = module;

    // First, register all functions in the global scope
    for (FunctionNode *fn : module->functions) {
        global_env.define(fn->name, Value::makeFunction(fn));
    }

    // Find and call main function
    if (module->symtab.find("main") != module->symtab.end()) {
        FunctionNode *main_fn = dynamic_cast<FunctionNode*>(module->symtab["main"]);
        if (main_fn) {
            std::vector<Value> args;
            return callFunction(main_fn, args);
        }
    }

    return Value::makeNil();
}

Value EvalVisitor::callFunction(FunctionNode *fn, std::vector<Value> &args)
{
    // Create new environment for function scope
    Environment func_env(&global_env);
    Environment *old_env = env;
    env = &func_env;

    // Bind parameters
    for (size_t i = 0; i < fn->params.size() && i < args.size(); i++) {
        func_env.define(fn->params[i].first, args[i]);
    }

    // Execute function body
    Value return_val = Value::makeNil();
    try {
        for (Node *stmt : fn->children) {
            evaluate(stmt);
        }
    } catch (ReturnException &ret) {
        return_val = ret.value;
    }

    env = old_env;
    return return_val;
}

void EvalVisitor::visit(Node *node)
{
    if (node) {
        node->visit(this);
    }
}

void EvalVisitor::visit(ModuleNode *node)
{
    // Module evaluation is handled by run()
    result = Value::makeNil();
}

void EvalVisitor::visit(ImportNode *node)
{
    // Imports are not executed at runtime
    result = Value::makeNil();
}

void EvalVisitor::visit(FunctionNode *node)
{
    // Function declarations just register the function
    result = Value::makeFunction(node);
}

void EvalVisitor::visit(ClosureNode *node)
{
    // TODO: Implement closures properly
    result = Value::makeNil();
}

void EvalVisitor::visit(FunctionCallNode *node)
{
    // Evaluate arguments
    std::vector<Value> args;
    for (Node *child : node->children) {
        args.push_back(evaluate(child));
    }

    // Look up the function
    Value func_val;
    try {
        func_val = env->get(node->name);
    } catch (...) {
        // Try to find in module symtab
        if (current_module && current_module->symtab.find(node->name) != current_module->symtab.end()) {
            FunctionNode *fn = dynamic_cast<FunctionNode*>(current_module->symtab[node->name]);
            if (fn) {
                result = callFunction(fn, args);
                return;
            }
        }
        throw std::runtime_error("Undefined function: " + node->name);
    }

    if (func_val.type == Value::Type::Function) {
        result = callFunction(func_val.function_val, args);
    } else if (func_val.type == Value::Type::BuiltinFunction) {
        result = func_val.builtin_val(args);
    } else {
        throw std::runtime_error("Not a function: " + node->name);
    }
}

void EvalVisitor::visit(AssignNode *node)
{
    Value value = evaluate(node->value);

    IdentifierNode *id = dynamic_cast<IdentifierNode*>(node->var);
    if (id) {
        env->set(id->name, value);
        result = value;
    } else {
        throw std::runtime_error("Invalid assignment target");
    }
}

void EvalVisitor::visit(LetNode *node)
{
    Value value = Value::makeNil();
    if (node->value) {
        value = evaluate(node->value);
    }
    env->define(node->name, value);
    result = value;
}

void EvalVisitor::visit(TypeNode *node)
{
    // Types are not evaluated at runtime
    result = Value::makeNil();
}

void EvalVisitor::visit(IntNode *node)
{
    result = Value::makeInt(node->value);
}

void EvalVisitor::visit(DoubleNode *node)
{
    result = Value::makeDouble(node->value);
}

void EvalVisitor::visit(StringNode *node)
{
    result = Value::makeString(node->str);
}

void EvalVisitor::visit(IdentifierNode *node)
{
    result = env->get(node->name);
}

void EvalVisitor::visit(BinaryOpNode *node)
{
    // Special case for logical operators (short-circuit evaluation)
    if (node->op == BinaryOp::And) {
        Value lhs = evaluate(node->lhs);
        if (!lhs.toBool()) {
            result = Value::makeBool(false);
            return;
        }
        Value rhs = evaluate(node->rhs);
        result = Value::makeBool(rhs.toBool());
        return;
    }

    if (node->op == BinaryOp::Or) {
        Value lhs = evaluate(node->lhs);
        if (lhs.toBool()) {
            result = Value::makeBool(true);
            return;
        }
        Value rhs = evaluate(node->rhs);
        result = Value::makeBool(rhs.toBool());
        return;
    }

    // Special case for assignment operators
    if (node->op == BinaryOp::AddAssign || node->op == BinaryOp::SubAssign) {
        IdentifierNode *id = dynamic_cast<IdentifierNode*>(node->lhs);
        if (!id) {
            throw std::runtime_error("Invalid assignment target");
        }

        Value current = env->get(id->name);
        Value rhs = evaluate(node->rhs);
        Value new_val;

        if (node->op == BinaryOp::AddAssign) {
            if (current.type == Value::Type::Int && rhs.type == Value::Type::Int) {
                new_val = Value::makeInt(current.int_val + rhs.int_val);
            } else {
                new_val = Value::makeDouble(current.toDouble() + rhs.toDouble());
            }
        } else {  // SubAssign
            if (current.type == Value::Type::Int && rhs.type == Value::Type::Int) {
                new_val = Value::makeInt(current.int_val - rhs.int_val);
            } else {
                new_val = Value::makeDouble(current.toDouble() - rhs.toDouble());
            }
        }

        env->set(id->name, new_val);
        result = new_val;
        return;
    }

    Value lhs = evaluate(node->lhs);
    Value rhs = evaluate(node->rhs);

    switch (node->op) {
        case BinaryOp::Add:
            if (lhs.type == Value::Type::String || rhs.type == Value::Type::String) {
                result = Value::makeString(lhs.toString() + rhs.toString());
            } else if (lhs.type == Value::Type::Int && rhs.type == Value::Type::Int) {
                result = Value::makeInt(lhs.int_val + rhs.int_val);
            } else {
                result = Value::makeDouble(lhs.toDouble() + rhs.toDouble());
            }
            break;

        case BinaryOp::Sub:
            if (lhs.type == Value::Type::Int && rhs.type == Value::Type::Int) {
                result = Value::makeInt(lhs.int_val - rhs.int_val);
            } else {
                result = Value::makeDouble(lhs.toDouble() - rhs.toDouble());
            }
            break;

        case BinaryOp::Mul:
            if (lhs.type == Value::Type::Int && rhs.type == Value::Type::Int) {
                result = Value::makeInt(lhs.int_val * rhs.int_val);
            } else {
                result = Value::makeDouble(lhs.toDouble() * rhs.toDouble());
            }
            break;

        case BinaryOp::Div:
            if (rhs.toDouble() == 0.0) {
                throw std::runtime_error("Division by zero");
            }
            if (lhs.type == Value::Type::Int && rhs.type == Value::Type::Int) {
                result = Value::makeInt(lhs.int_val / rhs.int_val);
            } else {
                result = Value::makeDouble(lhs.toDouble() / rhs.toDouble());
            }
            break;

        case BinaryOp::Mod:
            if (rhs.toInt() == 0) {
                throw std::runtime_error("Modulo by zero");
            }
            result = Value::makeInt(lhs.toInt() % rhs.toInt());
            break;

        case BinaryOp::Lt:
            result = Value::makeBool(lhs.toDouble() < rhs.toDouble());
            break;

        case BinaryOp::Gt:
            result = Value::makeBool(lhs.toDouble() > rhs.toDouble());
            break;

        case BinaryOp::Le:
            result = Value::makeBool(lhs.toDouble() <= rhs.toDouble());
            break;

        case BinaryOp::Ge:
            result = Value::makeBool(lhs.toDouble() >= rhs.toDouble());
            break;

        case BinaryOp::Eq:
            if (lhs.type == Value::Type::String && rhs.type == Value::Type::String) {
                result = Value::makeBool(lhs.string_val == rhs.string_val);
            } else if (lhs.isNumeric() && rhs.isNumeric()) {
                result = Value::makeBool(lhs.toDouble() == rhs.toDouble());
            } else {
                result = Value::makeBool(lhs.type == rhs.type && lhs.toBool() == rhs.toBool());
            }
            break;

        case BinaryOp::Ne:
            if (lhs.type == Value::Type::String && rhs.type == Value::Type::String) {
                result = Value::makeBool(lhs.string_val != rhs.string_val);
            } else if (lhs.isNumeric() && rhs.isNumeric()) {
                result = Value::makeBool(lhs.toDouble() != rhs.toDouble());
            } else {
                result = Value::makeBool(lhs.type != rhs.type || lhs.toBool() != rhs.toBool());
            }
            break;

        case BinaryOp::Assign:
            // Should be handled by AssignNode
            result = rhs;
            break;

        default:
            throw std::runtime_error("Unknown binary operator");
    }
}

void EvalVisitor::visit(UnaryOpNode *node)
{
    Value val = evaluate(node->expr);

    switch (node->op) {
        case UnaryOp::Neg:
            if (val.type == Value::Type::Int) {
                result = Value::makeInt(-val.int_val);
            } else {
                result = Value::makeDouble(-val.toDouble());
            }
            break;

        case UnaryOp::Not:
            result = Value::makeBool(!val.toBool());
            break;

        case UnaryOp::Inc:
        case UnaryOp::Dec:
            // TODO: Implement increment/decrement
            result = val;
            break;

        default:
            throw std::runtime_error("Unknown unary operator");
    }
}

void EvalVisitor::visit(ReturnNode *node)
{
    Value val = Value::makeNil();
    if (node->expr) {
        val = evaluate(node->expr);
    }
    throw ReturnException(val);
}

void EvalVisitor::visit(IfNode *node)
{
    Value cond = evaluate(node->condition);

    if (cond.toBool()) {
        if (node->then_block) {
            evaluate(node->then_block);
        }
    } else if (node->else_block) {
        evaluate(node->else_block);
    }

    result = Value::makeNil();
}

void EvalVisitor::visit(BlockNode *node)
{
    // Create new scope for block
    Environment block_env(env);
    Environment *old_env = env;
    env = &block_env;

    for (Node *stmt : node->children) {
        evaluate(stmt);
    }

    env = old_env;
    result = Value::makeNil();
}

}}
