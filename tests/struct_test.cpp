#include <cassert>
#include <iostream>
#include <string>

#include "compiler/ast.h"
#include "compiler/typecheck.h"
#include "compiler/backend/eval.h"

using namespace pie::compiler;

// ---------------------------------------------------------------------------
// Helper: build a module that defines `struct Point { x: int, y: int }` and
//         a `main` function whose body is the provided statements.
// ---------------------------------------------------------------------------
static ModuleNode *makeModuleWithPoint(std::vector<Node *> stmts,
                                       const std::string &retTypeName = "int")
{
    ModuleNode *mod = new ModuleNode();
    mod->name = "main";

    // struct Point { x: int, y: int }
    StructDefNode *point = new StructDefNode();
    point->name = "Point";
    point->fields.push_back({"x", new TypeNode("int")});
    point->fields.push_back({"y", new TypeNode("int")});
    mod->structs.push_back(point);
    mod->symtab["Point"] = point;

    // fn main() : retType { stmts... }
    FunctionNode *fn = new FunctionNode("main", 0);
    fn->return_type = new TypeNode(retTypeName);
    for (Node *s : stmts) fn->children.push_back(s);
    mod->functions.push_back(fn);
    mod->symtab["main"] = fn;

    return mod;
}

static bool hasError(const std::vector<TypeError> &errs,
                     const std::string &fragment)
{
    for (const TypeError &e : errs) {
        if (e.message.find(fragment) != std::string::npos) return true;
    }
    return false;
}

// ---------------------------------------------------------------------------
// Typecheck tests
// ---------------------------------------------------------------------------

static void test_struct_valid()
{
    // let p: Point = Point { x: 1, y: 2 }
    // return p.x
    StructLiteralNode *lit = new StructLiteralNode();
    lit->struct_name = "Point";
    lit->field_inits.push_back({"x", new IntNode(1)});
    lit->field_inits.push_back({"y", new IntNode(2)});

    ModuleNode *mod = makeModuleWithPoint({
        new LetNode("p", new TypeNode("Point"), lit),
        new ReturnNode(new FieldAccessNode(new IdentifierNode("p"), "x")),
    });

    TypeChecker checker;
    bool ok = checker.check(mod);
    for (auto &e : checker.errors()) std::cerr << "  " << e.message << "\n";
    assert(ok && "valid struct usage must pass");
    std::cout << "test_struct_valid: ok\n";
}

static void test_struct_unknown_field()
{
    // let p: Point = Point { x: 1, z: 2 }   <- 'z' does not exist, 'y' missing
    StructLiteralNode *lit = new StructLiteralNode();
    lit->struct_name = "Point";
    lit->field_inits.push_back({"x", new IntNode(1)});
    lit->field_inits.push_back({"z", new IntNode(2)});  // bad field

    ModuleNode *mod = makeModuleWithPoint({
        new LetNode("p", new TypeNode("Point"), lit),
        new ReturnNode(new IntNode(0)),
    });

    TypeChecker checker;
    bool ok = checker.check(mod);
    assert(!ok && "unknown field must be a type error");
    assert(hasError(checker.errors(), "no field 'z'") ||
           hasError(checker.errors(), "no field"));
    std::cout << "test_struct_unknown_field: ok\n";
}

static void test_struct_field_type_mismatch()
{
    // let p: Point = Point { x: "bad", y: 2 }   <- x should be int
    StructLiteralNode *lit = new StructLiteralNode();
    lit->struct_name = "Point";
    lit->field_inits.push_back({"x", new StringNode("bad")});
    lit->field_inits.push_back({"y", new IntNode(2)});

    ModuleNode *mod = makeModuleWithPoint({
        new LetNode("p", new TypeNode("Point"), lit),
        new ReturnNode(new IntNode(0)),
    });

    TypeChecker checker;
    bool ok = checker.check(mod);
    assert(!ok && "wrong field type must be caught");
    assert(hasError(checker.errors(), "Field 'x'") ||
           hasError(checker.errors(), "expects"));
    std::cout << "test_struct_field_type_mismatch: ok\n";
}

static void test_field_access_on_non_struct()
{
    // let n: int = 42
    // return n.x   <- int has no fields
    ModuleNode *mod = makeModuleWithPoint({
        new LetNode("n", new TypeNode("int"), new IntNode(42)),
        new ReturnNode(new FieldAccessNode(new IdentifierNode("n"), "x")),
    });

    TypeChecker checker;
    bool ok = checker.check(mod);
    assert(!ok && "field access on int must be caught");
    assert(hasError(checker.errors(), "non-struct"));
    std::cout << "test_field_access_on_non_struct: ok\n";
}

static void test_field_access_bad_field()
{
    // let p: Point = Point { x: 1, y: 2 }
    // return p.z   <- no field z
    StructLiteralNode *lit = new StructLiteralNode();
    lit->struct_name = "Point";
    lit->field_inits.push_back({"x", new IntNode(1)});
    lit->field_inits.push_back({"y", new IntNode(2)});

    ModuleNode *mod = makeModuleWithPoint({
        new LetNode("p", new TypeNode("Point"), lit),
        new ReturnNode(new FieldAccessNode(new IdentifierNode("p"), "z")),
    });

    TypeChecker checker;
    bool ok = checker.check(mod);
    assert(!ok && "access of undefined field must be caught");
    assert(hasError(checker.errors(), "no field 'z'") ||
           hasError(checker.errors(), "no field"));
    std::cout << "test_field_access_bad_field: ok\n";
}

