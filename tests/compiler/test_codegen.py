#!/usr/bin/env python3
"""Compile and execute generated lexers in all three supported languages.

Requires a C++23 compiler, Python 3, and Node.js. Everything is built in a temporary
folder; no generated sources or binaries are left in the checkout.
"""

import argparse
import importlib.util
import io
import json
from pathlib import Path
import shlex
import subprocess
import sys
import tempfile


def run(command, **kwargs):
    return subprocess.run(command, check=True, text=True, **kwargs)


def record(token):
    return [token.id, [ord(c) for c in token.lexeme] if hasattr(token, 'lexeme') else None,
            getattr(token, 'line_id', None), getattr(token, 'col_id', None)]


def check_python(folder, manifest, expected):
    for case, reference in zip(manifest, expected):
        name, flags = case['name'], case['flags']
        spec = importlib.util.spec_from_file_location(name, folder / (name + '.py'))
        module = importlib.util.module_from_spec(spec)
        sys.modules[name] = module
        spec.loader.exec_module(module)
        lexer = module.CustomLexer()
        actual = []
        for text in case['inputs']:
            tokens = lexer.Tokenize(text)
            assert lexer.GetNumTokens() == module.CustomLexer.NUM_TOKENS
            assert module.CustomDFA.GetStop(-1) == 0
            assert module.CustomDFA.GetNext(-1, 65) == -1
            assert module.CustomDFA.GetNext(0, 128) == -1
            assert lexer.TokenName(-1) == '_ERROR_'
            assert lexer.TokenName(0) == '_EOF_'
            assert lexer.TokenName(10) == "'\\n'"
            for token in tokens:
                assert hasattr(token, 'lexeme') == bool(flags & 1)
                assert hasattr(token, 'line_id') == bool(flags & 2)
                assert hasattr(token, 'col_id') == bool(flags & 4)
                assert int(token) == token.id
                assert lexer.Peek() is token
                assert lexer.Is(token.id)
                assert lexer.Use(token.id) is token
            assert lexer.None_() and not lexer.Any()
            assert lexer.Use().id == 0
            assert lexer.UseIf(999) == 0
            actual.append({'tokens': [record(t) for t in tokens], 'eof': record(lexer.Peek()),
                           'dfa': module.CustomDFA.Test(text)})
            lexer.Rewind(100000)
            if tokens:
                assert lexer.Peek(1).id == (tokens[1].id if len(tokens) > 1 else 0)
                assert lexer.UseIf(999, tokens[0].id) == tokens[0].id
                lexer.Rewind()
                assert lexer.Peek() is tokens[0]
            try:
                lexer.Use(999, 'custom failure')
                raise AssertionError('Use must raise for an unexpected token')
            except RuntimeError as error:
                assert 'custom failure' in str(error)
                assert (' (line ' in str(error)) == bool(flags & 2)
            assert [record(t) for t in lexer.Tokenize(io.StringIO(text))] == actual[-1]['tokens']
        assert actual == reference, (name, actual, reference)
    return len(manifest)


