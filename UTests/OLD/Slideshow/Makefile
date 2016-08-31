# Flags to use regardless of compiler
CFLAGS_all := -Wall -std=c++11 -I../../ -I./

# Emscripten compiler information
CXX_web := emcc
#OFLAGS_web := -g4 -pedantic -Wno-dollar-in-identifier-extension -s TOTAL_MEMORY=67108864 -s ASSERTIONS=2 -s DEMANGLE_SUPPORT=1 # -s SAFE_HEAP=1
#OFLAGS_web := -g4 -DEMP_NDEBUG -pedantic -Wno-dollar-in-identifier-extension -s TOTAL_MEMORY=67108864 -s ASSERTIONS=2 -s DEMANGLE_SUPPORT=1 # -s SAFE_HEAP=1
#OFLAGS_web := -g4 -DNDEBUG -s TOTAL_MEMORY=67108864 -s DEMANGLE_SUPPORT=1 # -s SAFE_HEAP=1
OFLAGS_web := -DNDEBUG -s TOTAL_MEMORY=67108864
#OFLAGS_web := -Oz -DNDEBUG -s TOTAL_MEMORY=67108864 -s ASSERTIONS=1
#OFLAGS_web := -Os -DNDEBUG -s TOTAL_MEMORY=67108864


CFLAGS_web := $(CFLAGS_all) $(OFLAGS_web) --js-library ../../web/library_emp.js -s EXPORTED_FUNCTIONS="['_main', '_empCppCallback']" -s NO_EXIT_RUNTIME=1
#CFLAGS_web := $(CFLAGS_all) $(OFLAGS_web) --js-library ../../web/library_emp.js -s EXPORTED_FUNCTIONS="['_main', '_empCppCallback']" -s DISABLE_EXCEPTION_CATCHING=1 -s NO_EXIT_RUNTIME=1


default: clean all
all:  	Slideshow.js

Slideshow.js: Slideshow.cc
	$(CXX_web) $(CFLAGS_web) Slideshow.cc -o Slideshow.js


clean:
	rm -f Slideshow.js *.js.map *~ source/*.o source/*/*.o

# Debugging information
#print-%: ; @echo $*=$($*)
print-%: ; @echo '$(subst ','\'',$*=$($*))'
