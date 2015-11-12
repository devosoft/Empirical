GCOV_FLAGS = -g -fprofile-arcs -ftest-coverage

test: test_driver.cc test_tools.cc
	g++ -std=c++11 test_driver.cc -o test.o
	# Execute tests
	./test.o
	
coverage:
	g++ ${GCOV_FLAGS} -std=c++11 test_driver.cc -o test.o
	./test.o

coverage-html: coverage
	../third-party/env/bin/gcovr -e "third-party/catch/single_include/catch.hpp" -r ../ --html -o coverage.html

coverage-xml: coverage
	../third-party/env/bin/gcovr -e "third-party/catch/single_include/catch.hpp" -r ../ --xml-pretty > coverage.xml

clean:
	rm -f *.out
	rm -f *.o
	rm -f *.gcda
	rm -f *.gcno
	rm -f *.info
	rm -f coverage*
