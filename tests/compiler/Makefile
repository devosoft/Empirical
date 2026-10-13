TEST_NAMES = DFA lexer_utils Lexer NFA Parser RegEx
TEST_DIR = $(CURDIR)

include ../Makefile-tests.mk

.PHONY: codegen

# Exercise the emitted programs, not just the C++ generator itself.
codegen:
	python3 test_codegen.py --cxx "$(CXX)"

test opt fulldebug: codegen
