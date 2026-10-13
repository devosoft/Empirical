# Compiler

## Generating lexers

`emp::Lexer` can emit independent C++, Python, and JavaScript implementations of
the same DFA-based ASCII lexer. Define tokens once on the lexer, then call the
generator for the file type you are building:

```cpp
emp::Lexer lexer;
lexer.AddToken("NUMBER", "[0-9]+");
lexer.IgnoreToken("SPACE", "[ \\t\\n]+");

emp::CPPFile cpp("lexer.hpp");
lexer.WriteCPP(cpp);
cpp.Write();

emp::PythonFile python("lexer.py");
lexer.WritePython(python);
python.Write();

emp::JSFile javascript("lexer.mjs");
lexer.WriteJS(javascript);
javascript.Write();
```

All three generators accept optional lexer, DFA, and token class names, followed
by three flags controlling storage of lexemes, line numbers, and columns. These
default to `"Lexer"`, `"DFA"`, `"Token"`, and `true` for all three fields.
`WriteCPP` also retains its final `use_emp` flag. Generated code uses the same
transition, acceptance, and trailing-context tables; no regular-expression
library is needed at runtime. `emp::DFA` also exposes `WritePython` and `WriteJS`
for emitting just a DFA class. JavaScript DFA-only output does not add exports;
the lexer generator exports all three classes as an ES module.

Python usage (Python 3.7 or later):

```python
from lexer import Lexer
lexer = Lexer()
tokens = lexer.Tokenize("123 456")
print(tokens[0].lexeme)
```

JavaScript usage (an ES2022-capable browser or Node.js):

```javascript
import { Lexer } from './lexer.mjs';
const lexer = new Lexer();
const tokens = lexer.Tokenize('123 456');
console.log(tokens[0].lexeme);
```

Python accepts a string or a text stream in `Tokenize`; JavaScript accepts a
string. Each call resets tokenization and the token cursor. Tokens expose `id`
and whichever of `lexeme`, `line_id`, and `col_id` were enabled. Lines begin at
one and columns at zero. Both targets provide `NextToken`, `TokenName`,
`IgnoreToken`, `GetNumTokens`, `Any`, `Is`, `Peek`, `Use`, `UseIf`, and `Rewind`.
Python uses `None_()` for the C++/JavaScript `None()` operation, because `None` is
a keyword. Failed `Use` calls raise `RuntimeError` in Python and throw `Error`
in JavaScript. JavaScript tokens are objects: check `token.id` explicitly rather
than relying on object truthiness or strict equality with a number.

Emplex (`demos/LexerGenerator/`) exposes these targets in the output-language
dropdown. Switching targets updates existing output and normal filename
extensions; preview, copy, and download use the same source. Saved `.emplex`
configurations remember the language. Configurations without this setting use
C++. Include guards and namespaces are C++-only options; custom class names and
token-field options apply to all targets.

The native Emplex program accepts `-l cpp`, `-l python`, or `-l javascript`, with
default output names `lexer.hpp`, `lexer.py`, and `lexer.mjs`. Use `-f` to override
the filename. Native command-line options take precedence over browser metadata;
the native loader reads token rules and skips `:` configuration lines.

Run `make -C tests/compiler codegen` to compile and execute generated lexers in
all three languages. This test requires Python 3, Node.js, and a C++23 compiler.
It compares tokenization, positions, EOF metadata, and DFA results across all
eight optional-field combinations, and exercises the token-consumption helpers.

## API

<!-- API TOC -->
<!-- The above comment tells the API generator that this file has API docs. Don't remove it. -->
```{eval-rst}
.. toctree::
   :glob:

   api/*
```
