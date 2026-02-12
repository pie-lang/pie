# CLAUDE.md - AI Assistant Guide for Pie Programming Language

This document provides a comprehensive guide for AI assistants working on the Pie programming language codebase.

## Project Overview

**Pie** is an experimental general-purpose programming language designed for learning language implementation. It combines imperative and functional programming paradigms with static typing, pattern matching, and user-definable operators.

- **Language**: C++ (with Flex/Bison for lexer/parser)
- **Author**: Reeze Xia (reeze.xia@gmail.com)
- **License**: MIT (2014)
- **Version**: 0.0.1-dev (Early Development)
- **Status**: Working interpreter, VM backend planned but not implemented

**Design Goals** (from README.md):
- General purpose language
- User definable operators
- Static typing
- Imperative/functional hybrid
- Pattern matching
- Type safe
- Register-based virtual machine (planned)

## Architecture Overview

### High-Level Architecture

```
┌─────────────────┐
│  Pie Source     │
│  (.pie files)   │
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│  Lexer          │  (Flex-based: compiler/lexer.ll)
│  Tokenization   │
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│  Parser         │  (Bison-based: compiler/parser.y)
│  AST Generation │
└────────┬────────┘
         │
         ▼
┌─────────────────────────────────┐
│  Backend Selection              │
│  ┌────────────┐  ┌────────────┐│
│  │PrintVisitor│  │EvalVisitor ││
│  │(--print)   │  │(default)   ││
│  └────────────┘  └────────────┘│
└─────────────────────────────────┘
         │
         ▼
┌─────────────────┐
│  Output/Execute │
└─────────────────┘
```

### Key Design Patterns

1. **Visitor Pattern**: Core pattern for AST traversal
   - Base class: `Visitor` (compiler/ast/visitor.h)
   - AST nodes implement: `void visit(Visitor*)`
   - Concrete visitors: `EvalVisitor`, `PrintVisitor`
   - Easy to add new backends without modifying AST

2. **Namespace Organization**:
   - `pie::compiler` - Frontend (lexer, parser, AST)
   - `pie::runtime` - Runtime infrastructure (stubs)
   - `pie::vm` - Virtual machine (stubs)

3. **Environment Chain**: Lexical scoping implementation
   - `Environment` class with parent pointer
   - Variable lookup walks parent chain
   - New scope = new `Environment` with current as parent

## Directory Structure

```
/home/user/pie/
├── compiler/              # Compiler frontend
│   ├── ast/              # Abstract Syntax Tree node definitions (17 node types)
│   │   ├── node.h        # Base Node class and Visitor interface
│   │   ├── module.h      # Module declaration
│   │   ├── function.h    # Function & closure definitions
│   │   ├── primitive.h   # Int, Double, String, Identifier
│   │   ├── op.h          # Binary/unary operators
│   │   ├── type.h        # Type annotations
│   │   ├── let.h         # Variable declarations
│   │   ├── assign.h      # Assignment statements
│   │   ├── return.h      # Return statements
│   │   ├── if.h          # Conditionals
│   │   ├── import.h      # Module imports
│   │   ├── block.h       # Code blocks
│   │   └── visitor.h     # Visitor interface
│   ├── backend/          # Code generation & execution
│   │   ├── eval.cpp/h    # Tree-walking interpreter (253 LOC)
│   │   ├── print.cpp/h   # AST pretty printer
│   │   └── vm.cpp/h      # VM backend (stub, not implemented)
│   ├── lexer.ll          # Flex lexer specification (~130 LOC)
│   ├── parser.y          # Bison grammar (~445 LOC)
│   ├── parser.cpp/h      # Parser implementation
│   ├── scanner.h         # Token scanner interface
│   └── ast.h             # AST header aggregator
├── runtime/              # Runtime infrastructure (mostly stubs)
│   ├── vm/
│   │   ├── bytecode.h    # Bytecode definitions (not implemented)
│   │   ├── interpreter.cpp/h  # Bytecode interpreter (empty)
│   │   └── vm.cpp        # VM implementation (empty)
│   ├── function.h        # Function runtime (stub)
│   └── module.h          # Module runtime (stub)
├── main/                 # Main executable
│   └── pie.cpp           # CLI entry point (92 LOC)
├── lib/                  # Standard library (stubs)
│   ├── prelude.pie       # Built-in library
│   └── std/io.pie        # I/O module
├── tests/                # Test suite
│   └── lang-001.pie      # First test case (49 LOC)
├── doc/                  # Documentation
│   └── module-system.md  # (empty stub)
├── vendor/               # Third-party dependencies
│   └── gtest-1.7.0/      # Google Test framework
├── CMakeLists.txt        # Root build configuration
└── .travis.yml           # CI configuration
```

