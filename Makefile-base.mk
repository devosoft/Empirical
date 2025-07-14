# This is base Makefile for Empirical that sets up variables for all of the others to include.

.PHONY: clean print-%

# Identify all directory locations
# Get the path of this file and then its directory.
MAKEFILE_PATH = $(lastword $(MAKEFILE_LIST))
EMP_DIR = $(dir $(abspath $(MAKEFILE_PATH)))

CATCH_DIR = $(EMP_DIR)/third-party/Catch/single_include/catch2

# Specify sets of compilation flags to use
FLAGS_version = -std=c++23
FLAGS_warn    = -Wall -Wextra -Wno-unused-function -Wnon-virtual-dtor -Wcast-align -Woverloaded-virtual -pedantic
FLAGS_include = -I$(EMP_DIR)/include/
FLAGS_main    = $(FLAGS_version) $(FLAGS_warn) $(FLAGS_include)
FLAGS_test    = -I$(CATCH_DIR)
FLAGS_threads = -pthread

FLAGS_QUICK  = $(FLAGS_main) -DNDEBUG
FLAGS_DEBUG  = $(FLAGS_main) -g -DEMP_TRACK_MEM
FLAGS_OPT    = $(FLAGS_main) -O3 -DNDEBUG
FLAGS_GRUMPY = $(FLAGS_main) -DNDEBUG -Wconversion -Weffc++
FLAGS_COVERAGE = $(FLAGS_main)  -O0 -DEMP_TRACK_MEM -ftemplate-backtrace-limit=0 -fprofile-instr-generate -fcoverage-mapping -fno-inline -fno-elide-constructors

# Extra flags for web compilation
EMP_METHODS = EXPORTED_RUNTIME_METHODS="['ccall', 'cwrap', 'UTF8ToString', 'stringToUTF8', 'lengthBytesUTF8']"
EMP_FUNCTIONS = EXPORTED_FUNCTIONS="['_main', "_malloc", "_free", '_empCppCallback']"
JS_LIB = --js-library $(EMP_DIR)/include/emp/web/library_emp.js
FLAGS_web_only = -s $(EMP_METHODS) -s TOTAL_MEMORY=67108864 $(JS_LIB) -s $(EMP_FUNCTIONS) -s DISABLE_EXCEPTION_CATCHING=1 -s NO_EXIT_RUNTIME=1 -Wno-dollar-in-identifier-extension
FLAGS_web_main = $(FLAGS_main) $(FLAGS_web_only)

FLAGS_WEB_OPT = $(FLAGS_web_main) -Oz -DNDEBUG
FLAGS_WEB_DEBUG = $(FLAGS_web_main) -g4 -pedantic -Wno-dollar-in-identifier-extension
FLAGS_WEB_THREADS = $(FLAGS_web_main) $(FLAGS_threads) -s USE_PTHREADS=1

CXX = c++
CXX_web = emcc

# Debugging information
print-%: ; @echo '$(subst ','\'',$*=$($*))'

CLEAN_BACKUP = *~ *.dSYM
CLEAN_COMMON = *.out *.o *.gcda *.gcno *.info *.gcov ./Coverage* ./temp

# Assuming web components are added to a web/ sub-folder.
CLEAN_WEB = web/*.js.mem web/*.js.map web/*.wasm web/*.wasm.map

# Update CLEAN_EXTRA in other Makefiles to handle custom cleaning.
CLEAN_EXTRA =

CLEAN_FILES = $(CLEAN_BACKUP) $(CLEAN_COMMON) $(CLEAN_WEB) $(CLEAN_EXTRA)

clean:
	@echo About to remove:
	@printf '%s\n' $(wildcard $(CLEAN_FILES))
	@echo ----
	@rm -rI $(wildcard $(CLEAN_FILES))