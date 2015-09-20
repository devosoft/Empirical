#!/bin/tcsh
foreach i (BitMatrix BitSet BitVector DynamicStringSet FunctionSet Graph Ptr Random Trait assert functions graph_utils macros macro_math mem_track reflection sequence_utils serialize string_utils tuple_struct vector)
  echo === $i
  $i
end