## Critical Implementation Files

### Compiler Frontend

| File | Lines | Purpose | Key Details |
|------|-------|---------|-------------|
| `compiler/lexer.ll` | ~130 | Tokenizer | Flex specification; recognizes keywords, operators, literals, comments |
| `compiler/parser.y` | ~445 | Parser | Bison LALR(1) grammar; builds AST from tokens |
| `compiler/parser.cpp` | ~113 | Parser driver | Coordinates lexer and parser; creates ModuleNode |

**Supported Language Features**:
- **Keywords**: `fn`, `if`, `else`, `return`, `module`, `import`, `let`, `public`
- **Operators**: Arithmetic (`+`, `-`, `*`, `/`, `%`), Comparison (`==`, `!=`, `<`, `>`, `<=`, `>=`), Logical (`&&`, `||`), Increment/Decrement (`++`, `--`), Compound Assignment (`+=`, `-=`)
- **Comments**: Single-line (`#`) and block (`{# ... #}`)
- **Literals**: Integers, floats, strings (with escape sequences)

### AST Nodes (compiler/ast/)

17 node types organized by category:

**Declarations**:
- `ModuleNode` - Module declaration
- `FunctionNode` - Function definition with params, return type, body
- `ImportNode` - Module imports (supports `*` and selective imports)
- `LetNode` - Variable declaration with optional type annotation
- `TypeNode` - Type annotations (int, double, string, arrays)

**Expressions**:
- `IntNode`, `DoubleNode`, `StringNode` - Literals
- `IdentifierNode` - Variable/function references (supports dot notation like `io.print`)
- `BinaryOpNode` - Binary operations
- `UnaryOpNode` - Unary operations (-, !)
- `FunctionCallNode` - Function invocation

**Statements**:
- `AssignNode` - Assignment (supports compound like `+=`)
- `ReturnNode` - Return statement
- `IfNode` - Conditional (if/else if/else chains)
- `BlockNode` - Statement blocks
- `ClosureNode` - Lambda/closure (AST support exists, not fully implemented)

### Backend: Tree-Walking Interpreter (compiler/backend/eval.cpp/h)

**Value System** (eval.h:15-122):
```cpp
struct Value {
    enum class Type {
        Nil, Int, Double, Bool, String, Function, BuiltinFunction
    };
    // Factory methods: makeInt(), makeDouble(), makeBool(), makeString(), etc.
    // Conversion methods: toInt(), toDouble(), toBool(), toString()
};
```

**Environment (Lexical Scoping)** (eval.h:132-174):
- Parent-pointer chain for scope nesting
- Methods: `define()`, `get()`, `set()`, `has()`
- Throws `std::runtime_error` on undefined variables

**Built-in Functions** (eval.cpp):
- `print(val)` - Print to stdout
- `io.print(val)` - Same as print
- `exit(code)` - Exit with code
- `len(str)` - String length
- `type(val)` - Get type name

**Control Flow**:
- Uses C++ exception (`ReturnException`) for return statements
- Falls through naturally for other control structures

### Main Entry Point (main/pie.cpp)

**Command-line Interface**:
```bash
pie [options] <file.pie>
Options:
  --print    # Print AST instead of executing
  --help     # Show usage
```

