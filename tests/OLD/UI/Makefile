# Flags to use regardless of compiler
#CFLAGS_all := -Wall -Wno-unused-function -std=c++11 -I../../ -I./
CFLAGS_all := -Wall -Wno-unused-variable -Wno-unused-function -std=c++11 -I../../ -I./

# Emscripten compiler information
CXX_web := emcc
OFLAGS_web := -g4 -pedantic -Wno-dollar-in-identifier-extension -s TOTAL_MEMORY=67108864 -s ASSERTIONS=2 -s DEMANGLE_SUPPORT=1 # -s SAFE_HEAP=1
#OFLAGS_web := -g4 -DEMP_NDEBUG -pedantic -Wno-dollar-in-identifier-extension -s TOTAL_MEMORY=67108864 -s ASSERTIONS=2 -s DEMANGLE_SUPPORT=1 # -s SAFE_HEAP=1
#OFLAGS_web := -g4 -DNDEBUG -s TOTAL_MEMORY=67108864 -s DEMANGLE_SUPPORT=1 # -s SAFE_HEAP=1
#OFLAGS_web := -Werror -DNDEBUG -s TOTAL_MEMORY=67108864
#OFLAGS_web := -Oz -DNDEBUG -s TOTAL_MEMORY=67108864 -s ASSERTIONS=1
#OFLAGS_web := -Os -DNDEBUG -s TOTAL_MEMORY=67108864


CFLAGS_web := $(CFLAGS_all) $(OFLAGS_web) --js-library ../../web/library_emp.js -s EXPORTED_FUNCTIONS="['_main', '_empCppCallback']" -s NO_EXIT_RUNTIME=1
#CFLAGS_web := $(CFLAGS_all) $(OFLAGS_web) --js-library ../../web/library_emp.js -s EXPORTED_FUNCTIONS="['_main', '_empCppCallback']" -s DISABLE_EXCEPTION_CATCHING=1 -s NO_EXIT_RUNTIME=1


default: all
all: Animate.js Canvas.js canvas_utils.js events.js ElementSlate.js ElementTable.js Example.js mini_evoke.js Slideshow.js

Animate.js: Animate.cc
	$(CXX_web) $(CFLAGS_web) Animate.cc -o Animate.js

Canvas.js: Canvas.cc
	$(CXX_web) $(CFLAGS_web) Canvas.cc -o Canvas.js

canvas_utils.js: canvas_utils.cc
	$(CXX_web) $(CFLAGS_web) canvas_utils.cc -o canvas_utils.js

events.js: events.cc
	$(CXX_web) $(CFLAGS_web) events.cc -o events.js

ElementSlate.js: ElementSlate.cc
	$(CXX_web) $(CFLAGS_web) ElementSlate.cc -o ElementSlate.js

ElementTable.js: ElementTable.cc
	$(CXX_web) $(CFLAGS_web) ElementTable.cc -o ElementTable.js

Example.js: Example.cc
	$(CXX_web) $(CFLAGS_web) Example.cc -o Example.js

mini_evoke.js: mini_evoke.cc
	$(CXX_web) $(CFLAGS_web) mini_evoke.cc -o mini_evoke.js

Slideshow.js: Slideshow.cc
	$(CXX_web) $(CFLAGS_web) Slideshow.cc -o Slideshow.js

# test3.js: test3.cc
# 	$(CXX_web) $(CFLAGS_web) test3.cc -o test3.js

# test4.js: test4.cc
# 	$(CXX_web) $(CFLAGS_web) test4.cc -o test4.js

clean:
	rm -f Animate.js Canvas.js canvas_utils.js events.js ElementSlate.js ElementTable.js Example.js mini_evoke.js Slideshow.js test?.js *.js.map *~ source/*.o source/*/*.o

# Debugging information
#print-%: ; @echo $*=$($*)
print-%: ; @echo '$(subst ','\'',$*=$($*))'
