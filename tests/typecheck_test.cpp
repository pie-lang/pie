#include <cassert>
#include <iostream>
#include <string>

#include "compiler/ast.h"
#include "compiler/typecheck.h"

using namespace pie::compiler;

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

static bool hasError(const std::vector<TypeError> &errs, const std::string &fragment)
{
    for (const TypeError &e : errs) {
        if (e.message.find(fragment) != std::string::npos) return true;
    }
    return false;
}

// Build a minimal module with a single `main` function whose body is the
// provided list of statements.
static ModuleNode *makeModule(std::vector<Node *> stmts)
{
    ModuleNode *mod = new ModuleNode();
    mod->name = "main";

    FunctionNode *fn = new FunctionNode("main", 0);
    // return type: int
    fn->return_type = new TypeNode("int");
    for (Node *s : stmts) fn->children.push_back(s);

    mod->functions.push_back(fn);
    mod->symtab["main"] = fn;
    return mod;
}

// ---------------------------------------------------------------------------
// Test: well-typed program passes
// ---------------------------------------------------------------------------
static void test_valid_program()
{
    // let a: int = 1
    // a += 2
    // return a
    ModuleNode *mod = makeModule({
        new LetNode("a", new TypeNode("int"), new IntNode(1)),
        new BinaryOpNode(BinaryOp::AddAssign,
                         new IdentifierNode("a"), new IntNode(2)),
        new ReturnNode(new IdentifierNode("a")),
    });

    TypeChecker checker;
    bool ok = checker.check(mod);
    assert(ok && "valid program must pass type checking");
    std::cout << "test_valid_program: ok\n";
}

// ---------------------------------------------------------------------------
// Test: type annotation mismatch is caught
// ---------------------------------------------------------------------------
static void test_annotation_mismatch()
{
    // let x: int = "hello"   <- error
    ModuleNode *mod = makeModule({
        new LetNode("x", new TypeNode("int"), new StringNode("hello")),
        new ReturnNode(new IntNode(0)),
    });

    TypeChecker checker;
    bool ok = checker.check(mod);
    assert(!ok && "annotation mismatch must be caught");
    assert(hasError(checker.errors(), "cannot initialise") ||
           hasError(checker.errors(), "Cannot initialise"));
    std::cout << "test_annotation_mismatch: ok\n";
}

// ---------------------------------------------------------------------------
// Test: arithmetic on mismatched types is caught
// ---------------------------------------------------------------------------
static void test_arithmetic_type_error()
{
    // let result: int = 1 + "oops"   <- error
    ModuleNode *mod = makeModule({
        new LetNode("result", new TypeNode("int"),
                    new BinaryOpNode(BinaryOp::Add,
                                     new IntNode(1),
                                     new StringNode("oops"))),
        new ReturnNode(new IntNode(0)),
    });

    TypeChecker checker;
    bool ok = checker.check(mod);
    assert(!ok && "int + string must be a type error");
    assert(hasError(checker.errors(), "+"));
    std::cout << "test_arithmetic_type_error: ok\n";
}

// ---------------------------------------------------------------------------
// Test: int inferred from literal, no annotation needed
// ---------------------------------------------------------------------------
static void test_type_inference()
{
    // let x = 42   <- inferred as int
    // return x
    ModuleNode *mod = makeModule({
        new LetNode("x", nullptr, new IntNode(42)),
        new ReturnNode(new IdentifierNode("x")),
    });

    TypeChecker checker;
    bool ok = checker.check(mod);
    assert(ok && "type inference of int literal must succeed");
    std::cout << "test_type_inference: ok\n";
}

// ---------------------------------------------------------------------------
// Test: return type mismatch is caught
// ---------------------------------------------------------------------------
static void test_return_type_mismatch()
{
    // fn main() : int { return "bad" }
    ModuleNode *mod = makeModule({
        new ReturnNode(new StringNode("bad")),
    });

    TypeChecker checker;
    bool ok = checker.check(mod);
    assert(!ok && "returning string from int function must be caught");
    assert(hasError(checker.errors(), "Return type mismatch"));
    std::cout << "test_return_type_mismatch: ok\n";
}

// ---------------------------------------------------------------------------
// Test: if-condition must be bool
// ---------------------------------------------------------------------------
static void test_if_condition_not_bool()
{
    // if (42) { return 1 }
    BlockNode *then_block = new BlockNode();
    then_block->children.push_back(new ReturnNode(new IntNode(1)));

    ModuleNode *mod = makeModule({
        new IfNode(new IntNode(42), then_block),
        new ReturnNode(new IntNode(0)),
    });

    TypeChecker checker;
    bool ok = checker.check(mod);
    assert(!ok && "non-bool if condition must be caught");
    assert(hasError(checker.errors(), "If condition must be bool"));
    std::cout << "test_if_condition_not_bool: ok\n";
}

