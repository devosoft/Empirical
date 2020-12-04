# TODO:
# * Remove "-Wno-unused-function" ; right now it's required because static library functions
#    are expected to be called at some point.
# * Remove "-Wno-gnu-zero-variadic-macro-arguments" ; right now it's in there because the
#    EM_ASM macro seems to require extra arguments...
# * Remove "-s WASM=0" ; right now some canvas example files don't seem to work in
#    WebAssembly.

# Flags to use regardless of compiler
CFLAGS_all := -std=c++17 -Wall -Wno-unused-function -I../../include/

# Emscripten compiler information
CXX_web := emcc
CXX_native := g++

OFLAGS_native_debug := -g -DEMP_TRACK_MEM  -Wnon-virtual-dtor -Wcast-align -Woverloaded-virtual -Wconversion -Weffc++
OFLAGS_native_opt := -O3 -DNDEBUG

OFLAGS_web_debug := -Wno-dollar-in-identifier-extension -s TOTAL_MEMORY=67108864 -s ASSERTIONS=2 -s DEMANGLE_SUPPORT=1 -s WASM=0 -Wnon-virtual-dtor -Wcast-align -Woverloaded-virtual -Wconversion -Weffc++
 # -s SAFE_HEAP=1
OFLAGS_web_opt := -Os -DNDEBUG -s TOTAL_MEMORY=67108864 -s WASM=0

CFLAGS_native_debug := $(CFLAGS_all) $(OFLAGS_native_debug)
CFLAGS_native_opt := $(CFLAGS_all) $(OFLAGS_native_opt)
CFLAGS_native := $(CFLAGS_native_opt)

CFLAGS_web_debug := $(CFLAGS_all) $(OFLAGS_web_debug) --js-library ../../include/emp/web/library_emp.js -s EXPORTED_FUNCTIONS="['_main', '_empCppCallback']" -s EXTRA_EXPORTED_RUNTIME_METHODS='["ccall", "cwrap"]' -s NO_EXIT_RUNTIME=1
CFLAGS_web_opt := $(CFLAGS_all) $(OFLAGS_web_opt) --js-library ../../include/emp/web/library_emp.js -s EXPORTED_FUNCTIONS="['_main', '_empCppCallback']" -s EXTRA_EXPORTED_RUNTIME_METHODS='["ccall", "cwrap"]' -s NO_EXIT_RUNTIME=1

JS_TARGETS := Card.js CodeBlock.js Collapse.js CommentBox.js ConfigPanel.js FontAwesomeIcon.js LoadingIcon.js LoadingModal.js Modal.js ToggleSwitch.js

CXX := $(CXX_web)
CFLAGS_web := $(CFLAGS_web_opt)

%.js: %.cpp
	$(CXX_web) $(CFLAGS_web) $< -o $@

debug: CFLAGS_web := $(CFLAGS_web_debug)
debug: $(JS_TARGETS)

web: CXX := $(CXX_web)
web: CFLAGS_web := $(CFLAGS_web_opt)
web: $(JS_TARGETS)

web-debug: CXX := $(CXX_web)
web-debug: CFLAGS := $(CFLAGS_web_debug)
web-debug: all

default: web

clean:
	rm -f debug-* $(JS_TARGETS) *.js.map *.js.mem *.wasm *.wasm.map *.wast *~ source/*.o source/*/*.o

# Debugging information
#print-%: ; @echo $*=$($*)
print-%: ; @echo '$(subst ','\'',$*=$($*))'
