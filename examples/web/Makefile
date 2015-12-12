# Flags to use regardless of compiler
#CFLAGS_all := -Wall -Wno-unused-function -std=c++11 -I../../ -I./
CFLAGS_all := -Wall -Wno-unused-variable -Wno-unused-function -std=c++11 -I../../ -I./

# Emscripten compiler information
CXX_web := emcc
CXX_native := g++

OFLAGS_native := -g -pedantic

OFLAGS_web := -g4 -pedantic -Wno-dollar-in-identifier-extension -s TOTAL_MEMORY=67108864 -s ASSERTIONS=2 -s DEMANGLE_SUPPORT=1 # -s SAFE_HEAP=1
#OFLAGS_web := -g4 -DEMP_NDEBUG -pedantic -Wno-dollar-in-identifier-extension -s TOTAL_MEMORY=67108864 -s ASSERTIONS=2 -s DEMANGLE_SUPPORT=1 # -s SAFE_HEAP=1
#OFLAGS_web := -g4 -DNDEBUG -s TOTAL_MEMORY=67108864 -s DEMANGLE_SUPPORT=1 # -s SAFE_HEAP=1
#OFLAGS_web := -Werror -DNDEBUG -s TOTAL_MEMORY=67108864
#OFLAGS_web := -Oz -DNDEBUG -s TOTAL_MEMORY=67108864 -s ASSERTIONS=1
#OFLAGS_web := -Os -DNDEBUG -s TOTAL_MEMORY=67108864


CFLAGS_native := $(CFLAGS_all) $(OFLAGS_native)

CFLAGS_web := $(CFLAGS_all) $(OFLAGS_web) --js-library ../../emtools/library_emp.js -s EXPORTED_FUNCTIONS="['_main', '_empCppCallback']" -s NO_EXIT_RUNTIME=1
#CFLAGS_web := $(CFLAGS_all) $(OFLAGS_web) --js-library ../../emtools/library_emp.js -s EXPORTED_FUNCTIONS="['_main', '_empCppCallback']" -s DISABLE_EXCEPTION_CATCHING=1 -s NO_EXIT_RUNTIME=1


default: web

web: Animate.js Example.js keypress.js Slate.js Table.js Web.js
native: debug-Animate debug-keypress debug-Slate debug-web

Animate.js: Animate.cc
	$(CXX_web) $(CFLAGS_web) Animate.cc -o Animate.js

Example.js: Example.cc
	$(CXX_web) $(CFLAGS_web) Example.cc -o Example.js

keypress.js: keypress.cc
	$(CXX_web) $(CFLAGS_web) keypress.cc -o keypress.js

Slate.js: Slate.cc
	$(CXX_web) $(CFLAGS_web) Slate.cc -o Slate.js

Table.js: Table.cc
	$(CXX_web) $(CFLAGS_web) Table.cc -o Table.js

TextArea.js: TextArea.cc
	$(CXX_web) $(CFLAGS_web) TextArea.cc -o TextArea.js

Tween.js: Tween.cc
	$(CXX_web) $(CFLAGS_web) Tween.cc -o Tween.js

Web.js: Web.cc
	$(CXX_web) $(CFLAGS_web) Web.cc -o Web.js

debug-%: $*.cc
	$(CXX_native) $(CFLAGS_native) $< -o $@

clean:
	rm -f debug-* Animate.js Example.js keypress.js Slate.js Table.js TextArea.js Tween.js Web.js *.js.map *~ source/*.o source/*/*.o

# Debugging information
#print-%: ; @echo $*=$($*)
print-%: ; @echo '$(subst ','\'',$*=$($*))'
