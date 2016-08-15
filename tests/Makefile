GCOV_FLAGS = -o0 -g --coverage

test: test_driver.cc test_tools.cc
	$(CXX) -std=c++11 test_driver.cc -o test.o
	mkdir -p temp
	# Execute tests
	./test.o

coverage:
	$(CXX) ${GCOV_FLAGS} -std=c++11 test_driver.cc -o test.o
	./test.o
	gcov test_driver.cc

coverage-html: coverage
	mkdir -p html
	../third-party/env/bin/gcovr -e "third-party/catch/single_include/catch.hpp" -r ../ --html -o coverage.html

diff-cover: coverage
	git diff origin/master > ../scripts/diff
	cd ../scripts && python3 vigilance.py -i ignore_list diff

coverage-xml: coverage
	../third-party/env/bin/gcovr -e "third-party/catch/single_include/catch.hpp" -r ../ --xml-pretty > coverage.xml

old-diff-cover: coverage-xml
	cp coverage.xml ../coverage.xml
	cd .. && diff-cover coverage.xml > diff_cover.txt
	cat ../diff_cover.txt
	@printf "\n\n\nIf this fails, your coverage is bad!\n\n\n"
	grep "Missing: 0 lines" ../diff_cover.txt || grep "No lines with coverage" ../diff_cover.txt
	rm ../diff_cover.txt


clean:
	rm -f *.out
	rm -f *.o
	rm -f *.gcda
	rm -f *.gcno
	rm -f *.info
	rm -f *.gcov
	rm -f ./Coverage*
	rm -rf ./temp
