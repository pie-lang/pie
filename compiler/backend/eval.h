#ifndef __PIE_BACKEND_EVAL__
#define __PIE_BACKEND_EVAL__

#include <string>
#include <map>
#include <vector>
#include <stdexcept>
#include <functional>

#include "compiler/ast.h"

namespace pie { namespace compiler {

// Runtime value representation
struct Value {
    enum class Type {
        Nil,
        Int,
        Double,
        Bool,
        String,
        Function,
        BuiltinFunction
    };

    Type type;
    int64_t int_val;
    double double_val;
    bool bool_val;
    std::string string_val;
    FunctionNode *function_val;
    std::function<Value(std::vector<Value>&)> builtin_val;

    Value() : type(Type::Nil), int_val(0), double_val(0.0), bool_val(false), function_val(nullptr) {}

    static Value makeNil() {
        return Value();
    }

    static Value makeInt(int64_t v) {
        Value val;
        val.type = Type::Int;
        val.int_val = v;
        return val;
    }

    static Value makeDouble(double v) {
        Value val;
        val.type = Type::Double;
        val.double_val = v;
        return val;
    }

    static Value makeBool(bool v) {
        Value val;
        val.type = Type::Bool;
        val.bool_val = v;
        return val;
    }

    static Value makeString(const std::string &v) {
        Value val;
        val.type = Type::String;
        val.string_val = v;
        return val;
    }

    static Value makeFunction(FunctionNode *fn) {
        Value val;
        val.type = Type::Function;
        val.function_val = fn;
        return val;
    }

    static Value makeBuiltin(std::function<Value(std::vector<Value>&)> fn) {
        Value val;
        val.type = Type::BuiltinFunction;
        val.builtin_val = fn;
        return val;
    }

    // Convert to numeric for arithmetic
    double toDouble() const {
        if (type == Type::Int) return (double)int_val;
        if (type == Type::Double) return double_val;
        return 0.0;
    }

    int64_t toInt() const {
        if (type == Type::Int) return int_val;
        if (type == Type::Double) return (int64_t)double_val;
        return 0;
    }

    bool toBool() const {
        switch (type) {
            case Type::Nil: return false;
            case Type::Bool: return bool_val;
            case Type::Int: return int_val != 0;
            case Type::Double: return double_val != 0.0;
            case Type::String: return !string_val.empty();
            default: return true;
        }
    }

    std::string toString() const {
        switch (type) {
            case Type::Nil: return "nil";
            case Type::Bool: return bool_val ? "true" : "false";
            case Type::Int: return std::to_string(int_val);
            case Type::Double: return std::to_string(double_val);
            case Type::String: return string_val;
            case Type::Function: return "<function>";
            case Type::BuiltinFunction: return "<builtin>";
            default: return "<unknown>";
        }
    }

    bool isNumeric() const {
        return type == Type::Int || type == Type::Double;
    }
};

// Exception for return statements
class ReturnException : public std::exception {
public:
    Value value;
    ReturnException(const Value &v) : value(v) {}
};

// Environment for variable storage
class Environment {
public:
    Environment(Environment *parent = nullptr) : parent(parent) {}

    void define(const std::string &name, const Value &value) {
        vars[name] = value;
    }

    Value get(const std::string &name) const {
        auto it = vars.find(name);
        if (it != vars.end()) {
            return it->second;
        }
        if (parent) {
            return parent->get(name);
        }
        throw std::runtime_error("Undefined variable: " + name);
    }

    bool has(const std::string &name) const {
        auto it = vars.find(name);
        if (it != vars.end()) return true;
        if (parent) return parent->has(name);
        return false;
    }

    void set(const std::string &name, const Value &value) {
        auto it = vars.find(name);
        if (it != vars.end()) {
            it->second = value;
            return;
        }
        if (parent) {
            parent->set(name, value);
            return;
        }
        throw std::runtime_error("Undefined variable: " + name);
    }

    const std::map<std::string, Value> &variables() const {
        return vars;
    }

    const Environment *parentEnv() const {
        return parent;
    }

private:
    std::map<std::string, Value> vars;
    Environment *parent;
};

// The interpreter visitor
class EvalVisitor : public Visitor
{
public:
    EvalVisitor();
    void setDebugMode(bool enabled);

    Value evaluate(Node *node);
    Value run(ModuleNode *module);

    void visit(Node *node) override;

    #define AST_NODE DECLARE_VISIT
    AST_NODES
    #undef AST_NODE

private:
    Value result;
    Environment *env;
    Environment global_env;
    ModuleNode *current_module;
    bool debug_mode;
    bool debug_continue;
    size_t debug_step;
    size_t debug_depth;

    void registerBuiltins();
    Value callFunction(FunctionNode *fn, std::vector<Value> &args);
    void debugBefore(Node *node);
    std::string debugNodeText(Node *node);
    void debugPrintEnvironment() const;
    void debugPrintHelp() const;
    void debugPrintValue(const std::string &name) const;
};

}}

#endif
