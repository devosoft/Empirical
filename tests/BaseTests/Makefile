TEST_NAMES = array assert errors ptr vector

#  -Wl,--stack,8388608 -ftrack-macro-expansion=0
FLAGS = -std=c++17 -DTDEBUG -Wall -Wno-unused-function -I../../source/ -I../../

default: test

cover-test-%: test_%.cc
	$(CXX) $(FLAGS) $< -o $@.out
	#echo "running $@.out"
	# execute test
	./$@.out
	llvm-profdata merge default.profraw -o default.profdata
	llvm-cov show ./$@.out -instr-profile=default.profdata > coverage_$@.txt
	python ../../third-party/force-cover/fix_coverage.py coverage_$@.txt

test-prep:
	mkdir -p temp

test-%: test_%.cc
	$(CXX) $(FLAGS) $< -o $@.out
	# execute test
	./$@.out

# Test in debug mode without pointer tracker
test: test-prep $(addprefix test-, $(TEST_NAMES))
	rm -rf test*.out
  
test-coverage: FLAGS := -std=c++17 -g -DTDEBUG -Wall -Wno-unused-function -I../../coverage_source/ -I../../ -DEMP_TRACK_MEM -Wnon-virtual-dtor -Wcast-align -Woverloaded-virtual -ftemplate-backtrace-limit=0 -fprofile-instr-generate -fcoverage-mapping -fno-inline -fno-elide-constructors -O0 
test-coverage: test-prep $(addprefix cover-test-, $(TEST_NAMES))
  
clean:
	rm -f *.out
	rm -f *.o
	rm -rf ./temp
