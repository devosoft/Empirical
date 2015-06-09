# Flags to use regardless of compiler
CFLAGS_all := -Wall -Wno-unused-function -std=c++11 -I../../ -I./

# Emscripten compiler information
CXX_web := emcc
OFLAGS_web := -g4 -Werror -pedantic -Wno-dollar-in-identifier-extension -s TOTAL_MEMORY=67108864 # -s DEMANGLE_SUPPORT=1 # -s SAFE_HEAP=1
#OFLAGS_web := -Werror -DNDEBUG -s TOTAL_MEMORY=67108864
#OFLAGS_web := -Oz -Werror -DNDEBUG -s TOTAL_MEMORY=67108864 -s ASSERTIONS=1
#OFLAGS_web := -Os -Werror -DNDEBUG -s TOTAL_MEMORY=67108864


CFLAGS_web := $(CFLAGS_all) $(OFLAGS_web) --js-library ../../emtools/library_emp.js -s EXPORTED_FUNCTIONS="['_main', '_empCppCallback']" -s DISABLE_EXCEPTION_CATCHING=1 -s COMPILER_ASSERTIONS=1 -s NO_EXIT_RUNTIME=1


default: all
all: events.js ElementSlate.js Example.js

events.js: events.cc
	$(CXX_web) $(CFLAGS_web) events.cc -o events.js

ElementSlate.js: ElementSlate.cc
	$(CXX_web) $(CFLAGS_web) ElementSlate.cc -o ElementSlate.js

Example.js: Example.cc
	$(CXX_web) $(CFLAGS_web) Example.cc -o Example.js

# test3.js: test3.cc
# 	$(CXX_web) $(CFLAGS_web) test3.cc -o test3.js

# test4.js: test4.cc
# 	$(CXX_web) $(CFLAGS_web) test4.cc -o test4.js

clean:
	rm -f events.js ElementSlate.js Example.js test?.js *.js.map *~ source/*.o source/*/*.o

# Debugging information
#print-%: ; @echo $*=$($*)
print-%: ; @echo '$(subst ','\'',$*=$($*))'
