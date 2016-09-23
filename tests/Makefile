GCOV_FLAGS = -o0 -g --coverage

EMC = emcc

TEST_NAMES = tools geometry evo scholar
EMCC_TEST_NAMES = tools geometry scholar

default: test

test-prep:
	mkdir -p temp

subtest-%: test_%.cc
	$(CXX) -std=c++11 $< -o $@.out
	#echo "running $@.out"
	# execute test
	./$@.out

emcc-subtest-%: test_%.cc
	$(EMC) -std=c++11 $< -o $@.js
	#echo "running $@.out"
	# execute test
	node $@.js

#native
test: test-prep $(addprefix subtest-, $(TEST_NAMES))
	rm -rf subtest*.out

# emcc
emcc-test: test-prep $(addprefix emcc-subtest-, $(EMCC_TEST_NAMES))
	rm -rf emcc-subtest*.js


clean:
	rm -f *.out
	rm -f *.o
	rm -f *.gcda
	rm -f *.gcno
	rm -f *.info
	rm -f *.gcov
	rm -f ./Coverage*
	rm -rf ./temp
	rm -rf *.js
