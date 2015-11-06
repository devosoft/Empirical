GCOV_FLAGS = -g -fprofile-arcs -ftest-coverage

test: test_driver.cc test_tools.cc
	g++ -std=c++11 test_driver.cc -o test.o
	# Execute tests
	./test.o
	
coverage:
	g++ ${GCOV_FLAGS} -std=c++11 test_driver.cc -o test.o
	./test.o
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
	rm -rf ./html
