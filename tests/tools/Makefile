TEST_NAMES = alert BitMatrix BitSet bitset_utils BitVector Cache combos ContiguousStream DFA Distribution DynamicString File flex_function functions FunctionSet genericfunction Graph graph_utils hash_utils IndexMap info_theory keyname_utils Lexer lexer_utils map_utils MatchBin matchbin_utils math memo_function mem_track NFA NullStream Random random_utils Range RegEx sequence_utils set_utils SolveState stats string_utils tuple_utils TypeTracker UnorderedIndexMap vector_utils

#  -Wl,--stack,8388608 -ftrack-macro-expansion=0
FLAGS = -std=c++17 -pthread -Wall -Wno-unused-function -I../../source/ -I../../ -I../../third-party/cereal/include/

default: test

cover-test-%: %.cc
	$(CXX) $(FLAGS) $< -lstdc++fs -o $@.out
	#echo "running $@.out"
	# execute test
	./$@.out
	llvm-profdata merge default.profraw -o default.profdata
	llvm-cov show ./$@.out -instr-profile=default.profdata > coverage_$@.txt
	python ../../third-party/force-cover/fix_coverage.py coverage_$@.txt

test-prep:
	mkdir -p temp

test-%: %.cc
	$(CXX) $(FLAGS) $< -lstdc++fs -o $@.out
	# execute test
	./$@.out

# Test in debug mode without pointer tracker
test: test-prep $(addprefix test-, $(TEST_NAMES))
	rm -rf test*.out

test-coverage: FLAGS := -std=c++17 -g -pthread -Wall -Wno-unused-function -I../../coverage_source/ -I../../ -I../../third-party/cereal/include/ -DEMP_TRACK_MEM -Wnon-virtual-dtor -Wcast-align -Woverloaded-virtual -ftemplate-backtrace-limit=0 -fprofile-instr-generate -fcoverage-mapping -fno-inline -fno-elide-constructors -O0
test-coverage: test-prep $(addprefix cover-test-, $(TEST_NAMES))

clean:
	rm -f *.out
	rm -f *.o
	rm -rf ./temp
