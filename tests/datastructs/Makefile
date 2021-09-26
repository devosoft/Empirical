TEST_NAMES = BloomFilter Bool Cache DynamicString Graph graph_utils hash_utils IndexMap map_utils QueueCache ra_set reference_vector set_utils SmallFifoMap SmallVector StringMap TimeQueue tuple_struct tuple_utils TypeMap UnorderedIndexMap valsort_map vector_utils

# -O3	  -Wl,--stack,8388608 -ftrack-macro-expansion=0
FLAGS = -std=c++17 -g -pthread -Wall -Wno-unused-function -Wno-unused-private-field -I../../include/ -I../../ -I../../third-party/cereal/include/ -DCATCH_CONFIG_MAIN

default: test

cov-%: %.cpp ../../third-party/Catch/single_include/catch2/catch.hpp
	$(CXX) $(FLAGS) $< -o $@.out
	#echo "running $@.out"
	# execute test
	./$@.out
	llvm-profdata merge default.profraw -o default.profdata
	llvm-cov show ./$@.out -instr-profile=default.profdata > coverage_$@.txt
	python ../../third-party/force-cover/fix_coverage.py coverage_$@.txt

test-prep:
	mkdir -p temp

test-%: %.cpp ../../third-party/Catch/single_include/catch2/catch.hpp
	$(CXX) $(FLAGS) $< -o $@.out
	# execute test; on fail, run again and backtrace
	./$@.out \
		|| { gdb ./$@.out --ex="catch throw" --ex="set confirm off" --ex="run" --ex="backtrace" --ex="quit"; exit 1; }

# Test in debug mode without pointer tracker
test: test-prep $(addprefix test-, $(TEST_NAMES))
	rm -rf test*.out

coverage: FLAGS := -std=c++17 -pthread -g -Wall -Wno-unused-function -I../../coverage_source/ -I../../ -I../../third-party/cereal/include/ -DEMP_TRACK_MEM -Wnon-virtual-dtor -Wcast-align -Woverloaded-virtual -ftemplate-backtrace-limit=0 -fprofile-instr-generate -fcoverage-mapping -fno-inline -fno-elide-constructors -O0 -DCATCH_CONFIG_MAIN
coverage: test-prep $(addprefix cov-, $(TEST_NAMES))

# Test optimized version without debug features
opt: FLAGS := -std=c++17 -pthread -DNDEBUG -O3 -Wno-unused-function -I../../include/ -I../../third-party/cereal/include/ -I../../ -DCATCH_CONFIG_MAIN
opt: test-prep $(addprefix test-, $(TEST_NAMES))
	rm -rf test*.out

# Test in debug mode with pointer tracking
fulldebug: FLAGS := -std=c++17 -pthread -g -Wall -Wno-unused-function -I../../include/ -I../../third-party/cereal/include/ -I../../ -pedantic -DEMP_TRACK_MEM -Wnon-virtual-dtor -Wcast-align -Woverloaded-virtual -ftemplate-backtrace-limit=0 -DCATCH_CONFIG_MAIN # -Wmisleading-indentation
fulldebug: test-prep $(addprefix test-, $(TEST_NAMES))
	rm -rf test*.out

cranky: FLAGS := -std=c++17 -pthread -g -Wall -Wno-unused-function -I../../include/ -I../../third-party/cereal/include/ -I../../ -pedantic -DEMP_TRACK_MEM -Wnon-virtual-dtor -Wcast-align -Woverloaded-virtual -Wconversion -Weffc++ -DCATCH_CONFIG_MAIN
cranky: test-prep $(addprefix test-, $(TEST_NAMES))
	rm -rf test*.out

../../third-party/Catch/single_include/catch2/catch.hpp:
	git submodule init
	git submodule update

clean:
	rm -f *.out
	rm -f *.o
	rm -f *.gcda
	rm -f *.gcno
	rm -f *.info
	rm -f *.gcov
	rm -f ./Coverage*
	rm -rf ./temp
