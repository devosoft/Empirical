# Flags to use regardless of compiler
CFLAGS_all := -Wall -Wno-unused-function -Wno-gnu-zero-variadic-macro-arguments -Wno-dollar-in-identifier-extension -std=c++17 -I../../../source/

# Emscripten compiler information
CXX_web := emcc
OFLAGS_web := -Werror -pedantic -Wno-dollar-in-identifier-extension -s TOTAL_MEMORY=67108864

#A bug in llvm compilers causes them to erroneously warn about braces around
#initializer lists - to compile files with them, we need to remove -Werror
OFLAGS_web_braces := -pedantic -Wno-dollar-in-identifier-extension -s TOTAL_MEMORY=67108864
#OFLAGS_web := -Werror -DNDEBUG -s TOTAL_MEMORY=67108864
#OFLAGS_web := -Oz -Werror -DNDEBUG -s TOTAL_MEMORY=67108864 -s ASSERTIONS=1
#OFLAGS_web := -O3 -Werror -DNDEBUG -s TOTAL_MEMORY=67108864

CFLAGS_web := $(CFLAGS_all) $(OFLAGS_web_braces) --js-library ../../../source/web/library_emp.js --js-library ../../../source/web/d3/library_d3.js -s EXPORTED_FUNCTIONS="['_main', '_empCppCallback']" -s DISABLE_EXCEPTION_CATCHING=1 -s NO_EXIT_RUNTIME=1 -s EXTRA_EXPORTED_RUNTIME_METHODS='["ccall", "cwrap", "stringToUTF8"]' -s WASM=0

default: all
all: d3_init.js selection.js transition.js


d3_init.js: d3_init.cc
	$(CXX_web) $(CFLAGS_web) d3_init.cc -o d3_init.js

selection.js: selection.cc
	$(CXX_web) $(CFLAGS_web) selection.cc -o selection.js

scales.js: scales.cc
	$(CXX_web) $(CFLAGS_web) scales.cc -o scales.js

transition.js: transition.cc
	$(CXX_web) $(CFLAGS_web) transition.cc -o transition.js

test_visualizations.js: visualizations.cc
	$(CXX_web) $(CFLAGS_web) visualizations.cc -o test_visualizations.js

clean:
	rm -f d3_init.js transition.js *.js.map *.js.mem *~

# Debugging information
#print-%: ; @echo $*=$($*)
print-%: ; @echo '$(subst ','\'',$*=$($*))'
