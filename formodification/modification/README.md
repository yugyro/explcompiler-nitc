# Modification compiler

This folder is a working copy of `stage8`. The original `stage8` files are
unchanged. The compiler source is in `ex1/`.

## Operator changes

- **Modulo (`%`)**: Added a lexer token and parser production. The expression
  type checker now treats modulo like the other integer arithmetic operators.
  The code generator already supported `MOD`, so the new grammar can use that
  existing backend implementation.
- **Unary minus (`-b`)**: Added a unary-minus AST node, integer-only semantic
  check, parser production, and code generation. It emits multiplication by
  `-1`.
- **Logical operators (`&&`, `||`, `!`)**: These were already recognized by the
  stage 8 lexer and supported by its parser, type checker, and code generator.
  This copy keeps those implementations and gives AND and OR distinct
  precedence levels.
- **Precedence and associativity**: The assignment token remains declared
  right-associative (assignments are statement-only in this grammar). From low
  to high, OR, AND, comparisons, addition/subtraction,
  multiplication/division/modulo, unary minus/NOT, and member access are
  declared. Binary arithmetic and logical operators associate left;
  comparisons are non-associative; unary operators associate right.

## Build

From `ex1/`, run:

```sh
./compile.sh
./exptree operators_test.txt
```

The script regenerates the parser and lexer and builds the `exptree` compiler.
The sample exercises modulo, unary minus, the logical operators, and arithmetic
versus logical precedence.
