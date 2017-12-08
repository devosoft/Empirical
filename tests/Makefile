GCOV_FLAGS = -o0 -g --coverage
 
TEST_NAMES = base evo geometry meta scholar tools
FLAGS = -std=c++14
#CXX = clang++
#CXX = g++

default: test

test-prep:
	mkdir -p temp

test-%: test_%.cc
	$(CXX) $(FLAGS) $< -o $@.out
	#echo "running $@.out"
	# execute test
	./$@.out

test: test-prep $(addprefix test-, $(TEST_NAMES))
	rm -rf test*.out

opt: FLAGS := -std=c++14 -DNDEBUG -O3
opt: test-prep $(addprefix test-, $(TEST_NAMES))
	rm -rf test*.out

debug: FLAGS := -std=c++14 -g -pedantic -DEMP_TRACK_MEM -Wnon-virtual-dtor -Wcast-align -Woverloaded-virtual -ftemplate-backtrace-limit=0 # -Wmisleading-indentation
debug: test-prep $(addprefix test-, $(TEST_NAMES))
	rm -rf test*.out

cranky: FLAGS := -std=c++14 -g -pedantic -DEMP_TRACK_MEM -Wnon-virtual-dtor -Wcast-align -Woverloaded-virtual -Wconversion -Weffc++
cranky: test-prep $(addprefix test-, $(TEST_NAMES))
	rm -rf test*.out

test-web:
	  cd web && bash run_tests.sh
	  cd .. && third-party/node_modules/karma/bin/karma start tests/web/karma.conf.js

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
