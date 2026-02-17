#include <cassert>
#include <iostream>
#include <sstream>
#include <string>

#include "compiler/ast.h"
#include "compiler/backend/eval.h"

using namespace pie::compiler;

static ModuleNode *buildSimpleModule()
{
    ModuleNode *module = new ModuleNode();
    module->name = "main";

    FunctionNode *main_fn = new FunctionNode("main", 0);
    main_fn->children.push_back(new LetNode("a", nullptr, new IntNode(1)));
    main_fn->children.push_back(new BinaryOpNode(BinaryOp::AddAssign, new IdentifierNode("a"), new IntNode(2)));
    main_fn->children.push_back(new ReturnNode(new IdentifierNode("a")));

    module->functions.push_back(main_fn);
    module->symtab["main"] = main_fn;

    return module;
}

static bool contains(const std::string &haystack, const std::string &needle)
{
    return haystack.find(needle) != std::string::npos;
}

int main()
{
    // Test 1: step once then continue.
    {
        ModuleNode *module = buildSimpleModule();
        EvalVisitor eval;
        eval.setDebugMode(true);

        std::istringstream in("s\nc\n");
        std::ostringstream out;

        std::streambuf *old_in = std::cin.rdbuf(in.rdbuf());
        std::streambuf *old_out = std::cout.rdbuf(out.rdbuf());

        Value result = eval.run(module);

        std::cin.rdbuf(old_in);
        std::cout.rdbuf(old_out);

        assert(result.type == Value::Type::Int);
        assert(result.int_val == 3);

        std::string text = out.str();
        assert(contains(text, "[debug] step"));
        assert(contains(text, "[debug] commands:"));
    }

    // Test 2: print a single variable before stepping.
    {
        ModuleNode *module = buildSimpleModule();
        EvalVisitor eval;
        eval.setDebugMode(true);

        std::istringstream in("s\ns\np a\nc\n");
        std::ostringstream out;

        std::streambuf *old_in = std::cin.rdbuf(in.rdbuf());
        std::streambuf *old_out = std::cout.rdbuf(out.rdbuf());

        Value result = eval.run(module);

        std::cin.rdbuf(old_in);
        std::cout.rdbuf(old_out);

        assert(result.type == Value::Type::Int);
        assert(result.int_val == 3);

        std::string text = out.str();
        assert(contains(text, "[debug] a = 1"));
    }

    std::cout << "debugger_native_test: ok" << std::endl;
    return 0;
}
