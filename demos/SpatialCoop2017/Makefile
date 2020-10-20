# Project-specific settings
PROJECT := SimplePDWorld
EMP_DIR := ../../include

# Flags to use regardless of compiler
CFLAGS_all := -Wall -Wno-unused-function -std=c++17 -I$(EMP_DIR)/

# Native compiler information
CXX_nat := g++
CFLAGS_nat := -O3 -DNDEBUG $(CFLAGS_all)
CFLAGS_nat_debug := -g $(CFLAGS_all)

# Emscripten compiler information
CXX_web := emcc
OFLAGS_web_all := -s "EXTRA_EXPORTED_RUNTIME_METHODS=['ccall', 'cwrap']" -s TOTAL_MEMORY=67108864 --js-library $(EMP_DIR)/emp/web/library_emp.js -s EXPORTED_FUNCTIONS="['_main', '_empCppCallback']" -s DISABLE_EXCEPTION_CATCHING=1 -s NO_EXIT_RUNTIME=1 #--embed-file configs
OFLAGS_web := -Oz -DNDEBUG
OFLAGS_web_debug := -g4 -pedantic -Wno-dollar-in-identifier-extension

CFLAGS_web := $(CFLAGS_all) $(OFLAGS_web) $(OFLAGS_web_all)
CFLAGS_web_debug := $(CFLAGS_all) $(OFLAGS_web_debug) $(OFLAGS_web_all)


default: $(PROJECT)
native: $(PROJECT)
web: $(PROJECT).js
all: $(PROJECT) $(PROJECT).js

debug:	CFLAGS_nat := $(CFLAGS_nat_debug)
debug:	$(PROJECT)

debug-web:	CFLAGS_web := $(CFLAGS_web_debug)
debug-web:	$(PROJECT).js

web-debug:	debug-web

$(PROJECT):	source/native/$(PROJECT).cpp
	$(CXX_nat) $(CFLAGS_nat) source/native/$(PROJECT).cpp -o $(PROJECT)
	@echo To build the web version use: make web

$(PROJECT).js: source/web/$(PROJECT)-web.cpp
	$(CXX_web) $(CFLAGS_web) source/web/$(PROJECT)-web.cpp -o web/$(PROJECT).js

clean:
	rm -f $(PROJECT) web/$(PROJECT).js web/$(PROJECT).js.mem *.js.map *~ source/*.o

# Debugging information
print-%: ; @echo '$(subst ','\'',$*=$($*))'
