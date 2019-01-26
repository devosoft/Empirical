GCOV_FLAGS = -o0 -g --coverage

TEST_NAMES = assert base constexpr data evo geometry meta scholar systematics tools games hardware

FLAGS = -std=c++14 -Wall -Wno-unused-function -I../source/ -I../
#CXX = clang++-6.0
#CXX = g++

default: test

test-prep:
	mkdir -p temp

test-%: test_%.cc
	$(CXX) $(FLAGS) $< -o $@.out
	#echo "running $@.out"
	# execute test
	./$@.out

cover-test-%: test_%.cc
	$(CXX) $(FLAGS) $< -o $@.out
	#echo "running $@.out"
	# execute test
	./$@.out
	llvm-profdata merge default.profraw -o default.profdata
	llvm-cov show ./$@.out -instr-profile=default.profdata > coverage_$@.txt
	python ../third-party/force-cover/fix_coverage.py coverage_$@.txt


# Test in debug mode without pointer tracker
test: test-prep $(addprefix test-, $(TEST_NAMES))
	rm -rf test*.out

# Test optimized version without debug features
opt: FLAGS := -std=c++14 -DNDEBUG -O3 -Wno-unused-function -I../source/ -I../ 
opt: test-prep $(addprefix test-, $(TEST_NAMES))
	rm -rf test*.out

# Test in debug mode with pointer tracking
fulldebug: FLAGS := -std=c++14 -g -Wall -Wno-unused-function -I../source/ -I../ -pedantic -DEMP_TRACK_MEM -Wnon-virtual-dtor -Wcast-align -Woverloaded-virtual -ftemplate-backtrace-limit=0 # -Wmisleading-indentation
fulldebug: test-prep $(addprefix test-, $(TEST_NAMES))
	rm -rf test*.out

cranky: FLAGS := -std=c++14 -g -Wall -Wno-unused-function -I../source/ -I../ -pedantic -DEMP_TRACK_MEM -Wnon-virtual-dtor -Wcast-align -Woverloaded-virtual -Wconversion -Weffc++
cranky: test-prep $(addprefix test-, $(TEST_NAMES))
	rm -rf test*.out

test-web:
	  cd web && bash run_tests.sh
	  cd .. && third-party/node_modules/karma/bin/karma start tests/web/karma.conf.js

coverage_conversion:
	  ./convert_for_tests.sh

test-coverage: FLAGS := -std=c++14 -g -Wall -Wno-unused-function -I../coverage_source/ -I../ -DEMP_TRACK_MEM -Wnon-virtual-dtor -Wcast-align -Woverloaded-virtual -ftemplate-backtrace-limit=0 -fprofile-instr-generate -fcoverage-mapping -fno-inline -fno-elide-constructors -O0 
test-coverage: coverage_conversion test-prep $(addprefix cover-test-, $(TEST_NAMES))
	       rm -r ../coverage_source

clean:
	rm -f *.out
	rm -f *.o
	rm -f *.gcda
	rm -f *.gcno
	rm -f *.info
	rm -f *.gcov
	rm -f ./Coverage*
	rm -rf ./temp
	cd web && make clean
