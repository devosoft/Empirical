# Flags to use regardless of compiler
CFLAGS_all := -Wall -Wno-unused-function -I../../
CFLAGS_version := -std=c++14

# Emscripten compiler information
CXX_web := emcc
CXX_native := g++
#CXX_native := g++-5

OFLAGS_native_debug := -g -pedantic -DEMP_TRACK_MEM
OFLAGS_native_opt := -O3 -DNDEBUG

OFLAGS_web_debug := -g4 -pedantic -Wno-dollar-in-identifier-extension -s TOTAL_MEMORY=67108864 -s ASSERTIONS=2 -s DEMANGLE_SUPPORT=1 # -s SAFE_HEAP=1
OFLAGS_web_opt := -Os -DNDEBUG -s TOTAL_MEMORY=67108864

CFLAGS_native_debug := $(CFLAGS_all) $(OFLAGS_native_debug)
CFLAGS_native_opt := $(CFLAGS_all) $(OFLAGS_native_opt)

CFLAGS_web_debug := $(CFLAGS_all) $(OFLAGS_web_debug) --js-library ../../emtools/library_emp.js -s EXPORTED_FUNCTIONS="['_main', '_empCppCallback']" -s NO_EXIT_RUNTIME=1
CFLAGS_web_opt := $(CFLAGS_all) $(OFLAGS_web_opt) --js-library ../../emtools/library_emp.js -s EXPORTED_FUNCTIONS="['_main', '_empCppCallback']" -s NO_EXIT_RUNTIME=1
#CFLAGS_web := $(CFLAGS_all) $(OFLAGS_web) --js-library ../../emtools/library_emp.js -s EXPORTED_FUNCTIONS="['_main', '_empCppCallback']" -s DISABLE_EXCEPTION_CATCHING=1 -s NO_EXIT_RUNTIME=1

TARGETS := meta reflection TypeSet

TOOLS := ../../tools
TOOL_DEPEND := $(TOOLS)/assert.h

default: native

CXX := $(CXX_native)
CFLAGS := $(CFLAGS_native_opt)

debug: CFLAGS := $(CFLAGS_native_debug)
debug: all

cpp14: CFLAGS_version := -std=c++14
cpp14: all

web: CXX := $(CXX_web)
web: CFLAGS := $(CFLAGS_web_opt)
web: all

web-debug: CXX := $(CXX_web)
web-debug: CFLAGS := $(CFLAGS_web_debug)
web-debug: all

native: all

all: $(TARGETS)

$(TARGETS): % : %.cc $(TOOL_DEPEND) $(TOOLS)/%.h
	$(CXX) $(CFLAGS_version) $(CFLAGS) $< -o $@

# Lexer: Lexer.cc $(TOOLS)/Lexer.h $(TOOL_DEPEND) $(TOOLS)/DFA.h $(TOOLS)/NFA.h
#	$(CXX) $(CFLAGS) Lexer.cc -o Lexer

# lexer_utils: lexer_utils.cc $(TOOLS)/lexer_utils.h $(TOOL_DEPEND) $(TOOLS)/DFA.h $(TOOLS)/NFA.h
# 	$(CXX) $(CFLAGS) lexer_utils.cc -o lexer_utils

# Parser: Parser.cc $(TOOLS)/Parser.h $(TOOL_DEPEND) $(TOOLS)/DFA.h $(TOOLS)/NFA.h
# 	$(CXX) $(CFLAGS) Parser.cc -o Parser

opt-%: %.cc $(TOOL_DEPEND) $(TOOLS)/%.h
	$(CXX) $(CFLAGS_version) $(CFLAGS_native_opt) $< -o $@

debug-%: %.cc $(TOOL_DEPEND) $(TOOLS)/%.h
	$(CXX) $(CFLAGS_version) $(CFLAGS_native_debug) $< -o $@

cpp14-%: %.cc $(TOOL_DEPEND) $(TOOLS)/%.h
	$(CXX) -std=c++14 $(CFLAGS_native_debug) $< -o $@

clean:
	rm -rf debug-* *~ *.dSYM $(TARGETS)

# Debugging information
#print-%: ; @echo $*=$($*)
print-%: ; @echo '$(subst ','\'',$*=$($*))'
