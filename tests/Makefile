GCOV_FLAGS = -o0 -g --coverage

TEST_NAMES = tools geometry evo scholar

default: test

test-prep:
	mkdir -p temp

subtest-%: test_%.cc
	$(CXX) -std=c++14 $< -o $@.out
	#echo "running $@.out"
	# execute test
	./$@.out

test: test-prep $(addprefix subtest-, $(TEST_NAMES))
	rm -rf subtest*.out

clean:
	rm -f *.out
	rm -f *.o
	rm -f *.gcda
	rm -f *.gcno
	rm -f *.info
	rm -f *.gcov
	rm -f ./Coverage*
	rm -rf ./temp
