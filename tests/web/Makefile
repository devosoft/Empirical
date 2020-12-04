SHELL := /bin/bash

TEST_NAMES = ConfigPanel Collapse LoadingModal Card CommentBox Modal ToggleSwitch CodeBlock LoadingIcon FontAwesomeIcon ClickCounterDemo ClickCollapseDemo Element TextFeed js_utils visualizations JSWrap Widget

# Flags to use regardless of compiler
CFLAGS_all := -Wall -Wno-unused-function -Wno-gnu-zero-variadic-macro-arguments -Wno-dollar-in-identifier-extension -std=c++17 -I../../include/ -I../../

# Emscripten compiler information
CXX_web := emcc
OFLAGS_web := -Werror -pedantic -Wno-dollar-in-identifier-extension -s TOTAL_MEMORY=67108864

#A bug in llvm compilers causes them to erroneously warn about braces around
#initializer lists - to compile files with them, we need to remove -Werror
OFLAGS_web_braces := -pedantic -Wno-dollar-in-identifier-extension -s TOTAL_MEMORY=67108864

CFLAGS_web := $(CFLAGS_all) $(OFLAGS_web_braces) --js-library ../../include/emp/web/library_emp.js --js-library ../../include/emp/web/d3/library_d3.js -s EXPORTED_FUNCTIONS="['_main', '_empCppCallback']" -s DISABLE_EXCEPTION_CATCHING=1 -s NO_EXIT_RUNTIME=1 -s EXTRA_EXPORTED_RUNTIME_METHODS='["ccall", "cwrap", "stringToUTF8"]' -s WASM=0

test: $(addprefix test-, $(TEST_NAMES)) test-color_map test-GetUrlParams

test-%: %.cpp
	source ../../third-party/emsdk/emsdk_env.sh; \
	$(CXX_web) $(CFLAGS_web) $< -o $@.js
	cp ../../third-party/package.json .
	npm install
	../../third-party/node_modules/karma/bin/karma start karma.conf.js --filename $@

test-color_map: color_map.cpp ../../third-party/Catch/single_include/catch2/catch.hpp
	g++ $(CFLAGS_all) color_map.cpp -o color_map.out
	./color_map.out

test-GetUrlParams: GetUrlParams.cpp
	source ../../third-party/emsdk/emsdk_env.sh; \
	$(CXX_web) $(CFLAGS_web) $< -o $@.js
	node $@.js

../../third-party/Catch/single_include/catch2/catch.hpp:
	git submodule init
	git submodule update

clean:
	rm -f *.js.map *.js.mem *~ color_map.out
	ls *.js | grep -v karma.conf.js | xargs rm

# Debugging information
#print-%: ; @echo $*=$($*)
print-%: ; @echo '$(subst ','\'',$*=$($*))'
