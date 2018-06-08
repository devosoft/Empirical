# Flags to use regardless of compiler
GLFW_LIBS := $(shell pkg-config --static --libs glfw3) \
    $(shell pkg-config --static --libs gl) \
    $(shell pkg-config --cflags --libs glew) \
    $(shell pkg-config --cflags --libs freetype2)
	  
CFLAGS_all := -Wall -Wno-unused-function -I../../source/
CFLAGS_version := -std=c++14

# Emscripten compiler information
CXX_web := emcc
CXX_native := clang++
# CXX_native := g++

OFLAGS_native_opt := $(GLFW_LIBS) -O3 -DNDEBUG
OFLAGS_native_debug := $(GLFW_LIBS) -g -pedantic -DEMP_TRACK_MEM  -Wnon-virtual-dtor -Wcast-align -Woverloaded-virtual
OFLAGS_native_grumpy := $(GLFW_LIBS) -g -pedantic -DEMP_TRACK_MEM  -Wnon-virtual-dtor -Wcast-align -Woverloaded-virtual -Wconversion -Weffc++

OFLAGS_web_opt := -s USE_WEBGL2=0 -Os -DNDEBUG -s TOTAL_MEMORY=67108864
OFLAGS_web_debug := -s USE_WEBGL2=0 -g4 -pedantic -Wno-dollar-in-identifier-extension -s TOTAL_MEMORY=67108864 -s ASSERTIONS=2 -s DEMANGLE_SUPPORT=1 # -s SAFE_HEAP=1

CFLAGS_native_opt := $(CFLAGS_all) $(OFLAGS_native_opt)
CFLAGS_native_debug := $(CFLAGS_all) $(OFLAGS_native_debug)
CFLAGS_native_grumpy := $(CFLAGS_all) $(OFLAGS_native_grumpy)

CFLAGS_web_debug := $(CFLAGS_all) $(OFLAGS_web_debug) --preload-file Assets -s USE_FREETYPE=1 --js-library ../../source/web/library_emp.js -s EXPORTED_FUNCTIONS="['_main', '_empCppCallback']" -s EXTRA_EXPORTED_RUNTIME_METHODS="['Pointer_stringify']" -s NO_EXIT_RUNTIME=1 -s DISABLE_EXCEPTION_CATCHING=0
CFLAGS_web_opt := $(CFLAGS_all) $(OFLAGS_web_opt)  --preload-file Assets -s USE_FREETYPE=1 --js-library ../../source/web/library_emp.js -s EXPORTED_FUNCTIONS="['_main', '_empCppCallback']" -s EXTRA_EXPORTED_RUNTIME_METHODS="['Pointer_stringify']" -s NO_EXIT_RUNTIME=1
#CFLAGS_web := $(CFLAGS_all) $(OFLAGS_web) --js-library ../../source/web/library_emp.js -s EXPORTED_FUNCTIONS="['_main', '_empCppCallback']" -s DISABLE_EXCEPTION_CATCHING=1 -s NO_EXIT_RUNTIME=1

JS_TARGETS := PlottingExperiment.js
TARGETS := PlottingExperiment

EVO_DEPEND := $(wildcard ../../source/opengl/*.h) $(wildcard ../../source/math/*.h) $(wildcard ../../source/plot/*.h) $(wildcard ../../source/scenegraph/*.h)

default: native

CXX := $(CXX_native)
CFLAGS := $(CFLAGS_native_opt)

debug: CFLAGS := $(CFLAGS_native_debug)
debug: all

grumpy: CFLAGS := $(CFLAGS_native_grumpy)
grumpy: all

web: CXX := $(CXX_web)
web: CFLAGS := $(CFLAGS_web_opt)
web: $(JS_TARGETS)

web-debug: CXX := $(CXX_web)
web-debug: CFLAGS := $(CFLAGS_web_debug)
web-debug: $(JS_TARGETS)

native: all

all: $(TARGETS)

$(TARGETS): % : %.cc $(EVO_DEPEND)
	$(CXX) $(CFLAGS_version) $(CFLAGS) $< -o $@

$(JS_TARGETS): %.js : %.cc $(EVO_DEPEND)
	$(CXX) $(CFLAGS_version) $(CFLAGS) $< -o $@

debug-%: $*.cc $(EVO_DEPEND)
	$(CXX) $(CFLAGS_version) $(CFLAGS_native_debug) $< -o $@

clean:
	rm -rf debug-* *~ *.dSYM $(TARGETS)
	rm -rf debug-* *~ *.dSYM $(JS_TARGETS)

# Debugging information
#print-%: ; @echo $*=$($*)
print-%: ; @echo '$(subst ','\'',$*=$($*))'
