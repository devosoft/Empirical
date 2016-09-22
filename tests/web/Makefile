# Flags to use regardless of compiler
CFLAGS_all := -Wall -Wno-unused-function -std=c++11 -I../../

# Emscripten compiler information
CXX_web := emcc
OFLAGS_web := -g4 -Werror -pedantic -Wno-dollar-in-identifier-extension -s TOTAL_MEMORY=67108864

#A bug in llvm compilers causes them to erroneously warn about braces around
#initializer lists - to compile files with them, we need to remove -Werror
OFLAGS_web_braces := -g4 -pedantic -Wno-dollar-in-identifier-extension -s TOTAL_MEMORY=67108864
#OFLAGS_web := -Werror -DNDEBUG -s TOTAL_MEMORY=67108864
#OFLAGS_web := -Oz -Werror -DNDEBUG -s TOTAL_MEMORY=67108864 -s ASSERTIONS=1
#OFLAGS_web := -O3 -Werror -DNDEBUG -s TOTAL_MEMORY=67108864


CFLAGS_web := $(CFLAGS_all) $(OFLAGS_web) --js-library ../../web/library_emp.js -s EXPORTED_FUNCTIONS="['_main', '_empCppCallback']" -s DISABLE_EXCEPTION_CATCHING=1 -s NO_EXIT_RUNTIME=1
CFLAGS_web_braces := $(CFLAGS_all) $(OFLAGS_web_braces) --js-library ../../web/library_emp.js -s EXPORTED_FUNCTIONS="['_main', '_empCppCallback']" -s DISABLE_EXCEPTION_CATCHING=1 -s NO_EXIT_RUNTIME=1 -s DEMANGLE_SUPPORT=1

default: all
all: JSWrap.js js_utils.js

JSWrap.js: JSWrap.cc
	$(CXX_web) $(CFLAGS_web) JSWrap.cc -o JSWrap.js

js_utils.js: js_utils.cc
	$(CXX_web) $(CFLAGS_web_braces) js_utils.cc -o js_utils.js

clean:
	rm -f JSWrap.js js_utils.js *.js.map *~

# Debugging information
#print-%: ; @echo $*=$($*)
print-%: ; @echo '$(subst ','\'',$*=$($*))'
