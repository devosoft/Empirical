# Flags to use regardless of compwdpiler
CFLAGS_all := -Wall -std=c++11 -I../../../Empirical/ -I./

# Emscripten compiler information
CXX_web := emcc
OFLAGS_web := -g4 -Werror -s TOTAL_MEMORY=67108864 # -s SAFE_HEAP=1
# OFLAGS_web := -Oz -Werror -DNDEBUG -s TOTAL_MEMORY=67108864
CFLAGS_web := $(CFLAGS_all) $(OFLAGS_web) --js-library ../../../Empirical/kinetic/library_kinetic.js  -s EXPORTED_FUNCTIONS="['_empMain', '_empJSDoCallback']" #-s INVOKE_RUN=0 -s DISABLE_EXCEPTION_CATCHING=1 -s COMPILER_ASSERTIONS=1 --embed-file configs

all: test.html LayerManagement.js Simple.js
default: all

test.html: test.cc
	$(CXX_web) $(CFLAGS_all) $(OFLAGS_web) --js-library library_test.js test.cc -o test.html

LayerManagement.js: LayerManagement.cc
	$(CXX_web) $(CFLAGS_web) LayerManagement.cc -o LayerManagement.js

Simple.js: Simple.cc
	$(CXX_web) $(CFLAGS_web) Simple.cc -o Simple.js

clean:
	rm -f *.js.map *~ *.o test.html LayerManagement.js Simple.js
