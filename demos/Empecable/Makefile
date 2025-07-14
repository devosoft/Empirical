# Project-specific settings
PROJECT := Empecable
EMP_DIR := ../../include
HEADERS := helpers.hpp Lexer.hpp ReviewFile.hpp

# Flags to use regardless of compiler
CFLAGS_all := -Wall -Wextra -Wno-unused-function -std=c++23 -I$(EMP_DIR)/

# Native compiler information
CXX_nat := c++
CFLAGS_nat := -O3 -DNDEBUG $(CFLAGS_all)
CFLAGS_nat_debug := -g $(CFLAGS_all)

default: $(PROJECT)
native: $(PROJECT)

debug:	CFLAGS_nat := $(CFLAGS_nat_debug)
debug:	$(PROJECT)

$(PROJECT):	$(PROJECT).cpp $(HEADERS)
	$(CXX_nat) $(CFLAGS_nat) $(PROJECT).cpp -o $(PROJECT)

clean:
	rm -rf $(PROJECT) $(PROJECT).dSYM *~ source/*.o

# Debugging information
print-%: ; @echo '$(subst ','\'',$*=$($*))'
