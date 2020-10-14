TEST_NAMES := base config constexpr data datastructs Evolve games geometry hardware matching meta scholar tools

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

test-web:
	  cd web && bash run_tests.sh

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
