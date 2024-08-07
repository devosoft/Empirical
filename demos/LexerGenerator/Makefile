# Project-specific settings
PROJECT := Emplex
EMP_DIR := ../../include

# Flags to use regardless of compiler
CFLAGS_all := -Wall -Wextra -Wno-unused-function -std=c++20 -I$(EMP_DIR)/

# Native compiler information
CXX_nat := g++
CFLAGS_nat := -O3 -DNDEBUG $(CFLAGS_all)
CFLAGS_nat_debug := -g $(CFLAGS_all)

# Emscripten compiler information
CXX_web := emcc
EMP_METHODS = EXPORTED_RUNTIME_METHODS="['ccall', 'cwrap', 'UTF8ToString', 'stringToUTF8', 'lengthBytesUTF8']"
EMP_FUNCTIONS = EXPORTED_FUNCTIONS="['_main', "_malloc", "_free", '_empCppCallback']"
JS_LIB = --js-library $(EMP_DIR)/emp/web/library_emp.js
OFLAGS_web_all := -s $(EMP_METHODS) -s TOTAL_MEMORY=67108864 $(JS_LIB) -s $(EMP_FUNCTIONS) -s DISABLE_EXCEPTION_CATCHING=1 -s NO_EXIT_RUNTIME=1
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
