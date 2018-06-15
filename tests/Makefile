GCOV_FLAGS = -o0 -g --coverage

TEST_NAMES = assert base evo constexpr data geometry meta scholar systematics tools games hardware

FLAGS = -std=c++14 -Wall -Wno-unused-function -I../source/ -I../
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
