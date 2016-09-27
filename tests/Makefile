GCOV_FLAGS = -o0 -g --coverage
CXX = g++-5
TEST_NAMES = tools geometry evo scholar config

default: test

test-prep:
	mkdir -p temp

subtest-%: test_%.cc
	$(CXX) -std=c++11 $< -o $@.out
	#echo "running $@.out"
	# execute test
	./$@.out

test: test-web test-prep $(addprefix subtest-, $(TEST_NAMES))
	rm -rf subtest*.out

test-web: 
	  cd web && bash run_tests.sh

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
