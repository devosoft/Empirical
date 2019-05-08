PROJECT := Emphatic
FILES := AST.h Emphatic.cc Emphatic.h EmphaticLexer.h 
EMP_DIR := ../../source

# Compiler type and flags
CXX := g++
CFLAGS_base := -Wall -Wno-unused-function -std=c++17 -I$(EMP_DIR)/
CFLAGS_debug := -g $(CFLAGS_base)
CFLAGS_release := -O3 -DNDEBUG $(CFLAGS_base)

default: release
all: release

# If in debug mode, swap out the optimization flags to be debug flags.
debug:	$(FILES)
				$(CXX) $(CFLAGS_debug) $(PROJECT).cc -o $(PROJECT)

release: $(FILES)
				$(CXX) $(CFLAGS_release) $(PROJECT).cc -o $(PROJECT)


clean:
	rm -rf $(PROJECT) $(PROJECT).dSYM/ *~

# Debugging information
print-%: ; @echo '$(subst ','\'',$*=$($*))'
