# Flags to use regardless of compiler
CFLAGS_all := -Wall -Wno-unused-function -I../../include/
CFLAGS_version := -std=c++17

# Emscripten compiler information
CXX_web := emcc
CXX_native := g++
#CXX_native := clang++

OFLAGS_native_debug := -g -pedantic -DEMP_TRACK_MEM  -Wnon-virtual-dtor -Wcast-align -Woverloaded-virtual -Wconversion
OFLAGS_native_opt := -O3 -DNDEBUG

OFLAGS_web_debug := -g4 -pedantic -Wno-dollar-in-identifier-extension -s TOTAL_MEMORY=67108864 -s ASSERTIONS=2 -s DEMANGLE_SUPPORT=1 # -s SAFE_HEAP=1
OFLAGS_web_opt := -Os -DNDEBUG -s TOTAL_MEMORY=67108864

CFLAGS_native_debug := $(CFLAGS_all) $(OFLAGS_native_debug)
CFLAGS_native_opt := $(CFLAGS_all) $(OFLAGS_native_opt)

CFLAGS_web_debug := $(CFLAGS_all) $(OFLAGS_web_debug) --js-library ../../include/emp/web/library_emp.js -s EXPORTED_FUNCTIONS="['_main', '_empCppCallback']" -s NO_EXIT_RUNTIME=1
CFLAGS_web_opt := $(CFLAGS_all) $(OFLAGS_web_opt) --js-library ../../include/emp/web/library_emp.js -s EXPORTED_FUNCTIONS="['_main', '_empCppCallback']" -s NO_EXIT_RUNTIME=1
#CFLAGS_web := $(CFLAGS_all) $(OFLAGS_web) --js-library ../../include/emp/web/library_emp.js -s EXPORTED_FUNCTIONS="['_main', '_empCppCallback']" -s DISABLE_EXCEPTION_CATCHING=1 -s NO_EXIT_RUNTIME=1

TARGETS := DataFile DataInterface DataManager DataNode Trait VarMap

default: native

CXX := $(CXX_native)
CFLAGS := $(CFLAGS_native_opt)

debug: CFLAGS := $(CFLAGS_native_debug)
debug: all

web: CXX := $(CXX_web)
web: CFLAGS := $(CFLAGS_web_opt)
web: all

web-debug: CXX := $(CXX_web)
web-debug: CFLAGS := $(CFLAGS_web_debug)
web-debug: all

native: all

all: $(TARGETS)

$(TARGETS): % : %.cpp
	$(CXX) $(CFLAGS_version) $(CFLAGS) $< -o $@

opt-%: %.cpp
	$(CXX) $(CFLAGS_version) $(CFLAGS_native_opt) $< -o $@

debug-%: %.cpp
	$(CXX) $(CFLAGS_version) $(CFLAGS_native_debug) $< -o $@

clean:
	rm -rf debug-* *~ *.dSYM $(TARGETS)

# Debugging information
#print-%: ; @echo $*=$($*)
print-%: ; @echo '$(subst ','\'',$*=$($*))'
