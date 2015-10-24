GCOV_FLAGS = -g -fprofile-arcs -ftest-coverage

test: test_tools.cc
	g++ ${GCOV_FLAGS} -std=c++11 test_tools.cc -o test_tools.o
	# Execute tests
	./test_tools.o
	
coverage: test
	lcov -c -d . -b . -o cov.info

	# sanitize the tracefile for things we don't care about
	lcov --remove cov.info "*/third-party/*" -o cov.info
	lcov --remove cov.info "*/Empirical/tests/*" -o cov.info
	lcov --extract cov.info "*/Empirical/*" -o cov.info
	# build html coverage info
	genhtml cov.info --output-directory ./html/

clean:
	rm -f *.out
	rm -f *.o
	rm -f *.gcda
	rm -f *.gcno 
