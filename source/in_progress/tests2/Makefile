FLAGS_all := -std=c++14 -Wall -Wno-unused-function -pedantic -I../source/
FLAGS_debug := -Wnon-virtual-dtor -Wcast-align -Woverloaded-virtual -Wconversion -Weffc++
FLAGS_opt := -DNDEBUG
FLAGS_web := -Wno-dollar-in-identifier-extension -s TOTAL_MEMORY=67108864 --js-library ../source/web/library_emp.js -s EXPORTED_FUNCTIONS="['_main', '_empCppCallback']" -s NO_EXIT_RUNTIME=1

FLAGS_native_debug := $(FLAGS_all) $(FLAGS_debug) -g -DEMP_TRACK_MEM  
FLAGS_native_opt := $(FLAGS_all) $(FLAGS_opt) -O3

FLAGS_web_debug := $(FLAGS_all) $(FLAGS_web) $(FLAGS_debug) -g4 -s ASSERTIONS=2 -s DEMANGLE_SUPPORT=1
FLAGS_web_opt := $(FLAGS_all) $(FLAGS_web) $(FLAGS_opt) -Os

CXX_native := clang++
CXX_alt := g++
CXX_web := emcc

# Defaults... can be changed by specific targets.
CXX := $(CXX_native)
FLAGS := $(FLAGS_native_opt)

# No exe flags by default.
EXE_FLAGS := 

# A recursive wildcard function from https://stackoverflow.com/questions/3774568/makefile-issue-smart-way-to-scan-directory-tree-for-c-files
rwildcard=$(wildcard $1$2) $(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2))

DIRS := . tools
ALL_SOURCES := $(call rwildcard,./,*/*.cc)
ALL_EXES := $(patsubst %.cc,%,$(ALL_SOURCES))

MAIN_DIR := $(CURDIR)

all:	$(ALL_EXES)

review:
	@echo 'SOURCES: ' $(ALL_SOURCES)
	@echo 'EXES: ' $(ALL_EXES)

native: all

default: all

debug: FLAGS := $(FLAGS_native_debug)
debug: all

web: CXX := $(CXX_web)
web: FLAGS := $(FLAGS_web_opt)
web: all

web-debug: CXX := $(CXX_web)
web-debug: FLAGS := $(FLAGS_web_debug)
web-debug: all

silent: EXE_FLAGS := --silent
silent: all

verbose: EXE_FLAGS := --verbose
verbose: all

$(ALL_EXES): % : %.cc
	$(CXX) $(FLAGS) $< -o $@
	./$@ $(EXE_FLAGS)
	rm -f ./$@



opt-%: %.cc $(TOOL_DEPEND) $(TOOLS)/%.h
	$(CXX) $(FLAGS_version) $(FLAGS_native_opt) $< -o $@

debug-%: %.cc $(TOOL_DEPEND) $(TOOLS)/%.h
	$(CXX) $(FLAGS_version) $(FLAGS_native_debug) $< -o $@

clean:
	rm -rf $(ALL_EXES) *~ *.dSYM $(TARGETS)

# Debugging information
#print-%: ; @echo $*=$($*)
print-%: ; @echo '$(subst ','\'',$*=$($*))'