**Execution Flow** (main/pie.cpp:21-92):
1. Parse arguments (handle `--print`, `--help`)
2. Open source file
3. Create `Scanner` from file
4. Create `Parser` with scanner
5. Call `parser.parse()` - returns 0 on success
6. Get `ModuleNode* module = parser.module`
7. If `--print`: Use `PrintVisitor` to output AST
8. Else: Use `EvalVisitor` to execute
9. Main function's return value becomes exit code

**Error Codes**:
- 0: Success
- 1: File not found or invalid arguments
- 2: Parse error
- 3: Runtime error

## Build System

**CMake Configuration**:
- Minimum version: 2.8.7
- Package: `pie` version `0.0.1-dev`
- Build targets:
  - Static library: `pie_compiler`
  - Static library: `pie_runtime`
  - Executable: `pie` (in `main/` directory)

**Build Commands**:
```bash
# Configure and build
cmake .
make

# Clean build
rm -rf CMakeFiles CMakeCache.txt
cmake .
make

# Run compiler
./main/pie tests/lang-001.pie
./main/pie --print tests/lang-001.pie
```

**Dependencies**:
- Flex (lexical analyzer generator)
- Bison (parser generator)
- C++ compiler with C++11 support
- CMake 2.8.7+
- Google Test 1.7.0 (vendored in `vendor/`)

**Generated Files** (not in git):
- `compiler/lexer.yy.cpp` - Generated lexer
- `compiler/parser.tab.cpp` - Generated parser
- `compiler/parser.tab.hpp` - Parser header

## Development Workflows

### Adding a New Language Feature

**Example: Adding a `while` loop**

1. **Update Lexer** (compiler/lexer.ll):
   ```flex
   "while"     { return TOKEN_WHILE; }
   ```

2. **Update Parser** (compiler/parser.y):
   ```yacc
   %token TOKEN_WHILE

   statement: /* ... */
       | TOKEN_WHILE '(' expression ')' block {
           $$ = new WhileNode($3, $5);
       }
   ```

3. **Create AST Node** (compiler/ast/while.h):
   ```cpp
   class WhileNode : public Node {
   public:
       Node *condition;
       BlockNode *body;

       void visit(Visitor *v) override { v->visit(this); }
   };
   ```

4. **Update Visitor Interface** (compiler/ast/node.h):
   ```cpp
   #define AST_NODES \
       AST_NODE(While) \
       /* ... existing nodes ... */
   ```

5. **Implement Evaluation** (compiler/backend/eval.cpp):
   ```cpp
   void EvalVisitor::visit(WhileNode *node) {
       while (evaluate(node->condition).toBool()) {
           evaluate(node->body);
       }
       result = Value::makeNil();
   }
   ```

6. **Implement Printing** (compiler/backend/print.cpp):
   ```cpp
   void PrintVisitor::visit(WhileNode *node) {
       output << "while (" << visit(node->condition)
              << ") " << visit(node->body);
   }
   ```

### Testing Changes

**Current Test Setup**:
- Minimal: Only `tests/lang-001.pie` exists
- No automated test harness yet
- Manual testing required

**Testing Process**:
```bash
# Build
cmake . && make

# Run test file
./main/pie tests/lang-001.pie
echo $?  # Check exit code

# Inspect AST
./main/pie --print tests/lang-001.pie

# Create new test
cat > tests/my-test.pie << 'EOF'
module test
fn main() : int {
    return 0
}
EOF
./main/pie tests/my-test.pie
```

### Debugging Tips

1. **Parser Issues**:
   - Check Bison conflicts: Look for shift/reduce warnings during build
   - Use `--print` to see if AST matches expectations
   - Add debug output in `parser.y` actions

2. **Runtime Issues**:
   - Add `std::cerr` debug output in `eval.cpp`
   - Check exception messages (caught in `main/pie.cpp:86`)
   - Verify environment chain for scope issues

3. **Lexer Issues**:
   - Add debug output in `lexer.ll` actions
   - Use a simple test case to isolate token recognition
   - Check regex patterns for conflicts

## Code Conventions

### Naming Conventions

- **Classes**: PascalCase with descriptive suffixes
  - AST nodes: `*Node` (e.g., `FunctionNode`, `BinaryOpNode`)
  - Visitors: `*Visitor` (e.g., `EvalVisitor`, `PrintVisitor`)
  - Exceptions: `*Exception` (e.g., `ReturnException`)

