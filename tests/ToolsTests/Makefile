TEST_NAMES = alert bitmatrix bitset bitset_utils bitvector cache combos dfa file flex_function graph graph_utils lexer memo_function nfa range regex solvestate tuple_utils unorderedindexmap vector_utils

FLAGS = -std=c++14 -Wall -Wl,--stack,8388608 -Wno-unused-function -I../../source/ -I../../

default: test

test-prep:
	mkdir -p temp

test-%: test_%.cc
	$(CXX) $(FLAGS) $< -o $@.out
	# execute test
	./$@.out

# Test in debug mode without pointer tracker
test: test-prep $(addprefix test-, $(TEST_NAMES))
	rm -rf test*.out
  
clean:
	rm -f *.out
	rm -f *.o
	rm -rf ./temp