// ---------------------------------------------------------------------------
// Test: function call arity error is caught
// ---------------------------------------------------------------------------
static void test_function_arity_error()
{
    // fn add(x: int, y: int): int { return x }
    // fn main(): int { return add(1) }   <- missing second arg

    ModuleNode *mod = new ModuleNode();
    mod->name = "main";

    FunctionNode *add_fn = new FunctionNode("add", 0);
    add_fn->return_type = new TypeNode("int");
    add_fn->params.push_back({"x", new TypeNode("int")});
    add_fn->params.push_back({"y", new TypeNode("int")});
    add_fn->children.push_back(new ReturnNode(new IdentifierNode("x")));

    FunctionNode *main_fn = new FunctionNode("main", 0);
    main_fn->return_type = new TypeNode("int");

    FunctionCallNode *call = new FunctionCallNode("add");
    call->children.push_back(new IntNode(1));  // only 1 arg, need 2

    main_fn->children.push_back(new ReturnNode(call));

    mod->functions.push_back(add_fn);
    mod->functions.push_back(main_fn);
    mod->symtab["add"]  = add_fn;
    mod->symtab["main"] = main_fn;

    TypeChecker checker;
    bool ok = checker.check(mod);
    assert(!ok && "arity mismatch must be caught");
    assert(hasError(checker.errors(), "expects"));
    std::cout << "test_function_arity_error: ok\n";
}

// ---------------------------------------------------------------------------
// Test: int widens to double in arithmetic (no error)
// ---------------------------------------------------------------------------
static void test_numeric_widening()
{
    // let x: double = 1 + 2.5   <- int + double → double, valid
    ModuleNode *mod = makeModule({
        new LetNode("x", new TypeNode("double"),
                    new BinaryOpNode(BinaryOp::Add,
                                     new IntNode(1),
                                     new DoubleNode(2.5))),
        new ReturnNode(new IntNode(0)),
    });

    TypeChecker checker;
    bool ok = checker.check(mod);
    assert(ok && "int + double widening must be valid");
    std::cout << "test_numeric_widening: ok\n";
}

// ---------------------------------------------------------------------------
// Test: logical operators require bool operands
// ---------------------------------------------------------------------------
static void test_logical_op_non_bool()
{
    // let x: bool = 1 && 0    <- int operands on && must be an error
    ModuleNode *mod = makeModule({
        new LetNode("x", new TypeNode("bool"),
                    new BinaryOpNode(BinaryOp::And,
                                     new IntNode(1),
                                     new IntNode(0))),
        new ReturnNode(new IntNode(0)),
    });

    TypeChecker checker;
    bool ok = checker.check(mod);
    assert(!ok && "int operands on && must be caught");
    assert(hasError(checker.errors(), "&&"));
    std::cout << "test_logical_op_non_bool: ok\n";
}

// ---------------------------------------------------------------------------
// Test: undefined variable is caught
// ---------------------------------------------------------------------------
static void test_undefined_variable()
{
    // return z   <- z was never declared
    ModuleNode *mod = makeModule({
        new ReturnNode(new IdentifierNode("z")),
    });

    TypeChecker checker;
    bool ok = checker.check(mod);
    assert(!ok && "undefined variable must be caught");
    assert(hasError(checker.errors(), "Undefined variable"));
    std::cout << "test_undefined_variable: ok\n";
}

// ---------------------------------------------------------------------------
// Test: function argument type mismatch
// ---------------------------------------------------------------------------
static void test_function_arg_type_mismatch()
{
    // fn square(x: int): int { return x }
    // fn main(): int { return square("not-a-number") }

    ModuleNode *mod = new ModuleNode();
    mod->name = "main";

    FunctionNode *square_fn = new FunctionNode("square", 0);
    square_fn->return_type = new TypeNode("int");
    square_fn->params.push_back({"x", new TypeNode("int")});
    square_fn->children.push_back(new ReturnNode(new IdentifierNode("x")));

    FunctionNode *main_fn = new FunctionNode("main", 0);
    main_fn->return_type = new TypeNode("int");

    FunctionCallNode *call = new FunctionCallNode("square");
    call->children.push_back(new StringNode("not-a-number"));

    main_fn->children.push_back(new ReturnNode(call));

    mod->functions.push_back(square_fn);
    mod->functions.push_back(main_fn);
    mod->symtab["square"] = square_fn;
    mod->symtab["main"]   = main_fn;

    TypeChecker checker;
    bool ok = checker.check(mod);
    assert(!ok && "wrong argument type must be caught");
    assert(hasError(checker.errors(), "Argument"));
    std::cout << "test_function_arg_type_mismatch: ok\n";
}

// ---------------------------------------------------------------------------
// Main
// ---------------------------------------------------------------------------
int main()
{
    test_valid_program();
    test_annotation_mismatch();
    test_arithmetic_type_error();
    test_type_inference();
    test_return_type_mismatch();
    test_if_condition_not_bool();
    test_function_arity_error();
    test_numeric_widening();
    test_logical_op_non_bool();
    test_undefined_variable();
    test_function_arg_type_mismatch();

    std::cout << "typecheck_test: ok\n";
    return 0;
}
