TARGETS := array errors macros map Ptr unordered_map vector
TARGET_INCLUDES := ../../source/base

# Flags to use regardless of compiler
CFLAGS_all := -Wall -Wno-unused-function -I../../source/
CFLAGS_version := -std=c++17

# Compiler selection
CXX_native := clang++
CXX_web := emcc

OFLAGS_debug := -pedantic -Wnon-virtual-dtor -Wcast-align -Woverloaded-virtual -Wconversion -Weffc++
OFLAGS_opt := -O3 -DNDEBUG
OFLAGS_web := -s TOTAL_MEMORY=67108864 -Wno-dollar-in-identifier-extension 

OFLAGS_native_debug := $(OFLAGS_debug) -g -DEMP_TRACK_MEM
OFLAGS_web_debug := $(OFLAGS_web) $(OFLAGS_debug) -g4 -s ASSERTIONS=2 -s DEMANGLE_SUPPORT=1 # -s SAFE_HEAP=1
OFLAGS_web_opt := $(OFLAGS_web) -Os -DNDEBUG 

CFLAGS_native_debug := $(CFLAGS_all) $(OFLAGS_native_debug)
CFLAGS_native_opt := $(CFLAGS_all) $(OFLAGS_opt)

CFLAGS_web_debug := $(CFLAGS_all) $(OFLAGS_web_debug) --js-library ../../source/web/library_emp.js -s EXPORTED_FUNCTIONS="['_main', '_empCppCallback']" -s NO_EXIT_RUNTIME=1
CFLAGS_web_opt := $(CFLAGS_all) $(OFLAGS_web_opt) --js-library ../../source/web/library_emp.js -s EXPORTED_FUNCTIONS="['_main', '_empCppCallback']" -s NO_EXIT_RUNTIME=1
#CFLAGS_web := $(CFLAGS_all) $(OFLAGS_web) --js-library ../../source/web/library_emp.js -s EXPORTED_FUNCTIONS="['_main', '_empCppCallback']" -s DISABLE_EXCEPTION_CATCHING=1 -s NO_EXIT_RUNTIME=1

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

$(TARGETS): % : %.cc $(TARGET_INCLUDES)/%.h
	$(CXX) $(CFLAGS_version) $(CFLAGS) $< -o $@

opt-%: %.cc $(TARGET_INCLUDES)/%.h
	$(CXX) $(CFLAGS_version) $(CFLAGS_native_opt) $< -o $@

debug-%: %.cc $(TARGET_INCLUDES)/%.h
	$(CXX) $(CFLAGS_version) $(CFLAGS_native_debug) $< -o $@

clean:
	rm -rf debug-* *~ *.dSYM $(TARGETS)

# Debugging information
#print-%: ; @echo $*=$($*)
print-%: ; @echo '$(subst ','\'',$*=$($*))'

