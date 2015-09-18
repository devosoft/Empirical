#ifndef EMP_MACRO_MATH_H
#define EMP_MACRO_MATH_H

//////////////////////////////////////////////////////////////////////////////////////////
//
//  These macros build a pre-processor calculator system.
//

#define EMP_VAL_TO_BIN(VAL) EMP_VAL_TO_BIN_ ## VAL

#define EMP_VAL_TO_BIN_0  0, 0, 0, 0
#define EMP_VAL_TO_BIN_1  0, 0, 0, 1
#define EMP_VAL_TO_BIN_2  0, 0, 1, 0
#define EMP_VAL_TO_BIN_3  0, 0, 1, 1
#define EMP_VAL_TO_BIN_4  0, 1, 0, 0
#define EMP_VAL_TO_BIN_5  0, 1, 0, 1
#define EMP_VAL_TO_BIN_6  0, 1, 1, 0
#define EMP_VAL_TO_BIN_7  0, 1, 1, 1
#define EMP_VAL_TO_BIN_8  1, 0, 0, 0
#define EMP_VAL_TO_BIN_9  1, 0, 0, 1
#define EMP_VAL_TO_BIN_10 1, 0, 1, 0
#define EMP_VAL_TO_BIN_11 1, 0, 1, 1
#define EMP_VAL_TO_BIN_12 1, 1, 0, 0
#define EMP_VAL_TO_BIN_13 1, 1, 0, 1
#define EMP_VAL_TO_BIN_14 1, 1, 1, 0
#define EMP_VAL_TO_BIN_15 1, 1, 1, 1

#define EMP_BIN_TO_VAL(...) EMP_BIN_TO_VAL_IMPL(__VA_ARGS__)
#define EMP_BIN_TO_VAL_IMPL(B1,B2,B3,B4) EMP_BIN_TO_VAL_ ## B1 ## B2 ## B3 ## B4

#define EMP_BIN_TO_VAL_0000 0
#define EMP_BIN_TO_VAL_0001 1
#define EMP_BIN_TO_VAL_0010 2
#define EMP_BIN_TO_VAL_0011 3
#define EMP_BIN_TO_VAL_0100 4
#define EMP_BIN_TO_VAL_0101 5
#define EMP_BIN_TO_VAL_0110 6
#define EMP_BIN_TO_VAL_0111 7
#define EMP_BIN_TO_VAL_1000 8
#define EMP_BIN_TO_VAL_1001 9
#define EMP_BIN_TO_VAL_1010 10
#define EMP_BIN_TO_VAL_1011 11
#define EMP_BIN_TO_VAL_1100 12
#define EMP_BIN_TO_VAL_1101 13
#define EMP_BIN_TO_VAL_1110 14
#define EMP_BIN_TO_VAL_1111 15

#define EMP_COUNT_BITS(A,B) EMP_COUNT_BITS_IMPL(A,B)
#define EMP_COUNT_BITS_IMPL(A,B) EMP_COUNT_BITS_ ## A ## B
#define EMP_COUNT_BITS_00 0
#define EMP_COUNT_BITS_01 1
#define EMP_COUNT_BITS_10 1
#define EMP_COUNT_BITS_11 2

#define EMP_MATH_GET_CARRY(A) EMP_MATH_GET_CARRY_IMPL(A)
#define EMP_MATH_GET_CARRY_IMPL(A) EMP_MATH_GET_CARRY_ ## A
#define EMP_MATH_GET_CARRY_0 0
#define EMP_MATH_GET_CARRY_1 0
#define EMP_MATH_GET_CARRY_2 1
#define EMP_MATH_GET_CARRY_3 1

#define EMP_MATH_CLEAR_CARRY(A) EMP_MATH_CLEAR_CARRY_IMPL(A)
#define EMP_MATH_CLEAR_CARRY_IMPL(A) EMP_MATH_CLEAR_CARRY_ ## A
#define EMP_MATH_CLEAR_CARRY_0 0
#define EMP_MATH_CLEAR_CARRY_1 1
#define EMP_MATH_CLEAR_CARRY_2 0
#define EMP_MATH_CLEAR_CARRY_3 1

// In order to handle carrys, we need to run RESTORE_BIN once per bit (-1)

#define EMP_MATH_RESTORE_BIN(A1, A2, A3, A4)                          \
  EMP_MATH_RESTORE_BIN_2(                                             \
    EMP_COUNT_BITS(EMP_MATH_CLEAR_CARRY(A1), EMP_MATH_GET_CARRY(A2)), \
    EMP_COUNT_BITS(EMP_MATH_CLEAR_CARRY(A2), EMP_MATH_GET_CARRY(A3)), \
    EMP_COUNT_BITS(EMP_MATH_CLEAR_CARRY(A3), EMP_MATH_GET_CARRY(A4)), \
    EMP_MATH_CLEAR_CARRY(A4)                                          \
  )

#define EMP_MATH_RESTORE_BIN_2(A1, A2, A3, A4)                        \
  EMP_MATH_RESTORE_BIN_3(                                             \
    EMP_COUNT_BITS(EMP_MATH_CLEAR_CARRY(A1), EMP_MATH_GET_CARRY(A2)), \
    EMP_COUNT_BITS(EMP_MATH_CLEAR_CARRY(A2), EMP_MATH_GET_CARRY(A3)), \
    EMP_MATH_CLEAR_CARRY(A3),                                         \
    A4                                                                \
  )

#define EMP_MATH_RESTORE_BIN_3(A1, A2, A3, A4)                        \
  EMP_MATH_RESTORE_BIN_4(                                             \
    EMP_COUNT_BITS(EMP_MATH_CLEAR_CARRY(A1), EMP_MATH_GET_CARRY(A2)), \
    EMP_MATH_CLEAR_CARRY(A2),                                         \
    A3,                                                               \
    A4                                                                \
  )

#define EMP_MATH_RESTORE_BIN_4(A1, A2, A3, A4)                        \
  EMP_MATH_CLEAR_CARRY(A1),                                           \
  A2,                                                                 \
  A3,                                                                 \
  A4

#define EMP_ADD_BIN(A1, A2, A3, A4, B1, B2, B3, B4) EMP_MATH_RESTORE_BIN( EMP_COUNT_BITS(A1, B1), EMP_COUNT_BITS(A2, B2), EMP_COUNT_BITS(A3, B3), EMP_COUNT_BITS(A4, B4) )

#define EMP_ADD(A, B) EMP_ADD_IMPL( EMP_VAL_TO_BIN(A), EMP_VAL_TO_BIN(B) )
#define EMP_ADD_IMPL(...) EMP_BIN_TO_VAL( EMP_ADD_BIN( __VA_ARGS__ ) )

#endif
