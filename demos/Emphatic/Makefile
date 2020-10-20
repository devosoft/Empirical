PROJECT := Emphatic
FILES := AST.hpp ElementInfo.hpp Emphatic.cpp Emphatic.hpp EmphaticLexer.hpp
EMP_DIR := ../../include

# Compiler type and flags
CXX := g++
CFLAGS_base := -Wall -Wno-unused-function -std=c++17 -I$(EMP_DIR)/
CFLAGS_debug := -g $(CFLAGS_base)
CFLAGS_release := -O3 -DNDEBUG $(CFLAGS_base)

default: $(PROJECT)
all: $(PROJECT)

# If in debug mode, swap out the optimization flags to be debug flags.
debug:	$(FILES)
				$(CXX) $(CFLAGS_debug) $(PROJECT).cpp -o $(PROJECT)

$(PROJECT): $(FILES)
				$(CXX) $(CFLAGS_release) $(PROJECT).cpp -o $(PROJECT)


clean:
	rm -rf $(PROJECT) $(PROJECT).dSYM/ *~

test: debug
	./Emphatic examples/ConceptTest.empcc examples/ConceptTest.cpp
	$(CXX) $(CFLAGS_debug) examples/ConceptTest.cpp -o examples/ConceptTest

# Debugging information
print-%: ; @echo '$(subst ','\'',$*=$($*))'
