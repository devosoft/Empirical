SHELL := /bin/bash

TEST_NAMES = ConfigPanel Collapse LoadingModal Card CommentBox Modal ToggleSwitch CodeBlock LoadingIcon FontAwesomeIcon ClickCounterDemo ClickCollapseDemo Element TextFeed js_utils JSWrap Widget visualizations

# Currently a couple of the tests won't compile to native so this is a separate list for now. Eventually we should fix
# that and just have one list
NATIVE_TEST_NAMES = ConfigPanel LoadingModal Card CommentBox Modal ToggleSwitch CodeBlock LoadingIcon FontAwesomeIcon ClickCounterDemo Element TextFeed js_utils JSWrap Widget

# Flags to use regardless of compiler
DISPLAY_ENV := ${DISPLAY}
CFLAGS_all := -Wall -Wno-unused-function -Wno-gnu-zero-variadic-macro-arguments -Wno-dollar-in-identifier-extension -std=c++17 -g -I../../include/ -I../../ -DEMP_HAS_X11=$(shell if test -z "${DISPLAY_ENV}" ; then echo 0; else echo 1; fi)

# Emscripten compiler information
CXX_web := emcc
OFLAGS_web := -Werror -pedantic -Wno-dollar-in-identifier-extension -s TOTAL_MEMORY=67108864

#A bug in llvm compilers causes them to erroneously warn about braces around
#initializer lists - to compile files with them, we need to remove -Werror
OFLAGS_web_braces := -pedantic -Wno-dollar-in-identifier-extension -s TOTAL_MEMORY=67108864

CFLAGS_web := $(CFLAGS_all) $(OFLAGS_web_braces) --js-library ../../include/emp/web/library_emp.js --js-library ../../include/emp/web/d3/library_d3.js -s EXPORTED_FUNCTIONS="['_main', '_empCppCallback', '_empDoCppCallback']" -s DISABLE_EXCEPTION_CATCHING=1 -s NO_EXIT_RUNTIME=1 -s EXTRA_EXPORTED_RUNTIME_METHODS='["ccall", "cwrap", "stringToUTF8"]' -s WASM=0

test-web: $(addprefix test-web-, $(TEST_NAMES)) test-GetUrlParams

test-web-%: %.cpp
	source ../../third-party/emsdk/emsdk_env.sh; \
	$(CXX_web) $(CFLAGS_web) $< -o $@.js
	cp ../../third-party/package.json .
	npm install
	../../third-party/node_modules/karma/bin/karma start karma.conf.js --filename $@

test: $(addprefix test-native-, $(NATIVE_TEST_NAMES)) test-color_map

test-native-%: %.cpp 
	$(CXX) $(CFLAGS_all) $< -lsfml-graphics -lsfml-window -lsfml-system -lstdc++fs -o $@.out
	# execute test; on fail, run again and backtrace
	./$@.out \
		|| { gdb ./$@.out --ex="catch throw" --ex="set confirm off" --ex="run" --ex="backtrace" --ex="quit"; exit 1; }

test-color_map: color_map.cpp ../../third-party/Catch/single_include/catch2/catch.hpp
	g++ $(CFLAGS_all) color_map.cpp -o color_map.out
	./color_map.out

test-GetUrlParams: GetUrlParams.cpp
	source ../../third-party/emsdk/emsdk_env.sh; \
	$(CXX_web) $(CFLAGS_web) $< -o $@.js
	node $@.js

test-CanvasShape: CanvasShape.cpp
	source ../../third-party/emsdk/emsdk_env.sh; \
	$(CXX_web) $(CFLAGS_web) $< -o $@.js
	node $@.js

../../third-party/Catch/single_include/catch2/catch.hpp:
	git submodule init
	git submodule update

cov-%: %.cpp ../../third-party/Catch/single_include/catch2/catch.hpp
	$(CXX) $(CFLAGS_all) $< -lsfml-graphics -lsfml-window -lsfml-system -lstdc++fs -o $@.out
	#echo "running $@.out"
	# execute test
	./$@.out
	llvm-profdata merge default.profraw -o default.profdata
	llvm-cov show ./$@.out -instr-profile=default.profdata > coverage_$@.txt
	python ../../third-party/force-cover/fix_coverage.py coverage_$@.txt

coverage: CFLAGS_all := -std=c++17 -pthread -g -Wall -Wno-unused-function -I../../coverage_source/ -I../../ -I../../third-party/cereal/include/ -DEMP_TRACK_MEM -Wnon-virtual-dtor -Wcast-align -Woverloaded-virtual -ftemplate-backtrace-limit=0 -fprofile-instr-generate -fcoverage-mapping -fno-inline -fno-elide-constructors -O0
coverage: $(addprefix cov-, $(NATIVE_TEST_NAMES))

# Test in debug mode with pointer tracking
fulldebug: CFLAGS_all := -std=c++17 -pthread -g -Wall -Wno-unused-function -I../../include/ -I../../third-party/cereal/include/ -I../../ -pedantic -DEMP_TRACK_MEM -Wnon-virtual-dtor -Wcast-align -Woverloaded-virtual -ftemplate-backtrace-limit=0 # -Wmisleading-indentation
fulldebug: $(addprefix test-native-, $(NATIVE_TEST_NAMES))
	rm -rf test*.out

# Test optimized version without debug features
opt: FLAGS := -std=c++17 -pthread -DNDEBUG -O3 -Wno-unused-function -I../../include/ -I../../third-party/cereal/include/ -I../../
opt: $(addprefix test-native-, $(NATIVE_TEST_NAMES))
	rm -rf test*.out

cranky: FLAGS := -std=c++17 -D_GLIBCXX_DEBUG -D_LIBCPP_DEBUG  -pthread -g -Wall -Wno-unused-function -I../../include/ -I../../third-party/cereal/include/ -I../../ -pedantic -DEMP_TRACK_MEM -Wnon-virtual-dtor -Wcast-align -Woverloaded-virtual -Wconversion -Weffc++
cranky: $(addprefix test-native-, $(NATIVE_TEST_NAMES))
	rm -rf test*.out

clean:
	rm -f *.js.map *.js.mem *~ color_map.out
	ls *.js | grep -v karma.conf.js | xargs rm || true

# Debugging information
#print-%: ; @echo $*=$($*)
print-%: ; @echo '$(subst ','\'',$*=$($*))'
