TEST_NAMES := base bits compiler config control data datastructs debug Evolve functional games geometry hardware io matching math meta native scholar testing tools web

CXX = clang++
#CXX := g++

default: test

test-%:
	cd $(@:test-%=%) && make test

test: $(addprefix test-, $(TEST_NAMES))

coverage-%:
	cd $(@:coverage-%=%) && make coverage

coverage: coverage_conversion $(addprefix coverage-, $(TEST_NAMES))

coverage_conversion:
	./convert_for_tests.sh

opt-%:
	cd $(@:opt-%=%) && make opt

opt: $(addprefix opt-, $(TEST_NAMES))

fulldebug-%:
	cd $(@:fulldebug-%=%) && make fulldebug

fulldebug: $(addprefix fulldebug-, $(TEST_NAMES))

cranky-%:
	cd $(@:cranky-%=%) && make cranky

cranky: $(addprefix cranky-, $(TEST_NAMES))

test-web-js:
	cd web && make test-web

clean: $(addprefix clean-, $(TEST_NAMES)) clean-web
	rm -f *.out
	rm -f *.o
	rm -f *.gcda
	rm -f *.gcno
	rm -f *.info
	rm -f *.gcov
	rm -f ./Coverage*
	rm -rf ./temp

clean-%:
	cd $(@:clean-%=%) && make clean
