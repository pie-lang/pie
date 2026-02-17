pie
===

The Pie Programming Language

Build status: 
[![Build Status](https://secure.travis-ci.org/pie-lang/pie.svg)]
(https://travis-ci.org/pie-lang/pie)


My playground for programming langauge design & implementation

Languages I will learn from: Ruby, Rust, Haskell

## Features

The features I want to Pie to have:

- general purpose langauge
- user defineable operators
- statical typing
- imperative/functional
- pattern matching
- type safe



With a register-based virtual machine. hopefully I will add other backend one day.


## Debugging Pie programs

Run a program in step-by-step mode with:

```bash
./pie --debug <file.pie>
```

Debugger commands:

- `s`, `step`, `n`, `next`, or empty input: execute the next node
- `c`, `continue`: run until completion
- `p`, `print`: print all visible scopes and variables
- `p <name>`: print one variable from the current scope chain
- `h`, `help`: show debugger command help
- `q`, `quit`: stop execution