- **Files**: snake_case
  - Headers: `.h` extension
  - Implementation: `.cpp` extension
  - Flex/Bison: `.ll` and `.y` extensions

- **Variables**: snake_case
  - Member variables: No prefix (e.g., `result`, `env`)
  - Function parameters: Descriptive names (e.g., `node`, `module`)

### Code Style

- **Indentation**: 4 spaces (no tabs in C++ code)
- **Braces**: K&R style (opening brace on same line)
  ```cpp
  if (condition) {
      // code
  } else {
      // code
  }
  ```

- **Includes**: System headers first, then project headers
  ```cpp
  #include <string>
  #include <vector>

  #include "compiler/ast.h"
  ```

- **Namespaces**: Always use `pie::compiler` or `pie::runtime`
  ```cpp
  namespace pie { namespace compiler {
      // code
  }}
  ```

### Error Handling

- **Parse Errors**: Return non-zero from `parser.parse()`
- **Runtime Errors**: Throw `std::runtime_error` with descriptive message
- **Return Statements**: Use `ReturnException` for control flow
- **User Errors**: Always provide context in error messages

### Memory Management

- **AST Nodes**: Raw pointers (parser owns root, nodes own children)
- **No explicit cleanup**: Relies on process termination (acceptable for simple interpreter)
- **Future**: Consider smart pointers or arena allocation for production

## Pie Language Syntax Reference

### Module Structure

```pie
module <name>

import <module>.*              # Import all
import <module>                # Import module
public import <module>         # Re-export module

fn main(argc, argv : string[]) : int {
    return 0
}
```

### Functions

```pie
# Function with parameters and return type
fn add(x : int, y : int) : int {
    return x + y
}

# Public function (exported from module)
public fn helper() {
    # No explicit return type means void
}

# Function calls
add(10, 20)
io.print("hello")
```

### Variables

```pie
let x = 10              # Type inferred
let y : int = 20        # Type explicit
let z : string[] = []   # Array type
```

### Control Flow

```pie
if (condition) {
    # code
} else if (other_condition) {
    # code
} else {
    # code
}
```

### Operators

```pie
# Arithmetic
x + y, x - y, x * y, x / y, x % y

# Comparison
x == y, x != y, x < y, x > y, x <= y, x >= y

# Logical
x && y, x || y, !x

# Assignment
x = value
x += value  # Compound assignment
x -= value

# Increment/Decrement (not fully tested)
x++, x--, ++x, --x
```

### Comments

```pie
# Single-line comment

{#
   Block comment
   Can span multiple lines
#}
```

## Things to Know

### What Works Well

1. **Basic interpreter is functional**: Can execute simple programs
2. **Module system**: Imports and namespace resolution work
3. **Lexer/Parser**: Stable, handles most language constructs
4. **AST design**: Clean visitor pattern, easy to extend
5. **Error messages**: Basic but functional

### Known Limitations

1. **No static type checking**: Types are annotations only, not enforced
2. **No garbage collection**: Memory grows unbounded
3. **No standard library**: Only 5 built-in functions
4. **No closures**: `ClosureNode` exists but not implemented in evaluator
5. **No pattern matching**: Planned but not started
6. **No user-defined operators**: Planned but not started
7. **No VM backend**: Stubs exist but no implementation
8. **Minimal error recovery**: Parser fails on first error
9. **No REPL**: Must execute files
10. **No debugger**: No step-through or breakpoint support

### What NOT to Do

1. **Don't modify generated files**: Never edit `lexer.yy.cpp` or `parser.tab.cpp` - they're regenerated on build
2. **Don't break the visitor pattern**: All AST operations should use visitors
3. **Don't add features without tests**: Even though test infrastructure is minimal, create test files
4. **Don't skip error handling**: Always validate and provide good error messages
5. **Don't assume types are checked**: The type system is purely documentary right now
6. **Don't use tabs in C++ code**: Use 4 spaces for indentation
7. **Don't commit build artifacts**: CMake output, binaries, generated files stay out of git