static void test_struct_as_function_param()
{
    // fn area(p: Point): int { return p.x }
    // fn main(): int { return area(Point { x: 3, y: 4 }) }
    ModuleNode *mod = new ModuleNode();
    mod->name = "main";

    StructDefNode *point = new StructDefNode();
    point->name = "Point";
    point->fields.push_back({"x", new TypeNode("int")});
    point->fields.push_back({"y", new TypeNode("int")});
    mod->structs.push_back(point);
    mod->symtab["Point"] = point;

    FunctionNode *area_fn = new FunctionNode("area", 0);
    area_fn->return_type = new TypeNode("int");
    area_fn->params.push_back({"p", new TypeNode("Point")});
    area_fn->children.push_back(
        new ReturnNode(new FieldAccessNode(new IdentifierNode("p"), "x")));

    StructLiteralNode *lit = new StructLiteralNode();
    lit->struct_name = "Point";
    lit->field_inits.push_back({"x", new IntNode(3)});
    lit->field_inits.push_back({"y", new IntNode(4)});

    FunctionCallNode *call = new FunctionCallNode("area");
    call->children.push_back(lit);

    FunctionNode *main_fn = new FunctionNode("main", 0);
    main_fn->return_type = new TypeNode("int");
    main_fn->children.push_back(new ReturnNode(call));

    mod->functions.push_back(area_fn);
    mod->functions.push_back(main_fn);
    mod->symtab["area"] = area_fn;
    mod->symtab["main"] = main_fn;

    TypeChecker checker;
    bool ok = checker.check(mod);
    for (auto &e : checker.errors()) std::cerr << "  " << e.message << "\n";
    assert(ok && "struct as function parameter must typecheck");
    std::cout << "test_struct_as_function_param: ok\n";
}

// ---------------------------------------------------------------------------
// Evaluator tests
// ---------------------------------------------------------------------------

static void test_eval_struct_literal()
{
    // let p = Point { x: 10, y: 20 }
    // return p.x
    StructLiteralNode *lit = new StructLiteralNode();
    lit->struct_name = "Point";
    lit->field_inits.push_back({"x", new IntNode(10)});
    lit->field_inits.push_back({"y", new IntNode(20)});

    ModuleNode *mod = makeModuleWithPoint({
        new LetNode("p", nullptr, lit),
        new ReturnNode(new FieldAccessNode(new IdentifierNode("p"), "x")),
    });

    EvalVisitor eval;
    Value result = eval.run(mod);
    assert(result.type == Value::Type::Int);
    assert(result.int_val == 10);
    std::cout << "test_eval_struct_literal: ok\n";
}

static void test_eval_field_access_y()
{
    // let p = Point { x: 3, y: 7 }
    // return p.y
    StructLiteralNode *lit = new StructLiteralNode();
    lit->struct_name = "Point";
    lit->field_inits.push_back({"x", new IntNode(3)});
    lit->field_inits.push_back({"y", new IntNode(7)});

    ModuleNode *mod = makeModuleWithPoint({
        new LetNode("p", nullptr, lit),
        new ReturnNode(new FieldAccessNode(new IdentifierNode("p"), "y")),
    });

    EvalVisitor eval;
    Value result = eval.run(mod);
    assert(result.type == Value::Type::Int);
    assert(result.int_val == 7);
    std::cout << "test_eval_field_access_y: ok\n";
}

static void test_eval_struct_field_arithmetic()
{
    // let p = Point { x: 3, y: 4 }
    // return p.x + p.y    (= 7)
    StructLiteralNode *lit = new StructLiteralNode();
    lit->struct_name = "Point";
    lit->field_inits.push_back({"x", new IntNode(3)});
    lit->field_inits.push_back({"y", new IntNode(4)});

    // We need p.x and p.y separately so make two FieldAccessNodes on p
    ModuleNode *mod = makeModuleWithPoint({
        new LetNode("p", nullptr, lit),
        new ReturnNode(new BinaryOpNode(
            BinaryOp::Add,
            new FieldAccessNode(new IdentifierNode("p"), "x"),
            new FieldAccessNode(new IdentifierNode("p"), "y"))),
    });

    // Skip type checker (identifiers evaluated at runtime from let inference)
    EvalVisitor eval;
    Value result = eval.run(mod);
    assert(result.type == Value::Type::Int);
    assert(result.int_val == 7);
    std::cout << "test_eval_struct_field_arithmetic: ok\n";
}

// ---------------------------------------------------------------------------
// Main
// ---------------------------------------------------------------------------
int main()
{
    test_struct_valid();
    test_struct_unknown_field();
    test_struct_field_type_mismatch();
    test_field_access_on_non_struct();
    test_field_access_bad_field();
    test_struct_as_function_param();
    test_eval_struct_literal();
    test_eval_field_access_y();
    test_eval_struct_field_arithmetic();

    std::cout << "struct_test: ok\n";
    return 0;
}
