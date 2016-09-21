GCOV_FLAGS = -o0 -g --coverage

TEST_NAMES = tools geometry evo scholar config

default: test

test-prep:
	mkdir -p temp

subtest-%: test_%.cc
	$(CXX) -std=c++11 $< -o $@.out
	#echo "running $@.out"
	# execute test
	./$@.out

test: test-prep $(addprefix subtest-, $(TEST_NAMES))
	rm -rf subtest*.out

test-web: 
	  cd web && make && for filename in *.js; do node $$filename; if [ $$? -gt 0 ]; then exit 1; fi; done

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
