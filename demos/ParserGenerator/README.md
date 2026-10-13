# Empala

Empala is Empirical's parser generator, a companion to
[Emplex](../LexerGenerator). It builds an `emp::Lexer` and `emp::Parser` from a
`.empala` grammar and emits a single, standalone C++ header containing a lexer,
DFA, tokens, and a recursive-descent parser with an AST.

## Build and run

From this directory, with a C++23 compiler:

```sh
make
./Empala example.empala -o parser.hpp
./Empala --help
make test
```

`make native` also builds `Empala`; `make debug` builds `Empala-debug`.
Override the compiler with `make CXX=clang++` if needed. Empirical's cereal
submodule must be available under `../../third-party/cereal/include`.
`make test` requires Python 3 and compiles and runs generated parsers.
`make test-parser` runs the grammar and generated-parser checks without the dice application.

For a small, fully documented application, start with the
[dice example](examples/dice/README.md). Its grammar and C++ handler show how to
connect generated AST actions to application state, including `emp::Random`:

```sh
make dice
./dice 3d6 - 2d4 + 10
make test-dice
```

For the browser version, activate an Emscripten SDK and run:

```sh
make web
# Serve the repository root, so shared Emplex icons are also available.
python3 -m http.server 8000 --directory ../..
```

Open `http://localhost:8000/demos/ParserGenerator/web/Empala.html` and select
**load an example**, or use **Load Grammar** to open `example.empala`.
The sandbox displays tokens and the parsed AST. **Generate C++ Code** enables
copying and downloading the header. The editor supports 100 token rows and
100 rule rows; oversized imports report an error without replacing the tables.