## Common Tasks

### Adding a Built-in Function

Edit `compiler/backend/eval.cpp`, find `EvalVisitor::registerBuiltins()`:

```cpp
void EvalVisitor::registerBuiltins() {
    // Existing builtins...

    // Add new builtin
    global_env.define("myFunction", Value::makeBuiltin(
        [](std::vector<Value> &args) -> Value {
            if (args.size() != 1) {
                throw std::runtime_error("myFunction expects 1 argument");
            }
            // Implementation here
            return Value::makeNil();
        }
    ));
}
```

### Adding an Operator

1. Add token to lexer (compiler/lexer.ll)
2. Add token declaration to parser (compiler/parser.y)
3. Add production rule to parser
4. Handle in `BinaryOpNode` or `UnaryOpNode`
5. Implement evaluation in `EvalVisitor::visit(BinaryOpNode*)` or `visit(UnaryOpNode*)`

### Fixing a Parser Bug

1. Identify which production rule is involved
2. Check operator precedence in parser.y
3. Look for shift/reduce or reduce/reduce conflicts
4. Test with `--print` to verify AST structure
5. Add test case to prevent regression

### Understanding Execution Flow

```
main/pie.cpp:main()
    → Scanner(file)
    → Parser(scanner).parse()
    → parser.module (ModuleNode*)
    → EvalVisitor.run(module)
        → registerBuiltins()
        → evaluate module's functions
        → call main() function
        → return main's return value
```

## Future Development

### Planned Features (from README)

- **VM Backend**: Register-based bytecode interpreter
- **Pattern Matching**: Match expressions for control flow
- **User-defined Operators**: Custom operator definitions
- **Type Safety**: Static type checking and inference
- **Standard Library**: Comprehensive built-in functions

### Architecture Evolution

- **Current**: Tree-walking interpreter (simple but slow)
- **Next**: Bytecode compiler + VM (better performance)
- **Future**: JIT compilation, optimization passes

### Areas Needing Work

1. **Test Infrastructure**: Need comprehensive test suite with test runner
2. **Error Messages**: More helpful diagnostics with line numbers, context
3. **Type System**: Implement actual type checking
4. **Standard Library**: File I/O, strings, collections, etc.
5. **Documentation**: Language specification, API docs
6. **Tooling**: Syntax highlighting, LSP, debugger
7. **Performance**: Optimize hot paths, consider caching
8. **Memory**: Add garbage collection or better ownership model

## CI/CD

**Travis CI Configuration** (.travis.yml):
- Language: C++
- Dependencies: cmake (via apt)
- Build: `cmake . && make`
- Notifications: Email to reeze.xia@gmail.com
- No tests run yet (just build verification)

## Getting Help

- **Bug Reports**: https://github.com/pie-lang/pie/issues
- **Author**: reeze.xia@gmail.com
- **License**: MIT (see LICENSE file)

## Quick Reference for AI Assistants

### When Asked to Add Features:

1. Start with lexer if new syntax needed
2. Update parser grammar
3. Create or modify AST node
4. Implement visitor methods (eval and print)
5. Test with a simple .pie file
6. Update this documentation if significant

### When Debugging:

1. Check parser output with `--print`
2. Add debug output in eval.cpp
3. Verify AST structure matches expectations
4. Check environment chain for scope issues
5. Look for exception messages in output

### When Refactoring:

1. Follow existing patterns (visitor, namespaces)
2. Keep AST nodes simple (data only)
3. Put logic in visitors
4. Maintain separation between frontend and backend
5. Update CMakeLists.txt if adding new files

### Important Context:

- This is a **learning project** - educational clarity over production robustness
- The VM backend is **planned but not implemented** - focus on interpreter
- Type annotations are **not enforced** - they're documentation only
- Error handling is **basic** - improvements welcome but keep it simple
- Memory management is **intentionally simple** - no need for complex RAII yet

---

**Last Updated**: 2026-02-12
**Codebase Version**: 0.0.1-dev
**Total Files**: 34 source files, ~2,484 lines of code
