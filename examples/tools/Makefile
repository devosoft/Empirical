# Flags to use regardless of compiler
CFLAGS_all := -Wall -Wno-unused-function -std=c++11 -I../../
#CFLAGS_all := -Wall -Wno-unused-variable -Wno-unused-function -Wno-extra-semi -std=c++11 -I../../ -I./

# Emscripten compiler information
CXX_web := emcc
CXX_native := g++

OFLAGS_native := -g -pedantic
#OFLAGS_native := -O3 -DNDEBUG

OFLAGS_web := -g4 -pedantic -Wno-dollar-in-identifier-extension -s TOTAL_MEMORY=67108864 -s ASSERTIONS=2 -s DEMANGLE_SUPPORT=1 # -s SAFE_HEAP=1
#OFLAGS_web := -Os -DNDEBUG -s TOTAL_MEMORY=67108864


CFLAGS_native := $(CFLAGS_all) $(OFLAGS_native)

CFLAGS_web := $(CFLAGS_all) $(OFLAGS_web) --js-library ../../emtools/library_emp.js -s EXPORTED_FUNCTIONS="['_main', '_empCppCallback']" -s NO_EXIT_RUNTIME=1
#CFLAGS_web := $(CFLAGS_all) $(OFLAGS_web) --js-library ../../emtools/library_emp.js -s EXPORTED_FUNCTIONS="['_main', '_empCppCallback']" -s DISABLE_EXCEPTION_CATCHING=1 -s NO_EXIT_RUNTIME=1


TARGETS := array DFA errors info_theory Lexer lexer_utils macros math meta NFA Parser Ptr Random reflection RegEx signal TypeSet TypeTracker vector

TOOLS := ../../tools
TOOL_DEPEND := $(TOOLS)/assert.h

default: native

native: $(TARGETS)


$(TARGETS): % : %.cc $(TOOL_DEPEND) $(TOOLS)/%.h
	$(CXX_native) $(CFLAGS_native) $< -o $@

# Lexer: Lexer.cc $(TOOLS)/Lexer.h $(TOOL_DEPEND) $(TOOLS)/DFA.h $(TOOLS)/NFA.h
#	$(CXX_native) $(CFLAGS_native) Lexer.cc -o Lexer

# lexer_utils: lexer_utils.cc $(TOOLS)/lexer_utils.h $(TOOL_DEPEND) $(TOOLS)/DFA.h $(TOOLS)/NFA.h
# 	$(CXX_native) $(CFLAGS_native) lexer_utils.cc -o lexer_utils

# Parser: Parser.cc $(TOOLS)/Parser.h $(TOOL_DEPEND) $(TOOLS)/DFA.h $(TOOLS)/NFA.h
# 	$(CXX_native) $(CFLAGS_native) Parser.cc -o Parser


debug-%: $*.cc
	$(CXX_native) $(CFLAGS_native) $< -o $@

clean:
	rm -rf debug-* *~ *.dSYM $(TARGETS)

# Debugging information
#print-%: ; @echo $*=$($*)
print-%: ; @echo '$(subst ','\'',$*=$($*))'
