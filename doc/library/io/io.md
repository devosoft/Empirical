# IO

## Building source files

`emp::CPPFile`, `emp::PythonFile`, and `emp::JSFile` build source files from
language-specific text. They share line storage, indentation, substitution, and
file output through `emp::CodeFileBase<Derived>`. Each file is managed independently;
these classes do not translate code or infer language syntax.

Include the corresponding header from `emp/io/`. All three classes support:

- `AddCode(args...)`: concatenate arguments into one line at the current indentation.
- `AddCodeBlock(lines...)`: add each argument as a separate line.
- `AppendCode(args...)` and `AppendPadding(width, character)`: extend the last line,
  creating one if necessary. Padding counts stored indentation.
- `IncIndent()` and `DecIndent()`: change indentation by the configured step.
  The default step is two spaces for C++ and JavaScript, four for Python.
  An explicit argument specifies a number of spaces; decreasing stops at zero.
  `SetIndentSize(spaces)` changes the default step for subsequent calls.
- `AddHeader(args...)`: add a single comment line before imports/includes and code.
- `AddVarSetting(name, value...)`: define a substitution used in subsequently added code.
- `AddRawCode(args...)` and `AppendRawCode(args...)`: bypass substitution.
- `GetCode()`: inspect the stored body lines, without the file's header or imports.
- `Write(stream)` or `Write()`: output the complete file to a stream or the stored
  filename. `SetFilename(name)` changes that filename; `GetFilename()` retrieves it.
- `Clear()`: remove the filename, header, body, imports/includes, guards, namespace,
  current indentation, and variable settings. Formatting and substitution options
  are retained.

Operations return the concrete file type for chaining. The base uses static
dispatch and is not intended as a runtime-polymorphic interface. Supply braces,
colons, semicolons, and other syntax explicitly. Each code/header call represents
one line; use `AddCodeBlock` for several lines. Whitespace-only body lines render
as empty lines. Stream state can be inspected when using `Write(stream)`; the
filename overload retains `CPPFile`'s existing behavior of not reporting I/O errors.

### C++

`CPPFile` supports `Include(header)`, `SetGuards(name)`, and `SetNamespace(name)`.
Includes are sorted and deduplicated. Bare include paths receive double quotes;
pass `<vector>` to request a system header.

```cpp
emp::CPPFile cpp("generated.hpp");
cpp.AddHeader("Generated C++")
   .SetGuards("GENERATED_HPP")
   .SetNamespace("example")
   .Include("<string>")
   .AddVarSetting("value", 42)
   .AddCode("inline int answer() { return ${value}; }")
   .Write();
```

C++ keeps its existing `${name}` substitution convention and skips double-quoted
text. Its writer also retains the historical two-space prefix on nonblank body
lines, with or without a namespace. `Clear()` now clears substitution variables
along with the other content.

### Python

`PythonFile` provides `Import(module, optional_alias)` and
`ImportFrom(module, names)`. Names are supplied as Python syntax, including commas,
aliases, or `*`. Imports retain insertion order and exact duplicates are omitted;
`GetImports()` exposes the stored declarations. Add `__future__` imports first.
Use `AddCode` for imports that belong inside a function or conditional.

```cpp
emp::PythonFile python("generated.py");
python.AddHeader("Generated Python")
      .ImportFrom("math", "sqrt")
      .AddVarSetting("value", 1764)
      .AddCode("def answer():")
      .IncIndent()
      .AddCode("return int(sqrt(@{value}))")
      .DecIndent()
      .AddCode("")
      .AddCode("print(answer())")
      .Write();
```

### JavaScript

`JSFile` provides ECMAScript module imports. `Import(module)` adds a side-effect
import. `ImportFrom(module, bindings)` accepts default, named, namespace, or
combined bindings as JavaScript syntax. Supply module specifiers without quotes;
the writer quotes and escapes them. Imports retain insertion order with exact
duplicates omitted, and are accessible through `GetImports()`. CommonJS `require`
calls can be supplied directly with `AddCode`.

```cpp
emp::JSFile js("generated.mjs");
js.ImportFrom("node:assert", "{ strict as assert }")
  .AddVarSetting("value", 42)
  .AddCode("const answer = @{value};")
  .AddCode("assert.equal(answer, 42);")
  .AddCode("console.log(`Answer: ${answer}`);")
  .Write();
```

Python and JavaScript use `@{name}` substitutions, including inside quoted text.
This leaves JavaScript's `${expression}` syntax untouched. Substitution is textual
and does not escape values as language literals. Use `AddRawCode`/`AppendRawCode`
to bypass it, `SetVarSymbol("")` to disable it, or `SetVarSymbol(marker)` to change
the marker. `SetVarQuotes(characters)` configures simple quote skipping within a
line or appended fragment; it is not a parser for multiline strings or language
expressions. Substitution runs only when at least one variable is defined;
unrecognized placeholders then follow `String::SetReplaceVars`' error behavior.
Headers and import declarations do not undergo substitution.

See `examples/io/CodeFile.cpp` for a runnable generator that writes an independent
program for each language.

## API

<!-- API TOC -->
<!-- The above comment tells the API generator that this file has API docs. Don't remove it. -->
```{eval-rst}
.. toctree::
   :glob:

   api/*
```
