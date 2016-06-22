# Flags to use regardless of compiler
CFLAGS_all := -Wall -Wno-unused-function -std=c++11 -I../../ -I./
#CFLAGS_all := -Wall -Wno-unused-variable -Wno-unused-function -Wno-extra-semi -std=c++11 -I../../ -I./

# Emscripten compiler information
CXX_web := emcc
CXX_native := g++

OFLAGS_native := -g -pedantic
#OFLAGS_native := -O3 -DNDEBUG

OFLAGS_web := -g4 -pedantic -Wno-dollar-in-identifier-extension -s TOTAL_MEMORY=67108864 -s ASSERTIONS=2 -s DEMANGLE_SUPPORT=1 # -s SAFE_HEAP=1
#OFLAGS_web := -Os -DNDEBUG -s TOTAL_MEMORY=67108864


CFLAGS_native := $(CFLAGS_all) $(OFLAGS_native)

CFLAGS_web := $(CFLAGS_all) $(OFLAGS_web) --js-library ../../emtools/library_emp.js -s EXPORTED_FUNCTIONS="['_main', '_empCppCallback']" -s NO_EXIT_RUNTIME=1
#CFLAGS_web := $(CFLAGS_all) $(OFLAGS_web) --js-library ../../emtools/library_emp.js -s EXPORTED_FUNCTIONS="['_main', '_empCppCallback']" -s DISABLE_EXCEPTION_CATCHING=1 -s NO_EXIT_RUNTIME=1


TARGETS := Fitness_Share_NK link_test NK NK-const NK-Serial Symbulation World bit_evolve replica

EVO_DEPEND := ../../evo/PopulationManager.h ../../evo/World.h
QUORUM_DEPEND := ../../evo/QuorumOrg.h ../../evo/QuorumManager.h ../../evo/PopulationManager.h

default: native

native: $(TARGETS)


Fitness_Share_NK: Fitness_Share_NK.cc $(EVO_DEPEND)
	$(CXX_native) $(CFLAGS_native) Fitness_Share_NK.cc -o Fitness_Share_NK

link_test: ../../tools/signal.h link_test.cc $(EVO_DEPEND)
	$(CXX_native) $(CFLAGS_native) link_test.cc -o link_test

NK: NK.cc ../../evo/NK.h $(EVO_DEPEND)
	$(CXX_native) $(CFLAGS_native) NK.cc -o NK

replica: replica.cc ../../evo/NK.h $(EVO_DEPEND)
	$(CXX_native) $(CFLAGS_native) -g replica.cc -o replica

grid-exp: grid-exp.cc ../../evo/NK.h $(EVO_DEPEND)
	$(CXX_native) $(CFLAGS_native) -g grid-exp.cc -o grid-exp

mixed-exp: mixed-exp.cc ../../evo/NK.h $(EVO_DEPEND)
	$(CXX_native) $(CFLAGS_native) -g mixed-exp.cc -o mixed-exp

fast-replica: replica.cc ../../evo/NK.h $(EVO_DEPEND)
	$(CXX_native) $(CFLAGS_native) -DNDEBUG -O3 replica.cc -o replica

fast-grid: grid-exp.cc ../../evo/NK.h $(EVO_DEPEND)
	$(CXX_native) $(CFLAGS_native) -DNDEBUG -O3 grid-exp.cc -o grid-exp

fast-mixed: mixed-exp.cc ../../evo/NK.h $(EVO_DEPEND)
	$(CXX_native) $(CFLAGS_native) -DNDEBUG -O3 mixed-exp.cc -o mixed-exp

fast-quorum-mixed: gladiator-mixed-grid-quorum.cc $(EVO_DEPEND) $(QUORUM_DEPEND)
	$(CXX_native) $(CFLAGS_native) -DNDEBUG -O3 gladiator-mixed-grid-quorum.cc -o gladiator-mixed.out

fast-quorum-standard: gladiator-standard-grid-quorum.cc $(EVO_DEPEND) $(QUORUM_DEPEND)
	$(CXX_native) $(CFLAGS_native) -DNDEBUG -O3 gladiator-standard-grid-quorum.cc -o gladiator-standard.out

NK-const: NK-const.cc ../../evo/NK-const.h $(EVO_DEPEND)
	$(CXX_native) $(CFLAGS_native) NK-const.cc -o NK-const

NK-Serial: NK-Serial.cc ../../evo/NK.h $(EVO_DEPEND)
	$(CXX_native) $(CFLAGS_native) NK-Serial.cc -o NK-Serial

Symbulation: Symbulation.cc ../../evo/Symbulation.h $(EVO_DEPEND)
	$(CXX_native) $(CFLAGS_native) Symbulation.cc -o Symbulation

World: World.cc $(EVO_DEPEND)
	$(CXX_native) $(CFLAGS_native) World.cc -o World

bit_evolve: bit_evolve.cc $(EVO_DEPEND)
	$(CXX_native) $(CFLAGS_native) bit_evolve.cc -o bit_evolve



debug-%: $*.cc
	$(CXX_native) $(CFLAGS_native) $< -o $@

clean:
	rm -rf debug-* *~ *.dSYM $(TARGETS)

# Debugging information
#print-%: ; @echo $*=$($*)
print-%: ; @echo '$(subst ','\'',$*=$($*))'
