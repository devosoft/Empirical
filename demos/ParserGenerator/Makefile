PROJECT := Empala
EMP_DIR ?= ../../include
CXX ?= c++
EMCXX ?= em++
CPPFLAGS += -I$(EMP_DIR) -I$(EMP_DIR)/../third-party/cereal/include
CXXFLAGS ?= -O3 -DNDEBUG
COMMON_FLAGS := -fexceptions -std=c++23 -Wall -Wextra -Wno-unused-function
WEB_FLAGS ?= -Oz -DNDEBUG
WEB_LINK_FLAGS := --js-library $(EMP_DIR)/emp/web/library_emp.js \
  -sEXPORTED_RUNTIME_METHODS="['ccall','cwrap','UTF8ToString','stringToUTF8','lengthBytesUTF8']" \
  -sEXPORTED_FUNCTIONS="['_main','_malloc','_free','_empCppCallback']" \
  -sINITIAL_MEMORY=67108864 -sALLOW_MEMORY_GROWTH=1 -sNO_EXIT_RUNTIME=1

.PHONY: default native web all debug debug-web web-debug clean test test-parser test-dice
default: native
native: $(PROJECT)
web: web/$(PROJECT).js
all: native web

$(PROJECT): build/native/$(PROJECT).o
	$(CXX) $(CXXFLAGS) $< $(LDFLAGS) $(LDLIBS) -o $@

build/native/$(PROJECT).o: source/native/$(PROJECT).cpp Makefile
	@mkdir -p $(@D)
	$(CXX) $(CPPFLAGS) $(COMMON_FLAGS) $(CXXFLAGS) -MMD -MP -c $< -o $@

web/$(PROJECT).js: source/web/$(PROJECT)-web.cpp Makefile
	@mkdir -p build/web
	$(EMCXX) $(CPPFLAGS) $(COMMON_FLAGS) $(WEB_FLAGS) -MMD -MP -MF build/web/$(PROJECT).d -MT $@ $< $(WEB_LINK_FLAGS) -o $@

# Separate debug binaries prevent a release build from satisfying a debug target.
debug: $(PROJECT)-debug
$(PROJECT)-debug: source/native/$(PROJECT).cpp Makefile
	@mkdir -p build/debug
	$(CXX) $(CPPFLAGS) $(COMMON_FLAGS) -O0 -g -MMD -MP -MF build/debug/$(PROJECT).d -MT $@ $< $(LDFLAGS) $(LDLIBS) -o $@

debug-web:
	$(MAKE) -B web WEB_FLAGS='-O0 -g -sASSERTIONS=2'
web-debug: debug-web

# The example's build shows the complete workflow: grammar -> header -> program.
build/dice_parser.hpp: examples/dice/dice.empala $(PROJECT)
	@mkdir -p $(@D)
	./$(PROJECT) $< -o $@

dice: examples/dice/dice.cpp build/dice_parser.hpp Makefile
	$(CXX) $(CPPFLAGS) -Ibuild $(COMMON_FLAGS) $(CXXFLAGS) -MMD -MP -MF build/dice.d -MT $@ $< $(LDFLAGS) $(LDLIBS) -o $@

test-dice: dice
	python3 tests/test_dice.py ./dice

build/test_grammar: tests/test_grammar.cpp source/Grammar.hpp Makefile
	@mkdir -p $(@D)
	$(CXX) $(CPPFLAGS) $(COMMON_FLAGS) -O0 -g -MMD -MP $< -o $@

test: test-parser test-dice

test-parser: native build/test_grammar
	./build/test_grammar
	python3 tests/test_empala.py ./$(PROJECT) "$(CXX)"

clean:
	rm -rf build $(PROJECT) $(PROJECT)-debug $(PROJECT)-debug.dSYM dice dice.dSYM
	rm -f web/$(PROJECT).js web/$(PROJECT).wasm web/$(PROJECT).js.map web/$(PROJECT).wasm.map

-include build/native/$(PROJECT).d build/debug/$(PROJECT).d build/web/$(PROJECT).d build/dice.d build/test_grammar.d