JS_TEST = r'''
import assert from 'node:assert/strict';
import { readFileSync } from 'node:fs';
const manifest = JSON.parse(readFileSync(new URL('manifest.json', import.meta.url)));
const expected = JSON.parse(readFileSync(new URL('expected.json', import.meta.url)));
const record = token => [token.id,
  Object.hasOwn(token, 'lexeme') ? Array.from(token.lexeme, c => c.charCodeAt(0)) : null,
  token.line_id ?? null, token.col_id ?? null];
for (let i = 0; i < manifest.length; ++i) {
  const {name, flags, inputs} = manifest[i];
  const {CustomLexer: Lexer, CustomDFA: DFA} = await import(`./${name}.mjs`);
  const lexer = new Lexer();
  const actual = [];
  for (const text of inputs) {
    const tokens = lexer.Tokenize(text);
    assert.equal(lexer.GetNumTokens(), Lexer.NUM_TOKENS);
    assert.equal(DFA.GetStop(-1), 0);
    assert.equal(DFA.GetNext(-1, 65), -1);
    assert.equal(DFA.GetNext(0, 128), -1);
    assert.equal(lexer.TokenName(-1), '_ERROR_');
    assert.equal(lexer.TokenName(0), '_EOF_');
    assert.equal(lexer.TokenName(10), "'\\n'");
    for (const token of tokens) {
      assert.equal(Object.hasOwn(token, 'lexeme'), Boolean(flags & 1));
      assert.equal(Object.hasOwn(token, 'line_id'), Boolean(flags & 2));
      assert.equal(Object.hasOwn(token, 'col_id'), Boolean(flags & 4));
      assert.equal(Number(token), token.id);
      assert.equal(lexer.Peek(), token);
      assert(lexer.Is(token.id));
      assert.equal(lexer.Use(token.id), token);
    }
    assert(lexer.None() && !lexer.Any());
    assert.equal(lexer.Use().id, 0);
    assert.equal(lexer.UseIf(999), 0);
    actual.push({tokens: tokens.map(record), eof: record(lexer.Peek()), dfa: DFA.Test(text)});
    lexer.Rewind(100000);
    if (tokens.length) {
      assert.equal(lexer.Peek(1).id, tokens[1]?.id ?? 0);
      assert.equal(lexer.UseIf(999, tokens[0].id), tokens[0].id);
      lexer.Rewind();
      assert.equal(lexer.Peek(), tokens[0]);
    }
    assert.throws(() => lexer.Use(999, 'custom failure'), error =>
      error.message.includes('custom failure') && error.message.includes(' (line ') === Boolean(flags & 2));
    assert.deepEqual(lexer.Tokenize(text).map(record), actual.at(-1).tokens);
  }
  assert.deepEqual(actual, expected[i], name);
}
console.log(`JavaScript: ${manifest.length} generated lexers passed runtime and C++ parity checks`);
'''


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--cxx', default='c++')
    args = parser.parse_args()
    compiler = shlex.split(args.cxx)
    root = Path(__file__).resolve().parents[2]
    with tempfile.TemporaryDirectory(prefix='emp-lexer-codegen-') as temporary:
        folder = Path(temporary)
        run(compiler + ['-std=c++23', '-O0', '-I' + str(root / 'include'),
                        str(root / 'tests/compiler/lexer_codegen_fixture.cpp'), '-o', str(folder / 'fixture')])
        run([str(folder / 'fixture'), str(folder)])
        run(compiler + ['-std=c++23', '-O0', str(folder / 'reference.cpp'), '-o', str(folder / 'reference')])
        result = run([str(folder / 'reference')], capture_output=True)
        expected = [json.loads(line) for line in result.stdout.splitlines()]
        manifest = json.loads((folder / 'manifest.json').read_text())
        assert len(expected) == len(manifest) == 48
        # Independent semantic checks: rule priority, longest match, context rollback, and anchors.
        full = {case['name']: reference for case, reference in zip(manifest, expected)}
        assert [t[0] for t in full['case0_7'][1]['tokens']] == [255, 254, 253, 63, 255, 254]
        assert [t[1] for t in full['case1_7'][0]['tokens']] == [[97], [58], [98]]
        assert [t[0] for t in full['case3_7'][0]['tokens']] == [255, 254, 254, 100, 33]
        assert [t[0] for t in full['case2_7'][0]['tokens']] == [255, 253, 254, 255, 254]
        count = check_python(folder, manifest, expected)
        print(f'Python: {count} generated lexers passed runtime and C++ parity checks', flush=True)
        (folder / 'expected.json').write_text(json.dumps(expected))
        (folder / 'check.mjs').write_text(JS_TEST)
        run(['node', str(folder / 'check.mjs')])


if __name__ == '__main__':
    main()
