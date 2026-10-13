# Dice: connecting an Empala parser to your code

This small command-line program rolls dice and adds or subtracts whole numbers.
Build and run it from `demos/ParserGenerator`:

```sh
make dice
./dice 3d6 - 2d4 + 10
./dice d6 + d4          # Omitted counts default to one die.
./dice "3d6-2d4+10"     # Quoting the whole expression also works.
./dice 20 - 5 - 3       # Always prints 12: operations apply left to right.
./dice 3d1 - 2d1 + 10   # Always prints 11: useful for checking the wiring.
./dice 42               # Standalone whole numbers are valid terms.
make test-dice
```

Successful runs print one integer. Invalid input produces a diagnostic and a
nonzero exit status. Dice rolls vary from run to run.

## The three pieces

1. **[dice.empala](dice.empala) defines syntax.** `INTEGER` recognizes digits;
   whitespace is ignored. The first rule, `expression`, matches a term followed
   by any number of operator/term pairs. A term is either `INTEGER?:1 'd' INTEGER`
   or just `INTEGER`. Put the dice alternative first because alternatives are
   tried in order and a plain integer is also the beginning of a dice term.
2. **Empala generates `build/dice_parser.hpp`.** `make dice` first builds Empala,
   runs `./Empala examples/dice/dice.empala -o build/dice_parser.hpp`, and then
   compiles the driver with `-Ibuild`. Changing the grammar regenerates the
   header automatically. Do not edit the generated header.
3. **[dice.cpp](dice.cpp) supplies behavior.** The driver includes that header,
   tokenizes the joined command-line arguments, parses the tokens, and passes a
   `DiceHandler` object to `root->Run(handler)`. The handler owns no parser state;
   it simply holds a reference to the application's `emp::Random` instance.

The generated header has no Empirical dependency. This application itself uses
`emp::Random` for rolls and `emp::String` to assemble command-line arguments.
Empala is a build-time tool; the `dice` executable does not invoke it at runtime.

## How grammar actions become C++ calls

The `@` names in the grammar are methods on your handler object:

| Grammar action | C++ method | AST children | Result |
| --- | --- | --- | --- |
| `@Number` | `DiceHandler::Number` | `INTEGER` | Convert the token's text to a number |
| `@Roll` | `DiceHandler::Roll` | `INTEGER`, `'d'`, `INTEGER` | Sum X rolls, each in 1..Y |
| `@Sum` | `DiceHandler::Sum` | term, operator, term, ... | Evaluate terms and accumulate left to right |

For `3d6 - 2d4 + 10`, the expression node's children are:

```text
child index:   0       1      2       3      4
             term    '-'   term    '+'   term
              3d6           2d4           10
```

`node.RunChild(i, *this)` evaluates a child rule using the same handler.
`node.GetChildLexeme(i)` reads a child token's original text. Literal characters
such as `d`, `+`, and `-` are tokens too, so they occupy child positions.
Parenthesized groups in the grammar are flattened into their parent's children.

`INTEGER?:1` inserts an `INTEGER` leaf with lexeme `"1"` when the count is
absent. Thus `d6` and `1d6` have identical child layouts and use the same `Roll`
method without a conditional. Defaults supply token text, validated against the
lexer, and consume no input. Synthetic source coordinates are zero. See the
commented grammar for quoted defaults and restrictions.

The parser builds the entire AST before any handler methods run. On success,
`root` owns the AST; on failure it is null, and `parser.PrintError()` explains
where parsing stopped. Keep the lexer's token vector alive while parsing and
printing errors. See the three numbered steps in `main()` for this integration.

`:value_type long long` selects the type returned by every handler method.
`Sum` deliberately uses a loop, so `20 - 5 - 3` means `(20 - 5) - 3`.
No precedence levels or left-recursive rules are needed. The grammar does not
include unary signs, parentheses in input, or multiplication.
An omitted count, as in `d6`, defaults to one die via `INTEGER?:1`. Results may be negative even though literal
numbers are nonnegative.

The handler also demonstrates checks that belong in application code: a die must
have 1..1,000,000 sides, and each dice term may roll 0..100,000 dice. Zero dice
produce zero. Oversized integer literals and arithmetic overflow are reported.
These limits keep the example quick to run and its dice subtotals in range.

For repeatable rolls while experimenting, change `emp::Random random;` to
`emp::Random random(42);` in `main()`. A new evaluation calls the handler again
and rolls again; the AST does not cache the random results.
