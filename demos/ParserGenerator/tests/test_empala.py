"""End-to-end CLI checks, including compilation and execution of generated code."""
import pathlib
import shlex
import subprocess
import sys
import tempfile

empala = pathlib.Path(sys.argv[1]).resolve()
cxx = shlex.split(sys.argv[2])
example = pathlib.Path(__file__).resolve().parents[1] / "example.empala"

with tempfile.TemporaryDirectory(prefix="empala-test-") as directory:
    work = pathlib.Path(directory)

    def run(*args, ok=True):
        result = subprocess.run([str(empala), *map(str, args)], cwd=work,
                                text=True, capture_output=True)
        assert result.returncode == (0 if ok else 1), (args, result.stdout, result.stderr)
        return result

    def compile_run(source):
        (work / "driver.cpp").write_text(source)
        subprocess.run([*cxx, "-std=c++23", "driver.cpp", "-o", "driver"], cwd=work, check=True)
        subprocess.run([str(work / "driver")], cwd=work, check=True)

    assert "Usage: Empala" in run("--help").stdout
    run(ok=False)
    run("--unknown", ok=False)
    run("--output", ok=False)
    run("missing.empala", ok=False)
    run(example, example, ok=False)
    run("-o", work / "missing" / "parser.hpp", example, ok=False)
    run("-o", example, example, ok=False)
    run("-v", example)
    compile_run('''#include "parser.hpp"
int main() {
  empala::Lexer lexer;
  empala::Parser parser;
  auto ast = parser.Parse(lexer.Tokenize(
    "int countdown = 10; while (countdown > 0) { print(countdown); "
    "countdown = countdown - 1; } print(\\\"Boom!\\\");"));
  if (!ast) return 1;
  if (parser.Parse(lexer.Tokenize("int = ;"))) return 2;
  return 0;
}
''')
    grammar = work / "calc.empala"
    grammar.write_text("""  # Comments may be indented.
:out_filename from-file.hpp
:name_space from_file
-WHITESPACE [ \\t\\n\\r]+
NUMBER [0-9]+
>expr @Sum number ('+' number)*
>number @Number NUMBER
""")
    result = run(grammar, "-o", "custom.hpp", "-c", "Calc", "-a", "Node",
                 "-l", "Scanner", "-d", "Machine", "-t", "Lexeme", "-n", "demo::calc",
                 "-g", "CALC_GENERATED_HPP", "-V", "long", "-v")
    assert "2 tokens and 2 rule rows" in result.stderr
    assert not (work / "from-file.hpp").exists()
    compile_run('''#include "custom.hpp"
#include "custom.hpp"
#include <string>
struct Handler {
  long Number(demo::calc::Calc::Node & n) { return std::stol(n.GetChildLexeme(0)); }
  long Sum(demo::calc::Calc::Node & n) {
    long result = 0;
    for (size_t i = 0; i < n.NumChildren(); i += 2) result += n.RunChild(i, *this);
    return result;
  }
};
int main() {
  demo::calc::Scanner lexer;
  demo::calc::Calc parser;
  Handler handler;
  auto ast = parser.Parse(lexer.Tokenize("12 + 30"));
  if (!ast || ast->Run(handler) != 42) return 1;
  return parser.Parse(lexer.Tokenize("12 +")) ? 2 : 0;
}
''')
    # Every combination of optional token data must produce compilable code.
    for mask in range(8):
        flags = []
        for bit, field in enumerate(("lexemes", "line_num", "column")):
            flags += ["--set", "use_token_" + field, str((mask >> bit) & 1)]
        run(grammar, "-o", "minimal.hpp", "-n", "", *flags)
        compile_run('''#include "minimal.hpp"
int main() { Lexer lexer; Parser parser; return parser.Parse(lexer.Tokenize("42")) ? 0 : 1; }
''')
    assert "namespace from_file" in run(grammar, "-o", "-", "-v").stdout
    run(grammar, "--set", "use_token_lexemes", "maybe", ok=False)
    run(grammar, "--set", "unknown", "1", ok=False)
    run(grammar, "-c", "bad-name", ok=False)
    run(grammar, "-c", "Lexer", ok=False)
    run(grammar, "-n", "bad::", ok=False)
    run(grammar, "-V", "", ok=False)
    linked = work / "grammar-link.empala"
    linked.symlink_to(grammar)
    run(grammar, "-o", linked, ok=False)
    many = work / "many.empala"
    many.write_text("".join(f"TOKEN{i} x\n" for i in range(129)) + ">root TOKEN0\n")
    run(many, ok=False)

    # Defaults must preserve the AST shape even when optional Token fields are omitted.
    defaults = work / "defaults.empala"
    defaults.write_text("NUMBER [0-9]+\n>root (NUMBER?:7 'x') | NUMBER?:2\n")
    for mask in range(8):
        flags = []
        for bit, field in enumerate(("lexemes", "line_num", "column")):
            flags += ["--set", "use_token_" + field, str((mask >> bit) & 1)]
        run(defaults, "-o", "defaults.hpp", "-n", "", *flags)
        checks = ""
        if mask & 1:
            checks += 'if (leaf.lexeme != "7") return 3;\n'
        if mask & 2:
            checks += 'if (leaf.line_id != 0) return 4;\n'
        if mask & 4:
            checks += 'if (leaf.col_id != 0) return 5;\n'
        compile_run('''#include "defaults.hpp"
int main() {
  Lexer lexer;
  Parser parser;
  const auto & tokens = lexer.Tokenize("x");
  auto ast = parser.Parse(tokens);
  if (!ast || ast->NumChildren() != 2 || tokens.size() != 1) return 1;
  const auto & leaf = ast->GetChild(0).token;
  if (leaf.id != Lexer::ID_NUMBER || !ast->GetChild(0).IsLeaf()) return 2;
''' + checks + '''
  ast = parser.Parse(lexer.Tokenize("3x"));
  if (!ast || ast->NumChildren() != 2) return 6;
  ast = parser.Parse(lexer.Tokenize("3"));
  if (!ast || ast->NumChildren() != 1) return 7;
  ast = parser.Parse(lexer.Tokenize(""));
  if (!ast || ast->NumChildren() != 1) return 8;
  if (parser.Parse(lexer.Tokenize("x!"))) return 9;
  return 0;
}
''')

    # Quoted defaults survive code generation, including escapes and grammar punctuation.
    defaults.write_text('TEXT (.|\\n|\\r)+\n>root TEXT?:"a\\\"b\\\\c\\t\\n\\r |()?:+"\n')
    run(defaults, "-o", "quoted.hpp")
    compile_run(r'''#include "quoted.hpp"
int main() {
  empala::Lexer lexer;
  empala::Parser parser;
  auto ast = parser.Parse(lexer.Tokenize(""));
  return !ast || ast->GetChildLexeme(0) != "a\"b\\c\t\n\r |()?:+";
}
''')
    for pattern in ["NUMBER?:", 'NUMBER?:""', 'NUMBER?:"7', 'NUMBER?:"\\q"',
                    "NUMBER?:x", 'NUMBER?:"7 8"', "NUMBER:7", "NUMBER*:7",
                    "NUMBER+:7", "'x'?:x", "child?:7", "(NUMBER)?:7", "(NUMBER?:7)*"]:
        defaults.write_text("NUMBER [0-9]+\n>root " + pattern + "\n")
        (work / "protected.hpp").write_text("keep this output")
        result = run(defaults, "-o", "protected.hpp", ok=False)
        assert result.stderr, pattern
        assert (work / "protected.hpp").read_text() == "keep this output"

    # Prune punctuation, whole subtrees, groups, and inserted defaults. Only an
    # actual binary operation gets a wrapper; a single operand becomes the root.
    grammar.write_text(r'''-SPACE [ ]+
NUMBER [0-9]+
>start !no_ast expression
>expression @Subtract !note="sub\"tract\nline" atom ('-' atom)+
>expression !no_ast atom
>atom !no_ast -'(' expression -')'
>atom @Number !note="number" NUMBER
>atom !no_ast -'!' -NUMBER?:1
''')
    run(grammar, "-o", "pruned.hpp")
    compile_run(r'''#include "pruned.hpp"
#include <cassert>
struct Handler {
  double Number(empala::Parser::ASTNode & n) { return std::stod(n.GetChildLexeme(0)); }
  double Subtract(empala::Parser::ASTNode & n) {
    double v = n.RunChild(0, *this);
    for (size_t i=2; i<n.NumChildren(); i+=2) v -= n.RunChild(i, *this);
    return v;
  }
};
int main() {
  empala::Lexer l; empala::Parser p; Handler h;
  auto root = p.Parse(l.Tokenize("(9)-2-1"));
  assert(p.LastParseSucceeded() && root);
  assert(root->note == "sub\"tract\nline" && root->NumChildren() == 5);
  assert(root->GetChild(0).note == "number");
  assert(root->Run(h) == 6);
  root = p.Parse(l.Tokenize("(7)"));
  assert(root && root->note == "number" && root->Run(h) == 7);
  root = p.Parse(l.Tokenize("!"));
  assert(!root && p.LastParseSucceeded());
  root = p.Parse(l.Tokenize("!3"));
  assert(!root && p.LastParseSucceeded());
  root = p.Parse(l.Tokenize("!x"));
  assert(!root && !p.LastParseSucceeded());
  root = p.Parse(l.Tokenize("(4)"));
  assert(root && p.LastParseSucceeded());
}
''')
    grammar.write_text("""-SPACE [ ]+
NUMBER [0-9]+
>start !no_ast -'{' -discarded+ -('x' 'y') -NUMBER?:1 keep -'}'
>discarded NUMBER
>keep !no_ast empty* NUMBER
>empty !no_ast -'!'
""")
    run(grammar, "-o", "no-notes.hpp")
    assert " note" not in (work / "no-notes.hpp").read_text()
    compile_run('''#include "no-notes.hpp"
#include <cassert>
int main() {
  empala::Lexer l; empala::Parser p;
  auto root = p.Parse(l.Tokenize("{1 2 xy !!!7}"));
  assert(root && root->IsLeaf() && root->GetLexeme() == "7");
  assert(!p.Parse(l.Tokenize("{1 x !!!7}")) && !p.LastParseSucceeded());
}
''')
    for rows in [
        ">root !no_ast NUMBER NUMBER?", ">root !no_ast (NUMBER 'x')?",
        ">root !no_ast NUMBER*", ">root !no_ast NUMBER+",
        ">root !no_ast child NUMBER\n>child !no_ast NUMBER",
        ">root !no_ast child*\n>child !no_ast NUMBER",
        ">root !no_ast -'(' root -')' NUMBER? | NUMBER",
        ">root !no_ast @Number NUMBER", '>root !no_ast !note="x" NUMBER',
        '>root !note="unterminated NUMBER', '>root !note="\\q" NUMBER',
        '>root !note=unquoted NUMBER', '>root !note="x"NUMBER',
        ">root !unknown NUMBER", ">root - NUMBER", ">root -",
    ]:
        grammar.write_text("NUMBER [0-9]+\n" + rows)
        result = run(grammar, "-o", "protected.hpp", ok=False)
        assert result.stderr, rows

    bad_cases = [
        "", "NUMBER [0-9]+", "NUMBER [0-9]+\nNUMBER [a-z]+\n>root NUMBER",
        "BAD\n>root BAD", "1BAD x\n>root 'x'", "BAD [\n>root BAD",
        ">root unknown", ">root root 'x' | 'x'", ">root ('x'?)*",
        ">root ('x'", ">root @bad-name 'x'", ">root @", ">root",
        ":unknown value\n>root 'x'", ":use_token_column maybe\n>root 'x'",
        "SAME x\n>SAME 'x'",
    ]
    for text in bad_cases:
        grammar.write_text("-WHITESPACE [ \\t\\n\\r]+\n" + text)
        (work / "protected.hpp").write_text("keep this output")
        result = run(grammar, "-o", "protected.hpp", ok=False)
        assert result.stderr, text
        assert (work / "protected.hpp").read_text() == "keep this output", text

    # Reject zero-token grammars: the underlying lexer requires a token definition.
    grammar.write_text(">root 'x'")
    run(grammar, ok=False)

    # Literal character rules and Windows line endings are valid.
    grammar.write_bytes(b"  # comment\r\n-WHITESPACE [ ]+\r\n>root 'x'\r\n")
    run(grammar, "-o", "literal.hpp")
    compile_run('''#include "literal.hpp"
int main() { empala::Lexer l; empala::Parser p; return p.Parse(l.Tokenize("x")) ? 0 : 1; }
''')

print("Empala CLI and generated-parser checks passed.")