Token Types has its own **Reset**, **Save Lexer**, **Load Lexer**, **Generate C++
Lexer**, and **Open Lexer Sandbox** controls. Reset clears only tokens. Lexer
files use Emplex's `.emplex` format; loading replaces tokens, lexer/token/DFA
class names, and token-storage flags, while preserving grammar rows and parser
settings (including the combined header's filename, namespace, and guards).
The lexer sandbox and lexer-only generation work even with incomplete grammar
rules. Lexer-only output downloads as `lexer.hpp`, with its own include guard.
The lexer sandbox includes Emplex's token info, type, line number, and ignored
text controls.

Above Grammar Rules, **No AST**, **C++ Function**, and **Note** checkboxes control
column visibility. Hiding a column preserves its values and meaning. A row's
No AST checkbox disables its function and note editors; existing values remain
visible and produce an error until you uncheck No AST and clear them.

`make all` builds both interfaces. `make debug-web` (also `make web-debug`)
rebuilds the browser version with debug information and assertions. Use
`make -B web` to switch back to an optimized web build. `make clean` removes
build products. The legacy generated script in `old/` is not used by the build.

## Grammar files

```text
# Token name followed by an Empirical regular expression.
-WHITESPACE [ \t\n\r]+
NUMBER [0-9]+

# First rule is the start rule; @method supplies an optional AST handler method.
>sum @Sum number ('+' number)*
>number @Number NUMBER

# Optional output settings (the same format is saved by the web editor).
:out_filename parser.hpp
:parser_name Parser
:node_name ASTNode
:value_type double
:name_space empala
```

Blank lines and lines beginning with `#` are ignored, including indented
comments. `-` marks ignored tokens. Token priority follows definition order;
put keywords before identifiers. At least one token definition is required,
even if it only ignores whitespace. The CLI supports up to 128 token types,
keeping their IDs separate from ASCII character literals.

Rules reference tokens, other rules, or single quoted character literals.
Use `|` for ordered alternatives, parentheses for grouping, and `?`, `*`, or
`+` for repetition. Repeated rule names append alternatives. Put longer
alternatives before their prefixes. Undefined rules, left recursion, malformed
patterns, and repetition of empty matches are rejected. The generator uses
Empirical's recursive-descent parser; it does not generate YACC/Bison LALR tables.

An optional named token can insert a default leaf: `INTEGER?:1 'd' INTEGER`
accepts both `d6` and `1d6`, with three AST children in either case. The default
is literal token text, not an evaluated expression. `INTEGER?:"1"` is equivalent;
use double quotes for spaces or grammar punctuation. Quoted text supports
`\n`, `\r`, `\t`, `\\`, and `\"`. Bare defaults end at whitespace, a quote,
or one of `()|?*+:`. Do not put spaces between the token, `?:`, and its default.
The text must lex as exactly one non-ignored token of the specified type under
the current lexer's priorities. Defaults are restricted to optional named tokens;
rules, groups, and character literals cannot have defaults. Insertion consumes
no input and uses zero for stored source coordinates. An ordinary `TOKEN?`
continues to produce no child when absent. Inserted defaults are discarded when
an alternative fails; a repetition of a default-only nullable rule is rejected.

### Controlling the AST

A minus sign immediately before an element matches it without keeping its
subtree: `-'{' statement* -'}'` keeps only statements as children. This works
with tokens (`-IF`), rules (`-comment`), groups (`-('x' 'y')`), and repetitions
(`-comment*`). A quoted `'-'` still matches and retains an ordinary minus token.
Suppressed elements must still match; suppression changes the tree, not the
accepted input. A suppressed optional default token contributes no leaf either.

`!no_ast` is the file equivalent of a row's **No AST** checkbox:

```text
>statement !no_ast block | expr_stmt
>block @Block -'{' statement* -'}'
>expr_stmt !no_ast expr -';'
>expr @Add !note="addition" term ('+' term)+
>expr !no_ast term
```

A No AST alternative contributes its one retained child directly to its parent,
or nothing if it retains zero children. Validation rejects alternatives that
could contribute more than one child, including through optional elements,
groups, other No AST rules, or repetitions. The setting applies to that row's
alternatives, not every row with the same rule name. No AST rows cannot have a
function or a nonempty note, because they have no node to carry that information.
Normal rules still produce a node even when all their children are suppressed.

A `!note="text"` annotation identifies the alternative used. Its node has a
`note` string containing that text; other nodes have an empty note. The generated
node class omits the field entirely if no alternatives have notes. Notes use the
same quoted escapes as token defaults. Metadata comes before the pattern;
`@Function`, `!note="text"`, and `!no_ast` may be ordered freely (subject to the
No AST restrictions). Save Grammar preserves these settings and annotations.

A No AST start rule can promote a child to become the root, including a token
leaf. It can also match successfully without producing a root. For generated
parsers, check `parser.LastParseSucceeded()` to distinguish this from failure,
then test the returned pointer before using the tree. Runtime `emp::Parser`
returns an AST wrapper whose Boolean conversion reports success and whose
`HasRoot()` reports whether a tree exists; it warns on success without a root.
The web sandbox shows the same warning.

The [sample language](example.empala) uses separate `+` operator alternatives
and No AST fallbacks so that a lone value is not wrapped at every precedence
level. Punctuation is suppressed; operator tokens remain where handlers need
them. Its retained alternatives also illustrate notes.

All settings below can appear as `:key value` in a grammar or be overridden
using `--set KEY VALUE`. CLI options always take precedence over the file.

| Setting | CLI option | Default |
| --- | --- | --- |
| `out_filename` | `-f`, `-o`, `--output` | `parser.hpp` |
| `parser_name` | `-c`, `--parser` | `Parser` |
| `node_name` | `-a`, `--node` | `ASTNode` |
| `lexer_name` | `-l`, `--lexer` | `Lexer` |
| `dfa_name` | `-d`, `--dfa` | `DFA` |
| `token_name` | `-t`, `--token` | `Token` |
| `name_space` | `-n`, `--namespace` | `empala` |
| `inc_guards` | `-g`, `--guards` | `EMPALA_PARSER_HPP_INCLUDE_` |
| `value_type` | `-V`, `--value-type` | `double` |
| `use_token_lexemes` | `--set use_token_lexemes 0` | `1` |
| `use_token_line_num` | `--set use_token_line_num 0` | `1` |
| `use_token_column` | `--set use_token_column 0` | `1` |
| `show_no_ast` | `--set show_no_ast 0` | `1` |
| `show_functions` | `--set show_functions 0` | `1` |
| `show_notes` | `--set show_notes 1` | `0` |

Booleans accept `0`, `1`, `false`, and `true`. Use `-n ''` for the global
namespace, `-o -` for stdout, and `-v` for progress on stderr. Diagnostics go to
stderr, with nonzero exit status on failure. Invalid grammars do not overwrite
an existing output file. C++ names and value types must be valid in the generated
header; custom value types must be declared before including it.

## Use a generated parser

```cpp
#include "parser.hpp"

int main() {
  empala::Lexer lexer;
  empala::Parser parser;
  const auto & tokens = lexer.Tokenize("int count = 10;");
  auto root = parser.Parse(tokens);
  if (!parser.LastParseSucceeded()) {
    parser.PrintError();
    return 1;
  }
  if (!root) std::cerr << "Parse succeeded without an AST.\n";
}
```

The generated header has no Empirical dependency. Keep the token vector alive
while parsing or printing errors. `Parse()` returns a `std::unique_ptr` to the
root node and requires all tokens to match by default. The AST node type is
nested inside the parser (`empala::Parser::ASTNode` by default).

To evaluate an AST, provide a handler object with a method for every `@method`
used in the grammar, then call `root->Run(handler)`. Each method receives an AST
node reference and returns the configured `value_type`; `node.RunChild(i, handler)`
evaluates a child. The supplied language example names handlers but leaves their
semantics to the application. Parsing and inspecting an AST does not require a
handler. See `tests/test_empala.py` for a complete calculator evaluation example.
