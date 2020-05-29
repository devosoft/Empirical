TEST_NAMES := base config constexpr data Evolve games geometry hardware meta scholar tools

#CXX = clang++
CXX := g++

default: test

test-%:
	cd $(@:test-%=%) && make test

cover-test-%:
	cd $(@:test-%=%) && make cover-test

# Test in debug mode without pointer tracker
test: $(addprefix test-, $(TEST_NAMES))

opt-%:
	cd $(@:test-%=%) && make opt

fulldebug-%:
	cd $(@:test-%=%) && make fulldebug

cranky-%:
	cd $(@:test-%=%) && make cranky

test-web:
	  cd web && bash run_tests.sh
	  cd .. && third-party/node_modules/karma/bin/karma start tests/web/karma.conf.js

coverage_conversion:
	  ./convert_for_tests.sh

test-coverage: $(addprefix cover-test-, $(TEST_NAMES))

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
	cd $(@:test-%=%) && make clean
