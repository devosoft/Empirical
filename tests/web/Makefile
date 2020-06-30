# Flags to use regardless of compiler
CFLAGS_all := -Wall -Wno-unused-function -Wno-gnu-zero-variadic-macro-arguments -Wno-dollar-in-identifier-extension -std=c++17 -I../../source/

# Emscripten compiler information
CXX_web := emcc
OFLAGS_web := -Werror -pedantic -Wno-dollar-in-identifier-extension -s TOTAL_MEMORY=67108864

#A bug in llvm compilers causes them to erroneously warn about braces around
#initializer lists - to compile files with them, we need to remove -Werror
OFLAGS_web_braces := -pedantic -Wno-dollar-in-identifier-extension -s TOTAL_MEMORY=67108864
#OFLAGS_web := -Werror -DNDEBUG -s TOTAL_MEMORY=67108864
#OFLAGS_web := -Oz -Werror -DNDEBUG -s TOTAL_MEMORY=67108864 -s ASSERTIONS=1
#OFLAGS_web := -O3 -Werror -DNDEBUG -s TOTAL_MEMORY=67108864


CFLAGS_web := $(CFLAGS_all) $(OFLAGS_web_braces) --js-library ../../source/web/library_emp.js --js-library ../../source/web/d3/library_d3.js -s EXPORTED_FUNCTIONS="['_main', '_empCppCallback']" -s DISABLE_EXCEPTION_CATCHING=1 -s NO_EXIT_RUNTIME=1 -s EXTRA_EXPORTED_RUNTIME_METHODS='["ccall", "cwrap", "stringToUTF8"]' -s WASM=0

default: all
all: GetUrlParams.js js_utils.js test_visualizations.js test_widget.js test_jswrap.js color_map

color_map: color_map.cc
	g++ $(CFLAGS_all) color_map.cc -o color_map

GetUrlParams.js: GetUrlParams.cc
	$(CXX_web) $(CFLAGS_web) GetUrlParams.cc -o GetUrlParams.js

js_utils.js: js_utils.cc
	$(CXX_web) $(CFLAGS_web) js_utils.cc -o js_utils.js

test_visualizations.js: visualizations.cc
	$(CXX_web) $(CFLAGS_web) visualizations.cc -o test_visualizations.js

test_widget.js: test_widget.cc
	$(CXX_web) $(CFLAGS_web) test_widget.cc -o test_widget.js

test_jswrap.js: TestJSWrap.cc
	$(CXX_web) $(CFLAGS_web) TestJSWrap.cc -o test_jswrap.js
	../../third-party/node_modules/karma/bin/karma start karma.conf.js --filename test_jswrap


clean:
	rm -f GetUrlParams.js JSWrap.js js_utils.js test_visualizations.js test_widget.js test_jswrap.js *.js.map *.js.mem *~ color_map

# Debugging information
#print-%: ; @echo $*=$($*)
print-%: ; @echo '$(subst ','\'',$*=$($*))'
