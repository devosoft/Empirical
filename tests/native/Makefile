SHELL := /bin/bash
TEST_NAMES = CanvasShape Canvas CanvasImages Color

# -O3	  -Wl,--stack,8388608 -ftrack-macro-expansion=0
DISPLAY_ENV := ${DISPLAY}
FLAGS = -std=c++17 -pthread -Wall -Wno-unused-function -Wno-unused-private-field -I../../include/ -I../../ -I../../third-party/cereal/include/ -DEMP_HAS_X11=$(shell if test -z "${DISPLAY_ENV}" ; then echo 0; else echo 1; fi)

default: test

cov-%: %.cpp ../../third-party/Catch/single_include/catch2/catch.hpp
	$(CXX) $(FLAGS) $< -lsfml-graphics -lsfml-window -lsfml-system -o $@.out
	#echo "running $@.out"
	# execute test
	test -f ../../third-party/env/bin/activate && . ../../third-party/env/bin/activate; \
	until (./$@.out); do ((++fail > 5)) && echo "too many fails" && break || echo "failure ${fail}, retrying"; done
	llvm-profdata merge default.profraw -o default.profdata
	llvm-cov show ./$@.out -instr-profile=default.profdata > coverage_$@.txt
	python ../../third-party/force-cover/fix_coverage.py coverage_$@.txt

test-prep:
	mkdir -p temp

test-%: %.cpp ../../third-party/Catch/single_include/catch2/catch.hpp
	$(CXX) $(FLAGS) $< -lsfml-graphics -lsfml-window -lsfml-system -o $@.out
	# execute test
	test -f ../../third-party/env/bin/activate && . ../../third-party/env/bin/activate; \
	until (./$@.out); do ((++fail > 5)) && echo "too many fails" && break || echo "failure ${fail}, retrying"; done

# Test in debug mode without pointer tracker
test: test-prep $(addprefix test-, $(TEST_NAMES))
	rm -rf test*.out test_*.png

coverage: FLAGS := -std=c++17 -pthread -g -Wall -Wno-unused-function -I../../coverage_source/ -I../../ -I../../third-party/cereal/include/ -DEMP_HAS_X11=$(shell if test -z "${DISPLAY_ENV}" ; then echo 0; else echo 1; fi) -DEMP_TRACK_MEM -Wnon-virtual-dtor -Wcast-align -Woverloaded-virtual -ftemplate-backtrace-limit=0 -fprofile-instr-generate -fcoverage-mapping -fno-inline -fno-elide-constructors -O0
coverage: test-prep $(addprefix cov-, $(TEST_NAMES))

# Test optimized version without debug features
opt: FLAGS := -std=c++17 -pthread -DNDEBUG -O3 -Wno-unused-function -I../../include/ -I../../third-party/cereal/include/ -I../../
opt: test-prep $(addprefix test-, $(TEST_NAMES))
	rm -rf test*.out test_*.png

# Test in debug mode with pointer tracking
fulldebug: FLAGS := -std=c++17 -pthread -g -Wall -Wno-unused-function -I../../include/ -I../../third-party/cereal/include/ -I../../ -pedantic -DEMP_TRACK_MEM -Wnon-virtual-dtor -Wcast-align -Woverloaded-virtual -ftemplate-backtrace-limit=0 # -Wmisleading-indentation
fulldebug: test-prep $(addprefix test-, $(TEST_NAMES))
	rm -rf test*.out test_*.png

cranky: FLAGS := -std=c++17 -pthread -g -Wall -Wno-unused-function -I../../include/ -I../../third-party/cereal/include/ -I../../ -pedantic -DEMP_TRACK_MEM -Wnon-virtual-dtor -Wcast-align -Woverloaded-virtual -Wconversion -Weffc++
cranky: test-prep $(addprefix test-, $(TEST_NAMES))
	rm -rf test*.out test_*.png

../../third-party/Catch/single_include/catch2/catch.hpp:
	git submodule init
	git submodule update

clean:
	rm -f *.out
	rm -f *.o
	rm -f test_*.png
	rm -f *.gcda
	rm -f *.gcno
	rm -f *.info
	rm -f *.gcov
	rm -f ./Coverage*
	rm -rf ./temp
