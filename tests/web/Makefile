SHELL := /bin/bash

# @MAM 04-18-2024
# turned off as part of dejquerification, manually verified still working
# these tests should get fixed and reactivated at some point
# TODO ConfigPanel Card Modal ToggleSwitch CodeBlock FontAwesomeIcon ClickCounterDemo ClickCollapseDemo ValueBox ReadoutPanel
TEST_NAMES = Collapse LoadingModal CommentBox LoadingIcon Element TextFeed js_utils JSWrap Widget

# Currently a couple of the tests won't compile to native so this is a separate list for now. Eventually we should fix
# that and just have one list
NATIVE_TEST_NAMES = ConfigPanel LoadingModal Card CommentBox Modal ToggleSwitch CodeBlock LoadingIcon FontAwesomeIcon ClickCounterDemo Element TextFeed js_utils JSWrap Widget

# Flags to use regardless of compiler
CFLAGS_all := -Wall -Wno-unused-function -Wno-gnu-zero-variadic-macro-arguments -Wno-dollar-in-identifier-extension -std=c++20 -g -I../../include/ -I../../

# Emscripten compiler information
CXX_web := emcc
OFLAGS_web := -Werror -pedantic -Wno-dollar-in-identifier-extension -s TOTAL_MEMORY=67108864

#A bug in llvm compilers causes them to erroneously warn about braces around
#initializer lists - to compile files with them, we need to remove -Werror
OFLAGS_web_braces := -pedantic -Wno-dollar-in-identifier-extension -s TOTAL_MEMORY=67108864

CFLAGS_web := $(CFLAGS_all) $(OFLAGS_web_braces) --js-library ../../include/emp/web/library_emp.js --js-library ../../include/emp/web/d3/library_d3.js -s EXPORTED_FUNCTIONS="['_main', '_empCppCallback']" -s DISABLE_EXCEPTION_CATCHING=1 -s NO_EXIT_RUNTIME=1 -s EXTRA_EXPORTED_RUNTIME_METHODS='["ccall", "cwrap", "stringToUTF8"]' -s WASM=0

test-web: $(addprefix test-web-, $(TEST_NAMES))
# test-GetUrlParams

test-web-%: %.cpp
	source ../../third-party/emsdk/emsdk_env.sh; \
	$(CXX_web) $(CFLAGS_web) $< -o $@.js
	cp ../../third-party/package.json .
	npm install
	../../third-party/node_modules/karma/bin/karma start karma.conf.js --filename $@

test: $(addprefix test-native-, $(NATIVE_TEST_NAMES)) test-color_map

test-native-%: %.cpp
	$(CXX) $(CFLAGS_all) $< -o $@.out
	# execute test; on fail, run again and backtrace
	./$@.out \
		|| { gdb ./$@.out --ex="catch throw" --ex="set confirm off" --ex="run" --ex="backtrace" --ex="quit"; exit 1; }

test-color_map: color_map.cpp ../../third-party/Catch/single_include/catch2/catch.hpp
	g++ $(CFLAGS_all) -DCATCH_CONFIG_MAIN color_map.cpp -o color_map.out
	./color_map.out

test-GetUrlParams: GetUrlParams.cpp
	source ../../third-party/emsdk/emsdk_env.sh; \
	$(CXX_web) $(CFLAGS_web) $< -o $@.js
	node $@.js

../../third-party/Catch/single_include/catch2/catch.hpp:
	git submodule init
	git submodule update

cov-%: %.cpp ../../third-party/Catch/single_include/catch2/catch.hpp
	$(CXX) $(CFLAGS_all) $< -lstdc++fs -o $@.out
	#echo "running $@.out"
	# execute test
	./$@.out
	llvm-profdata merge default.profraw -o default.profdata
	llvm-cov show ./$@.out -instr-profile=default.profdata > coverage_$@.txt
	python ../../third-party/force-cover/fix_coverage.py coverage_$@.txt

coverage: CFLAGS_all := -std=c++20 -pthread -g -Wall -Wno-unused-function -I../../coverage_source/ -I../../ -I../../third-party/cereal/include/ -DEMP_TRACK_MEM -Wnon-virtual-dtor -Wcast-align -Woverloaded-virtual -ftemplate-backtrace-limit=0 -fprofile-instr-generate -fcoverage-mapping -fno-inline -fno-elide-constructors -O0
coverage: $(addprefix cov-, $(NATIVE_TEST_NAMES))

# Test in debug mode with pointer tracking
fulldebug: CFLAGS_all := -std=c++20 -pthread -g -Wall -Wno-unused-function -I../../include/ -I../../third-party/cereal/include/ -I../../ -pedantic -DEMP_TRACK_MEM -Wnon-virtual-dtor -Wcast-align -Woverloaded-virtual -ftemplate-backtrace-limit=0 # -Wmisleading-indentation
fulldebug: $(addprefix test-native-, $(NATIVE_TEST_NAMES))
	rm -rf test*.out

# Test optimized version without debug features
opt: FLAGS := -std=c++20 -pthread -DNDEBUG -O3 -Wno-unused-function -I../../include/ -I../../third-party/cereal/include/ -I../../ -DCATCH_CONFIG_MAIN
opt: $(addprefix test-native-, $(NATIVE_TEST_NAMES))
	rm -rf test*.out

cranky: FLAGS := -std=c++20 -pthread -g -Wall -Wno-unused-function -I../../include/ -I../../third-party/cereal/include/ -I../../ -pedantic -DEMP_TRACK_MEM -Wnon-virtual-dtor -Wcast-align -Woverloaded-virtual -Wconversion -Weffc++ -DCATCH_CONFIG_MAIN
cranky: $(addprefix test-native-, $(NATIVE_TEST_NAMES))
	rm -rf test*.out

clean:
	rm -f *.js.map *.js.mem *~ color_map.out
	ls *.js | grep -v karma.conf.js | xargs rm || true

# Debugging information
#print-%: ; @echo $*=$($*)
print-%: ; @echo '$(subst ','\'',$*=$($*))'
