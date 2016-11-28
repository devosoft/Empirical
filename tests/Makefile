GCOV_FLAGS = -o0 -g --coverage

TEST_NAMES = evo geometry meta scholar tools

default: test

test-prep:
	mkdir -p temp

test-%: test_%.cc
	$(CXX) -std=c++14 $< -o $@.out
	#echo "running $@.out"
	# execute test
	./$@.out

test: test-prep $(addprefix test-, $(TEST_NAMES))
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
