SHELL := /bin/bash

# Flags to use regardless of compiler
CFLAGS_all := -Wall -Wno-unused-function -Wno-gnu-zero-variadic-macro-arguments -Wno-dollar-in-identifier-extension -std=c++17 -I../../../include/emp/ -I../../../include/

TEST_NAMES = d3_init selection scales axis transition shapes

# Emscripten compiler information
CXX_web := emcc
OFLAGS_web := -Werror -pedantic -Wno-dollar-in-identifier-extension -s TOTAL_MEMORY=67108864

#A bug in llvm compilers causes them to erroneously warn about braces around
#initializer lists - to compile files with them, we need to remove -Werror
OFLAGS_web_braces := -pedantic -Wno-dollar-in-identifier-extension -s TOTAL_MEMORY=67108864
#OFLAGS_web := -Werror -DNDEBUG -s TOTAL_MEMORY=67108864
#OFLAGS_web := -Oz -Werror -DNDEBUG -s TOTAL_MEMORY=67108864 -s ASSERTIONS=1
#OFLAGS_web := -O3 -Werror -DNDEBUG -s TOTAL_MEMORY=67108864

CFLAGS_web := $(CFLAGS_all) $(OFLAGS_web_braces) --js-library ../../../include/emp/web/library_emp.js --js-library ../../../include/emp/web/d3/library_d3.js -s EXPORTED_FUNCTIONS="['_main', '_empCppCallback']" -s DISABLE_EXCEPTION_CATCHING=1 -s NO_EXIT_RUNTIME=1 -s EXTRA_EXPORTED_RUNTIME_METHODS='["ccall", "cwrap", "stringToUTF8"]' -s WASM=0

default: test-web

test-web: $(addprefix test-web-, $(TEST_NAMES))

test-web-%: %.cpp
	source ../../../third-party/emsdk/emsdk_env.sh; \
	$(CXX_web) $(CFLAGS_web) $< -o $@.js
	cd ../../../ && third-party/node_modules/karma/bin/karma start tests/web/karma.conf.js --filename d3/$@
	
../../third-party/Catch/single_include/catch2/catch.hpp:
	git submodule init
	git submodule update

coverage: CFLAGS_all := -std=c++17 -pthread -g -Wall -Wno-unused-function -I../../coverage_source/emp -I../../../include/emp -I../../../include -I../../third-party/cereal/include/ -Wnon-virtual-dtor -Wcast-align -Woverloaded-virtual -ftemplate-backtrace-limit=0 -fprofile-instr-generate -fcoverage-mapping -fno-inline -fno-elide-constructors -O0 # -DEMP_TRACK_MEM
coverage: $(addprefix cov-, $(TEST_NAMES))

cov-%: %.cpp ../../../third-party/Catch/single_include/catch2/catch.hpp
	$(CXX) $(CFLAGS_all) $< -lstdc++fs -o $@.out
	#echo "running $@.out"
	# execute test
	./$@.out
	llvm-profdata merge default.profraw -o default.profdata
	llvm-cov show ./$@.out -instr-profile=default.profdata > coverage_$@.txt
	python ../../../third-party/force-cover/fix_coverage.py coverage_$@.txt

clean:
	rm -f d3_init.js selection.js scales.js axis.js transition.js *.js.map *.js.mem *~

# Debugging information
#print-%: ; @echo $*=$($*)
print-%: ; @echo '$(subst ','\'',$*=$($*))'
