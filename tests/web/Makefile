# Flags to use regardless of compiler
CFLAGS_all := -Wall -Wno-unused-function -Wno-gnu-zero-variadic-macro-arguments -Wno-dollar-in-identifier-extension -std=c++17 -I../../include/ -I../../

# Emscripten compiler information
CXX_web := emcc
OFLAGS_web := -Werror -pedantic -Wno-dollar-in-identifier-extension -s TOTAL_MEMORY=67108864

#A bug in llvm compilers causes them to erroneously warn about braces around
#initializer lists - to compile files with them, we need to remove -Werror
OFLAGS_web_braces := -pedantic -Wno-dollar-in-identifier-extension -s TOTAL_MEMORY=67108864
#OFLAGS_web := -Werror -DNDEBUG -s TOTAL_MEMORY=67108864
#OFLAGS_web := -Oz -Werror -DNDEBUG -s TOTAL_MEMORY=67108864 -s ASSERTIONS=1
#OFLAGS_web := -O3 -Werror -DNDEBUG -s TOTAL_MEMORY=67108864

CFLAGS_web := $(CFLAGS_all) $(OFLAGS_web_braces) --js-library ../../include/emp/web/library_emp.js --js-library ../../include/emp/web/d3/library_d3.js -s EXPORTED_FUNCTIONS="['_main', '_empCppCallback']" -s DISABLE_EXCEPTION_CATCHING=1 -s NO_EXIT_RUNTIME=1 -s EXTRA_EXPORTED_RUNTIME_METHODS='["ccall", "cwrap", "stringToUTF8"]' -s WASM=0

default: all
all: jswrap.js element.js textfeed.js GetUrlParams.js js_utils.js test_visualizations.js widget.js color_map

color_map: color_map.cpp
	g++ $(CFLAGS_all) color_map.cpp -o color_map.out

GetUrlParams.js: GetUrlParams.cpp
	$(CXX_web) $(CFLAGS_web) GetUrlParams.cpp -o GetUrlParams.js

js_utils.js: js_utils.cpp
	$(CXX_web) $(CFLAGS_web) js_utils.cpp -o js_utils.js

test_visualizations.js: visualizations.cpp
	$(CXX_web) $(CFLAGS_web) visualizations.cpp -o test_visualizations.js

jswrap.js: JSWrap.cpp
	$(CXX_web) $(CFLAGS_web) JSWrap.cpp -o jswrap.js

widget.js: Widget.cpp
	$(CXX_web) $(CFLAGS_web) Widget.cpp -o widget.js

element.js: Element.cpp
	$(CXX_web) $(CFLAGS_web) Element.cpp -o element.js

textfeed.js: TextFeed.cpp
	$(CXX_web) $(CFLAGS_web) TextFeed.cpp -o textfeed.js

clean:
	rm -f element.js TextFeed.js GetUrlParams.js JSWrap.js widget.js js_utils.js test_visualizations.js jswrap.js *.js.map *.js.mem *~ color_map.out

# Debugging information
#print-%: ; @echo $*=$($*)
print-%: ; @echo '$(subst ','\'',$*=$($*))'
