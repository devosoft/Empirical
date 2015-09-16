# Flags to use regardless of compiler
CFLAGS_all := -Wall -Wno-unused-variable -Wno-unused-function -std=c++11

# Emscripten compiler information
CXX_web := emcc
CXX_native := g++

OFLAGS_native := -pedantic

OFLAGS_web := -g4 -pedantic -Wno-dollar-in-identifier-extension -s TOTAL_MEMORY=67108864 -s ASSERTIONS=2 -s DEMANGLE_SUPPORT=1 # -s SAFE_HEAP=1
#OFLAGS_web := -g4 -DEMP_NDEBUG -pedantic -Wno-dollar-in-identifier-extension -s TOTAL_MEMORY=67108864 -s ASSERTIONS=2 -s DEMANGLE_SUPPORT=1 # -s SAFE_HEAP=1
#OFLAGS_web := -g4 -DNDEBUG -s TOTAL_MEMORY=67108864 -s DEMANGLE_SUPPORT=1 # -s SAFE_HEAP=1
#OFLAGS_web := -Werror -DNDEBUG -s TOTAL_MEMORY=67108864
#OFLAGS_web := -Oz -DNDEBUG -s TOTAL_MEMORY=67108864 -s ASSERTIONS=1
#OFLAGS_web := -Os -DNDEBUG -s TOTAL_MEMORY=67108864


CFLAGS_native := $(CFLAGS_all) $(OFLAGS_native)

CFLAGS_web := $(CFLAGS_all) $(OFLAGS_web) --js-library ../../emtools/library_emp.js -s EXPORTED_FUNCTIONS="['_main', '_empCppCallback']" -s NO_EXIT_RUNTIME=1
#CFLAGS_web := $(CFLAGS_all) $(OFLAGS_web) --js-library ../../emtools/library_emp.js -s EXPORTED_FUNCTIONS="['_main', '_empCppCallback']" -s DISABLE_EXCEPTION_CATCHING=1 -s NO_EXIT_RUNTIME=1


default: native
native: BitMatrix BitSet BitVector DynamicStringSet FunctionSet Graph Ptr Random Trait assert functions graph_util macros mem_track reflection sequence_utils serialize string_utils tuple_struct vector

BitMatrix:	BitMatrix.cc ../../tools/BitMatrix.h
	$(CXX_native) $(CFLAGS_native) BitMatrix.cc -o BitMatrix

BitSet:	BitSet.cc ../../tools/BitSet.h
	$(CXX_native) $(CFLAGS_native) BitSet.cc -o BitSet

BitVector:	BitVector.cc ../../tools/BitVector.h
	$(CXX_native) $(CFLAGS_native) BitVector.cc -o BitVector

DynamicStringSet:	DynamicStringSet.cc ../../tools/DynamicStringSet.h
	$(CXX_native) $(CFLAGS_native) DynamicStringSet.cc -o DynamicStringSet

FunctionSet:	FunctionSet.cc ../../tools/FunctionSet.h
	$(CXX_native) $(CFLAGS_native) FunctionSet.cc -o FunctionSet

Graph:	Graph.cc ../../tools/Graph.h
	$(CXX_native) $(CFLAGS_native) Graph.cc -o Graph

Ptr:	Ptr.cc ../../tools/Ptr.h
	$(CXX_native) $(CFLAGS_native) Ptr.cc -o Ptr

Random:	Random.cc ../../tools/Random.h
	$(CXX_native) $(CFLAGS_native) Random.cc -o Random

Trait:	Trait.cc ../../tools/Trait.h
	$(CXX_native) $(CFLAGS_native) Trait.cc -o Trait

assert:	assert.cc ../../tools/assert.h
	$(CXX_native) $(CFLAGS_native) assert.cc -o assert

functions:	functions.cc ../../tools/functions.h
	$(CXX_native) $(CFLAGS_native) functions.cc -o functions

graph_util:	graph_util.cc ../../tools/graph_util.h
	$(CXX_native) $(CFLAGS_native) graph_util.cc -o graph_util

macros:	macros.cc ../../tools/macros.h
	$(CXX_native) $(CFLAGS_native) macros.cc -o macros

mem_track:	mem_track.cc ../../tools/mem_track.h
	$(CXX_native) $(CFLAGS_native) mem_track.cc -o mem_track

reflection:	reflection.cc ../../tools/reflection.h
	$(CXX_native) $(CFLAGS_native) reflection.cc -o reflection

sequence_utils:	sequence_utils.cc ../../tools/sequence_utils.h
	$(CXX_native) $(CFLAGS_native) sequence_utils.cc -o sequence_utils

serialize:	serialize.cc ../../tools/serialize.h
	$(CXX_native) $(CFLAGS_native) serialize.cc -o serialize

string_utils:	string_utils.cc ../../tools/string_utils.h
	$(CXX_native) $(CFLAGS_native) string_utils.cc -o string_utils

tuple_struct:	tuple_struct.cc ../../tools/tuple_struct.h
	$(CXX_native) $(CFLAGS_native) tuple_struct.cc -o tuple_struct

vector:	vector.cc ../../tools/vector.h
	$(CXX_native) $(CFLAGS_native) vector.cc -o vector

clean:
	rm -rf *.dSYM BitMatrix BitSet BitVector DynamicStringSet FunctionSet Graph Ptr Random Trait assert functions graph_util macros mem_track reflection sequence_utils serialize string_utils tuple_struct vector *.js.map *~

# Debugging information
#print-%: ; @echo $*=$($*)
print-%: ; @echo '$(subst ','\'',$*=$($*))'
