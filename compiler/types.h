#ifndef __PIE_TYPES__
#define __PIE_TYPES__

#include <string>
#include <vector>
#include <map>
#include <memory>

namespace pie { namespace compiler {

// A resolved type in the Pie type system.
// Used by the type checker; distinct from the runtime Value::Type enum.
struct PieType {
    enum class Kind {
        Int,
        Double,
        Bool,
        String,
        Void,       // return type of functions that don't return a value
        Array,      // element_type must be set
        Function,   // param_types + return_type must be set
        Struct,     // struct_name + struct_fields must be set
        Unknown,    // placeholder during inference / for builtins with any-type params
    };

    Kind kind;

    // Array: T[]
    std::shared_ptr<PieType> element_type;

    // Function: fn(T1, T2, ...) -> R
    std::vector<std::shared_ptr<PieType>> param_types;
    std::shared_ptr<PieType> return_type;

    // Struct
    std::string struct_name;
    std::map<std::string, std::shared_ptr<PieType>> struct_fields;

    explicit PieType(Kind k) : kind(k) {}

    // --- factories ---
    static std::shared_ptr<PieType> makeInt()     { return std::make_shared<PieType>(Kind::Int); }
    static std::shared_ptr<PieType> makeDouble()  { return std::make_shared<PieType>(Kind::Double); }
    static std::shared_ptr<PieType> makeBool()    { return std::make_shared<PieType>(Kind::Bool); }
    static std::shared_ptr<PieType> makeString()  { return std::make_shared<PieType>(Kind::String); }
    static std::shared_ptr<PieType> makeVoid()    { return std::make_shared<PieType>(Kind::Void); }
    static std::shared_ptr<PieType> makeUnknown() { return std::make_shared<PieType>(Kind::Unknown); }

    static std::shared_ptr<PieType> makeStruct(
        const std::string &name,
        std::map<std::string, std::shared_ptr<PieType>> fields)
    {
        auto t = std::make_shared<PieType>(Kind::Struct);
        t->struct_name = name;
        t->struct_fields = std::move(fields);
        return t;
    }

    static std::shared_ptr<PieType> makeArray(std::shared_ptr<PieType> elem)
    {
        auto t = std::make_shared<PieType>(Kind::Array);
        t->element_type = std::move(elem);
        return t;
    }

    static std::shared_ptr<PieType> makeFunction(
        std::vector<std::shared_ptr<PieType>> params,
        std::shared_ptr<PieType> ret)
    {
        auto t = std::make_shared<PieType>(Kind::Function);
        t->param_types = std::move(params);
        t->return_type = std::move(ret);
        return t;
    }

    // --- queries ---
    bool isNumeric() const { return kind == Kind::Int || kind == Kind::Double; }
    bool isUnknown() const { return kind == Kind::Unknown; }

    // Structural equality (not identity)
    bool equals(const PieType &o) const
    {
        if (kind != o.kind) return false;
        if (kind == Kind::Array) {
            return element_type && o.element_type &&
                   element_type->equals(*o.element_type);
        }
        if (kind == Kind::Struct) {
            return struct_name == o.struct_name;
        }
        if (kind == Kind::Function) {
            if (param_types.size() != o.param_types.size()) return false;
            for (size_t i = 0; i < param_types.size(); i++) {
                if (!param_types[i]->equals(*o.param_types[i])) return false;
            }
            return return_type && o.return_type &&
                   return_type->equals(*o.return_type);
        }
        return true;
    }

    // Human-readable name
    std::string toString() const
    {
        switch (kind) {
            case Kind::Int:     return "int";
            case Kind::Double:  return "double";
            case Kind::Bool:    return "bool";
            case Kind::String:  return "string";
            case Kind::Void:    return "void";
            case Kind::Unknown: return "<unknown>";
            case Kind::Struct:  return struct_name;
            case Kind::Array:
                return element_type ? element_type->toString() + "[]" : "<array>";
            case Kind::Function: {
                std::string s = "fn(";
                for (size_t i = 0; i < param_types.size(); i++) {
                    if (i > 0) s += ", ";
                    s += param_types[i] ? param_types[i]->toString() : "<unknown>";
                }
                s += ") -> ";
                s += return_type ? return_type->toString() : "void";
                return s;
            }
            default: return "<type>";
        }
    }

    // Parse a TypeNode annotation into a PieType.
    // Returns Unknown for unrecognised names so the checker can report an error.
    static std::shared_ptr<PieType> fromName(const std::string &name,
                                              bool is_array = false)
    {
        std::shared_ptr<PieType> base;
        if      (name == "int")    base = makeInt();
        else if (name == "double") base = makeDouble();
        else if (name == "bool")   base = makeBool();
        else if (name == "string") base = makeString();
        else if (name == "void")   base = makeVoid();
        // User-defined struct names are NOT resolved here — the type checker
        // looks them up in the type environment.  Return Unknown so the checker
        // can handle the lookup and report a proper error if needed.
        else                       base = makeUnknown();
        return is_array ? makeArray(base) : base;
    }

    // Returns the wider of two numeric types (int + double → double).
    // Returns nullptr if either type is not numeric.
    static std::shared_ptr<PieType> numericPromotion(
        const PieType &a, const PieType &b)
    {
        if (!a.isNumeric() || !b.isNumeric()) return nullptr;
        if (a.kind == Kind::Double || b.kind == Kind::Double) return makeDouble();
        return makeInt();
    }
};

using PieTypePtr = std::shared_ptr<PieType>;

}}

#endif
