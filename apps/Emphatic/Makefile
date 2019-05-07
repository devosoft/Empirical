PROJECT := Emphatic
EMP_DIR := ../../source

# Compiler type and flags
CXX := g++
CFLAGS_base := -Wall -Wno-unused-function -std=c++17 -I$(EMP_DIR)/
CFLAGS_debug := -g $(CFLAGS_base)
CFLAGS := -O3 -DNDEBUG $(CFLAGS_base)

default: $(PROJECT)
all: $(PROJECT)

# If in debug mode, swap out the optimization flags to be debug flags.
debug:	CFLAGS := $(CFLAGS_debug)
debug:	$(PROJECT)

$(PROJECT):	$(PROJECT).cc
	$(CXX) $(CFLAGS) $(PROJECT).cc -o $(PROJECT)
	@echo $(PROJECT) Compiled.

clean:
	rm -f $(PROJECT) *~

# Debugging information
print-%: ; @echo '$(subst ','\'',$*=$($*))'
