default: test

include ../../Makefile-base.mk

TEST_FLAGS = -pthread -DCATCH_CONFIG_MAIN -I$(EMP_DIR)/ -I$(EMP_DIR)/third-party/cereal/include/
FLAGS = $(FLAGS_DEBUG) $(TEST_FLAGS)

cov-%: %.cpp ../../third-party/Catch/single_include/catch2/catch.hpp
	$(CXX) $(FLAGS_DEBUG) $< -o $@.out
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

coverage: FLAGS :=  $(FLAGS_DEBUG)  $(TEST_FLAGS) -I../../coverage_source/ -ftemplate-backtrace-limit=0 -fprofile-instr-generate -fcoverage-mapping -fno-inline -fno-elide-constructors -O0
coverage: test-prep $(addprefix cov-, $(TEST_NAMES))

# Test optimized version without debug features
opt: FLAGS :=  $(FLAGS_OPT) $(TEST_FLAGS)
opt: test-prep $(addprefix test-, $(TEST_NAMES))
	rm -rf test*.out

# Test in debug mode with pointer tracking
fulldebug: FLAGS :=  $(FLAGS_DEBUG)  $(TEST_FLAGS) -ftemplate-backtrace-limit=0
fulldebug: test-prep $(addprefix test-, $(TEST_NAMES))
	rm -rf test*.out

grumpy: FLAGS :=  $(FLAGS_GRUMPY) $(TEST_FLAGS)
grumpy: test-prep $(addprefix test-, $(TEST_NAMES))
	rm -rf test*.out

../../third-party/Catch/single_include/catch2/catch.hpp:
	git submodule init
	git submodule update

CLEAN_EXTRA = $(TEST_DIR)*.out $(TEST_DIR)*.o $(TEST_DIR)*.gcda $(TEST_DIR)*.gcno $(TEST_DIR)*.info $(TEST_DIR)*.gcov $(TEST_DIR)/Coverage* $(TEST_DIR)/temp $(TEST_DIR)/*.dSYM
