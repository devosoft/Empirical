GCOV_FLAGS := -o0 -g --coverage

TEST_NAMES :=

FLAGS := -std=c++17 -pthread -Wall -Wno-unused-function -I../source/ -I../ -I../third-party/cereal/include/ -msse4.2

#CXX = clang++
CXX := g++

default: test

test-prep:
	mkdir -p temp

test-%: test_%.cc
	$(CXX) $(FLAGS) $< -lstdc++fs -o $@.out
	#echo "running $@.out"
	# execute test
	./$@.out

cover-test-%: test_%.cc
	$(CXX) $(FLAGS) $< -lstdc++fs -o $@.out
	#echo "running $@.out"
	# execute test
	./$@.out
	llvm-profdata merge default.profraw -o default.profdata
	llvm-cov show ./$@.out -instr-profile=default.profdata > coverage_$@.txt
	python ../third-party/force-cover/fix_coverage.py coverage_$@.txt


# Test in debug mode without pointer tracker
test: test-prep $(addprefix test-, $(TEST_NAMES))
	rm -rf test*.out
	cd tools && make test
	cd base && make test
	cd Evolve && make test
	cd games && make test
	cd hardware && make test
	cd scholar && make test
	cd meta && make test
	cd constexpr && make test
	cd config && make test

# Test optimized version without debug features
opt: FLAGS := -std=c++17 -pthread -DNDEBUG -O3 -Wno-unused-function -DCI=$(CI) -I../source/ -I../third-party/cereal/include/ -I../
opt: test-prep $(addprefix test-, $(TEST_NAMES))
	rm -rf test*.out

# Test in debug mode with pointer tracking
fulldebug: FLAGS := -std=c++17 -pthread -g -Wall -Wno-unused-function -I../source/ -I../third-party/cereal/include/ -I../ -pedantic -DEMP_TRACK_MEM -Wnon-virtual-dtor -Wcast-align -Woverloaded-virtual -ftemplate-backtrace-limit=0 # -Wmisleading-indentation
fulldebug: test-prep $(addprefix test-, $(TEST_NAMES))
	rm -rf test*.out

cranky: FLAGS := -std=c++17 -pthread -g -Wall -Wno-unused-function -I../source/ -I../third-party/cereal/include/ -I../ -pedantic -DEMP_TRACK_MEM -Wnon-virtual-dtor -Wcast-align -Woverloaded-virtual -Wconversion -Weffc++
cranky: test-prep $(addprefix test-, $(TEST_NAMES))
	rm -rf test*.out

test-web:
	  cd web && bash run_tests.sh
	  cd .. && third-party/node_modules/karma/bin/karma start tests/web/karma.conf.js

coverage_conversion:
	  ./convert_for_tests.sh

test-coverage: FLAGS := -std=c++17 -pthread -g -Wall -Wno-unused-function -I../coverage_source/ -I../ -I../third-party/cereal/include/ -DEMP_TRACK_MEM -Wnon-virtual-dtor -Wcast-align -Woverloaded-virtual -ftemplate-backtrace-limit=0 -fprofile-instr-generate -fcoverage-mapping -fno-inline -fno-elide-constructors -O0
test-coverage: coverage_conversion test-prep $(addprefix cover-test-, $(TEST_NAMES))
	cd tools && make test-coverage
	cd base && make test-coverage
	cd Evolve && make test-coverage
	cd data && make test-coverage
	cd games && make test-coverage
	cd hardware && make test-coverage
	cd geometry && make test-coverage
	cd scholar && make test-coverage
	cd meta && make test-coverage
	cd constexpr && make test-coverage
	cd config && make test-coverage
	rm -r ../coverage_source

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
