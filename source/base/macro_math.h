/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2016-2018
 *
 *  @file macro_math.h
 *  @brief Macros to build a pre-processor calculator system.
 *  @note Status: RELEASE
 *
 *  Working macros include:
 *   EMP_INC(A)    : converts to result of A+1
 *   EMP_DEC(A)    : converts to result of A-1
 *   EMP_SHIFTL(A) : converts to result of A*2
 *   EMP_SHIFTR(A) : converts to result of A/2
 *
 *   EMP_ADD(A,B)  : converts to result of A+B
 *   EMP_SUB(A,B)  : converts to result of A-B
 *   EMP_MULT(A,B) : converts to result of A*B
 *   EMP_DIV(A,B)  : converts to result of A/B
 *   EMP_MOD(A,B)  : converts to result of A%B
 *   EMP_LOG2(A)   : converts to (int) log2(A)
 *
 *   EMP_NOT(X)     : Logical NOT
 *   EMP_AND(X, Y)  : Logical AND
 *   EMP_OR(X, Y)   : Logical OR
 *   EMP_NAND(X, Y) : Logical NAND
 *   EMP_NOR(X, Y)  : Logical NOR
 *   EMP_XOR(X, Y)  : Logical XOR
 *
 *   EMP_COUNT_ONES(A) : count the number of ones in the binary representation of A
 *
 *   EMP_IF(TEST, T, F) examines the first argument; if it's 0, it resolves to the third
 *   argument, otherwise it resolves to the second argument.
 *
 *  The core idea behind these macros is that we can use brute-force to convert numbers
 *  to binary, but once there we can easily perform math on them, or convert them to
 *  yet other forms.
 *
 *  Representations include:
 *   DEC - Standard decimal values (e.g., 91)
 *   BIN - Binary numbers, with bits separated by commas (e.g.  0, 0, 0,  1, 0,  1, 1, 0, 1, 1 )
 *   SUM - Like BIN, but stored as zero or magnitude.    (e.g., 0, 0, 0, 64, 0, 16, 8, 0, 2, 1 )
 *   PACK - Like SUM, but without zeros and in parens    (e.g., (64,16,8,2,1) )
 *   HEX - Hexidecimal representation (e.g., 0x5B)  [todo]
 */

#ifndef EMP_MACRO_MATH_H
#define EMP_MACRO_MATH_H

/// @cond MACROS

#define EMP_EMPTY()
#define EMP_EMPTY2() EMP_EMPTY()
#define EMP_EMPTY3() EMP_EMPTY2()
#define EMP_EMPTY4() EMP_EMPTY3()
#define EMP_EMPTY5() EMP_EMPTY4()
#define EMP_EMPTY6() EMP_EMPTY5()
#define EMP_EMPTY7() EMP_EMPTY6()
#define EMP_EMPTY8() EMP_EMPTY7()
#define EMP_EMPTY9() EMP_EMPTY8()
#define EMP_EMPTY10() EMP_EMPTY9()

// Several versions of eval depending on how many levels of evaluation are needed.
#define EMP_EVAL(...)  __VA_ARGS__
#define EMP_EVAL2(...) EMP_EVAL(__VA_ARGS__)
#define EMP_EVAL3(...) EMP_EVAL2(__VA_ARGS__)
#define EMP_EVAL4(...) EMP_EVAL3(__VA_ARGS__)
#define EMP_EVAL5(...) EMP_EVAL4(__VA_ARGS__)
#define EMP_EVAL6(...) EMP_EVAL5(__VA_ARGS__)
#define EMP_EVAL7(...) EMP_EVAL6(__VA_ARGS__)
#define EMP_EVAL8(...) EMP_EVAL7(__VA_ARGS__)
#define EMP_EVAL9(...) EMP_EVAL8(__VA_ARGS__)
#define EMP_EVAL10(...) EMP_EVAL9(__VA_ARGS__)

// And allowing dynamic nested evals.
#define EMP_EVAL_A(...)  __VA_ARGS__
#define EMP_EVAL_B(...)  __VA_ARGS__
#define EMP_EVAL_C(...)  __VA_ARGS__
#define EMP_EVAL_D(...)  __VA_ARGS__
#define EMP_EVAL_E(...)  __VA_ARGS__
#define EMP_EVAL_F(...)  __VA_ARGS__
#define EMP_EVAL_G(...)  __VA_ARGS__
#define EMP_EVAL_H(...)  __VA_ARGS__

/// @endcond

/// Full set of conversions from Decimal representations of numbers to their
/// binary versions (up to 1024).

#define EMP_DEC_TO_BIN(VAL) EMP_DEC_TO_BIN_ ## VAL

/// @cond MACROS

#define EMP_DEC_TO_BIN_0    0, 0, 0, 0, 0, 0, 0, 0, 0, 0
#define EMP_DEC_TO_BIN_1    0, 0, 0, 0, 0, 0, 0, 0, 0, 1
#define EMP_DEC_TO_BIN_2    0, 0, 0, 0, 0, 0, 0, 0, 1, 0
#define EMP_DEC_TO_BIN_3    0, 0, 0, 0, 0, 0, 0, 0, 1, 1
#define EMP_DEC_TO_BIN_4    0, 0, 0, 0, 0, 0, 0, 1, 0, 0
#define EMP_DEC_TO_BIN_5    0, 0, 0, 0, 0, 0, 0, 1, 0, 1
#define EMP_DEC_TO_BIN_6    0, 0, 0, 0, 0, 0, 0, 1, 1, 0
#define EMP_DEC_TO_BIN_7    0, 0, 0, 0, 0, 0, 0, 1, 1, 1
#define EMP_DEC_TO_BIN_8    0, 0, 0, 0, 0, 0, 1, 0, 0, 0
#define EMP_DEC_TO_BIN_9    0, 0, 0, 0, 0, 0, 1, 0, 0, 1
#define EMP_DEC_TO_BIN_10   0, 0, 0, 0, 0, 0, 1, 0, 1, 0
#define EMP_DEC_TO_BIN_11   0, 0, 0, 0, 0, 0, 1, 0, 1, 1
#define EMP_DEC_TO_BIN_12   0, 0, 0, 0, 0, 0, 1, 1, 0, 0
#define EMP_DEC_TO_BIN_13   0, 0, 0, 0, 0, 0, 1, 1, 0, 1
#define EMP_DEC_TO_BIN_14   0, 0, 0, 0, 0, 0, 1, 1, 1, 0
#define EMP_DEC_TO_BIN_15   0, 0, 0, 0, 0, 0, 1, 1, 1, 1
#define EMP_DEC_TO_BIN_16   0, 0, 0, 0, 0, 1, 0, 0, 0, 0
#define EMP_DEC_TO_BIN_17   0, 0, 0, 0, 0, 1, 0, 0, 0, 1
#define EMP_DEC_TO_BIN_18   0, 0, 0, 0, 0, 1, 0, 0, 1, 0
#define EMP_DEC_TO_BIN_19   0, 0, 0, 0, 0, 1, 0, 0, 1, 1
#define EMP_DEC_TO_BIN_20   0, 0, 0, 0, 0, 1, 0, 1, 0, 0
#define EMP_DEC_TO_BIN_21   0, 0, 0, 0, 0, 1, 0, 1, 0, 1
#define EMP_DEC_TO_BIN_22   0, 0, 0, 0, 0, 1, 0, 1, 1, 0
#define EMP_DEC_TO_BIN_23   0, 0, 0, 0, 0, 1, 0, 1, 1, 1
#define EMP_DEC_TO_BIN_24   0, 0, 0, 0, 0, 1, 1, 0, 0, 0
#define EMP_DEC_TO_BIN_25   0, 0, 0, 0, 0, 1, 1, 0, 0, 1
#define EMP_DEC_TO_BIN_26   0, 0, 0, 0, 0, 1, 1, 0, 1, 0
#define EMP_DEC_TO_BIN_27   0, 0, 0, 0, 0, 1, 1, 0, 1, 1
#define EMP_DEC_TO_BIN_28   0, 0, 0, 0, 0, 1, 1, 1, 0, 0
#define EMP_DEC_TO_BIN_29   0, 0, 0, 0, 0, 1, 1, 1, 0, 1
#define EMP_DEC_TO_BIN_30   0, 0, 0, 0, 0, 1, 1, 1, 1, 0
#define EMP_DEC_TO_BIN_31   0, 0, 0, 0, 0, 1, 1, 1, 1, 1
#define EMP_DEC_TO_BIN_32   0, 0, 0, 0, 1, 0, 0, 0, 0, 0
#define EMP_DEC_TO_BIN_33   0, 0, 0, 0, 1, 0, 0, 0, 0, 1
#define EMP_DEC_TO_BIN_34   0, 0, 0, 0, 1, 0, 0, 0, 1, 0
#define EMP_DEC_TO_BIN_35   0, 0, 0, 0, 1, 0, 0, 0, 1, 1
#define EMP_DEC_TO_BIN_36   0, 0, 0, 0, 1, 0, 0, 1, 0, 0
#define EMP_DEC_TO_BIN_37   0, 0, 0, 0, 1, 0, 0, 1, 0, 1
#define EMP_DEC_TO_BIN_38   0, 0, 0, 0, 1, 0, 0, 1, 1, 0
#define EMP_DEC_TO_BIN_39   0, 0, 0, 0, 1, 0, 0, 1, 1, 1
#define EMP_DEC_TO_BIN_40   0, 0, 0, 0, 1, 0, 1, 0, 0, 0
#define EMP_DEC_TO_BIN_41   0, 0, 0, 0, 1, 0, 1, 0, 0, 1
#define EMP_DEC_TO_BIN_42   0, 0, 0, 0, 1, 0, 1, 0, 1, 0
#define EMP_DEC_TO_BIN_43   0, 0, 0, 0, 1, 0, 1, 0, 1, 1
#define EMP_DEC_TO_BIN_44   0, 0, 0, 0, 1, 0, 1, 1, 0, 0
#define EMP_DEC_TO_BIN_45   0, 0, 0, 0, 1, 0, 1, 1, 0, 1
#define EMP_DEC_TO_BIN_46   0, 0, 0, 0, 1, 0, 1, 1, 1, 0
#define EMP_DEC_TO_BIN_47   0, 0, 0, 0, 1, 0, 1, 1, 1, 1
#define EMP_DEC_TO_BIN_48   0, 0, 0, 0, 1, 1, 0, 0, 0, 0
#define EMP_DEC_TO_BIN_49   0, 0, 0, 0, 1, 1, 0, 0, 0, 1
#define EMP_DEC_TO_BIN_50   0, 0, 0, 0, 1, 1, 0, 0, 1, 0
#define EMP_DEC_TO_BIN_51   0, 0, 0, 0, 1, 1, 0, 0, 1, 1
#define EMP_DEC_TO_BIN_52   0, 0, 0, 0, 1, 1, 0, 1, 0, 0
#define EMP_DEC_TO_BIN_53   0, 0, 0, 0, 1, 1, 0, 1, 0, 1
#define EMP_DEC_TO_BIN_54   0, 0, 0, 0, 1, 1, 0, 1, 1, 0
#define EMP_DEC_TO_BIN_55   0, 0, 0, 0, 1, 1, 0, 1, 1, 1
#define EMP_DEC_TO_BIN_56   0, 0, 0, 0, 1, 1, 1, 0, 0, 0
#define EMP_DEC_TO_BIN_57   0, 0, 0, 0, 1, 1, 1, 0, 0, 1
#define EMP_DEC_TO_BIN_58   0, 0, 0, 0, 1, 1, 1, 0, 1, 0
#define EMP_DEC_TO_BIN_59   0, 0, 0, 0, 1, 1, 1, 0, 1, 1
#define EMP_DEC_TO_BIN_60   0, 0, 0, 0, 1, 1, 1, 1, 0, 0
#define EMP_DEC_TO_BIN_61   0, 0, 0, 0, 1, 1, 1, 1, 0, 1
#define EMP_DEC_TO_BIN_62   0, 0, 0, 0, 1, 1, 1, 1, 1, 0
#define EMP_DEC_TO_BIN_63   0, 0, 0, 0, 1, 1, 1, 1, 1, 1
#define EMP_DEC_TO_BIN_64   0, 0, 0, 1, 0, 0, 0, 0, 0, 0
#define EMP_DEC_TO_BIN_65   0, 0, 0, 1, 0, 0, 0, 0, 0, 1
#define EMP_DEC_TO_BIN_66   0, 0, 0, 1, 0, 0, 0, 0, 1, 0
#define EMP_DEC_TO_BIN_67   0, 0, 0, 1, 0, 0, 0, 0, 1, 1
#define EMP_DEC_TO_BIN_68   0, 0, 0, 1, 0, 0, 0, 1, 0, 0
#define EMP_DEC_TO_BIN_69   0, 0, 0, 1, 0, 0, 0, 1, 0, 1
#define EMP_DEC_TO_BIN_70   0, 0, 0, 1, 0, 0, 0, 1, 1, 0
#define EMP_DEC_TO_BIN_71   0, 0, 0, 1, 0, 0, 0, 1, 1, 1
#define EMP_DEC_TO_BIN_72   0, 0, 0, 1, 0, 0, 1, 0, 0, 0
#define EMP_DEC_TO_BIN_73   0, 0, 0, 1, 0, 0, 1, 0, 0, 1
#define EMP_DEC_TO_BIN_74   0, 0, 0, 1, 0, 0, 1, 0, 1, 0
#define EMP_DEC_TO_BIN_75   0, 0, 0, 1, 0, 0, 1, 0, 1, 1
#define EMP_DEC_TO_BIN_76   0, 0, 0, 1, 0, 0, 1, 1, 0, 0
#define EMP_DEC_TO_BIN_77   0, 0, 0, 1, 0, 0, 1, 1, 0, 1
#define EMP_DEC_TO_BIN_78   0, 0, 0, 1, 0, 0, 1, 1, 1, 0
#define EMP_DEC_TO_BIN_79   0, 0, 0, 1, 0, 0, 1, 1, 1, 1
#define EMP_DEC_TO_BIN_80   0, 0, 0, 1, 0, 1, 0, 0, 0, 0
#define EMP_DEC_TO_BIN_81   0, 0, 0, 1, 0, 1, 0, 0, 0, 1
#define EMP_DEC_TO_BIN_82   0, 0, 0, 1, 0, 1, 0, 0, 1, 0
#define EMP_DEC_TO_BIN_83   0, 0, 0, 1, 0, 1, 0, 0, 1, 1
#define EMP_DEC_TO_BIN_84   0, 0, 0, 1, 0, 1, 0, 1, 0, 0
#define EMP_DEC_TO_BIN_85   0, 0, 0, 1, 0, 1, 0, 1, 0, 1
#define EMP_DEC_TO_BIN_86   0, 0, 0, 1, 0, 1, 0, 1, 1, 0
#define EMP_DEC_TO_BIN_87   0, 0, 0, 1, 0, 1, 0, 1, 1, 1
#define EMP_DEC_TO_BIN_88   0, 0, 0, 1, 0, 1, 1, 0, 0, 0
#define EMP_DEC_TO_BIN_89   0, 0, 0, 1, 0, 1, 1, 0, 0, 1
#define EMP_DEC_TO_BIN_90   0, 0, 0, 1, 0, 1, 1, 0, 1, 0
#define EMP_DEC_TO_BIN_91   0, 0, 0, 1, 0, 1, 1, 0, 1, 1
#define EMP_DEC_TO_BIN_92   0, 0, 0, 1, 0, 1, 1, 1, 0, 0
#define EMP_DEC_TO_BIN_93   0, 0, 0, 1, 0, 1, 1, 1, 0, 1
#define EMP_DEC_TO_BIN_94   0, 0, 0, 1, 0, 1, 1, 1, 1, 0
#define EMP_DEC_TO_BIN_95   0, 0, 0, 1, 0, 1, 1, 1, 1, 1
#define EMP_DEC_TO_BIN_96   0, 0, 0, 1, 1, 0, 0, 0, 0, 0
#define EMP_DEC_TO_BIN_97   0, 0, 0, 1, 1, 0, 0, 0, 0, 1
#define EMP_DEC_TO_BIN_98   0, 0, 0, 1, 1, 0, 0, 0, 1, 0
#define EMP_DEC_TO_BIN_99   0, 0, 0, 1, 1, 0, 0, 0, 1, 1
#define EMP_DEC_TO_BIN_100  0, 0, 0, 1, 1, 0, 0, 1, 0, 0
#define EMP_DEC_TO_BIN_101  0, 0, 0, 1, 1, 0, 0, 1, 0, 1
#define EMP_DEC_TO_BIN_102  0, 0, 0, 1, 1, 0, 0, 1, 1, 0
#define EMP_DEC_TO_BIN_103  0, 0, 0, 1, 1, 0, 0, 1, 1, 1
#define EMP_DEC_TO_BIN_104  0, 0, 0, 1, 1, 0, 1, 0, 0, 0
#define EMP_DEC_TO_BIN_105  0, 0, 0, 1, 1, 0, 1, 0, 0, 1
#define EMP_DEC_TO_BIN_106  0, 0, 0, 1, 1, 0, 1, 0, 1, 0
#define EMP_DEC_TO_BIN_107  0, 0, 0, 1, 1, 0, 1, 0, 1, 1
#define EMP_DEC_TO_BIN_108  0, 0, 0, 1, 1, 0, 1, 1, 0, 0
#define EMP_DEC_TO_BIN_109  0, 0, 0, 1, 1, 0, 1, 1, 0, 1
#define EMP_DEC_TO_BIN_110  0, 0, 0, 1, 1, 0, 1, 1, 1, 0
#define EMP_DEC_TO_BIN_111  0, 0, 0, 1, 1, 0, 1, 1, 1, 1
#define EMP_DEC_TO_BIN_112  0, 0, 0, 1, 1, 1, 0, 0, 0, 0
#define EMP_DEC_TO_BIN_113  0, 0, 0, 1, 1, 1, 0, 0, 0, 1
#define EMP_DEC_TO_BIN_114  0, 0, 0, 1, 1, 1, 0, 0, 1, 0
#define EMP_DEC_TO_BIN_115  0, 0, 0, 1, 1, 1, 0, 0, 1, 1
#define EMP_DEC_TO_BIN_116  0, 0, 0, 1, 1, 1, 0, 1, 0, 0
#define EMP_DEC_TO_BIN_117  0, 0, 0, 1, 1, 1, 0, 1, 0, 1
#define EMP_DEC_TO_BIN_118  0, 0, 0, 1, 1, 1, 0, 1, 1, 0
#define EMP_DEC_TO_BIN_119  0, 0, 0, 1, 1, 1, 0, 1, 1, 1
#define EMP_DEC_TO_BIN_120  0, 0, 0, 1, 1, 1, 1, 0, 0, 0
#define EMP_DEC_TO_BIN_121  0, 0, 0, 1, 1, 1, 1, 0, 0, 1
#define EMP_DEC_TO_BIN_122  0, 0, 0, 1, 1, 1, 1, 0, 1, 0
#define EMP_DEC_TO_BIN_123  0, 0, 0, 1, 1, 1, 1, 0, 1, 1
#define EMP_DEC_TO_BIN_124  0, 0, 0, 1, 1, 1, 1, 1, 0, 0
#define EMP_DEC_TO_BIN_125  0, 0, 0, 1, 1, 1, 1, 1, 0, 1
#define EMP_DEC_TO_BIN_126  0, 0, 0, 1, 1, 1, 1, 1, 1, 0
#define EMP_DEC_TO_BIN_127  0, 0, 0, 1, 1, 1, 1, 1, 1, 1
#define EMP_DEC_TO_BIN_128  0, 0, 1, 0, 0, 0, 0, 0, 0, 0
#define EMP_DEC_TO_BIN_129  0, 0, 1, 0, 0, 0, 0, 0, 0, 1
#define EMP_DEC_TO_BIN_130  0, 0, 1, 0, 0, 0, 0, 0, 1, 0
#define EMP_DEC_TO_BIN_131  0, 0, 1, 0, 0, 0, 0, 0, 1, 1
#define EMP_DEC_TO_BIN_132  0, 0, 1, 0, 0, 0, 0, 1, 0, 0
#define EMP_DEC_TO_BIN_133  0, 0, 1, 0, 0, 0, 0, 1, 0, 1
#define EMP_DEC_TO_BIN_134  0, 0, 1, 0, 0, 0, 0, 1, 1, 0
#define EMP_DEC_TO_BIN_135  0, 0, 1, 0, 0, 0, 0, 1, 1, 1
#define EMP_DEC_TO_BIN_136  0, 0, 1, 0, 0, 0, 1, 0, 0, 0
#define EMP_DEC_TO_BIN_137  0, 0, 1, 0, 0, 0, 1, 0, 0, 1
#define EMP_DEC_TO_BIN_138  0, 0, 1, 0, 0, 0, 1, 0, 1, 0
#define EMP_DEC_TO_BIN_139  0, 0, 1, 0, 0, 0, 1, 0, 1, 1
#define EMP_DEC_TO_BIN_140  0, 0, 1, 0, 0, 0, 1, 1, 0, 0
#define EMP_DEC_TO_BIN_141  0, 0, 1, 0, 0, 0, 1, 1, 0, 1
#define EMP_DEC_TO_BIN_142  0, 0, 1, 0, 0, 0, 1, 1, 1, 0
#define EMP_DEC_TO_BIN_143  0, 0, 1, 0, 0, 0, 1, 1, 1, 1
#define EMP_DEC_TO_BIN_144  0, 0, 1, 0, 0, 1, 0, 0, 0, 0
#define EMP_DEC_TO_BIN_145  0, 0, 1, 0, 0, 1, 0, 0, 0, 1
#define EMP_DEC_TO_BIN_146  0, 0, 1, 0, 0, 1, 0, 0, 1, 0
#define EMP_DEC_TO_BIN_147  0, 0, 1, 0, 0, 1, 0, 0, 1, 1
#define EMP_DEC_TO_BIN_148  0, 0, 1, 0, 0, 1, 0, 1, 0, 0
#define EMP_DEC_TO_BIN_149  0, 0, 1, 0, 0, 1, 0, 1, 0, 1
#define EMP_DEC_TO_BIN_150  0, 0, 1, 0, 0, 1, 0, 1, 1, 0
#define EMP_DEC_TO_BIN_151  0, 0, 1, 0, 0, 1, 0, 1, 1, 1
#define EMP_DEC_TO_BIN_152  0, 0, 1, 0, 0, 1, 1, 0, 0, 0
#define EMP_DEC_TO_BIN_153  0, 0, 1, 0, 0, 1, 1, 0, 0, 1
#define EMP_DEC_TO_BIN_154  0, 0, 1, 0, 0, 1, 1, 0, 1, 0
#define EMP_DEC_TO_BIN_155  0, 0, 1, 0, 0, 1, 1, 0, 1, 1
#define EMP_DEC_TO_BIN_156  0, 0, 1, 0, 0, 1, 1, 1, 0, 0
#define EMP_DEC_TO_BIN_157  0, 0, 1, 0, 0, 1, 1, 1, 0, 1
#define EMP_DEC_TO_BIN_158  0, 0, 1, 0, 0, 1, 1, 1, 1, 0
#define EMP_DEC_TO_BIN_159  0, 0, 1, 0, 0, 1, 1, 1, 1, 1
#define EMP_DEC_TO_BIN_160  0, 0, 1, 0, 1, 0, 0, 0, 0, 0
#define EMP_DEC_TO_BIN_161  0, 0, 1, 0, 1, 0, 0, 0, 0, 1
#define EMP_DEC_TO_BIN_162  0, 0, 1, 0, 1, 0, 0, 0, 1, 0
#define EMP_DEC_TO_BIN_163  0, 0, 1, 0, 1, 0, 0, 0, 1, 1
#define EMP_DEC_TO_BIN_164  0, 0, 1, 0, 1, 0, 0, 1, 0, 0
#define EMP_DEC_TO_BIN_165  0, 0, 1, 0, 1, 0, 0, 1, 0, 1
#define EMP_DEC_TO_BIN_166  0, 0, 1, 0, 1, 0, 0, 1, 1, 0
#define EMP_DEC_TO_BIN_167  0, 0, 1, 0, 1, 0, 0, 1, 1, 1
#define EMP_DEC_TO_BIN_168  0, 0, 1, 0, 1, 0, 1, 0, 0, 0
#define EMP_DEC_TO_BIN_169  0, 0, 1, 0, 1, 0, 1, 0, 0, 1
#define EMP_DEC_TO_BIN_170  0, 0, 1, 0, 1, 0, 1, 0, 1, 0
#define EMP_DEC_TO_BIN_171  0, 0, 1, 0, 1, 0, 1, 0, 1, 1
#define EMP_DEC_TO_BIN_172  0, 0, 1, 0, 1, 0, 1, 1, 0, 0
#define EMP_DEC_TO_BIN_173  0, 0, 1, 0, 1, 0, 1, 1, 0, 1
#define EMP_DEC_TO_BIN_174  0, 0, 1, 0, 1, 0, 1, 1, 1, 0
#define EMP_DEC_TO_BIN_175  0, 0, 1, 0, 1, 0, 1, 1, 1, 1
#define EMP_DEC_TO_BIN_176  0, 0, 1, 0, 1, 1, 0, 0, 0, 0
#define EMP_DEC_TO_BIN_177  0, 0, 1, 0, 1, 1, 0, 0, 0, 1
#define EMP_DEC_TO_BIN_178  0, 0, 1, 0, 1, 1, 0, 0, 1, 0
#define EMP_DEC_TO_BIN_179  0, 0, 1, 0, 1, 1, 0, 0, 1, 1
#define EMP_DEC_TO_BIN_180  0, 0, 1, 0, 1, 1, 0, 1, 0, 0
#define EMP_DEC_TO_BIN_181  0, 0, 1, 0, 1, 1, 0, 1, 0, 1
#define EMP_DEC_TO_BIN_182  0, 0, 1, 0, 1, 1, 0, 1, 1, 0
#define EMP_DEC_TO_BIN_183  0, 0, 1, 0, 1, 1, 0, 1, 1, 1
#define EMP_DEC_TO_BIN_184  0, 0, 1, 0, 1, 1, 1, 0, 0, 0
#define EMP_DEC_TO_BIN_185  0, 0, 1, 0, 1, 1, 1, 0, 0, 1
#define EMP_DEC_TO_BIN_186  0, 0, 1, 0, 1, 1, 1, 0, 1, 0
#define EMP_DEC_TO_BIN_187  0, 0, 1, 0, 1, 1, 1, 0, 1, 1
#define EMP_DEC_TO_BIN_188  0, 0, 1, 0, 1, 1, 1, 1, 0, 0
#define EMP_DEC_TO_BIN_189  0, 0, 1, 0, 1, 1, 1, 1, 0, 1
#define EMP_DEC_TO_BIN_190  0, 0, 1, 0, 1, 1, 1, 1, 1, 0
#define EMP_DEC_TO_BIN_191  0, 0, 1, 0, 1, 1, 1, 1, 1, 1
#define EMP_DEC_TO_BIN_192  0, 0, 1, 1, 0, 0, 0, 0, 0, 0
#define EMP_DEC_TO_BIN_193  0, 0, 1, 1, 0, 0, 0, 0, 0, 1
#define EMP_DEC_TO_BIN_194  0, 0, 1, 1, 0, 0, 0, 0, 1, 0
#define EMP_DEC_TO_BIN_195  0, 0, 1, 1, 0, 0, 0, 0, 1, 1
#define EMP_DEC_TO_BIN_196  0, 0, 1, 1, 0, 0, 0, 1, 0, 0
#define EMP_DEC_TO_BIN_197  0, 0, 1, 1, 0, 0, 0, 1, 0, 1
#define EMP_DEC_TO_BIN_198  0, 0, 1, 1, 0, 0, 0, 1, 1, 0
#define EMP_DEC_TO_BIN_199  0, 0, 1, 1, 0, 0, 0, 1, 1, 1
#define EMP_DEC_TO_BIN_200  0, 0, 1, 1, 0, 0, 1, 0, 0, 0
#define EMP_DEC_TO_BIN_201  0, 0, 1, 1, 0, 0, 1, 0, 0, 1
#define EMP_DEC_TO_BIN_202  0, 0, 1, 1, 0, 0, 1, 0, 1, 0
#define EMP_DEC_TO_BIN_203  0, 0, 1, 1, 0, 0, 1, 0, 1, 1
#define EMP_DEC_TO_BIN_204  0, 0, 1, 1, 0, 0, 1, 1, 0, 0
#define EMP_DEC_TO_BIN_205  0, 0, 1, 1, 0, 0, 1, 1, 0, 1
#define EMP_DEC_TO_BIN_206  0, 0, 1, 1, 0, 0, 1, 1, 1, 0
#define EMP_DEC_TO_BIN_207  0, 0, 1, 1, 0, 0, 1, 1, 1, 1
#define EMP_DEC_TO_BIN_208  0, 0, 1, 1, 0, 1, 0, 0, 0, 0
#define EMP_DEC_TO_BIN_209  0, 0, 1, 1, 0, 1, 0, 0, 0, 1
#define EMP_DEC_TO_BIN_210  0, 0, 1, 1, 0, 1, 0, 0, 1, 0
#define EMP_DEC_TO_BIN_211  0, 0, 1, 1, 0, 1, 0, 0, 1, 1
#define EMP_DEC_TO_BIN_212  0, 0, 1, 1, 0, 1, 0, 1, 0, 0
#define EMP_DEC_TO_BIN_213  0, 0, 1, 1, 0, 1, 0, 1, 0, 1
#define EMP_DEC_TO_BIN_214  0, 0, 1, 1, 0, 1, 0, 1, 1, 0
#define EMP_DEC_TO_BIN_215  0, 0, 1, 1, 0, 1, 0, 1, 1, 1
#define EMP_DEC_TO_BIN_216  0, 0, 1, 1, 0, 1, 1, 0, 0, 0
#define EMP_DEC_TO_BIN_217  0, 0, 1, 1, 0, 1, 1, 0, 0, 1
#define EMP_DEC_TO_BIN_218  0, 0, 1, 1, 0, 1, 1, 0, 1, 0
#define EMP_DEC_TO_BIN_219  0, 0, 1, 1, 0, 1, 1, 0, 1, 1
#define EMP_DEC_TO_BIN_220  0, 0, 1, 1, 0, 1, 1, 1, 0, 0
#define EMP_DEC_TO_BIN_221  0, 0, 1, 1, 0, 1, 1, 1, 0, 1
#define EMP_DEC_TO_BIN_222  0, 0, 1, 1, 0, 1, 1, 1, 1, 0
#define EMP_DEC_TO_BIN_223  0, 0, 1, 1, 0, 1, 1, 1, 1, 1
#define EMP_DEC_TO_BIN_224  0, 0, 1, 1, 1, 0, 0, 0, 0, 0
#define EMP_DEC_TO_BIN_225  0, 0, 1, 1, 1, 0, 0, 0, 0, 1
#define EMP_DEC_TO_BIN_226  0, 0, 1, 1, 1, 0, 0, 0, 1, 0
#define EMP_DEC_TO_BIN_227  0, 0, 1, 1, 1, 0, 0, 0, 1, 1
#define EMP_DEC_TO_BIN_228  0, 0, 1, 1, 1, 0, 0, 1, 0, 0
#define EMP_DEC_TO_BIN_229  0, 0, 1, 1, 1, 0, 0, 1, 0, 1
#define EMP_DEC_TO_BIN_230  0, 0, 1, 1, 1, 0, 0, 1, 1, 0
#define EMP_DEC_TO_BIN_231  0, 0, 1, 1, 1, 0, 0, 1, 1, 1
#define EMP_DEC_TO_BIN_232  0, 0, 1, 1, 1, 0, 1, 0, 0, 0
#define EMP_DEC_TO_BIN_233  0, 0, 1, 1, 1, 0, 1, 0, 0, 1
#define EMP_DEC_TO_BIN_234  0, 0, 1, 1, 1, 0, 1, 0, 1, 0
#define EMP_DEC_TO_BIN_235  0, 0, 1, 1, 1, 0, 1, 0, 1, 1
#define EMP_DEC_TO_BIN_236  0, 0, 1, 1, 1, 0, 1, 1, 0, 0
#define EMP_DEC_TO_BIN_237  0, 0, 1, 1, 1, 0, 1, 1, 0, 1
#define EMP_DEC_TO_BIN_238  0, 0, 1, 1, 1, 0, 1, 1, 1, 0
#define EMP_DEC_TO_BIN_239  0, 0, 1, 1, 1, 0, 1, 1, 1, 1
#define EMP_DEC_TO_BIN_240  0, 0, 1, 1, 1, 1, 0, 0, 0, 0
#define EMP_DEC_TO_BIN_241  0, 0, 1, 1, 1, 1, 0, 0, 0, 1
#define EMP_DEC_TO_BIN_242  0, 0, 1, 1, 1, 1, 0, 0, 1, 0
#define EMP_DEC_TO_BIN_243  0, 0, 1, 1, 1, 1, 0, 0, 1, 1
#define EMP_DEC_TO_BIN_244  0, 0, 1, 1, 1, 1, 0, 1, 0, 0
#define EMP_DEC_TO_BIN_245  0, 0, 1, 1, 1, 1, 0, 1, 0, 1
#define EMP_DEC_TO_BIN_246  0, 0, 1, 1, 1, 1, 0, 1, 1, 0
#define EMP_DEC_TO_BIN_247  0, 0, 1, 1, 1, 1, 0, 1, 1, 1
#define EMP_DEC_TO_BIN_248  0, 0, 1, 1, 1, 1, 1, 0, 0, 0
#define EMP_DEC_TO_BIN_249  0, 0, 1, 1, 1, 1, 1, 0, 0, 1
#define EMP_DEC_TO_BIN_250  0, 0, 1, 1, 1, 1, 1, 0, 1, 0
#define EMP_DEC_TO_BIN_251  0, 0, 1, 1, 1, 1, 1, 0, 1, 1
#define EMP_DEC_TO_BIN_252  0, 0, 1, 1, 1, 1, 1, 1, 0, 0
#define EMP_DEC_TO_BIN_253  0, 0, 1, 1, 1, 1, 1, 1, 0, 1
#define EMP_DEC_TO_BIN_254  0, 0, 1, 1, 1, 1, 1, 1, 1, 0
#define EMP_DEC_TO_BIN_255  0, 0, 1, 1, 1, 1, 1, 1, 1, 1
#define EMP_DEC_TO_BIN_256  0, 1, 0, 0, 0, 0, 0, 0, 0, 0
#define EMP_DEC_TO_BIN_257  0, 1, 0, 0, 0, 0, 0, 0, 0, 1
#define EMP_DEC_TO_BIN_258  0, 1, 0, 0, 0, 0, 0, 0, 1, 0
#define EMP_DEC_TO_BIN_259  0, 1, 0, 0, 0, 0, 0, 0, 1, 1
#define EMP_DEC_TO_BIN_260  0, 1, 0, 0, 0, 0, 0, 1, 0, 0
#define EMP_DEC_TO_BIN_261  0, 1, 0, 0, 0, 0, 0, 1, 0, 1
#define EMP_DEC_TO_BIN_262  0, 1, 0, 0, 0, 0, 0, 1, 1, 0
#define EMP_DEC_TO_BIN_263  0, 1, 0, 0, 0, 0, 0, 1, 1, 1
#define EMP_DEC_TO_BIN_264  0, 1, 0, 0, 0, 0, 1, 0, 0, 0
#define EMP_DEC_TO_BIN_265  0, 1, 0, 0, 0, 0, 1, 0, 0, 1
#define EMP_DEC_TO_BIN_266  0, 1, 0, 0, 0, 0, 1, 0, 1, 0
#define EMP_DEC_TO_BIN_267  0, 1, 0, 0, 0, 0, 1, 0, 1, 1
#define EMP_DEC_TO_BIN_268  0, 1, 0, 0, 0, 0, 1, 1, 0, 0
#define EMP_DEC_TO_BIN_269  0, 1, 0, 0, 0, 0, 1, 1, 0, 1
#define EMP_DEC_TO_BIN_270  0, 1, 0, 0, 0, 0, 1, 1, 1, 0
#define EMP_DEC_TO_BIN_271  0, 1, 0, 0, 0, 0, 1, 1, 1, 1
#define EMP_DEC_TO_BIN_272  0, 1, 0, 0, 0, 1, 0, 0, 0, 0
#define EMP_DEC_TO_BIN_273  0, 1, 0, 0, 0, 1, 0, 0, 0, 1
#define EMP_DEC_TO_BIN_274  0, 1, 0, 0, 0, 1, 0, 0, 1, 0
#define EMP_DEC_TO_BIN_275  0, 1, 0, 0, 0, 1, 0, 0, 1, 1
#define EMP_DEC_TO_BIN_276  0, 1, 0, 0, 0, 1, 0, 1, 0, 0
#define EMP_DEC_TO_BIN_277  0, 1, 0, 0, 0, 1, 0, 1, 0, 1
#define EMP_DEC_TO_BIN_278  0, 1, 0, 0, 0, 1, 0, 1, 1, 0
#define EMP_DEC_TO_BIN_279  0, 1, 0, 0, 0, 1, 0, 1, 1, 1
#define EMP_DEC_TO_BIN_280  0, 1, 0, 0, 0, 1, 1, 0, 0, 0
#define EMP_DEC_TO_BIN_281  0, 1, 0, 0, 0, 1, 1, 0, 0, 1
#define EMP_DEC_TO_BIN_282  0, 1, 0, 0, 0, 1, 1, 0, 1, 0
#define EMP_DEC_TO_BIN_283  0, 1, 0, 0, 0, 1, 1, 0, 1, 1
#define EMP_DEC_TO_BIN_284  0, 1, 0, 0, 0, 1, 1, 1, 0, 0
#define EMP_DEC_TO_BIN_285  0, 1, 0, 0, 0, 1, 1, 1, 0, 1
#define EMP_DEC_TO_BIN_286  0, 1, 0, 0, 0, 1, 1, 1, 1, 0
#define EMP_DEC_TO_BIN_287  0, 1, 0, 0, 0, 1, 1, 1, 1, 1
#define EMP_DEC_TO_BIN_288  0, 1, 0, 0, 1, 0, 0, 0, 0, 0
#define EMP_DEC_TO_BIN_289  0, 1, 0, 0, 1, 0, 0, 0, 0, 1
#define EMP_DEC_TO_BIN_290  0, 1, 0, 0, 1, 0, 0, 0, 1, 0
#define EMP_DEC_TO_BIN_291  0, 1, 0, 0, 1, 0, 0, 0, 1, 1
#define EMP_DEC_TO_BIN_292  0, 1, 0, 0, 1, 0, 0, 1, 0, 0
#define EMP_DEC_TO_BIN_293  0, 1, 0, 0, 1, 0, 0, 1, 0, 1
#define EMP_DEC_TO_BIN_294  0, 1, 0, 0, 1, 0, 0, 1, 1, 0
#define EMP_DEC_TO_BIN_295  0, 1, 0, 0, 1, 0, 0, 1, 1, 1
#define EMP_DEC_TO_BIN_296  0, 1, 0, 0, 1, 0, 1, 0, 0, 0
#define EMP_DEC_TO_BIN_297  0, 1, 0, 0, 1, 0, 1, 0, 0, 1
#define EMP_DEC_TO_BIN_298  0, 1, 0, 0, 1, 0, 1, 0, 1, 0
#define EMP_DEC_TO_BIN_299  0, 1, 0, 0, 1, 0, 1, 0, 1, 1
#define EMP_DEC_TO_BIN_300  0, 1, 0, 0, 1, 0, 1, 1, 0, 0
#define EMP_DEC_TO_BIN_301  0, 1, 0, 0, 1, 0, 1, 1, 0, 1
#define EMP_DEC_TO_BIN_302  0, 1, 0, 0, 1, 0, 1, 1, 1, 0
#define EMP_DEC_TO_BIN_303  0, 1, 0, 0, 1, 0, 1, 1, 1, 1
#define EMP_DEC_TO_BIN_304  0, 1, 0, 0, 1, 1, 0, 0, 0, 0
#define EMP_DEC_TO_BIN_305  0, 1, 0, 0, 1, 1, 0, 0, 0, 1
#define EMP_DEC_TO_BIN_306  0, 1, 0, 0, 1, 1, 0, 0, 1, 0
#define EMP_DEC_TO_BIN_307  0, 1, 0, 0, 1, 1, 0, 0, 1, 1
#define EMP_DEC_TO_BIN_308  0, 1, 0, 0, 1, 1, 0, 1, 0, 0
#define EMP_DEC_TO_BIN_309  0, 1, 0, 0, 1, 1, 0, 1, 0, 1
#define EMP_DEC_TO_BIN_310  0, 1, 0, 0, 1, 1, 0, 1, 1, 0
#define EMP_DEC_TO_BIN_311  0, 1, 0, 0, 1, 1, 0, 1, 1, 1
#define EMP_DEC_TO_BIN_312  0, 1, 0, 0, 1, 1, 1, 0, 0, 0
#define EMP_DEC_TO_BIN_313  0, 1, 0, 0, 1, 1, 1, 0, 0, 1
#define EMP_DEC_TO_BIN_314  0, 1, 0, 0, 1, 1, 1, 0, 1, 0
#define EMP_DEC_TO_BIN_315  0, 1, 0, 0, 1, 1, 1, 0, 1, 1
#define EMP_DEC_TO_BIN_316  0, 1, 0, 0, 1, 1, 1, 1, 0, 0
#define EMP_DEC_TO_BIN_317  0, 1, 0, 0, 1, 1, 1, 1, 0, 1
#define EMP_DEC_TO_BIN_318  0, 1, 0, 0, 1, 1, 1, 1, 1, 0
#define EMP_DEC_TO_BIN_319  0, 1, 0, 0, 1, 1, 1, 1, 1, 1
#define EMP_DEC_TO_BIN_320  0, 1, 0, 1, 0, 0, 0, 0, 0, 0
#define EMP_DEC_TO_BIN_321  0, 1, 0, 1, 0, 0, 0, 0, 0, 1
#define EMP_DEC_TO_BIN_322  0, 1, 0, 1, 0, 0, 0, 0, 1, 0
#define EMP_DEC_TO_BIN_323  0, 1, 0, 1, 0, 0, 0, 0, 1, 1
#define EMP_DEC_TO_BIN_324  0, 1, 0, 1, 0, 0, 0, 1, 0, 0
#define EMP_DEC_TO_BIN_325  0, 1, 0, 1, 0, 0, 0, 1, 0, 1
#define EMP_DEC_TO_BIN_326  0, 1, 0, 1, 0, 0, 0, 1, 1, 0
#define EMP_DEC_TO_BIN_327  0, 1, 0, 1, 0, 0, 0, 1, 1, 1
#define EMP_DEC_TO_BIN_328  0, 1, 0, 1, 0, 0, 1, 0, 0, 0
#define EMP_DEC_TO_BIN_329  0, 1, 0, 1, 0, 0, 1, 0, 0, 1
#define EMP_DEC_TO_BIN_330  0, 1, 0, 1, 0, 0, 1, 0, 1, 0
#define EMP_DEC_TO_BIN_331  0, 1, 0, 1, 0, 0, 1, 0, 1, 1
#define EMP_DEC_TO_BIN_332  0, 1, 0, 1, 0, 0, 1, 1, 0, 0
#define EMP_DEC_TO_BIN_333  0, 1, 0, 1, 0, 0, 1, 1, 0, 1
#define EMP_DEC_TO_BIN_334  0, 1, 0, 1, 0, 0, 1, 1, 1, 0
#define EMP_DEC_TO_BIN_335  0, 1, 0, 1, 0, 0, 1, 1, 1, 1
#define EMP_DEC_TO_BIN_336  0, 1, 0, 1, 0, 1, 0, 0, 0, 0
#define EMP_DEC_TO_BIN_337  0, 1, 0, 1, 0, 1, 0, 0, 0, 1
#define EMP_DEC_TO_BIN_338  0, 1, 0, 1, 0, 1, 0, 0, 1, 0
#define EMP_DEC_TO_BIN_339  0, 1, 0, 1, 0, 1, 0, 0, 1, 1
#define EMP_DEC_TO_BIN_340  0, 1, 0, 1, 0, 1, 0, 1, 0, 0
#define EMP_DEC_TO_BIN_341  0, 1, 0, 1, 0, 1, 0, 1, 0, 1
#define EMP_DEC_TO_BIN_342  0, 1, 0, 1, 0, 1, 0, 1, 1, 0
#define EMP_DEC_TO_BIN_343  0, 1, 0, 1, 0, 1, 0, 1, 1, 1
#define EMP_DEC_TO_BIN_344  0, 1, 0, 1, 0, 1, 1, 0, 0, 0
#define EMP_DEC_TO_BIN_345  0, 1, 0, 1, 0, 1, 1, 0, 0, 1
#define EMP_DEC_TO_BIN_346  0, 1, 0, 1, 0, 1, 1, 0, 1, 0
#define EMP_DEC_TO_BIN_347  0, 1, 0, 1, 0, 1, 1, 0, 1, 1
#define EMP_DEC_TO_BIN_348  0, 1, 0, 1, 0, 1, 1, 1, 0, 0
#define EMP_DEC_TO_BIN_349  0, 1, 0, 1, 0, 1, 1, 1, 0, 1
#define EMP_DEC_TO_BIN_350  0, 1, 0, 1, 0, 1, 1, 1, 1, 0
#define EMP_DEC_TO_BIN_351  0, 1, 0, 1, 0, 1, 1, 1, 1, 1
#define EMP_DEC_TO_BIN_352  0, 1, 0, 1, 1, 0, 0, 0, 0, 0
#define EMP_DEC_TO_BIN_353  0, 1, 0, 1, 1, 0, 0, 0, 0, 1
#define EMP_DEC_TO_BIN_354  0, 1, 0, 1, 1, 0, 0, 0, 1, 0
#define EMP_DEC_TO_BIN_355  0, 1, 0, 1, 1, 0, 0, 0, 1, 1
#define EMP_DEC_TO_BIN_356  0, 1, 0, 1, 1, 0, 0, 1, 0, 0
#define EMP_DEC_TO_BIN_357  0, 1, 0, 1, 1, 0, 0, 1, 0, 1
#define EMP_DEC_TO_BIN_358  0, 1, 0, 1, 1, 0, 0, 1, 1, 0
#define EMP_DEC_TO_BIN_359  0, 1, 0, 1, 1, 0, 0, 1, 1, 1
#define EMP_DEC_TO_BIN_360  0, 1, 0, 1, 1, 0, 1, 0, 0, 0
#define EMP_DEC_TO_BIN_361  0, 1, 0, 1, 1, 0, 1, 0, 0, 1
#define EMP_DEC_TO_BIN_362  0, 1, 0, 1, 1, 0, 1, 0, 1, 0
#define EMP_DEC_TO_BIN_363  0, 1, 0, 1, 1, 0, 1, 0, 1, 1
#define EMP_DEC_TO_BIN_364  0, 1, 0, 1, 1, 0, 1, 1, 0, 0
#define EMP_DEC_TO_BIN_365  0, 1, 0, 1, 1, 0, 1, 1, 0, 1
#define EMP_DEC_TO_BIN_366  0, 1, 0, 1, 1, 0, 1, 1, 1, 0
#define EMP_DEC_TO_BIN_367  0, 1, 0, 1, 1, 0, 1, 1, 1, 1
#define EMP_DEC_TO_BIN_368  0, 1, 0, 1, 1, 1, 0, 0, 0, 0
#define EMP_DEC_TO_BIN_369  0, 1, 0, 1, 1, 1, 0, 0, 0, 1
#define EMP_DEC_TO_BIN_370  0, 1, 0, 1, 1, 1, 0, 0, 1, 0
#define EMP_DEC_TO_BIN_371  0, 1, 0, 1, 1, 1, 0, 0, 1, 1
#define EMP_DEC_TO_BIN_372  0, 1, 0, 1, 1, 1, 0, 1, 0, 0
#define EMP_DEC_TO_BIN_373  0, 1, 0, 1, 1, 1, 0, 1, 0, 1
#define EMP_DEC_TO_BIN_374  0, 1, 0, 1, 1, 1, 0, 1, 1, 0
#define EMP_DEC_TO_BIN_375  0, 1, 0, 1, 1, 1, 0, 1, 1, 1
#define EMP_DEC_TO_BIN_376  0, 1, 0, 1, 1, 1, 1, 0, 0, 0
#define EMP_DEC_TO_BIN_377  0, 1, 0, 1, 1, 1, 1, 0, 0, 1
#define EMP_DEC_TO_BIN_378  0, 1, 0, 1, 1, 1, 1, 0, 1, 0
#define EMP_DEC_TO_BIN_379  0, 1, 0, 1, 1, 1, 1, 0, 1, 1
#define EMP_DEC_TO_BIN_380  0, 1, 0, 1, 1, 1, 1, 1, 0, 0
#define EMP_DEC_TO_BIN_381  0, 1, 0, 1, 1, 1, 1, 1, 0, 1
#define EMP_DEC_TO_BIN_382  0, 1, 0, 1, 1, 1, 1, 1, 1, 0
#define EMP_DEC_TO_BIN_383  0, 1, 0, 1, 1, 1, 1, 1, 1, 1
#define EMP_DEC_TO_BIN_384  0, 1, 1, 0, 0, 0, 0, 0, 0, 0
#define EMP_DEC_TO_BIN_385  0, 1, 1, 0, 0, 0, 0, 0, 0, 1
#define EMP_DEC_TO_BIN_386  0, 1, 1, 0, 0, 0, 0, 0, 1, 0
#define EMP_DEC_TO_BIN_387  0, 1, 1, 0, 0, 0, 0, 0, 1, 1
#define EMP_DEC_TO_BIN_388  0, 1, 1, 0, 0, 0, 0, 1, 0, 0
#define EMP_DEC_TO_BIN_389  0, 1, 1, 0, 0, 0, 0, 1, 0, 1
#define EMP_DEC_TO_BIN_390  0, 1, 1, 0, 0, 0, 0, 1, 1, 0
#define EMP_DEC_TO_BIN_391  0, 1, 1, 0, 0, 0, 0, 1, 1, 1
#define EMP_DEC_TO_BIN_392  0, 1, 1, 0, 0, 0, 1, 0, 0, 0
#define EMP_DEC_TO_BIN_393  0, 1, 1, 0, 0, 0, 1, 0, 0, 1
#define EMP_DEC_TO_BIN_394  0, 1, 1, 0, 0, 0, 1, 0, 1, 0
#define EMP_DEC_TO_BIN_395  0, 1, 1, 0, 0, 0, 1, 0, 1, 1
#define EMP_DEC_TO_BIN_396  0, 1, 1, 0, 0, 0, 1, 1, 0, 0
#define EMP_DEC_TO_BIN_397  0, 1, 1, 0, 0, 0, 1, 1, 0, 1
#define EMP_DEC_TO_BIN_398  0, 1, 1, 0, 0, 0, 1, 1, 1, 0
#define EMP_DEC_TO_BIN_399  0, 1, 1, 0, 0, 0, 1, 1, 1, 1
#define EMP_DEC_TO_BIN_400  0, 1, 1, 0, 0, 1, 0, 0, 0, 0
#define EMP_DEC_TO_BIN_401  0, 1, 1, 0, 0, 1, 0, 0, 0, 1
#define EMP_DEC_TO_BIN_402  0, 1, 1, 0, 0, 1, 0, 0, 1, 0
#define EMP_DEC_TO_BIN_403  0, 1, 1, 0, 0, 1, 0, 0, 1, 1
#define EMP_DEC_TO_BIN_404  0, 1, 1, 0, 0, 1, 0, 1, 0, 0
#define EMP_DEC_TO_BIN_405  0, 1, 1, 0, 0, 1, 0, 1, 0, 1
#define EMP_DEC_TO_BIN_406  0, 1, 1, 0, 0, 1, 0, 1, 1, 0
#define EMP_DEC_TO_BIN_407  0, 1, 1, 0, 0, 1, 0, 1, 1, 1
#define EMP_DEC_TO_BIN_408  0, 1, 1, 0, 0, 1, 1, 0, 0, 0
#define EMP_DEC_TO_BIN_409  0, 1, 1, 0, 0, 1, 1, 0, 0, 1
#define EMP_DEC_TO_BIN_410  0, 1, 1, 0, 0, 1, 1, 0, 1, 0
#define EMP_DEC_TO_BIN_411  0, 1, 1, 0, 0, 1, 1, 0, 1, 1
#define EMP_DEC_TO_BIN_412  0, 1, 1, 0, 0, 1, 1, 1, 0, 0
#define EMP_DEC_TO_BIN_413  0, 1, 1, 0, 0, 1, 1, 1, 0, 1
#define EMP_DEC_TO_BIN_414  0, 1, 1, 0, 0, 1, 1, 1, 1, 0
#define EMP_DEC_TO_BIN_415  0, 1, 1, 0, 0, 1, 1, 1, 1, 1
#define EMP_DEC_TO_BIN_416  0, 1, 1, 0, 1, 0, 0, 0, 0, 0
#define EMP_DEC_TO_BIN_417  0, 1, 1, 0, 1, 0, 0, 0, 0, 1
#define EMP_DEC_TO_BIN_418  0, 1, 1, 0, 1, 0, 0, 0, 1, 0
#define EMP_DEC_TO_BIN_419  0, 1, 1, 0, 1, 0, 0, 0, 1, 1
#define EMP_DEC_TO_BIN_420  0, 1, 1, 0, 1, 0, 0, 1, 0, 0
#define EMP_DEC_TO_BIN_421  0, 1, 1, 0, 1, 0, 0, 1, 0, 1
#define EMP_DEC_TO_BIN_422  0, 1, 1, 0, 1, 0, 0, 1, 1, 0
#define EMP_DEC_TO_BIN_423  0, 1, 1, 0, 1, 0, 0, 1, 1, 1
#define EMP_DEC_TO_BIN_424  0, 1, 1, 0, 1, 0, 1, 0, 0, 0
#define EMP_DEC_TO_BIN_425  0, 1, 1, 0, 1, 0, 1, 0, 0, 1
#define EMP_DEC_TO_BIN_426  0, 1, 1, 0, 1, 0, 1, 0, 1, 0
#define EMP_DEC_TO_BIN_427  0, 1, 1, 0, 1, 0, 1, 0, 1, 1
#define EMP_DEC_TO_BIN_428  0, 1, 1, 0, 1, 0, 1, 1, 0, 0
#define EMP_DEC_TO_BIN_429  0, 1, 1, 0, 1, 0, 1, 1, 0, 1
#define EMP_DEC_TO_BIN_430  0, 1, 1, 0, 1, 0, 1, 1, 1, 0
#define EMP_DEC_TO_BIN_431  0, 1, 1, 0, 1, 0, 1, 1, 1, 1
#define EMP_DEC_TO_BIN_432  0, 1, 1, 0, 1, 1, 0, 0, 0, 0
#define EMP_DEC_TO_BIN_433  0, 1, 1, 0, 1, 1, 0, 0, 0, 1
#define EMP_DEC_TO_BIN_434  0, 1, 1, 0, 1, 1, 0, 0, 1, 0
#define EMP_DEC_TO_BIN_435  0, 1, 1, 0, 1, 1, 0, 0, 1, 1
#define EMP_DEC_TO_BIN_436  0, 1, 1, 0, 1, 1, 0, 1, 0, 0
#define EMP_DEC_TO_BIN_437  0, 1, 1, 0, 1, 1, 0, 1, 0, 1
#define EMP_DEC_TO_BIN_438  0, 1, 1, 0, 1, 1, 0, 1, 1, 0
#define EMP_DEC_TO_BIN_439  0, 1, 1, 0, 1, 1, 0, 1, 1, 1
#define EMP_DEC_TO_BIN_440  0, 1, 1, 0, 1, 1, 1, 0, 0, 0
#define EMP_DEC_TO_BIN_441  0, 1, 1, 0, 1, 1, 1, 0, 0, 1
#define EMP_DEC_TO_BIN_442  0, 1, 1, 0, 1, 1, 1, 0, 1, 0
#define EMP_DEC_TO_BIN_443  0, 1, 1, 0, 1, 1, 1, 0, 1, 1
#define EMP_DEC_TO_BIN_444  0, 1, 1, 0, 1, 1, 1, 1, 0, 0
#define EMP_DEC_TO_BIN_445  0, 1, 1, 0, 1, 1, 1, 1, 0, 1
#define EMP_DEC_TO_BIN_446  0, 1, 1, 0, 1, 1, 1, 1, 1, 0
#define EMP_DEC_TO_BIN_447  0, 1, 1, 0, 1, 1, 1, 1, 1, 1
#define EMP_DEC_TO_BIN_448  0, 1, 1, 1, 0, 0, 0, 0, 0, 0
#define EMP_DEC_TO_BIN_449  0, 1, 1, 1, 0, 0, 0, 0, 0, 1
#define EMP_DEC_TO_BIN_450  0, 1, 1, 1, 0, 0, 0, 0, 1, 0
#define EMP_DEC_TO_BIN_451  0, 1, 1, 1, 0, 0, 0, 0, 1, 1
#define EMP_DEC_TO_BIN_452  0, 1, 1, 1, 0, 0, 0, 1, 0, 0
#define EMP_DEC_TO_BIN_453  0, 1, 1, 1, 0, 0, 0, 1, 0, 1
#define EMP_DEC_TO_BIN_454  0, 1, 1, 1, 0, 0, 0, 1, 1, 0
#define EMP_DEC_TO_BIN_455  0, 1, 1, 1, 0, 0, 0, 1, 1, 1
#define EMP_DEC_TO_BIN_456  0, 1, 1, 1, 0, 0, 1, 0, 0, 0
#define EMP_DEC_TO_BIN_457  0, 1, 1, 1, 0, 0, 1, 0, 0, 1
#define EMP_DEC_TO_BIN_458  0, 1, 1, 1, 0, 0, 1, 0, 1, 0
#define EMP_DEC_TO_BIN_459  0, 1, 1, 1, 0, 0, 1, 0, 1, 1
#define EMP_DEC_TO_BIN_460  0, 1, 1, 1, 0, 0, 1, 1, 0, 0
#define EMP_DEC_TO_BIN_461  0, 1, 1, 1, 0, 0, 1, 1, 0, 1
#define EMP_DEC_TO_BIN_462  0, 1, 1, 1, 0, 0, 1, 1, 1, 0
#define EMP_DEC_TO_BIN_463  0, 1, 1, 1, 0, 0, 1, 1, 1, 1
#define EMP_DEC_TO_BIN_464  0, 1, 1, 1, 0, 1, 0, 0, 0, 0
#define EMP_DEC_TO_BIN_465  0, 1, 1, 1, 0, 1, 0, 0, 0, 1
#define EMP_DEC_TO_BIN_466  0, 1, 1, 1, 0, 1, 0, 0, 1, 0
#define EMP_DEC_TO_BIN_467  0, 1, 1, 1, 0, 1, 0, 0, 1, 1
#define EMP_DEC_TO_BIN_468  0, 1, 1, 1, 0, 1, 0, 1, 0, 0
#define EMP_DEC_TO_BIN_469  0, 1, 1, 1, 0, 1, 0, 1, 0, 1
#define EMP_DEC_TO_BIN_470  0, 1, 1, 1, 0, 1, 0, 1, 1, 0
#define EMP_DEC_TO_BIN_471  0, 1, 1, 1, 0, 1, 0, 1, 1, 1
#define EMP_DEC_TO_BIN_472  0, 1, 1, 1, 0, 1, 1, 0, 0, 0
#define EMP_DEC_TO_BIN_473  0, 1, 1, 1, 0, 1, 1, 0, 0, 1
#define EMP_DEC_TO_BIN_474  0, 1, 1, 1, 0, 1, 1, 0, 1, 0
#define EMP_DEC_TO_BIN_475  0, 1, 1, 1, 0, 1, 1, 0, 1, 1
#define EMP_DEC_TO_BIN_476  0, 1, 1, 1, 0, 1, 1, 1, 0, 0
#define EMP_DEC_TO_BIN_477  0, 1, 1, 1, 0, 1, 1, 1, 0, 1
#define EMP_DEC_TO_BIN_478  0, 1, 1, 1, 0, 1, 1, 1, 1, 0
#define EMP_DEC_TO_BIN_479  0, 1, 1, 1, 0, 1, 1, 1, 1, 1
#define EMP_DEC_TO_BIN_480  0, 1, 1, 1, 1, 0, 0, 0, 0, 0
#define EMP_DEC_TO_BIN_481  0, 1, 1, 1, 1, 0, 0, 0, 0, 1
#define EMP_DEC_TO_BIN_482  0, 1, 1, 1, 1, 0, 0, 0, 1, 0
#define EMP_DEC_TO_BIN_483  0, 1, 1, 1, 1, 0, 0, 0, 1, 1
#define EMP_DEC_TO_BIN_484  0, 1, 1, 1, 1, 0, 0, 1, 0, 0
#define EMP_DEC_TO_BIN_485  0, 1, 1, 1, 1, 0, 0, 1, 0, 1
#define EMP_DEC_TO_BIN_486  0, 1, 1, 1, 1, 0, 0, 1, 1, 0
#define EMP_DEC_TO_BIN_487  0, 1, 1, 1, 1, 0, 0, 1, 1, 1
#define EMP_DEC_TO_BIN_488  0, 1, 1, 1, 1, 0, 1, 0, 0, 0
#define EMP_DEC_TO_BIN_489  0, 1, 1, 1, 1, 0, 1, 0, 0, 1
#define EMP_DEC_TO_BIN_490  0, 1, 1, 1, 1, 0, 1, 0, 1, 0
#define EMP_DEC_TO_BIN_491  0, 1, 1, 1, 1, 0, 1, 0, 1, 1
#define EMP_DEC_TO_BIN_492  0, 1, 1, 1, 1, 0, 1, 1, 0, 0
#define EMP_DEC_TO_BIN_493  0, 1, 1, 1, 1, 0, 1, 1, 0, 1
#define EMP_DEC_TO_BIN_494  0, 1, 1, 1, 1, 0, 1, 1, 1, 0
#define EMP_DEC_TO_BIN_495  0, 1, 1, 1, 1, 0, 1, 1, 1, 1
#define EMP_DEC_TO_BIN_496  0, 1, 1, 1, 1, 1, 0, 0, 0, 0
#define EMP_DEC_TO_BIN_497  0, 1, 1, 1, 1, 1, 0, 0, 0, 1
#define EMP_DEC_TO_BIN_498  0, 1, 1, 1, 1, 1, 0, 0, 1, 0
#define EMP_DEC_TO_BIN_499  0, 1, 1, 1, 1, 1, 0, 0, 1, 1
#define EMP_DEC_TO_BIN_500  0, 1, 1, 1, 1, 1, 0, 1, 0, 0
#define EMP_DEC_TO_BIN_501  0, 1, 1, 1, 1, 1, 0, 1, 0, 1
#define EMP_DEC_TO_BIN_502  0, 1, 1, 1, 1, 1, 0, 1, 1, 0
#define EMP_DEC_TO_BIN_503  0, 1, 1, 1, 1, 1, 0, 1, 1, 1
#define EMP_DEC_TO_BIN_504  0, 1, 1, 1, 1, 1, 1, 0, 0, 0
#define EMP_DEC_TO_BIN_505  0, 1, 1, 1, 1, 1, 1, 0, 0, 1
#define EMP_DEC_TO_BIN_506  0, 1, 1, 1, 1, 1, 1, 0, 1, 0
#define EMP_DEC_TO_BIN_507  0, 1, 1, 1, 1, 1, 1, 0, 1, 1
#define EMP_DEC_TO_BIN_508  0, 1, 1, 1, 1, 1, 1, 1, 0, 0
#define EMP_DEC_TO_BIN_509  0, 1, 1, 1, 1, 1, 1, 1, 0, 1
#define EMP_DEC_TO_BIN_510  0, 1, 1, 1, 1, 1, 1, 1, 1, 0
#define EMP_DEC_TO_BIN_511  0, 1, 1, 1, 1, 1, 1, 1, 1, 1
#define EMP_DEC_TO_BIN_512  1, 0, 0, 0, 0, 0, 0, 0, 0, 0
#define EMP_DEC_TO_BIN_513  1, 0, 0, 0, 0, 0, 0, 0, 0, 1
#define EMP_DEC_TO_BIN_514  1, 0, 0, 0, 0, 0, 0, 0, 1, 0
#define EMP_DEC_TO_BIN_515  1, 0, 0, 0, 0, 0, 0, 0, 1, 1
#define EMP_DEC_TO_BIN_516  1, 0, 0, 0, 0, 0, 0, 1, 0, 0
#define EMP_DEC_TO_BIN_517  1, 0, 0, 0, 0, 0, 0, 1, 0, 1
#define EMP_DEC_TO_BIN_518  1, 0, 0, 0, 0, 0, 0, 1, 1, 0
#define EMP_DEC_TO_BIN_519  1, 0, 0, 0, 0, 0, 0, 1, 1, 1
#define EMP_DEC_TO_BIN_520  1, 0, 0, 0, 0, 0, 1, 0, 0, 0
#define EMP_DEC_TO_BIN_521  1, 0, 0, 0, 0, 0, 1, 0, 0, 1
#define EMP_DEC_TO_BIN_522  1, 0, 0, 0, 0, 0, 1, 0, 1, 0
#define EMP_DEC_TO_BIN_523  1, 0, 0, 0, 0, 0, 1, 0, 1, 1
#define EMP_DEC_TO_BIN_524  1, 0, 0, 0, 0, 0, 1, 1, 0, 0
#define EMP_DEC_TO_BIN_525  1, 0, 0, 0, 0, 0, 1, 1, 0, 1
#define EMP_DEC_TO_BIN_526  1, 0, 0, 0, 0, 0, 1, 1, 1, 0
#define EMP_DEC_TO_BIN_527  1, 0, 0, 0, 0, 0, 1, 1, 1, 1
#define EMP_DEC_TO_BIN_528  1, 0, 0, 0, 0, 1, 0, 0, 0, 0
#define EMP_DEC_TO_BIN_529  1, 0, 0, 0, 0, 1, 0, 0, 0, 1
#define EMP_DEC_TO_BIN_530  1, 0, 0, 0, 0, 1, 0, 0, 1, 0
#define EMP_DEC_TO_BIN_531  1, 0, 0, 0, 0, 1, 0, 0, 1, 1
#define EMP_DEC_TO_BIN_532  1, 0, 0, 0, 0, 1, 0, 1, 0, 0
#define EMP_DEC_TO_BIN_533  1, 0, 0, 0, 0, 1, 0, 1, 0, 1
#define EMP_DEC_TO_BIN_534  1, 0, 0, 0, 0, 1, 0, 1, 1, 0
#define EMP_DEC_TO_BIN_535  1, 0, 0, 0, 0, 1, 0, 1, 1, 1
#define EMP_DEC_TO_BIN_536  1, 0, 0, 0, 0, 1, 1, 0, 0, 0
#define EMP_DEC_TO_BIN_537  1, 0, 0, 0, 0, 1, 1, 0, 0, 1
#define EMP_DEC_TO_BIN_538  1, 0, 0, 0, 0, 1, 1, 0, 1, 0
#define EMP_DEC_TO_BIN_539  1, 0, 0, 0, 0, 1, 1, 0, 1, 1
#define EMP_DEC_TO_BIN_540  1, 0, 0, 0, 0, 1, 1, 1, 0, 0
#define EMP_DEC_TO_BIN_541  1, 0, 0, 0, 0, 1, 1, 1, 0, 1
#define EMP_DEC_TO_BIN_542  1, 0, 0, 0, 0, 1, 1, 1, 1, 0
#define EMP_DEC_TO_BIN_543  1, 0, 0, 0, 0, 1, 1, 1, 1, 1
#define EMP_DEC_TO_BIN_544  1, 0, 0, 0, 1, 0, 0, 0, 0, 0
#define EMP_DEC_TO_BIN_545  1, 0, 0, 0, 1, 0, 0, 0, 0, 1
#define EMP_DEC_TO_BIN_546  1, 0, 0, 0, 1, 0, 0, 0, 1, 0
#define EMP_DEC_TO_BIN_547  1, 0, 0, 0, 1, 0, 0, 0, 1, 1
#define EMP_DEC_TO_BIN_548  1, 0, 0, 0, 1, 0, 0, 1, 0, 0
#define EMP_DEC_TO_BIN_549  1, 0, 0, 0, 1, 0, 0, 1, 0, 1
#define EMP_DEC_TO_BIN_550  1, 0, 0, 0, 1, 0, 0, 1, 1, 0
#define EMP_DEC_TO_BIN_551  1, 0, 0, 0, 1, 0, 0, 1, 1, 1
#define EMP_DEC_TO_BIN_552  1, 0, 0, 0, 1, 0, 1, 0, 0, 0
#define EMP_DEC_TO_BIN_553  1, 0, 0, 0, 1, 0, 1, 0, 0, 1
#define EMP_DEC_TO_BIN_554  1, 0, 0, 0, 1, 0, 1, 0, 1, 0
#define EMP_DEC_TO_BIN_555  1, 0, 0, 0, 1, 0, 1, 0, 1, 1
#define EMP_DEC_TO_BIN_556  1, 0, 0, 0, 1, 0, 1, 1, 0, 0
#define EMP_DEC_TO_BIN_557  1, 0, 0, 0, 1, 0, 1, 1, 0, 1
#define EMP_DEC_TO_BIN_558  1, 0, 0, 0, 1, 0, 1, 1, 1, 0
#define EMP_DEC_TO_BIN_559  1, 0, 0, 0, 1, 0, 1, 1, 1, 1
#define EMP_DEC_TO_BIN_560  1, 0, 0, 0, 1, 1, 0, 0, 0, 0
#define EMP_DEC_TO_BIN_561  1, 0, 0, 0, 1, 1, 0, 0, 0, 1
#define EMP_DEC_TO_BIN_562  1, 0, 0, 0, 1, 1, 0, 0, 1, 0
#define EMP_DEC_TO_BIN_563  1, 0, 0, 0, 1, 1, 0, 0, 1, 1
#define EMP_DEC_TO_BIN_564  1, 0, 0, 0, 1, 1, 0, 1, 0, 0
#define EMP_DEC_TO_BIN_565  1, 0, 0, 0, 1, 1, 0, 1, 0, 1
#define EMP_DEC_TO_BIN_566  1, 0, 0, 0, 1, 1, 0, 1, 1, 0
#define EMP_DEC_TO_BIN_567  1, 0, 0, 0, 1, 1, 0, 1, 1, 1
#define EMP_DEC_TO_BIN_568  1, 0, 0, 0, 1, 1, 1, 0, 0, 0
#define EMP_DEC_TO_BIN_569  1, 0, 0, 0, 1, 1, 1, 0, 0, 1
#define EMP_DEC_TO_BIN_570  1, 0, 0, 0, 1, 1, 1, 0, 1, 0
#define EMP_DEC_TO_BIN_571  1, 0, 0, 0, 1, 1, 1, 0, 1, 1
#define EMP_DEC_TO_BIN_572  1, 0, 0, 0, 1, 1, 1, 1, 0, 0
#define EMP_DEC_TO_BIN_573  1, 0, 0, 0, 1, 1, 1, 1, 0, 1
#define EMP_DEC_TO_BIN_574  1, 0, 0, 0, 1, 1, 1, 1, 1, 0
#define EMP_DEC_TO_BIN_575  1, 0, 0, 0, 1, 1, 1, 1, 1, 1
#define EMP_DEC_TO_BIN_576  1, 0, 0, 1, 0, 0, 0, 0, 0, 0
#define EMP_DEC_TO_BIN_577  1, 0, 0, 1, 0, 0, 0, 0, 0, 1
#define EMP_DEC_TO_BIN_578  1, 0, 0, 1, 0, 0, 0, 0, 1, 0
#define EMP_DEC_TO_BIN_579  1, 0, 0, 1, 0, 0, 0, 0, 1, 1
#define EMP_DEC_TO_BIN_580  1, 0, 0, 1, 0, 0, 0, 1, 0, 0
#define EMP_DEC_TO_BIN_581  1, 0, 0, 1, 0, 0, 0, 1, 0, 1
#define EMP_DEC_TO_BIN_582  1, 0, 0, 1, 0, 0, 0, 1, 1, 0
#define EMP_DEC_TO_BIN_583  1, 0, 0, 1, 0, 0, 0, 1, 1, 1
#define EMP_DEC_TO_BIN_584  1, 0, 0, 1, 0, 0, 1, 0, 0, 0
#define EMP_DEC_TO_BIN_585  1, 0, 0, 1, 0, 0, 1, 0, 0, 1
#define EMP_DEC_TO_BIN_586  1, 0, 0, 1, 0, 0, 1, 0, 1, 0
#define EMP_DEC_TO_BIN_587  1, 0, 0, 1, 0, 0, 1, 0, 1, 1
#define EMP_DEC_TO_BIN_588  1, 0, 0, 1, 0, 0, 1, 1, 0, 0
#define EMP_DEC_TO_BIN_589  1, 0, 0, 1, 0, 0, 1, 1, 0, 1
#define EMP_DEC_TO_BIN_590  1, 0, 0, 1, 0, 0, 1, 1, 1, 0
#define EMP_DEC_TO_BIN_591  1, 0, 0, 1, 0, 0, 1, 1, 1, 1
#define EMP_DEC_TO_BIN_592  1, 0, 0, 1, 0, 1, 0, 0, 0, 0
#define EMP_DEC_TO_BIN_593  1, 0, 0, 1, 0, 1, 0, 0, 0, 1
#define EMP_DEC_TO_BIN_594  1, 0, 0, 1, 0, 1, 0, 0, 1, 0
#define EMP_DEC_TO_BIN_595  1, 0, 0, 1, 0, 1, 0, 0, 1, 1
#define EMP_DEC_TO_BIN_596  1, 0, 0, 1, 0, 1, 0, 1, 0, 0
#define EMP_DEC_TO_BIN_597  1, 0, 0, 1, 0, 1, 0, 1, 0, 1
#define EMP_DEC_TO_BIN_598  1, 0, 0, 1, 0, 1, 0, 1, 1, 0
#define EMP_DEC_TO_BIN_599  1, 0, 0, 1, 0, 1, 0, 1, 1, 1
#define EMP_DEC_TO_BIN_600  1, 0, 0, 1, 0, 1, 1, 0, 0, 0
#define EMP_DEC_TO_BIN_601  1, 0, 0, 1, 0, 1, 1, 0, 0, 1
#define EMP_DEC_TO_BIN_602  1, 0, 0, 1, 0, 1, 1, 0, 1, 0
#define EMP_DEC_TO_BIN_603  1, 0, 0, 1, 0, 1, 1, 0, 1, 1
#define EMP_DEC_TO_BIN_604  1, 0, 0, 1, 0, 1, 1, 1, 0, 0
#define EMP_DEC_TO_BIN_605  1, 0, 0, 1, 0, 1, 1, 1, 0, 1
#define EMP_DEC_TO_BIN_606  1, 0, 0, 1, 0, 1, 1, 1, 1, 0
#define EMP_DEC_TO_BIN_607  1, 0, 0, 1, 0, 1, 1, 1, 1, 1
#define EMP_DEC_TO_BIN_608  1, 0, 0, 1, 1, 0, 0, 0, 0, 0
#define EMP_DEC_TO_BIN_609  1, 0, 0, 1, 1, 0, 0, 0, 0, 1
#define EMP_DEC_TO_BIN_610  1, 0, 0, 1, 1, 0, 0, 0, 1, 0
#define EMP_DEC_TO_BIN_611  1, 0, 0, 1, 1, 0, 0, 0, 1, 1
#define EMP_DEC_TO_BIN_612  1, 0, 0, 1, 1, 0, 0, 1, 0, 0
#define EMP_DEC_TO_BIN_613  1, 0, 0, 1, 1, 0, 0, 1, 0, 1
#define EMP_DEC_TO_BIN_614  1, 0, 0, 1, 1, 0, 0, 1, 1, 0
#define EMP_DEC_TO_BIN_615  1, 0, 0, 1, 1, 0, 0, 1, 1, 1
#define EMP_DEC_TO_BIN_616  1, 0, 0, 1, 1, 0, 1, 0, 0, 0
#define EMP_DEC_TO_BIN_617  1, 0, 0, 1, 1, 0, 1, 0, 0, 1
#define EMP_DEC_TO_BIN_618  1, 0, 0, 1, 1, 0, 1, 0, 1, 0
#define EMP_DEC_TO_BIN_619  1, 0, 0, 1, 1, 0, 1, 0, 1, 1
#define EMP_DEC_TO_BIN_620  1, 0, 0, 1, 1, 0, 1, 1, 0, 0
#define EMP_DEC_TO_BIN_621  1, 0, 0, 1, 1, 0, 1, 1, 0, 1
#define EMP_DEC_TO_BIN_622  1, 0, 0, 1, 1, 0, 1, 1, 1, 0
#define EMP_DEC_TO_BIN_623  1, 0, 0, 1, 1, 0, 1, 1, 1, 1
#define EMP_DEC_TO_BIN_624  1, 0, 0, 1, 1, 1, 0, 0, 0, 0
#define EMP_DEC_TO_BIN_625  1, 0, 0, 1, 1, 1, 0, 0, 0, 1
#define EMP_DEC_TO_BIN_626  1, 0, 0, 1, 1, 1, 0, 0, 1, 0
#define EMP_DEC_TO_BIN_627  1, 0, 0, 1, 1, 1, 0, 0, 1, 1
#define EMP_DEC_TO_BIN_628  1, 0, 0, 1, 1, 1, 0, 1, 0, 0
#define EMP_DEC_TO_BIN_629  1, 0, 0, 1, 1, 1, 0, 1, 0, 1
#define EMP_DEC_TO_BIN_630  1, 0, 0, 1, 1, 1, 0, 1, 1, 0
#define EMP_DEC_TO_BIN_631  1, 0, 0, 1, 1, 1, 0, 1, 1, 1
#define EMP_DEC_TO_BIN_632  1, 0, 0, 1, 1, 1, 1, 0, 0, 0
#define EMP_DEC_TO_BIN_633  1, 0, 0, 1, 1, 1, 1, 0, 0, 1
#define EMP_DEC_TO_BIN_634  1, 0, 0, 1, 1, 1, 1, 0, 1, 0
#define EMP_DEC_TO_BIN_635  1, 0, 0, 1, 1, 1, 1, 0, 1, 1
#define EMP_DEC_TO_BIN_636  1, 0, 0, 1, 1, 1, 1, 1, 0, 0
#define EMP_DEC_TO_BIN_637  1, 0, 0, 1, 1, 1, 1, 1, 0, 1
#define EMP_DEC_TO_BIN_638  1, 0, 0, 1, 1, 1, 1, 1, 1, 0
#define EMP_DEC_TO_BIN_639  1, 0, 0, 1, 1, 1, 1, 1, 1, 1
#define EMP_DEC_TO_BIN_640  1, 0, 1, 0, 0, 0, 0, 0, 0, 0
#define EMP_DEC_TO_BIN_641  1, 0, 1, 0, 0, 0, 0, 0, 0, 1
#define EMP_DEC_TO_BIN_642  1, 0, 1, 0, 0, 0, 0, 0, 1, 0
#define EMP_DEC_TO_BIN_643  1, 0, 1, 0, 0, 0, 0, 0, 1, 1
#define EMP_DEC_TO_BIN_644  1, 0, 1, 0, 0, 0, 0, 1, 0, 0
#define EMP_DEC_TO_BIN_645  1, 0, 1, 0, 0, 0, 0, 1, 0, 1
#define EMP_DEC_TO_BIN_646  1, 0, 1, 0, 0, 0, 0, 1, 1, 0
#define EMP_DEC_TO_BIN_647  1, 0, 1, 0, 0, 0, 0, 1, 1, 1
#define EMP_DEC_TO_BIN_648  1, 0, 1, 0, 0, 0, 1, 0, 0, 0
#define EMP_DEC_TO_BIN_649  1, 0, 1, 0, 0, 0, 1, 0, 0, 1
#define EMP_DEC_TO_BIN_650  1, 0, 1, 0, 0, 0, 1, 0, 1, 0
#define EMP_DEC_TO_BIN_651  1, 0, 1, 0, 0, 0, 1, 0, 1, 1
#define EMP_DEC_TO_BIN_652  1, 0, 1, 0, 0, 0, 1, 1, 0, 0
#define EMP_DEC_TO_BIN_653  1, 0, 1, 0, 0, 0, 1, 1, 0, 1
#define EMP_DEC_TO_BIN_654  1, 0, 1, 0, 0, 0, 1, 1, 1, 0
#define EMP_DEC_TO_BIN_655  1, 0, 1, 0, 0, 0, 1, 1, 1, 1
#define EMP_DEC_TO_BIN_656  1, 0, 1, 0, 0, 1, 0, 0, 0, 0
#define EMP_DEC_TO_BIN_657  1, 0, 1, 0, 0, 1, 0, 0, 0, 1
#define EMP_DEC_TO_BIN_658  1, 0, 1, 0, 0, 1, 0, 0, 1, 0
#define EMP_DEC_TO_BIN_659  1, 0, 1, 0, 0, 1, 0, 0, 1, 1
#define EMP_DEC_TO_BIN_660  1, 0, 1, 0, 0, 1, 0, 1, 0, 0
#define EMP_DEC_TO_BIN_661  1, 0, 1, 0, 0, 1, 0, 1, 0, 1
#define EMP_DEC_TO_BIN_662  1, 0, 1, 0, 0, 1, 0, 1, 1, 0
#define EMP_DEC_TO_BIN_663  1, 0, 1, 0, 0, 1, 0, 1, 1, 1
#define EMP_DEC_TO_BIN_664  1, 0, 1, 0, 0, 1, 1, 0, 0, 0
#define EMP_DEC_TO_BIN_665  1, 0, 1, 0, 0, 1, 1, 0, 0, 1
#define EMP_DEC_TO_BIN_666  1, 0, 1, 0, 0, 1, 1, 0, 1, 0
#define EMP_DEC_TO_BIN_667  1, 0, 1, 0, 0, 1, 1, 0, 1, 1
#define EMP_DEC_TO_BIN_668  1, 0, 1, 0, 0, 1, 1, 1, 0, 0
#define EMP_DEC_TO_BIN_669  1, 0, 1, 0, 0, 1, 1, 1, 0, 1
#define EMP_DEC_TO_BIN_670  1, 0, 1, 0, 0, 1, 1, 1, 1, 0
#define EMP_DEC_TO_BIN_671  1, 0, 1, 0, 0, 1, 1, 1, 1, 1
#define EMP_DEC_TO_BIN_672  1, 0, 1, 0, 1, 0, 0, 0, 0, 0
#define EMP_DEC_TO_BIN_673  1, 0, 1, 0, 1, 0, 0, 0, 0, 1
#define EMP_DEC_TO_BIN_674  1, 0, 1, 0, 1, 0, 0, 0, 1, 0
#define EMP_DEC_TO_BIN_675  1, 0, 1, 0, 1, 0, 0, 0, 1, 1
#define EMP_DEC_TO_BIN_676  1, 0, 1, 0, 1, 0, 0, 1, 0, 0
#define EMP_DEC_TO_BIN_677  1, 0, 1, 0, 1, 0, 0, 1, 0, 1
#define EMP_DEC_TO_BIN_678  1, 0, 1, 0, 1, 0, 0, 1, 1, 0
#define EMP_DEC_TO_BIN_679  1, 0, 1, 0, 1, 0, 0, 1, 1, 1
#define EMP_DEC_TO_BIN_680  1, 0, 1, 0, 1, 0, 1, 0, 0, 0
#define EMP_DEC_TO_BIN_681  1, 0, 1, 0, 1, 0, 1, 0, 0, 1
#define EMP_DEC_TO_BIN_682  1, 0, 1, 0, 1, 0, 1, 0, 1, 0
#define EMP_DEC_TO_BIN_683  1, 0, 1, 0, 1, 0, 1, 0, 1, 1
#define EMP_DEC_TO_BIN_684  1, 0, 1, 0, 1, 0, 1, 1, 0, 0
#define EMP_DEC_TO_BIN_685  1, 0, 1, 0, 1, 0, 1, 1, 0, 1
#define EMP_DEC_TO_BIN_686  1, 0, 1, 0, 1, 0, 1, 1, 1, 0
#define EMP_DEC_TO_BIN_687  1, 0, 1, 0, 1, 0, 1, 1, 1, 1
#define EMP_DEC_TO_BIN_688  1, 0, 1, 0, 1, 1, 0, 0, 0, 0
#define EMP_DEC_TO_BIN_689  1, 0, 1, 0, 1, 1, 0, 0, 0, 1
#define EMP_DEC_TO_BIN_690  1, 0, 1, 0, 1, 1, 0, 0, 1, 0
#define EMP_DEC_TO_BIN_691  1, 0, 1, 0, 1, 1, 0, 0, 1, 1
#define EMP_DEC_TO_BIN_692  1, 0, 1, 0, 1, 1, 0, 1, 0, 0
#define EMP_DEC_TO_BIN_693  1, 0, 1, 0, 1, 1, 0, 1, 0, 1
#define EMP_DEC_TO_BIN_694  1, 0, 1, 0, 1, 1, 0, 1, 1, 0
#define EMP_DEC_TO_BIN_695  1, 0, 1, 0, 1, 1, 0, 1, 1, 1
#define EMP_DEC_TO_BIN_696  1, 0, 1, 0, 1, 1, 1, 0, 0, 0
#define EMP_DEC_TO_BIN_697  1, 0, 1, 0, 1, 1, 1, 0, 0, 1
#define EMP_DEC_TO_BIN_698  1, 0, 1, 0, 1, 1, 1, 0, 1, 0
#define EMP_DEC_TO_BIN_699  1, 0, 1, 0, 1, 1, 1, 0, 1, 1
#define EMP_DEC_TO_BIN_700  1, 0, 1, 0, 1, 1, 1, 1, 0, 0
#define EMP_DEC_TO_BIN_701  1, 0, 1, 0, 1, 1, 1, 1, 0, 1
#define EMP_DEC_TO_BIN_702  1, 0, 1, 0, 1, 1, 1, 1, 1, 0
#define EMP_DEC_TO_BIN_703  1, 0, 1, 0, 1, 1, 1, 1, 1, 1
#define EMP_DEC_TO_BIN_704  1, 0, 1, 1, 0, 0, 0, 0, 0, 0
#define EMP_DEC_TO_BIN_705  1, 0, 1, 1, 0, 0, 0, 0, 0, 1
#define EMP_DEC_TO_BIN_706  1, 0, 1, 1, 0, 0, 0, 0, 1, 0
#define EMP_DEC_TO_BIN_707  1, 0, 1, 1, 0, 0, 0, 0, 1, 1
#define EMP_DEC_TO_BIN_708  1, 0, 1, 1, 0, 0, 0, 1, 0, 0
#define EMP_DEC_TO_BIN_709  1, 0, 1, 1, 0, 0, 0, 1, 0, 1
#define EMP_DEC_TO_BIN_710  1, 0, 1, 1, 0, 0, 0, 1, 1, 0
#define EMP_DEC_TO_BIN_711  1, 0, 1, 1, 0, 0, 0, 1, 1, 1
#define EMP_DEC_TO_BIN_712  1, 0, 1, 1, 0, 0, 1, 0, 0, 0
#define EMP_DEC_TO_BIN_713  1, 0, 1, 1, 0, 0, 1, 0, 0, 1
#define EMP_DEC_TO_BIN_714  1, 0, 1, 1, 0, 0, 1, 0, 1, 0
#define EMP_DEC_TO_BIN_715  1, 0, 1, 1, 0, 0, 1, 0, 1, 1
#define EMP_DEC_TO_BIN_716  1, 0, 1, 1, 0, 0, 1, 1, 0, 0
#define EMP_DEC_TO_BIN_717  1, 0, 1, 1, 0, 0, 1, 1, 0, 1
#define EMP_DEC_TO_BIN_718  1, 0, 1, 1, 0, 0, 1, 1, 1, 0
#define EMP_DEC_TO_BIN_719  1, 0, 1, 1, 0, 0, 1, 1, 1, 1
#define EMP_DEC_TO_BIN_720  1, 0, 1, 1, 0, 1, 0, 0, 0, 0
#define EMP_DEC_TO_BIN_721  1, 0, 1, 1, 0, 1, 0, 0, 0, 1
#define EMP_DEC_TO_BIN_722  1, 0, 1, 1, 0, 1, 0, 0, 1, 0
#define EMP_DEC_TO_BIN_723  1, 0, 1, 1, 0, 1, 0, 0, 1, 1
#define EMP_DEC_TO_BIN_724  1, 0, 1, 1, 0, 1, 0, 1, 0, 0
#define EMP_DEC_TO_BIN_725  1, 0, 1, 1, 0, 1, 0, 1, 0, 1
#define EMP_DEC_TO_BIN_726  1, 0, 1, 1, 0, 1, 0, 1, 1, 0
#define EMP_DEC_TO_BIN_727  1, 0, 1, 1, 0, 1, 0, 1, 1, 1
#define EMP_DEC_TO_BIN_728  1, 0, 1, 1, 0, 1, 1, 0, 0, 0
#define EMP_DEC_TO_BIN_729  1, 0, 1, 1, 0, 1, 1, 0, 0, 1
#define EMP_DEC_TO_BIN_730  1, 0, 1, 1, 0, 1, 1, 0, 1, 0
#define EMP_DEC_TO_BIN_731  1, 0, 1, 1, 0, 1, 1, 0, 1, 1
#define EMP_DEC_TO_BIN_732  1, 0, 1, 1, 0, 1, 1, 1, 0, 0
#define EMP_DEC_TO_BIN_733  1, 0, 1, 1, 0, 1, 1, 1, 0, 1
#define EMP_DEC_TO_BIN_734  1, 0, 1, 1, 0, 1, 1, 1, 1, 0
#define EMP_DEC_TO_BIN_735  1, 0, 1, 1, 0, 1, 1, 1, 1, 1
#define EMP_DEC_TO_BIN_736  1, 0, 1, 1, 1, 0, 0, 0, 0, 0
#define EMP_DEC_TO_BIN_737  1, 0, 1, 1, 1, 0, 0, 0, 0, 1
#define EMP_DEC_TO_BIN_738  1, 0, 1, 1, 1, 0, 0, 0, 1, 0
#define EMP_DEC_TO_BIN_739  1, 0, 1, 1, 1, 0, 0, 0, 1, 1
#define EMP_DEC_TO_BIN_740  1, 0, 1, 1, 1, 0, 0, 1, 0, 0
#define EMP_DEC_TO_BIN_741  1, 0, 1, 1, 1, 0, 0, 1, 0, 1
#define EMP_DEC_TO_BIN_742  1, 0, 1, 1, 1, 0, 0, 1, 1, 0
#define EMP_DEC_TO_BIN_743  1, 0, 1, 1, 1, 0, 0, 1, 1, 1
#define EMP_DEC_TO_BIN_744  1, 0, 1, 1, 1, 0, 1, 0, 0, 0
#define EMP_DEC_TO_BIN_745  1, 0, 1, 1, 1, 0, 1, 0, 0, 1
#define EMP_DEC_TO_BIN_746  1, 0, 1, 1, 1, 0, 1, 0, 1, 0
#define EMP_DEC_TO_BIN_747  1, 0, 1, 1, 1, 0, 1, 0, 1, 1
#define EMP_DEC_TO_BIN_748  1, 0, 1, 1, 1, 0, 1, 1, 0, 0
#define EMP_DEC_TO_BIN_749  1, 0, 1, 1, 1, 0, 1, 1, 0, 1
#define EMP_DEC_TO_BIN_750  1, 0, 1, 1, 1, 0, 1, 1, 1, 0
#define EMP_DEC_TO_BIN_751  1, 0, 1, 1, 1, 0, 1, 1, 1, 1
#define EMP_DEC_TO_BIN_752  1, 0, 1, 1, 1, 1, 0, 0, 0, 0
#define EMP_DEC_TO_BIN_753  1, 0, 1, 1, 1, 1, 0, 0, 0, 1
#define EMP_DEC_TO_BIN_754  1, 0, 1, 1, 1, 1, 0, 0, 1, 0
#define EMP_DEC_TO_BIN_755  1, 0, 1, 1, 1, 1, 0, 0, 1, 1
#define EMP_DEC_TO_BIN_756  1, 0, 1, 1, 1, 1, 0, 1, 0, 0
#define EMP_DEC_TO_BIN_757  1, 0, 1, 1, 1, 1, 0, 1, 0, 1
#define EMP_DEC_TO_BIN_758  1, 0, 1, 1, 1, 1, 0, 1, 1, 0
#define EMP_DEC_TO_BIN_759  1, 0, 1, 1, 1, 1, 0, 1, 1, 1
#define EMP_DEC_TO_BIN_760  1, 0, 1, 1, 1, 1, 1, 0, 0, 0
#define EMP_DEC_TO_BIN_761  1, 0, 1, 1, 1, 1, 1, 0, 0, 1
#define EMP_DEC_TO_BIN_762  1, 0, 1, 1, 1, 1, 1, 0, 1, 0
#define EMP_DEC_TO_BIN_763  1, 0, 1, 1, 1, 1, 1, 0, 1, 1
#define EMP_DEC_TO_BIN_764  1, 0, 1, 1, 1, 1, 1, 1, 0, 0
#define EMP_DEC_TO_BIN_765  1, 0, 1, 1, 1, 1, 1, 1, 0, 1
#define EMP_DEC_TO_BIN_766  1, 0, 1, 1, 1, 1, 1, 1, 1, 0
#define EMP_DEC_TO_BIN_767  1, 0, 1, 1, 1, 1, 1, 1, 1, 1
#define EMP_DEC_TO_BIN_768  1, 1, 0, 0, 0, 0, 0, 0, 0, 0
#define EMP_DEC_TO_BIN_769  1, 1, 0, 0, 0, 0, 0, 0, 0, 1
#define EMP_DEC_TO_BIN_770  1, 1, 0, 0, 0, 0, 0, 0, 1, 0
#define EMP_DEC_TO_BIN_771  1, 1, 0, 0, 0, 0, 0, 0, 1, 1
#define EMP_DEC_TO_BIN_772  1, 1, 0, 0, 0, 0, 0, 1, 0, 0
#define EMP_DEC_TO_BIN_773  1, 1, 0, 0, 0, 0, 0, 1, 0, 1
#define EMP_DEC_TO_BIN_774  1, 1, 0, 0, 0, 0, 0, 1, 1, 0
#define EMP_DEC_TO_BIN_775  1, 1, 0, 0, 0, 0, 0, 1, 1, 1
#define EMP_DEC_TO_BIN_776  1, 1, 0, 0, 0, 0, 1, 0, 0, 0
#define EMP_DEC_TO_BIN_777  1, 1, 0, 0, 0, 0, 1, 0, 0, 1
#define EMP_DEC_TO_BIN_778  1, 1, 0, 0, 0, 0, 1, 0, 1, 0
#define EMP_DEC_TO_BIN_779  1, 1, 0, 0, 0, 0, 1, 0, 1, 1
#define EMP_DEC_TO_BIN_780  1, 1, 0, 0, 0, 0, 1, 1, 0, 0
#define EMP_DEC_TO_BIN_781  1, 1, 0, 0, 0, 0, 1, 1, 0, 1
#define EMP_DEC_TO_BIN_782  1, 1, 0, 0, 0, 0, 1, 1, 1, 0
#define EMP_DEC_TO_BIN_783  1, 1, 0, 0, 0, 0, 1, 1, 1, 1
#define EMP_DEC_TO_BIN_784  1, 1, 0, 0, 0, 1, 0, 0, 0, 0
#define EMP_DEC_TO_BIN_785  1, 1, 0, 0, 0, 1, 0, 0, 0, 1
#define EMP_DEC_TO_BIN_786  1, 1, 0, 0, 0, 1, 0, 0, 1, 0
#define EMP_DEC_TO_BIN_787  1, 1, 0, 0, 0, 1, 0, 0, 1, 1
#define EMP_DEC_TO_BIN_788  1, 1, 0, 0, 0, 1, 0, 1, 0, 0
#define EMP_DEC_TO_BIN_789  1, 1, 0, 0, 0, 1, 0, 1, 0, 1
#define EMP_DEC_TO_BIN_790  1, 1, 0, 0, 0, 1, 0, 1, 1, 0
#define EMP_DEC_TO_BIN_791  1, 1, 0, 0, 0, 1, 0, 1, 1, 1
#define EMP_DEC_TO_BIN_792  1, 1, 0, 0, 0, 1, 1, 0, 0, 0
#define EMP_DEC_TO_BIN_793  1, 1, 0, 0, 0, 1, 1, 0, 0, 1
#define EMP_DEC_TO_BIN_794  1, 1, 0, 0, 0, 1, 1, 0, 1, 0
#define EMP_DEC_TO_BIN_795  1, 1, 0, 0, 0, 1, 1, 0, 1, 1
#define EMP_DEC_TO_BIN_796  1, 1, 0, 0, 0, 1, 1, 1, 0, 0
#define EMP_DEC_TO_BIN_797  1, 1, 0, 0, 0, 1, 1, 1, 0, 1
#define EMP_DEC_TO_BIN_798  1, 1, 0, 0, 0, 1, 1, 1, 1, 0
#define EMP_DEC_TO_BIN_799  1, 1, 0, 0, 0, 1, 1, 1, 1, 1
#define EMP_DEC_TO_BIN_800  1, 1, 0, 0, 1, 0, 0, 0, 0, 0
#define EMP_DEC_TO_BIN_801  1, 1, 0, 0, 1, 0, 0, 0, 0, 1
#define EMP_DEC_TO_BIN_802  1, 1, 0, 0, 1, 0, 0, 0, 1, 0
#define EMP_DEC_TO_BIN_803  1, 1, 0, 0, 1, 0, 0, 0, 1, 1
#define EMP_DEC_TO_BIN_804  1, 1, 0, 0, 1, 0, 0, 1, 0, 0
#define EMP_DEC_TO_BIN_805  1, 1, 0, 0, 1, 0, 0, 1, 0, 1
#define EMP_DEC_TO_BIN_806  1, 1, 0, 0, 1, 0, 0, 1, 1, 0
#define EMP_DEC_TO_BIN_807  1, 1, 0, 0, 1, 0, 0, 1, 1, 1
#define EMP_DEC_TO_BIN_808  1, 1, 0, 0, 1, 0, 1, 0, 0, 0
#define EMP_DEC_TO_BIN_809  1, 1, 0, 0, 1, 0, 1, 0, 0, 1
#define EMP_DEC_TO_BIN_810  1, 1, 0, 0, 1, 0, 1, 0, 1, 0
#define EMP_DEC_TO_BIN_811  1, 1, 0, 0, 1, 0, 1, 0, 1, 1
#define EMP_DEC_TO_BIN_812  1, 1, 0, 0, 1, 0, 1, 1, 0, 0
#define EMP_DEC_TO_BIN_813  1, 1, 0, 0, 1, 0, 1, 1, 0, 1
#define EMP_DEC_TO_BIN_814  1, 1, 0, 0, 1, 0, 1, 1, 1, 0
#define EMP_DEC_TO_BIN_815  1, 1, 0, 0, 1, 0, 1, 1, 1, 1
#define EMP_DEC_TO_BIN_816  1, 1, 0, 0, 1, 1, 0, 0, 0, 0
#define EMP_DEC_TO_BIN_817  1, 1, 0, 0, 1, 1, 0, 0, 0, 1
#define EMP_DEC_TO_BIN_818  1, 1, 0, 0, 1, 1, 0, 0, 1, 0
#define EMP_DEC_TO_BIN_819  1, 1, 0, 0, 1, 1, 0, 0, 1, 1
#define EMP_DEC_TO_BIN_820  1, 1, 0, 0, 1, 1, 0, 1, 0, 0
#define EMP_DEC_TO_BIN_821  1, 1, 0, 0, 1, 1, 0, 1, 0, 1
#define EMP_DEC_TO_BIN_822  1, 1, 0, 0, 1, 1, 0, 1, 1, 0
#define EMP_DEC_TO_BIN_823  1, 1, 0, 0, 1, 1, 0, 1, 1, 1
#define EMP_DEC_TO_BIN_824  1, 1, 0, 0, 1, 1, 1, 0, 0, 0
#define EMP_DEC_TO_BIN_825  1, 1, 0, 0, 1, 1, 1, 0, 0, 1
#define EMP_DEC_TO_BIN_826  1, 1, 0, 0, 1, 1, 1, 0, 1, 0
#define EMP_DEC_TO_BIN_827  1, 1, 0, 0, 1, 1, 1, 0, 1, 1
#define EMP_DEC_TO_BIN_828  1, 1, 0, 0, 1, 1, 1, 1, 0, 0
#define EMP_DEC_TO_BIN_829  1, 1, 0, 0, 1, 1, 1, 1, 0, 1
#define EMP_DEC_TO_BIN_830  1, 1, 0, 0, 1, 1, 1, 1, 1, 0
#define EMP_DEC_TO_BIN_831  1, 1, 0, 0, 1, 1, 1, 1, 1, 1
#define EMP_DEC_TO_BIN_832  1, 1, 0, 1, 0, 0, 0, 0, 0, 0
#define EMP_DEC_TO_BIN_833  1, 1, 0, 1, 0, 0, 0, 0, 0, 1
#define EMP_DEC_TO_BIN_834  1, 1, 0, 1, 0, 0, 0, 0, 1, 0
#define EMP_DEC_TO_BIN_835  1, 1, 0, 1, 0, 0, 0, 0, 1, 1
#define EMP_DEC_TO_BIN_836  1, 1, 0, 1, 0, 0, 0, 1, 0, 0
#define EMP_DEC_TO_BIN_837  1, 1, 0, 1, 0, 0, 0, 1, 0, 1
#define EMP_DEC_TO_BIN_838  1, 1, 0, 1, 0, 0, 0, 1, 1, 0
#define EMP_DEC_TO_BIN_839  1, 1, 0, 1, 0, 0, 0, 1, 1, 1
#define EMP_DEC_TO_BIN_840  1, 1, 0, 1, 0, 0, 1, 0, 0, 0
#define EMP_DEC_TO_BIN_841  1, 1, 0, 1, 0, 0, 1, 0, 0, 1
#define EMP_DEC_TO_BIN_842  1, 1, 0, 1, 0, 0, 1, 0, 1, 0
#define EMP_DEC_TO_BIN_843  1, 1, 0, 1, 0, 0, 1, 0, 1, 1
#define EMP_DEC_TO_BIN_844  1, 1, 0, 1, 0, 0, 1, 1, 0, 0
#define EMP_DEC_TO_BIN_845  1, 1, 0, 1, 0, 0, 1, 1, 0, 1
#define EMP_DEC_TO_BIN_846  1, 1, 0, 1, 0, 0, 1, 1, 1, 0
#define EMP_DEC_TO_BIN_847  1, 1, 0, 1, 0, 0, 1, 1, 1, 1
#define EMP_DEC_TO_BIN_848  1, 1, 0, 1, 0, 1, 0, 0, 0, 0
#define EMP_DEC_TO_BIN_849  1, 1, 0, 1, 0, 1, 0, 0, 0, 1
#define EMP_DEC_TO_BIN_850  1, 1, 0, 1, 0, 1, 0, 0, 1, 0
#define EMP_DEC_TO_BIN_851  1, 1, 0, 1, 0, 1, 0, 0, 1, 1
#define EMP_DEC_TO_BIN_852  1, 1, 0, 1, 0, 1, 0, 1, 0, 0
#define EMP_DEC_TO_BIN_853  1, 1, 0, 1, 0, 1, 0, 1, 0, 1
#define EMP_DEC_TO_BIN_854  1, 1, 0, 1, 0, 1, 0, 1, 1, 0
#define EMP_DEC_TO_BIN_855  1, 1, 0, 1, 0, 1, 0, 1, 1, 1
#define EMP_DEC_TO_BIN_856  1, 1, 0, 1, 0, 1, 1, 0, 0, 0
#define EMP_DEC_TO_BIN_857  1, 1, 0, 1, 0, 1, 1, 0, 0, 1
#define EMP_DEC_TO_BIN_858  1, 1, 0, 1, 0, 1, 1, 0, 1, 0
#define EMP_DEC_TO_BIN_859  1, 1, 0, 1, 0, 1, 1, 0, 1, 1
#define EMP_DEC_TO_BIN_860  1, 1, 0, 1, 0, 1, 1, 1, 0, 0
#define EMP_DEC_TO_BIN_861  1, 1, 0, 1, 0, 1, 1, 1, 0, 1
#define EMP_DEC_TO_BIN_862  1, 1, 0, 1, 0, 1, 1, 1, 1, 0
#define EMP_DEC_TO_BIN_863  1, 1, 0, 1, 0, 1, 1, 1, 1, 1
#define EMP_DEC_TO_BIN_864  1, 1, 0, 1, 1, 0, 0, 0, 0, 0
#define EMP_DEC_TO_BIN_865  1, 1, 0, 1, 1, 0, 0, 0, 0, 1
#define EMP_DEC_TO_BIN_866  1, 1, 0, 1, 1, 0, 0, 0, 1, 0
#define EMP_DEC_TO_BIN_867  1, 1, 0, 1, 1, 0, 0, 0, 1, 1
#define EMP_DEC_TO_BIN_868  1, 1, 0, 1, 1, 0, 0, 1, 0, 0
#define EMP_DEC_TO_BIN_869  1, 1, 0, 1, 1, 0, 0, 1, 0, 1
#define EMP_DEC_TO_BIN_870  1, 1, 0, 1, 1, 0, 0, 1, 1, 0
#define EMP_DEC_TO_BIN_871  1, 1, 0, 1, 1, 0, 0, 1, 1, 1
#define EMP_DEC_TO_BIN_872  1, 1, 0, 1, 1, 0, 1, 0, 0, 0
#define EMP_DEC_TO_BIN_873  1, 1, 0, 1, 1, 0, 1, 0, 0, 1
#define EMP_DEC_TO_BIN_874  1, 1, 0, 1, 1, 0, 1, 0, 1, 0
#define EMP_DEC_TO_BIN_875  1, 1, 0, 1, 1, 0, 1, 0, 1, 1
#define EMP_DEC_TO_BIN_876  1, 1, 0, 1, 1, 0, 1, 1, 0, 0
#define EMP_DEC_TO_BIN_877  1, 1, 0, 1, 1, 0, 1, 1, 0, 1
#define EMP_DEC_TO_BIN_878  1, 1, 0, 1, 1, 0, 1, 1, 1, 0
#define EMP_DEC_TO_BIN_879  1, 1, 0, 1, 1, 0, 1, 1, 1, 1
#define EMP_DEC_TO_BIN_880  1, 1, 0, 1, 1, 1, 0, 0, 0, 0
#define EMP_DEC_TO_BIN_881  1, 1, 0, 1, 1, 1, 0, 0, 0, 1
#define EMP_DEC_TO_BIN_882  1, 1, 0, 1, 1, 1, 0, 0, 1, 0
#define EMP_DEC_TO_BIN_883  1, 1, 0, 1, 1, 1, 0, 0, 1, 1
#define EMP_DEC_TO_BIN_884  1, 1, 0, 1, 1, 1, 0, 1, 0, 0
#define EMP_DEC_TO_BIN_885  1, 1, 0, 1, 1, 1, 0, 1, 0, 1
#define EMP_DEC_TO_BIN_886  1, 1, 0, 1, 1, 1, 0, 1, 1, 0
#define EMP_DEC_TO_BIN_887  1, 1, 0, 1, 1, 1, 0, 1, 1, 1
#define EMP_DEC_TO_BIN_888  1, 1, 0, 1, 1, 1, 1, 0, 0, 0
#define EMP_DEC_TO_BIN_889  1, 1, 0, 1, 1, 1, 1, 0, 0, 1
#define EMP_DEC_TO_BIN_890  1, 1, 0, 1, 1, 1, 1, 0, 1, 0
#define EMP_DEC_TO_BIN_891  1, 1, 0, 1, 1, 1, 1, 0, 1, 1
#define EMP_DEC_TO_BIN_892  1, 1, 0, 1, 1, 1, 1, 1, 0, 0
#define EMP_DEC_TO_BIN_893  1, 1, 0, 1, 1, 1, 1, 1, 0, 1
#define EMP_DEC_TO_BIN_894  1, 1, 0, 1, 1, 1, 1, 1, 1, 0
#define EMP_DEC_TO_BIN_895  1, 1, 0, 1, 1, 1, 1, 1, 1, 1
#define EMP_DEC_TO_BIN_896  1, 1, 1, 0, 0, 0, 0, 0, 0, 0
#define EMP_DEC_TO_BIN_897  1, 1, 1, 0, 0, 0, 0, 0, 0, 1
#define EMP_DEC_TO_BIN_898  1, 1, 1, 0, 0, 0, 0, 0, 1, 0
#define EMP_DEC_TO_BIN_899  1, 1, 1, 0, 0, 0, 0, 0, 1, 1
#define EMP_DEC_TO_BIN_900  1, 1, 1, 0, 0, 0, 0, 1, 0, 0
#define EMP_DEC_TO_BIN_901  1, 1, 1, 0, 0, 0, 0, 1, 0, 1
#define EMP_DEC_TO_BIN_902  1, 1, 1, 0, 0, 0, 0, 1, 1, 0
#define EMP_DEC_TO_BIN_903  1, 1, 1, 0, 0, 0, 0, 1, 1, 1
#define EMP_DEC_TO_BIN_904  1, 1, 1, 0, 0, 0, 1, 0, 0, 0
#define EMP_DEC_TO_BIN_905  1, 1, 1, 0, 0, 0, 1, 0, 0, 1
#define EMP_DEC_TO_BIN_906  1, 1, 1, 0, 0, 0, 1, 0, 1, 0
#define EMP_DEC_TO_BIN_907  1, 1, 1, 0, 0, 0, 1, 0, 1, 1
#define EMP_DEC_TO_BIN_908  1, 1, 1, 0, 0, 0, 1, 1, 0, 0
#define EMP_DEC_TO_BIN_909  1, 1, 1, 0, 0, 0, 1, 1, 0, 1
#define EMP_DEC_TO_BIN_910  1, 1, 1, 0, 0, 0, 1, 1, 1, 0
#define EMP_DEC_TO_BIN_911  1, 1, 1, 0, 0, 0, 1, 1, 1, 1
#define EMP_DEC_TO_BIN_912  1, 1, 1, 0, 0, 1, 0, 0, 0, 0
#define EMP_DEC_TO_BIN_913  1, 1, 1, 0, 0, 1, 0, 0, 0, 1
#define EMP_DEC_TO_BIN_914  1, 1, 1, 0, 0, 1, 0, 0, 1, 0
#define EMP_DEC_TO_BIN_915  1, 1, 1, 0, 0, 1, 0, 0, 1, 1
#define EMP_DEC_TO_BIN_916  1, 1, 1, 0, 0, 1, 0, 1, 0, 0
#define EMP_DEC_TO_BIN_917  1, 1, 1, 0, 0, 1, 0, 1, 0, 1
#define EMP_DEC_TO_BIN_918  1, 1, 1, 0, 0, 1, 0, 1, 1, 0
#define EMP_DEC_TO_BIN_919  1, 1, 1, 0, 0, 1, 0, 1, 1, 1
#define EMP_DEC_TO_BIN_920  1, 1, 1, 0, 0, 1, 1, 0, 0, 0
#define EMP_DEC_TO_BIN_921  1, 1, 1, 0, 0, 1, 1, 0, 0, 1
#define EMP_DEC_TO_BIN_922  1, 1, 1, 0, 0, 1, 1, 0, 1, 0
#define EMP_DEC_TO_BIN_923  1, 1, 1, 0, 0, 1, 1, 0, 1, 1
#define EMP_DEC_TO_BIN_924  1, 1, 1, 0, 0, 1, 1, 1, 0, 0
#define EMP_DEC_TO_BIN_925  1, 1, 1, 0, 0, 1, 1, 1, 0, 1
#define EMP_DEC_TO_BIN_926  1, 1, 1, 0, 0, 1, 1, 1, 1, 0
#define EMP_DEC_TO_BIN_927  1, 1, 1, 0, 0, 1, 1, 1, 1, 1
#define EMP_DEC_TO_BIN_928  1, 1, 1, 0, 1, 0, 0, 0, 0, 0
#define EMP_DEC_TO_BIN_929  1, 1, 1, 0, 1, 0, 0, 0, 0, 1
#define EMP_DEC_TO_BIN_930  1, 1, 1, 0, 1, 0, 0, 0, 1, 0
#define EMP_DEC_TO_BIN_931  1, 1, 1, 0, 1, 0, 0, 0, 1, 1
#define EMP_DEC_TO_BIN_932  1, 1, 1, 0, 1, 0, 0, 1, 0, 0
#define EMP_DEC_TO_BIN_933  1, 1, 1, 0, 1, 0, 0, 1, 0, 1
#define EMP_DEC_TO_BIN_934  1, 1, 1, 0, 1, 0, 0, 1, 1, 0
#define EMP_DEC_TO_BIN_935  1, 1, 1, 0, 1, 0, 0, 1, 1, 1
#define EMP_DEC_TO_BIN_936  1, 1, 1, 0, 1, 0, 1, 0, 0, 0
#define EMP_DEC_TO_BIN_937  1, 1, 1, 0, 1, 0, 1, 0, 0, 1
#define EMP_DEC_TO_BIN_938  1, 1, 1, 0, 1, 0, 1, 0, 1, 0
#define EMP_DEC_TO_BIN_939  1, 1, 1, 0, 1, 0, 1, 0, 1, 1
#define EMP_DEC_TO_BIN_940  1, 1, 1, 0, 1, 0, 1, 1, 0, 0
#define EMP_DEC_TO_BIN_941  1, 1, 1, 0, 1, 0, 1, 1, 0, 1
#define EMP_DEC_TO_BIN_942  1, 1, 1, 0, 1, 0, 1, 1, 1, 0
#define EMP_DEC_TO_BIN_943  1, 1, 1, 0, 1, 0, 1, 1, 1, 1
#define EMP_DEC_TO_BIN_944  1, 1, 1, 0, 1, 1, 0, 0, 0, 0
#define EMP_DEC_TO_BIN_945  1, 1, 1, 0, 1, 1, 0, 0, 0, 1
#define EMP_DEC_TO_BIN_946  1, 1, 1, 0, 1, 1, 0, 0, 1, 0
#define EMP_DEC_TO_BIN_947  1, 1, 1, 0, 1, 1, 0, 0, 1, 1
#define EMP_DEC_TO_BIN_948  1, 1, 1, 0, 1, 1, 0, 1, 0, 0
#define EMP_DEC_TO_BIN_949  1, 1, 1, 0, 1, 1, 0, 1, 0, 1
#define EMP_DEC_TO_BIN_950  1, 1, 1, 0, 1, 1, 0, 1, 1, 0
#define EMP_DEC_TO_BIN_951  1, 1, 1, 0, 1, 1, 0, 1, 1, 1
#define EMP_DEC_TO_BIN_952  1, 1, 1, 0, 1, 1, 1, 0, 0, 0
#define EMP_DEC_TO_BIN_953  1, 1, 1, 0, 1, 1, 1, 0, 0, 1
#define EMP_DEC_TO_BIN_954  1, 1, 1, 0, 1, 1, 1, 0, 1, 0
#define EMP_DEC_TO_BIN_955  1, 1, 1, 0, 1, 1, 1, 0, 1, 1
#define EMP_DEC_TO_BIN_956  1, 1, 1, 0, 1, 1, 1, 1, 0, 0
#define EMP_DEC_TO_BIN_957  1, 1, 1, 0, 1, 1, 1, 1, 0, 1
#define EMP_DEC_TO_BIN_958  1, 1, 1, 0, 1, 1, 1, 1, 1, 0
#define EMP_DEC_TO_BIN_959  1, 1, 1, 0, 1, 1, 1, 1, 1, 1
#define EMP_DEC_TO_BIN_960  1, 1, 1, 1, 0, 0, 0, 0, 0, 0
#define EMP_DEC_TO_BIN_961  1, 1, 1, 1, 0, 0, 0, 0, 0, 1
#define EMP_DEC_TO_BIN_962  1, 1, 1, 1, 0, 0, 0, 0, 1, 0
#define EMP_DEC_TO_BIN_963  1, 1, 1, 1, 0, 0, 0, 0, 1, 1
#define EMP_DEC_TO_BIN_964  1, 1, 1, 1, 0, 0, 0, 1, 0, 0
#define EMP_DEC_TO_BIN_965  1, 1, 1, 1, 0, 0, 0, 1, 0, 1
#define EMP_DEC_TO_BIN_966  1, 1, 1, 1, 0, 0, 0, 1, 1, 0
#define EMP_DEC_TO_BIN_967  1, 1, 1, 1, 0, 0, 0, 1, 1, 1
#define EMP_DEC_TO_BIN_968  1, 1, 1, 1, 0, 0, 1, 0, 0, 0
#define EMP_DEC_TO_BIN_969  1, 1, 1, 1, 0, 0, 1, 0, 0, 1
#define EMP_DEC_TO_BIN_970  1, 1, 1, 1, 0, 0, 1, 0, 1, 0
#define EMP_DEC_TO_BIN_971  1, 1, 1, 1, 0, 0, 1, 0, 1, 1
#define EMP_DEC_TO_BIN_972  1, 1, 1, 1, 0, 0, 1, 1, 0, 0
#define EMP_DEC_TO_BIN_973  1, 1, 1, 1, 0, 0, 1, 1, 0, 1
#define EMP_DEC_TO_BIN_974  1, 1, 1, 1, 0, 0, 1, 1, 1, 0
#define EMP_DEC_TO_BIN_975  1, 1, 1, 1, 0, 0, 1, 1, 1, 1
#define EMP_DEC_TO_BIN_976  1, 1, 1, 1, 0, 1, 0, 0, 0, 0
#define EMP_DEC_TO_BIN_977  1, 1, 1, 1, 0, 1, 0, 0, 0, 1
#define EMP_DEC_TO_BIN_978  1, 1, 1, 1, 0, 1, 0, 0, 1, 0
#define EMP_DEC_TO_BIN_979  1, 1, 1, 1, 0, 1, 0, 0, 1, 1
#define EMP_DEC_TO_BIN_980  1, 1, 1, 1, 0, 1, 0, 1, 0, 0
#define EMP_DEC_TO_BIN_981  1, 1, 1, 1, 0, 1, 0, 1, 0, 1
#define EMP_DEC_TO_BIN_982  1, 1, 1, 1, 0, 1, 0, 1, 1, 0
#define EMP_DEC_TO_BIN_983  1, 1, 1, 1, 0, 1, 0, 1, 1, 1
#define EMP_DEC_TO_BIN_984  1, 1, 1, 1, 0, 1, 1, 0, 0, 0
#define EMP_DEC_TO_BIN_985  1, 1, 1, 1, 0, 1, 1, 0, 0, 1
#define EMP_DEC_TO_BIN_986  1, 1, 1, 1, 0, 1, 1, 0, 1, 0
#define EMP_DEC_TO_BIN_987  1, 1, 1, 1, 0, 1, 1, 0, 1, 1
#define EMP_DEC_TO_BIN_988  1, 1, 1, 1, 0, 1, 1, 1, 0, 0
#define EMP_DEC_TO_BIN_989  1, 1, 1, 1, 0, 1, 1, 1, 0, 1
#define EMP_DEC_TO_BIN_990  1, 1, 1, 1, 0, 1, 1, 1, 1, 0
#define EMP_DEC_TO_BIN_991  1, 1, 1, 1, 0, 1, 1, 1, 1, 1
#define EMP_DEC_TO_BIN_992  1, 1, 1, 1, 1, 0, 0, 0, 0, 0
#define EMP_DEC_TO_BIN_993  1, 1, 1, 1, 1, 0, 0, 0, 0, 1
#define EMP_DEC_TO_BIN_994  1, 1, 1, 1, 1, 0, 0, 0, 1, 0
#define EMP_DEC_TO_BIN_995  1, 1, 1, 1, 1, 0, 0, 0, 1, 1
#define EMP_DEC_TO_BIN_996  1, 1, 1, 1, 1, 0, 0, 1, 0, 0
#define EMP_DEC_TO_BIN_997  1, 1, 1, 1, 1, 0, 0, 1, 0, 1
#define EMP_DEC_TO_BIN_998  1, 1, 1, 1, 1, 0, 0, 1, 1, 0
#define EMP_DEC_TO_BIN_999  1, 1, 1, 1, 1, 0, 0, 1, 1, 1
#define EMP_DEC_TO_BIN_1000  1, 1, 1, 1, 1, 0, 1, 0, 0, 0
#define EMP_DEC_TO_BIN_1001  1, 1, 1, 1, 1, 0, 1, 0, 0, 1
#define EMP_DEC_TO_BIN_1002  1, 1, 1, 1, 1, 0, 1, 0, 1, 0
#define EMP_DEC_TO_BIN_1003  1, 1, 1, 1, 1, 0, 1, 0, 1, 1
#define EMP_DEC_TO_BIN_1004  1, 1, 1, 1, 1, 0, 1, 1, 0, 0
#define EMP_DEC_TO_BIN_1005  1, 1, 1, 1, 1, 0, 1, 1, 0, 1
#define EMP_DEC_TO_BIN_1006  1, 1, 1, 1, 1, 0, 1, 1, 1, 0
#define EMP_DEC_TO_BIN_1007  1, 1, 1, 1, 1, 0, 1, 1, 1, 1
#define EMP_DEC_TO_BIN_1008  1, 1, 1, 1, 1, 1, 0, 0, 0, 0
#define EMP_DEC_TO_BIN_1009  1, 1, 1, 1, 1, 1, 0, 0, 0, 1
#define EMP_DEC_TO_BIN_1010  1, 1, 1, 1, 1, 1, 0, 0, 1, 0
#define EMP_DEC_TO_BIN_1011  1, 1, 1, 1, 1, 1, 0, 0, 1, 1
#define EMP_DEC_TO_BIN_1012  1, 1, 1, 1, 1, 1, 0, 1, 0, 0
#define EMP_DEC_TO_BIN_1013  1, 1, 1, 1, 1, 1, 0, 1, 0, 1
#define EMP_DEC_TO_BIN_1014  1, 1, 1, 1, 1, 1, 0, 1, 1, 0
#define EMP_DEC_TO_BIN_1015  1, 1, 1, 1, 1, 1, 0, 1, 1, 1
#define EMP_DEC_TO_BIN_1016  1, 1, 1, 1, 1, 1, 1, 0, 0, 0
#define EMP_DEC_TO_BIN_1017  1, 1, 1, 1, 1, 1, 1, 0, 0, 1
#define EMP_DEC_TO_BIN_1018  1, 1, 1, 1, 1, 1, 1, 0, 1, 0
#define EMP_DEC_TO_BIN_1019  1, 1, 1, 1, 1, 1, 1, 0, 1, 1
#define EMP_DEC_TO_BIN_1020  1, 1, 1, 1, 1, 1, 1, 1, 0, 0
#define EMP_DEC_TO_BIN_1021  1, 1, 1, 1, 1, 1, 1, 1, 0, 1
#define EMP_DEC_TO_BIN_1022  1, 1, 1, 1, 1, 1, 1, 1, 1, 0
#define EMP_DEC_TO_BIN_1023  1, 1, 1, 1, 1, 1, 1, 1, 1, 1

/// @endcond

/// Convert binary values back to decimal (10 bits).

#define EMP_BIN_TO_DEC(...) EMP_BIN_TO_DEC_IMPL EMP_EMPTY() (__VA_ARGS__)

/// @cond MACROS

#define EMP_BIN_TO_DEC_IMPL(B0,B1,B2,B3,B4,B5,B6,B7,B8,B9)              \
  EMP_BIN_TO_DEC_ ## B0 ## B1 ## B2 ## B3 ## B4 ## B5 ## B6 ## B7 ## B8 ## B9

#define EMP_BIN_TO_DEC_0000000000  0
#define EMP_BIN_TO_DEC_0000000001  1
#define EMP_BIN_TO_DEC_0000000010  2
#define EMP_BIN_TO_DEC_0000000011  3
#define EMP_BIN_TO_DEC_0000000100  4
#define EMP_BIN_TO_DEC_0000000101  5
#define EMP_BIN_TO_DEC_0000000110  6
#define EMP_BIN_TO_DEC_0000000111  7
#define EMP_BIN_TO_DEC_0000001000  8
#define EMP_BIN_TO_DEC_0000001001  9
#define EMP_BIN_TO_DEC_0000001010  10
#define EMP_BIN_TO_DEC_0000001011  11
#define EMP_BIN_TO_DEC_0000001100  12
#define EMP_BIN_TO_DEC_0000001101  13
#define EMP_BIN_TO_DEC_0000001110  14
#define EMP_BIN_TO_DEC_0000001111  15
#define EMP_BIN_TO_DEC_0000010000  16
#define EMP_BIN_TO_DEC_0000010001  17
#define EMP_BIN_TO_DEC_0000010010  18
#define EMP_BIN_TO_DEC_0000010011  19
#define EMP_BIN_TO_DEC_0000010100  20
#define EMP_BIN_TO_DEC_0000010101  21
#define EMP_BIN_TO_DEC_0000010110  22
#define EMP_BIN_TO_DEC_0000010111  23
#define EMP_BIN_TO_DEC_0000011000  24
#define EMP_BIN_TO_DEC_0000011001  25
#define EMP_BIN_TO_DEC_0000011010  26
#define EMP_BIN_TO_DEC_0000011011  27
#define EMP_BIN_TO_DEC_0000011100  28
#define EMP_BIN_TO_DEC_0000011101  29
#define EMP_BIN_TO_DEC_0000011110  30
#define EMP_BIN_TO_DEC_0000011111  31
#define EMP_BIN_TO_DEC_0000100000  32
#define EMP_BIN_TO_DEC_0000100001  33
#define EMP_BIN_TO_DEC_0000100010  34
#define EMP_BIN_TO_DEC_0000100011  35
#define EMP_BIN_TO_DEC_0000100100  36
#define EMP_BIN_TO_DEC_0000100101  37
#define EMP_BIN_TO_DEC_0000100110  38
#define EMP_BIN_TO_DEC_0000100111  39
#define EMP_BIN_TO_DEC_0000101000  40
#define EMP_BIN_TO_DEC_0000101001  41
#define EMP_BIN_TO_DEC_0000101010  42
#define EMP_BIN_TO_DEC_0000101011  43
#define EMP_BIN_TO_DEC_0000101100  44
#define EMP_BIN_TO_DEC_0000101101  45
#define EMP_BIN_TO_DEC_0000101110  46
#define EMP_BIN_TO_DEC_0000101111  47
#define EMP_BIN_TO_DEC_0000110000  48
#define EMP_BIN_TO_DEC_0000110001  49
#define EMP_BIN_TO_DEC_0000110010  50
#define EMP_BIN_TO_DEC_0000110011  51
#define EMP_BIN_TO_DEC_0000110100  52
#define EMP_BIN_TO_DEC_0000110101  53
#define EMP_BIN_TO_DEC_0000110110  54
#define EMP_BIN_TO_DEC_0000110111  55
#define EMP_BIN_TO_DEC_0000111000  56
#define EMP_BIN_TO_DEC_0000111001  57
#define EMP_BIN_TO_DEC_0000111010  58
#define EMP_BIN_TO_DEC_0000111011  59
#define EMP_BIN_TO_DEC_0000111100  60
#define EMP_BIN_TO_DEC_0000111101  61
#define EMP_BIN_TO_DEC_0000111110  62
#define EMP_BIN_TO_DEC_0000111111  63
#define EMP_BIN_TO_DEC_0001000000  64
#define EMP_BIN_TO_DEC_0001000001  65
#define EMP_BIN_TO_DEC_0001000010  66
#define EMP_BIN_TO_DEC_0001000011  67
#define EMP_BIN_TO_DEC_0001000100  68
#define EMP_BIN_TO_DEC_0001000101  69
#define EMP_BIN_TO_DEC_0001000110  70
#define EMP_BIN_TO_DEC_0001000111  71
#define EMP_BIN_TO_DEC_0001001000  72
#define EMP_BIN_TO_DEC_0001001001  73
#define EMP_BIN_TO_DEC_0001001010  74
#define EMP_BIN_TO_DEC_0001001011  75
#define EMP_BIN_TO_DEC_0001001100  76
#define EMP_BIN_TO_DEC_0001001101  77
#define EMP_BIN_TO_DEC_0001001110  78
#define EMP_BIN_TO_DEC_0001001111  79
#define EMP_BIN_TO_DEC_0001010000  80
#define EMP_BIN_TO_DEC_0001010001  81
#define EMP_BIN_TO_DEC_0001010010  82
#define EMP_BIN_TO_DEC_0001010011  83
#define EMP_BIN_TO_DEC_0001010100  84
#define EMP_BIN_TO_DEC_0001010101  85
#define EMP_BIN_TO_DEC_0001010110  86
#define EMP_BIN_TO_DEC_0001010111  87
#define EMP_BIN_TO_DEC_0001011000  88
#define EMP_BIN_TO_DEC_0001011001  89
#define EMP_BIN_TO_DEC_0001011010  90
#define EMP_BIN_TO_DEC_0001011011  91
#define EMP_BIN_TO_DEC_0001011100  92
#define EMP_BIN_TO_DEC_0001011101  93
#define EMP_BIN_TO_DEC_0001011110  94
#define EMP_BIN_TO_DEC_0001011111  95
#define EMP_BIN_TO_DEC_0001100000  96
#define EMP_BIN_TO_DEC_0001100001  97
#define EMP_BIN_TO_DEC_0001100010  98
#define EMP_BIN_TO_DEC_0001100011  99
#define EMP_BIN_TO_DEC_0001100100  100
#define EMP_BIN_TO_DEC_0001100101  101
#define EMP_BIN_TO_DEC_0001100110  102
#define EMP_BIN_TO_DEC_0001100111  103
#define EMP_BIN_TO_DEC_0001101000  104
#define EMP_BIN_TO_DEC_0001101001  105
#define EMP_BIN_TO_DEC_0001101010  106
#define EMP_BIN_TO_DEC_0001101011  107
#define EMP_BIN_TO_DEC_0001101100  108
#define EMP_BIN_TO_DEC_0001101101  109
#define EMP_BIN_TO_DEC_0001101110  110
#define EMP_BIN_TO_DEC_0001101111  111
#define EMP_BIN_TO_DEC_0001110000  112
#define EMP_BIN_TO_DEC_0001110001  113
#define EMP_BIN_TO_DEC_0001110010  114
#define EMP_BIN_TO_DEC_0001110011  115
#define EMP_BIN_TO_DEC_0001110100  116
#define EMP_BIN_TO_DEC_0001110101  117
#define EMP_BIN_TO_DEC_0001110110  118
#define EMP_BIN_TO_DEC_0001110111  119
#define EMP_BIN_TO_DEC_0001111000  120
#define EMP_BIN_TO_DEC_0001111001  121
#define EMP_BIN_TO_DEC_0001111010  122
#define EMP_BIN_TO_DEC_0001111011  123
#define EMP_BIN_TO_DEC_0001111100  124
#define EMP_BIN_TO_DEC_0001111101  125
#define EMP_BIN_TO_DEC_0001111110  126
#define EMP_BIN_TO_DEC_0001111111  127
#define EMP_BIN_TO_DEC_0010000000  128
#define EMP_BIN_TO_DEC_0010000001  129
#define EMP_BIN_TO_DEC_0010000010  130
#define EMP_BIN_TO_DEC_0010000011  131
#define EMP_BIN_TO_DEC_0010000100  132
#define EMP_BIN_TO_DEC_0010000101  133
#define EMP_BIN_TO_DEC_0010000110  134
#define EMP_BIN_TO_DEC_0010000111  135
#define EMP_BIN_TO_DEC_0010001000  136
#define EMP_BIN_TO_DEC_0010001001  137
#define EMP_BIN_TO_DEC_0010001010  138
#define EMP_BIN_TO_DEC_0010001011  139
#define EMP_BIN_TO_DEC_0010001100  140
#define EMP_BIN_TO_DEC_0010001101  141
#define EMP_BIN_TO_DEC_0010001110  142
#define EMP_BIN_TO_DEC_0010001111  143
#define EMP_BIN_TO_DEC_0010010000  144
#define EMP_BIN_TO_DEC_0010010001  145
#define EMP_BIN_TO_DEC_0010010010  146
#define EMP_BIN_TO_DEC_0010010011  147
#define EMP_BIN_TO_DEC_0010010100  148
#define EMP_BIN_TO_DEC_0010010101  149
#define EMP_BIN_TO_DEC_0010010110  150
#define EMP_BIN_TO_DEC_0010010111  151
#define EMP_BIN_TO_DEC_0010011000  152
#define EMP_BIN_TO_DEC_0010011001  153
#define EMP_BIN_TO_DEC_0010011010  154
#define EMP_BIN_TO_DEC_0010011011  155
#define EMP_BIN_TO_DEC_0010011100  156
#define EMP_BIN_TO_DEC_0010011101  157
#define EMP_BIN_TO_DEC_0010011110  158
#define EMP_BIN_TO_DEC_0010011111  159
#define EMP_BIN_TO_DEC_0010100000  160
#define EMP_BIN_TO_DEC_0010100001  161
#define EMP_BIN_TO_DEC_0010100010  162
#define EMP_BIN_TO_DEC_0010100011  163
#define EMP_BIN_TO_DEC_0010100100  164
#define EMP_BIN_TO_DEC_0010100101  165
#define EMP_BIN_TO_DEC_0010100110  166
#define EMP_BIN_TO_DEC_0010100111  167
#define EMP_BIN_TO_DEC_0010101000  168
#define EMP_BIN_TO_DEC_0010101001  169
#define EMP_BIN_TO_DEC_0010101010  170
#define EMP_BIN_TO_DEC_0010101011  171
#define EMP_BIN_TO_DEC_0010101100  172
#define EMP_BIN_TO_DEC_0010101101  173
#define EMP_BIN_TO_DEC_0010101110  174
#define EMP_BIN_TO_DEC_0010101111  175
#define EMP_BIN_TO_DEC_0010110000  176
#define EMP_BIN_TO_DEC_0010110001  177
#define EMP_BIN_TO_DEC_0010110010  178
#define EMP_BIN_TO_DEC_0010110011  179
#define EMP_BIN_TO_DEC_0010110100  180
#define EMP_BIN_TO_DEC_0010110101  181
#define EMP_BIN_TO_DEC_0010110110  182
#define EMP_BIN_TO_DEC_0010110111  183
#define EMP_BIN_TO_DEC_0010111000  184
#define EMP_BIN_TO_DEC_0010111001  185
#define EMP_BIN_TO_DEC_0010111010  186
#define EMP_BIN_TO_DEC_0010111011  187
#define EMP_BIN_TO_DEC_0010111100  188
#define EMP_BIN_TO_DEC_0010111101  189
#define EMP_BIN_TO_DEC_0010111110  190
#define EMP_BIN_TO_DEC_0010111111  191
#define EMP_BIN_TO_DEC_0011000000  192
#define EMP_BIN_TO_DEC_0011000001  193
#define EMP_BIN_TO_DEC_0011000010  194
#define EMP_BIN_TO_DEC_0011000011  195
#define EMP_BIN_TO_DEC_0011000100  196
#define EMP_BIN_TO_DEC_0011000101  197
#define EMP_BIN_TO_DEC_0011000110  198
#define EMP_BIN_TO_DEC_0011000111  199
#define EMP_BIN_TO_DEC_0011001000  200
#define EMP_BIN_TO_DEC_0011001001  201
#define EMP_BIN_TO_DEC_0011001010  202
#define EMP_BIN_TO_DEC_0011001011  203
#define EMP_BIN_TO_DEC_0011001100  204
#define EMP_BIN_TO_DEC_0011001101  205
#define EMP_BIN_TO_DEC_0011001110  206
#define EMP_BIN_TO_DEC_0011001111  207
#define EMP_BIN_TO_DEC_0011010000  208
#define EMP_BIN_TO_DEC_0011010001  209
#define EMP_BIN_TO_DEC_0011010010  210
#define EMP_BIN_TO_DEC_0011010011  211
#define EMP_BIN_TO_DEC_0011010100  212
#define EMP_BIN_TO_DEC_0011010101  213
#define EMP_BIN_TO_DEC_0011010110  214
#define EMP_BIN_TO_DEC_0011010111  215
#define EMP_BIN_TO_DEC_0011011000  216
#define EMP_BIN_TO_DEC_0011011001  217
#define EMP_BIN_TO_DEC_0011011010  218
#define EMP_BIN_TO_DEC_0011011011  219
#define EMP_BIN_TO_DEC_0011011100  220
#define EMP_BIN_TO_DEC_0011011101  221
#define EMP_BIN_TO_DEC_0011011110  222
#define EMP_BIN_TO_DEC_0011011111  223
#define EMP_BIN_TO_DEC_0011100000  224
#define EMP_BIN_TO_DEC_0011100001  225
#define EMP_BIN_TO_DEC_0011100010  226
#define EMP_BIN_TO_DEC_0011100011  227
#define EMP_BIN_TO_DEC_0011100100  228
#define EMP_BIN_TO_DEC_0011100101  229
#define EMP_BIN_TO_DEC_0011100110  230
#define EMP_BIN_TO_DEC_0011100111  231
#define EMP_BIN_TO_DEC_0011101000  232
#define EMP_BIN_TO_DEC_0011101001  233
#define EMP_BIN_TO_DEC_0011101010  234
#define EMP_BIN_TO_DEC_0011101011  235
#define EMP_BIN_TO_DEC_0011101100  236
#define EMP_BIN_TO_DEC_0011101101  237
#define EMP_BIN_TO_DEC_0011101110  238
#define EMP_BIN_TO_DEC_0011101111  239
#define EMP_BIN_TO_DEC_0011110000  240
#define EMP_BIN_TO_DEC_0011110001  241
#define EMP_BIN_TO_DEC_0011110010  242
#define EMP_BIN_TO_DEC_0011110011  243
#define EMP_BIN_TO_DEC_0011110100  244
#define EMP_BIN_TO_DEC_0011110101  245
#define EMP_BIN_TO_DEC_0011110110  246
#define EMP_BIN_TO_DEC_0011110111  247
#define EMP_BIN_TO_DEC_0011111000  248
#define EMP_BIN_TO_DEC_0011111001  249
#define EMP_BIN_TO_DEC_0011111010  250
#define EMP_BIN_TO_DEC_0011111011  251
#define EMP_BIN_TO_DEC_0011111100  252
#define EMP_BIN_TO_DEC_0011111101  253
#define EMP_BIN_TO_DEC_0011111110  254
#define EMP_BIN_TO_DEC_0011111111  255
#define EMP_BIN_TO_DEC_0100000000  256
#define EMP_BIN_TO_DEC_0100000001  257
#define EMP_BIN_TO_DEC_0100000010  258
#define EMP_BIN_TO_DEC_0100000011  259
#define EMP_BIN_TO_DEC_0100000100  260
#define EMP_BIN_TO_DEC_0100000101  261
#define EMP_BIN_TO_DEC_0100000110  262
#define EMP_BIN_TO_DEC_0100000111  263
#define EMP_BIN_TO_DEC_0100001000  264
#define EMP_BIN_TO_DEC_0100001001  265
#define EMP_BIN_TO_DEC_0100001010  266
#define EMP_BIN_TO_DEC_0100001011  267
#define EMP_BIN_TO_DEC_0100001100  268
#define EMP_BIN_TO_DEC_0100001101  269
#define EMP_BIN_TO_DEC_0100001110  270
#define EMP_BIN_TO_DEC_0100001111  271
#define EMP_BIN_TO_DEC_0100010000  272
#define EMP_BIN_TO_DEC_0100010001  273
#define EMP_BIN_TO_DEC_0100010010  274
#define EMP_BIN_TO_DEC_0100010011  275
#define EMP_BIN_TO_DEC_0100010100  276
#define EMP_BIN_TO_DEC_0100010101  277
#define EMP_BIN_TO_DEC_0100010110  278
#define EMP_BIN_TO_DEC_0100010111  279
#define EMP_BIN_TO_DEC_0100011000  280
#define EMP_BIN_TO_DEC_0100011001  281
#define EMP_BIN_TO_DEC_0100011010  282
#define EMP_BIN_TO_DEC_0100011011  283
#define EMP_BIN_TO_DEC_0100011100  284
#define EMP_BIN_TO_DEC_0100011101  285
#define EMP_BIN_TO_DEC_0100011110  286
#define EMP_BIN_TO_DEC_0100011111  287
#define EMP_BIN_TO_DEC_0100100000  288
#define EMP_BIN_TO_DEC_0100100001  289
#define EMP_BIN_TO_DEC_0100100010  290
#define EMP_BIN_TO_DEC_0100100011  291
#define EMP_BIN_TO_DEC_0100100100  292
#define EMP_BIN_TO_DEC_0100100101  293
#define EMP_BIN_TO_DEC_0100100110  294
#define EMP_BIN_TO_DEC_0100100111  295
#define EMP_BIN_TO_DEC_0100101000  296
#define EMP_BIN_TO_DEC_0100101001  297
#define EMP_BIN_TO_DEC_0100101010  298
#define EMP_BIN_TO_DEC_0100101011  299
#define EMP_BIN_TO_DEC_0100101100  300
#define EMP_BIN_TO_DEC_0100101101  301
#define EMP_BIN_TO_DEC_0100101110  302
#define EMP_BIN_TO_DEC_0100101111  303
#define EMP_BIN_TO_DEC_0100110000  304
#define EMP_BIN_TO_DEC_0100110001  305
#define EMP_BIN_TO_DEC_0100110010  306
#define EMP_BIN_TO_DEC_0100110011  307
#define EMP_BIN_TO_DEC_0100110100  308
#define EMP_BIN_TO_DEC_0100110101  309
#define EMP_BIN_TO_DEC_0100110110  310
#define EMP_BIN_TO_DEC_0100110111  311
#define EMP_BIN_TO_DEC_0100111000  312
#define EMP_BIN_TO_DEC_0100111001  313
#define EMP_BIN_TO_DEC_0100111010  314
#define EMP_BIN_TO_DEC_0100111011  315
#define EMP_BIN_TO_DEC_0100111100  316
#define EMP_BIN_TO_DEC_0100111101  317
#define EMP_BIN_TO_DEC_0100111110  318
#define EMP_BIN_TO_DEC_0100111111  319
#define EMP_BIN_TO_DEC_0101000000  320
#define EMP_BIN_TO_DEC_0101000001  321
#define EMP_BIN_TO_DEC_0101000010  322
#define EMP_BIN_TO_DEC_0101000011  323
#define EMP_BIN_TO_DEC_0101000100  324
#define EMP_BIN_TO_DEC_0101000101  325
#define EMP_BIN_TO_DEC_0101000110  326
#define EMP_BIN_TO_DEC_0101000111  327
#define EMP_BIN_TO_DEC_0101001000  328
#define EMP_BIN_TO_DEC_0101001001  329
#define EMP_BIN_TO_DEC_0101001010  330
#define EMP_BIN_TO_DEC_0101001011  331
#define EMP_BIN_TO_DEC_0101001100  332
#define EMP_BIN_TO_DEC_0101001101  333
#define EMP_BIN_TO_DEC_0101001110  334
#define EMP_BIN_TO_DEC_0101001111  335
#define EMP_BIN_TO_DEC_0101010000  336
#define EMP_BIN_TO_DEC_0101010001  337
#define EMP_BIN_TO_DEC_0101010010  338
#define EMP_BIN_TO_DEC_0101010011  339
#define EMP_BIN_TO_DEC_0101010100  340
#define EMP_BIN_TO_DEC_0101010101  341
#define EMP_BIN_TO_DEC_0101010110  342
#define EMP_BIN_TO_DEC_0101010111  343
#define EMP_BIN_TO_DEC_0101011000  344
#define EMP_BIN_TO_DEC_0101011001  345
#define EMP_BIN_TO_DEC_0101011010  346
#define EMP_BIN_TO_DEC_0101011011  347
#define EMP_BIN_TO_DEC_0101011100  348
#define EMP_BIN_TO_DEC_0101011101  349
#define EMP_BIN_TO_DEC_0101011110  350
#define EMP_BIN_TO_DEC_0101011111  351
#define EMP_BIN_TO_DEC_0101100000  352
#define EMP_BIN_TO_DEC_0101100001  353
#define EMP_BIN_TO_DEC_0101100010  354
#define EMP_BIN_TO_DEC_0101100011  355
#define EMP_BIN_TO_DEC_0101100100  356
#define EMP_BIN_TO_DEC_0101100101  357
#define EMP_BIN_TO_DEC_0101100110  358
#define EMP_BIN_TO_DEC_0101100111  359
#define EMP_BIN_TO_DEC_0101101000  360
#define EMP_BIN_TO_DEC_0101101001  361
#define EMP_BIN_TO_DEC_0101101010  362
#define EMP_BIN_TO_DEC_0101101011  363
#define EMP_BIN_TO_DEC_0101101100  364
#define EMP_BIN_TO_DEC_0101101101  365
#define EMP_BIN_TO_DEC_0101101110  366
#define EMP_BIN_TO_DEC_0101101111  367
#define EMP_BIN_TO_DEC_0101110000  368
#define EMP_BIN_TO_DEC_0101110001  369
#define EMP_BIN_TO_DEC_0101110010  370
#define EMP_BIN_TO_DEC_0101110011  371
#define EMP_BIN_TO_DEC_0101110100  372
#define EMP_BIN_TO_DEC_0101110101  373
#define EMP_BIN_TO_DEC_0101110110  374
#define EMP_BIN_TO_DEC_0101110111  375
#define EMP_BIN_TO_DEC_0101111000  376
#define EMP_BIN_TO_DEC_0101111001  377
#define EMP_BIN_TO_DEC_0101111010  378
#define EMP_BIN_TO_DEC_0101111011  379
#define EMP_BIN_TO_DEC_0101111100  380
#define EMP_BIN_TO_DEC_0101111101  381
#define EMP_BIN_TO_DEC_0101111110  382
#define EMP_BIN_TO_DEC_0101111111  383
#define EMP_BIN_TO_DEC_0110000000  384
#define EMP_BIN_TO_DEC_0110000001  385
#define EMP_BIN_TO_DEC_0110000010  386
#define EMP_BIN_TO_DEC_0110000011  387
#define EMP_BIN_TO_DEC_0110000100  388
#define EMP_BIN_TO_DEC_0110000101  389
#define EMP_BIN_TO_DEC_0110000110  390
#define EMP_BIN_TO_DEC_0110000111  391
#define EMP_BIN_TO_DEC_0110001000  392
#define EMP_BIN_TO_DEC_0110001001  393
#define EMP_BIN_TO_DEC_0110001010  394
#define EMP_BIN_TO_DEC_0110001011  395
#define EMP_BIN_TO_DEC_0110001100  396
#define EMP_BIN_TO_DEC_0110001101  397
#define EMP_BIN_TO_DEC_0110001110  398
#define EMP_BIN_TO_DEC_0110001111  399
#define EMP_BIN_TO_DEC_0110010000  400
#define EMP_BIN_TO_DEC_0110010001  401
#define EMP_BIN_TO_DEC_0110010010  402
#define EMP_BIN_TO_DEC_0110010011  403
#define EMP_BIN_TO_DEC_0110010100  404
#define EMP_BIN_TO_DEC_0110010101  405
#define EMP_BIN_TO_DEC_0110010110  406
#define EMP_BIN_TO_DEC_0110010111  407
#define EMP_BIN_TO_DEC_0110011000  408
#define EMP_BIN_TO_DEC_0110011001  409
#define EMP_BIN_TO_DEC_0110011010  410
#define EMP_BIN_TO_DEC_0110011011  411
#define EMP_BIN_TO_DEC_0110011100  412
#define EMP_BIN_TO_DEC_0110011101  413
#define EMP_BIN_TO_DEC_0110011110  414
#define EMP_BIN_TO_DEC_0110011111  415
#define EMP_BIN_TO_DEC_0110100000  416
#define EMP_BIN_TO_DEC_0110100001  417
#define EMP_BIN_TO_DEC_0110100010  418
#define EMP_BIN_TO_DEC_0110100011  419
#define EMP_BIN_TO_DEC_0110100100  420
#define EMP_BIN_TO_DEC_0110100101  421
#define EMP_BIN_TO_DEC_0110100110  422
#define EMP_BIN_TO_DEC_0110100111  423
#define EMP_BIN_TO_DEC_0110101000  424
#define EMP_BIN_TO_DEC_0110101001  425
#define EMP_BIN_TO_DEC_0110101010  426
#define EMP_BIN_TO_DEC_0110101011  427
#define EMP_BIN_TO_DEC_0110101100  428
#define EMP_BIN_TO_DEC_0110101101  429
#define EMP_BIN_TO_DEC_0110101110  430
#define EMP_BIN_TO_DEC_0110101111  431
#define EMP_BIN_TO_DEC_0110110000  432
#define EMP_BIN_TO_DEC_0110110001  433
#define EMP_BIN_TO_DEC_0110110010  434
#define EMP_BIN_TO_DEC_0110110011  435
#define EMP_BIN_TO_DEC_0110110100  436
#define EMP_BIN_TO_DEC_0110110101  437
#define EMP_BIN_TO_DEC_0110110110  438
#define EMP_BIN_TO_DEC_0110110111  439
#define EMP_BIN_TO_DEC_0110111000  440
#define EMP_BIN_TO_DEC_0110111001  441
#define EMP_BIN_TO_DEC_0110111010  442
#define EMP_BIN_TO_DEC_0110111011  443
#define EMP_BIN_TO_DEC_0110111100  444
#define EMP_BIN_TO_DEC_0110111101  445
#define EMP_BIN_TO_DEC_0110111110  446
#define EMP_BIN_TO_DEC_0110111111  447
#define EMP_BIN_TO_DEC_0111000000  448
#define EMP_BIN_TO_DEC_0111000001  449
#define EMP_BIN_TO_DEC_0111000010  450
#define EMP_BIN_TO_DEC_0111000011  451
#define EMP_BIN_TO_DEC_0111000100  452
#define EMP_BIN_TO_DEC_0111000101  453
#define EMP_BIN_TO_DEC_0111000110  454
#define EMP_BIN_TO_DEC_0111000111  455
#define EMP_BIN_TO_DEC_0111001000  456
#define EMP_BIN_TO_DEC_0111001001  457
#define EMP_BIN_TO_DEC_0111001010  458
#define EMP_BIN_TO_DEC_0111001011  459
#define EMP_BIN_TO_DEC_0111001100  460
#define EMP_BIN_TO_DEC_0111001101  461
#define EMP_BIN_TO_DEC_0111001110  462
#define EMP_BIN_TO_DEC_0111001111  463
#define EMP_BIN_TO_DEC_0111010000  464
#define EMP_BIN_TO_DEC_0111010001  465
#define EMP_BIN_TO_DEC_0111010010  466
#define EMP_BIN_TO_DEC_0111010011  467
#define EMP_BIN_TO_DEC_0111010100  468
#define EMP_BIN_TO_DEC_0111010101  469
#define EMP_BIN_TO_DEC_0111010110  470
#define EMP_BIN_TO_DEC_0111010111  471
#define EMP_BIN_TO_DEC_0111011000  472
#define EMP_BIN_TO_DEC_0111011001  473
#define EMP_BIN_TO_DEC_0111011010  474
#define EMP_BIN_TO_DEC_0111011011  475
#define EMP_BIN_TO_DEC_0111011100  476
#define EMP_BIN_TO_DEC_0111011101  477
#define EMP_BIN_TO_DEC_0111011110  478
#define EMP_BIN_TO_DEC_0111011111  479
#define EMP_BIN_TO_DEC_0111100000  480
#define EMP_BIN_TO_DEC_0111100001  481
#define EMP_BIN_TO_DEC_0111100010  482
#define EMP_BIN_TO_DEC_0111100011  483
#define EMP_BIN_TO_DEC_0111100100  484
#define EMP_BIN_TO_DEC_0111100101  485
#define EMP_BIN_TO_DEC_0111100110  486
#define EMP_BIN_TO_DEC_0111100111  487
#define EMP_BIN_TO_DEC_0111101000  488
#define EMP_BIN_TO_DEC_0111101001  489
#define EMP_BIN_TO_DEC_0111101010  490
#define EMP_BIN_TO_DEC_0111101011  491
#define EMP_BIN_TO_DEC_0111101100  492
#define EMP_BIN_TO_DEC_0111101101  493
#define EMP_BIN_TO_DEC_0111101110  494
#define EMP_BIN_TO_DEC_0111101111  495
#define EMP_BIN_TO_DEC_0111110000  496
#define EMP_BIN_TO_DEC_0111110001  497
#define EMP_BIN_TO_DEC_0111110010  498
#define EMP_BIN_TO_DEC_0111110011  499
#define EMP_BIN_TO_DEC_0111110100  500
#define EMP_BIN_TO_DEC_0111110101  501
#define EMP_BIN_TO_DEC_0111110110  502
#define EMP_BIN_TO_DEC_0111110111  503
#define EMP_BIN_TO_DEC_0111111000  504
#define EMP_BIN_TO_DEC_0111111001  505
#define EMP_BIN_TO_DEC_0111111010  506
#define EMP_BIN_TO_DEC_0111111011  507
#define EMP_BIN_TO_DEC_0111111100  508
#define EMP_BIN_TO_DEC_0111111101  509
#define EMP_BIN_TO_DEC_0111111110  510
#define EMP_BIN_TO_DEC_0111111111  511
#define EMP_BIN_TO_DEC_1000000000  512
#define EMP_BIN_TO_DEC_1000000001  513
#define EMP_BIN_TO_DEC_1000000010  514
#define EMP_BIN_TO_DEC_1000000011  515
#define EMP_BIN_TO_DEC_1000000100  516
#define EMP_BIN_TO_DEC_1000000101  517
#define EMP_BIN_TO_DEC_1000000110  518
#define EMP_BIN_TO_DEC_1000000111  519
#define EMP_BIN_TO_DEC_1000001000  520
#define EMP_BIN_TO_DEC_1000001001  521
#define EMP_BIN_TO_DEC_1000001010  522
#define EMP_BIN_TO_DEC_1000001011  523
#define EMP_BIN_TO_DEC_1000001100  524
#define EMP_BIN_TO_DEC_1000001101  525
#define EMP_BIN_TO_DEC_1000001110  526
#define EMP_BIN_TO_DEC_1000001111  527
#define EMP_BIN_TO_DEC_1000010000  528
#define EMP_BIN_TO_DEC_1000010001  529
#define EMP_BIN_TO_DEC_1000010010  530
#define EMP_BIN_TO_DEC_1000010011  531
#define EMP_BIN_TO_DEC_1000010100  532
#define EMP_BIN_TO_DEC_1000010101  533
#define EMP_BIN_TO_DEC_1000010110  534
#define EMP_BIN_TO_DEC_1000010111  535
#define EMP_BIN_TO_DEC_1000011000  536
#define EMP_BIN_TO_DEC_1000011001  537
#define EMP_BIN_TO_DEC_1000011010  538
#define EMP_BIN_TO_DEC_1000011011  539
#define EMP_BIN_TO_DEC_1000011100  540
#define EMP_BIN_TO_DEC_1000011101  541
#define EMP_BIN_TO_DEC_1000011110  542
#define EMP_BIN_TO_DEC_1000011111  543
#define EMP_BIN_TO_DEC_1000100000  544
#define EMP_BIN_TO_DEC_1000100001  545
#define EMP_BIN_TO_DEC_1000100010  546
#define EMP_BIN_TO_DEC_1000100011  547
#define EMP_BIN_TO_DEC_1000100100  548
#define EMP_BIN_TO_DEC_1000100101  549
#define EMP_BIN_TO_DEC_1000100110  550
#define EMP_BIN_TO_DEC_1000100111  551
#define EMP_BIN_TO_DEC_1000101000  552
#define EMP_BIN_TO_DEC_1000101001  553
#define EMP_BIN_TO_DEC_1000101010  554
#define EMP_BIN_TO_DEC_1000101011  555
#define EMP_BIN_TO_DEC_1000101100  556
#define EMP_BIN_TO_DEC_1000101101  557
#define EMP_BIN_TO_DEC_1000101110  558
#define EMP_BIN_TO_DEC_1000101111  559
#define EMP_BIN_TO_DEC_1000110000  560
#define EMP_BIN_TO_DEC_1000110001  561
#define EMP_BIN_TO_DEC_1000110010  562
#define EMP_BIN_TO_DEC_1000110011  563
#define EMP_BIN_TO_DEC_1000110100  564
#define EMP_BIN_TO_DEC_1000110101  565
#define EMP_BIN_TO_DEC_1000110110  566
#define EMP_BIN_TO_DEC_1000110111  567
#define EMP_BIN_TO_DEC_1000111000  568
#define EMP_BIN_TO_DEC_1000111001  569
#define EMP_BIN_TO_DEC_1000111010  570
#define EMP_BIN_TO_DEC_1000111011  571
#define EMP_BIN_TO_DEC_1000111100  572
#define EMP_BIN_TO_DEC_1000111101  573
#define EMP_BIN_TO_DEC_1000111110  574
#define EMP_BIN_TO_DEC_1000111111  575
#define EMP_BIN_TO_DEC_1001000000  576
#define EMP_BIN_TO_DEC_1001000001  577
#define EMP_BIN_TO_DEC_1001000010  578
#define EMP_BIN_TO_DEC_1001000011  579
#define EMP_BIN_TO_DEC_1001000100  580
#define EMP_BIN_TO_DEC_1001000101  581
#define EMP_BIN_TO_DEC_1001000110  582
#define EMP_BIN_TO_DEC_1001000111  583
#define EMP_BIN_TO_DEC_1001001000  584
#define EMP_BIN_TO_DEC_1001001001  585
#define EMP_BIN_TO_DEC_1001001010  586
#define EMP_BIN_TO_DEC_1001001011  587
#define EMP_BIN_TO_DEC_1001001100  588
#define EMP_BIN_TO_DEC_1001001101  589
#define EMP_BIN_TO_DEC_1001001110  590
#define EMP_BIN_TO_DEC_1001001111  591
#define EMP_BIN_TO_DEC_1001010000  592
#define EMP_BIN_TO_DEC_1001010001  593
#define EMP_BIN_TO_DEC_1001010010  594
#define EMP_BIN_TO_DEC_1001010011  595
#define EMP_BIN_TO_DEC_1001010100  596
#define EMP_BIN_TO_DEC_1001010101  597
#define EMP_BIN_TO_DEC_1001010110  598
#define EMP_BIN_TO_DEC_1001010111  599
#define EMP_BIN_TO_DEC_1001011000  600
#define EMP_BIN_TO_DEC_1001011001  601
#define EMP_BIN_TO_DEC_1001011010  602
#define EMP_BIN_TO_DEC_1001011011  603
#define EMP_BIN_TO_DEC_1001011100  604
#define EMP_BIN_TO_DEC_1001011101  605
#define EMP_BIN_TO_DEC_1001011110  606
#define EMP_BIN_TO_DEC_1001011111  607
#define EMP_BIN_TO_DEC_1001100000  608
#define EMP_BIN_TO_DEC_1001100001  609
#define EMP_BIN_TO_DEC_1001100010  610
#define EMP_BIN_TO_DEC_1001100011  611
#define EMP_BIN_TO_DEC_1001100100  612
#define EMP_BIN_TO_DEC_1001100101  613
#define EMP_BIN_TO_DEC_1001100110  614
#define EMP_BIN_TO_DEC_1001100111  615
#define EMP_BIN_TO_DEC_1001101000  616
#define EMP_BIN_TO_DEC_1001101001  617
#define EMP_BIN_TO_DEC_1001101010  618
#define EMP_BIN_TO_DEC_1001101011  619
#define EMP_BIN_TO_DEC_1001101100  620
#define EMP_BIN_TO_DEC_1001101101  621
#define EMP_BIN_TO_DEC_1001101110  622
#define EMP_BIN_TO_DEC_1001101111  623
#define EMP_BIN_TO_DEC_1001110000  624
#define EMP_BIN_TO_DEC_1001110001  625
#define EMP_BIN_TO_DEC_1001110010  626
#define EMP_BIN_TO_DEC_1001110011  627
#define EMP_BIN_TO_DEC_1001110100  628
#define EMP_BIN_TO_DEC_1001110101  629
#define EMP_BIN_TO_DEC_1001110110  630
#define EMP_BIN_TO_DEC_1001110111  631
#define EMP_BIN_TO_DEC_1001111000  632
#define EMP_BIN_TO_DEC_1001111001  633
#define EMP_BIN_TO_DEC_1001111010  634
#define EMP_BIN_TO_DEC_1001111011  635
#define EMP_BIN_TO_DEC_1001111100  636
#define EMP_BIN_TO_DEC_1001111101  637
#define EMP_BIN_TO_DEC_1001111110  638
#define EMP_BIN_TO_DEC_1001111111  639
#define EMP_BIN_TO_DEC_1010000000  640
#define EMP_BIN_TO_DEC_1010000001  641
#define EMP_BIN_TO_DEC_1010000010  642
#define EMP_BIN_TO_DEC_1010000011  643
#define EMP_BIN_TO_DEC_1010000100  644
#define EMP_BIN_TO_DEC_1010000101  645
#define EMP_BIN_TO_DEC_1010000110  646
#define EMP_BIN_TO_DEC_1010000111  647
#define EMP_BIN_TO_DEC_1010001000  648
#define EMP_BIN_TO_DEC_1010001001  649
#define EMP_BIN_TO_DEC_1010001010  650
#define EMP_BIN_TO_DEC_1010001011  651
#define EMP_BIN_TO_DEC_1010001100  652
#define EMP_BIN_TO_DEC_1010001101  653
#define EMP_BIN_TO_DEC_1010001110  654
#define EMP_BIN_TO_DEC_1010001111  655
#define EMP_BIN_TO_DEC_1010010000  656
#define EMP_BIN_TO_DEC_1010010001  657
#define EMP_BIN_TO_DEC_1010010010  658
#define EMP_BIN_TO_DEC_1010010011  659
#define EMP_BIN_TO_DEC_1010010100  660
#define EMP_BIN_TO_DEC_1010010101  661
#define EMP_BIN_TO_DEC_1010010110  662
#define EMP_BIN_TO_DEC_1010010111  663
#define EMP_BIN_TO_DEC_1010011000  664
#define EMP_BIN_TO_DEC_1010011001  665
#define EMP_BIN_TO_DEC_1010011010  666
#define EMP_BIN_TO_DEC_1010011011  667
#define EMP_BIN_TO_DEC_1010011100  668
#define EMP_BIN_TO_DEC_1010011101  669
#define EMP_BIN_TO_DEC_1010011110  670
#define EMP_BIN_TO_DEC_1010011111  671
#define EMP_BIN_TO_DEC_1010100000  672
#define EMP_BIN_TO_DEC_1010100001  673
#define EMP_BIN_TO_DEC_1010100010  674
#define EMP_BIN_TO_DEC_1010100011  675
#define EMP_BIN_TO_DEC_1010100100  676
#define EMP_BIN_TO_DEC_1010100101  677
#define EMP_BIN_TO_DEC_1010100110  678
#define EMP_BIN_TO_DEC_1010100111  679
#define EMP_BIN_TO_DEC_1010101000  680
#define EMP_BIN_TO_DEC_1010101001  681
#define EMP_BIN_TO_DEC_1010101010  682
#define EMP_BIN_TO_DEC_1010101011  683
#define EMP_BIN_TO_DEC_1010101100  684
#define EMP_BIN_TO_DEC_1010101101  685
#define EMP_BIN_TO_DEC_1010101110  686
#define EMP_BIN_TO_DEC_1010101111  687
#define EMP_BIN_TO_DEC_1010110000  688
#define EMP_BIN_TO_DEC_1010110001  689
#define EMP_BIN_TO_DEC_1010110010  690
#define EMP_BIN_TO_DEC_1010110011  691
#define EMP_BIN_TO_DEC_1010110100  692
#define EMP_BIN_TO_DEC_1010110101  693
#define EMP_BIN_TO_DEC_1010110110  694
#define EMP_BIN_TO_DEC_1010110111  695
#define EMP_BIN_TO_DEC_1010111000  696
#define EMP_BIN_TO_DEC_1010111001  697
#define EMP_BIN_TO_DEC_1010111010  698
#define EMP_BIN_TO_DEC_1010111011  699
#define EMP_BIN_TO_DEC_1010111100  700
#define EMP_BIN_TO_DEC_1010111101  701
#define EMP_BIN_TO_DEC_1010111110  702
#define EMP_BIN_TO_DEC_1010111111  703
#define EMP_BIN_TO_DEC_1011000000  704
#define EMP_BIN_TO_DEC_1011000001  705
#define EMP_BIN_TO_DEC_1011000010  706
#define EMP_BIN_TO_DEC_1011000011  707
#define EMP_BIN_TO_DEC_1011000100  708
#define EMP_BIN_TO_DEC_1011000101  709
#define EMP_BIN_TO_DEC_1011000110  710
#define EMP_BIN_TO_DEC_1011000111  711
#define EMP_BIN_TO_DEC_1011001000  712
#define EMP_BIN_TO_DEC_1011001001  713
#define EMP_BIN_TO_DEC_1011001010  714
#define EMP_BIN_TO_DEC_1011001011  715
#define EMP_BIN_TO_DEC_1011001100  716
#define EMP_BIN_TO_DEC_1011001101  717
#define EMP_BIN_TO_DEC_1011001110  718
#define EMP_BIN_TO_DEC_1011001111  719
#define EMP_BIN_TO_DEC_1011010000  720
#define EMP_BIN_TO_DEC_1011010001  721
#define EMP_BIN_TO_DEC_1011010010  722
#define EMP_BIN_TO_DEC_1011010011  723
#define EMP_BIN_TO_DEC_1011010100  724
#define EMP_BIN_TO_DEC_1011010101  725
#define EMP_BIN_TO_DEC_1011010110  726
#define EMP_BIN_TO_DEC_1011010111  727
#define EMP_BIN_TO_DEC_1011011000  728
#define EMP_BIN_TO_DEC_1011011001  729
#define EMP_BIN_TO_DEC_1011011010  730
#define EMP_BIN_TO_DEC_1011011011  731
#define EMP_BIN_TO_DEC_1011011100  732
#define EMP_BIN_TO_DEC_1011011101  733
#define EMP_BIN_TO_DEC_1011011110  734
#define EMP_BIN_TO_DEC_1011011111  735
#define EMP_BIN_TO_DEC_1011100000  736
#define EMP_BIN_TO_DEC_1011100001  737
#define EMP_BIN_TO_DEC_1011100010  738
#define EMP_BIN_TO_DEC_1011100011  739
#define EMP_BIN_TO_DEC_1011100100  740
#define EMP_BIN_TO_DEC_1011100101  741
#define EMP_BIN_TO_DEC_1011100110  742
#define EMP_BIN_TO_DEC_1011100111  743
#define EMP_BIN_TO_DEC_1011101000  744
#define EMP_BIN_TO_DEC_1011101001  745
#define EMP_BIN_TO_DEC_1011101010  746
#define EMP_BIN_TO_DEC_1011101011  747
#define EMP_BIN_TO_DEC_1011101100  748
#define EMP_BIN_TO_DEC_1011101101  749
#define EMP_BIN_TO_DEC_1011101110  750
#define EMP_BIN_TO_DEC_1011101111  751
#define EMP_BIN_TO_DEC_1011110000  752
#define EMP_BIN_TO_DEC_1011110001  753
#define EMP_BIN_TO_DEC_1011110010  754
#define EMP_BIN_TO_DEC_1011110011  755
#define EMP_BIN_TO_DEC_1011110100  756
#define EMP_BIN_TO_DEC_1011110101  757
#define EMP_BIN_TO_DEC_1011110110  758
#define EMP_BIN_TO_DEC_1011110111  759
#define EMP_BIN_TO_DEC_1011111000  760
#define EMP_BIN_TO_DEC_1011111001  761
#define EMP_BIN_TO_DEC_1011111010  762
#define EMP_BIN_TO_DEC_1011111011  763
#define EMP_BIN_TO_DEC_1011111100  764
#define EMP_BIN_TO_DEC_1011111101  765
#define EMP_BIN_TO_DEC_1011111110  766
#define EMP_BIN_TO_DEC_1011111111  767
#define EMP_BIN_TO_DEC_1100000000  768
#define EMP_BIN_TO_DEC_1100000001  769
#define EMP_BIN_TO_DEC_1100000010  770
#define EMP_BIN_TO_DEC_1100000011  771
#define EMP_BIN_TO_DEC_1100000100  772
#define EMP_BIN_TO_DEC_1100000101  773
#define EMP_BIN_TO_DEC_1100000110  774
#define EMP_BIN_TO_DEC_1100000111  775
#define EMP_BIN_TO_DEC_1100001000  776
#define EMP_BIN_TO_DEC_1100001001  777
#define EMP_BIN_TO_DEC_1100001010  778
#define EMP_BIN_TO_DEC_1100001011  779
#define EMP_BIN_TO_DEC_1100001100  780
#define EMP_BIN_TO_DEC_1100001101  781
#define EMP_BIN_TO_DEC_1100001110  782
#define EMP_BIN_TO_DEC_1100001111  783
#define EMP_BIN_TO_DEC_1100010000  784
#define EMP_BIN_TO_DEC_1100010001  785
#define EMP_BIN_TO_DEC_1100010010  786
#define EMP_BIN_TO_DEC_1100010011  787
#define EMP_BIN_TO_DEC_1100010100  788
#define EMP_BIN_TO_DEC_1100010101  789
#define EMP_BIN_TO_DEC_1100010110  790
#define EMP_BIN_TO_DEC_1100010111  791
#define EMP_BIN_TO_DEC_1100011000  792
#define EMP_BIN_TO_DEC_1100011001  793
#define EMP_BIN_TO_DEC_1100011010  794
#define EMP_BIN_TO_DEC_1100011011  795
#define EMP_BIN_TO_DEC_1100011100  796
#define EMP_BIN_TO_DEC_1100011101  797
#define EMP_BIN_TO_DEC_1100011110  798
#define EMP_BIN_TO_DEC_1100011111  799
#define EMP_BIN_TO_DEC_1100100000  800
#define EMP_BIN_TO_DEC_1100100001  801
#define EMP_BIN_TO_DEC_1100100010  802
#define EMP_BIN_TO_DEC_1100100011  803
#define EMP_BIN_TO_DEC_1100100100  804
#define EMP_BIN_TO_DEC_1100100101  805
#define EMP_BIN_TO_DEC_1100100110  806
#define EMP_BIN_TO_DEC_1100100111  807
#define EMP_BIN_TO_DEC_1100101000  808
#define EMP_BIN_TO_DEC_1100101001  809
#define EMP_BIN_TO_DEC_1100101010  810
#define EMP_BIN_TO_DEC_1100101011  811
#define EMP_BIN_TO_DEC_1100101100  812
#define EMP_BIN_TO_DEC_1100101101  813
#define EMP_BIN_TO_DEC_1100101110  814
#define EMP_BIN_TO_DEC_1100101111  815
#define EMP_BIN_TO_DEC_1100110000  816
#define EMP_BIN_TO_DEC_1100110001  817
#define EMP_BIN_TO_DEC_1100110010  818
#define EMP_BIN_TO_DEC_1100110011  819
#define EMP_BIN_TO_DEC_1100110100  820
#define EMP_BIN_TO_DEC_1100110101  821
#define EMP_BIN_TO_DEC_1100110110  822
#define EMP_BIN_TO_DEC_1100110111  823
#define EMP_BIN_TO_DEC_1100111000  824
#define EMP_BIN_TO_DEC_1100111001  825
#define EMP_BIN_TO_DEC_1100111010  826
#define EMP_BIN_TO_DEC_1100111011  827
#define EMP_BIN_TO_DEC_1100111100  828
#define EMP_BIN_TO_DEC_1100111101  829
#define EMP_BIN_TO_DEC_1100111110  830
#define EMP_BIN_TO_DEC_1100111111  831
#define EMP_BIN_TO_DEC_1101000000  832
#define EMP_BIN_TO_DEC_1101000001  833
#define EMP_BIN_TO_DEC_1101000010  834
#define EMP_BIN_TO_DEC_1101000011  835
#define EMP_BIN_TO_DEC_1101000100  836
#define EMP_BIN_TO_DEC_1101000101  837
#define EMP_BIN_TO_DEC_1101000110  838
#define EMP_BIN_TO_DEC_1101000111  839
#define EMP_BIN_TO_DEC_1101001000  840
#define EMP_BIN_TO_DEC_1101001001  841
#define EMP_BIN_TO_DEC_1101001010  842
#define EMP_BIN_TO_DEC_1101001011  843
#define EMP_BIN_TO_DEC_1101001100  844
#define EMP_BIN_TO_DEC_1101001101  845
#define EMP_BIN_TO_DEC_1101001110  846
#define EMP_BIN_TO_DEC_1101001111  847
#define EMP_BIN_TO_DEC_1101010000  848
#define EMP_BIN_TO_DEC_1101010001  849
#define EMP_BIN_TO_DEC_1101010010  850
#define EMP_BIN_TO_DEC_1101010011  851
#define EMP_BIN_TO_DEC_1101010100  852
#define EMP_BIN_TO_DEC_1101010101  853
#define EMP_BIN_TO_DEC_1101010110  854
#define EMP_BIN_TO_DEC_1101010111  855
#define EMP_BIN_TO_DEC_1101011000  856
#define EMP_BIN_TO_DEC_1101011001  857
#define EMP_BIN_TO_DEC_1101011010  858
#define EMP_BIN_TO_DEC_1101011011  859
#define EMP_BIN_TO_DEC_1101011100  860
#define EMP_BIN_TO_DEC_1101011101  861
#define EMP_BIN_TO_DEC_1101011110  862
#define EMP_BIN_TO_DEC_1101011111  863
#define EMP_BIN_TO_DEC_1101100000  864
#define EMP_BIN_TO_DEC_1101100001  865
#define EMP_BIN_TO_DEC_1101100010  866
#define EMP_BIN_TO_DEC_1101100011  867
#define EMP_BIN_TO_DEC_1101100100  868
#define EMP_BIN_TO_DEC_1101100101  869
#define EMP_BIN_TO_DEC_1101100110  870
#define EMP_BIN_TO_DEC_1101100111  871
#define EMP_BIN_TO_DEC_1101101000  872
#define EMP_BIN_TO_DEC_1101101001  873
#define EMP_BIN_TO_DEC_1101101010  874
#define EMP_BIN_TO_DEC_1101101011  875
#define EMP_BIN_TO_DEC_1101101100  876
#define EMP_BIN_TO_DEC_1101101101  877
#define EMP_BIN_TO_DEC_1101101110  878
#define EMP_BIN_TO_DEC_1101101111  879
#define EMP_BIN_TO_DEC_1101110000  880
#define EMP_BIN_TO_DEC_1101110001  881
#define EMP_BIN_TO_DEC_1101110010  882
#define EMP_BIN_TO_DEC_1101110011  883
#define EMP_BIN_TO_DEC_1101110100  884
#define EMP_BIN_TO_DEC_1101110101  885
#define EMP_BIN_TO_DEC_1101110110  886
#define EMP_BIN_TO_DEC_1101110111  887
#define EMP_BIN_TO_DEC_1101111000  888
#define EMP_BIN_TO_DEC_1101111001  889
#define EMP_BIN_TO_DEC_1101111010  890
#define EMP_BIN_TO_DEC_1101111011  891
#define EMP_BIN_TO_DEC_1101111100  892
#define EMP_BIN_TO_DEC_1101111101  893
#define EMP_BIN_TO_DEC_1101111110  894
#define EMP_BIN_TO_DEC_1101111111  895
#define EMP_BIN_TO_DEC_1110000000  896
#define EMP_BIN_TO_DEC_1110000001  897
#define EMP_BIN_TO_DEC_1110000010  898
#define EMP_BIN_TO_DEC_1110000011  899
#define EMP_BIN_TO_DEC_1110000100  900
#define EMP_BIN_TO_DEC_1110000101  901
#define EMP_BIN_TO_DEC_1110000110  902
#define EMP_BIN_TO_DEC_1110000111  903
#define EMP_BIN_TO_DEC_1110001000  904
#define EMP_BIN_TO_DEC_1110001001  905
#define EMP_BIN_TO_DEC_1110001010  906
#define EMP_BIN_TO_DEC_1110001011  907
#define EMP_BIN_TO_DEC_1110001100  908
#define EMP_BIN_TO_DEC_1110001101  909
#define EMP_BIN_TO_DEC_1110001110  910
#define EMP_BIN_TO_DEC_1110001111  911
#define EMP_BIN_TO_DEC_1110010000  912
#define EMP_BIN_TO_DEC_1110010001  913
#define EMP_BIN_TO_DEC_1110010010  914
#define EMP_BIN_TO_DEC_1110010011  915
#define EMP_BIN_TO_DEC_1110010100  916
#define EMP_BIN_TO_DEC_1110010101  917
#define EMP_BIN_TO_DEC_1110010110  918
#define EMP_BIN_TO_DEC_1110010111  919
#define EMP_BIN_TO_DEC_1110011000  920
#define EMP_BIN_TO_DEC_1110011001  921
#define EMP_BIN_TO_DEC_1110011010  922
#define EMP_BIN_TO_DEC_1110011011  923
#define EMP_BIN_TO_DEC_1110011100  924
#define EMP_BIN_TO_DEC_1110011101  925
#define EMP_BIN_TO_DEC_1110011110  926
#define EMP_BIN_TO_DEC_1110011111  927
#define EMP_BIN_TO_DEC_1110100000  928
#define EMP_BIN_TO_DEC_1110100001  929
#define EMP_BIN_TO_DEC_1110100010  930
#define EMP_BIN_TO_DEC_1110100011  931
#define EMP_BIN_TO_DEC_1110100100  932
#define EMP_BIN_TO_DEC_1110100101  933
#define EMP_BIN_TO_DEC_1110100110  934
#define EMP_BIN_TO_DEC_1110100111  935
#define EMP_BIN_TO_DEC_1110101000  936
#define EMP_BIN_TO_DEC_1110101001  937
#define EMP_BIN_TO_DEC_1110101010  938
#define EMP_BIN_TO_DEC_1110101011  939
#define EMP_BIN_TO_DEC_1110101100  940
#define EMP_BIN_TO_DEC_1110101101  941
#define EMP_BIN_TO_DEC_1110101110  942
#define EMP_BIN_TO_DEC_1110101111  943
#define EMP_BIN_TO_DEC_1110110000  944
#define EMP_BIN_TO_DEC_1110110001  945
#define EMP_BIN_TO_DEC_1110110010  946
#define EMP_BIN_TO_DEC_1110110011  947
#define EMP_BIN_TO_DEC_1110110100  948
#define EMP_BIN_TO_DEC_1110110101  949
#define EMP_BIN_TO_DEC_1110110110  950
#define EMP_BIN_TO_DEC_1110110111  951
#define EMP_BIN_TO_DEC_1110111000  952
#define EMP_BIN_TO_DEC_1110111001  953
#define EMP_BIN_TO_DEC_1110111010  954
#define EMP_BIN_TO_DEC_1110111011  955
#define EMP_BIN_TO_DEC_1110111100  956
#define EMP_BIN_TO_DEC_1110111101  957
#define EMP_BIN_TO_DEC_1110111110  958
#define EMP_BIN_TO_DEC_1110111111  959
#define EMP_BIN_TO_DEC_1111000000  960
#define EMP_BIN_TO_DEC_1111000001  961
#define EMP_BIN_TO_DEC_1111000010  962
#define EMP_BIN_TO_DEC_1111000011  963
#define EMP_BIN_TO_DEC_1111000100  964
#define EMP_BIN_TO_DEC_1111000101  965
#define EMP_BIN_TO_DEC_1111000110  966
#define EMP_BIN_TO_DEC_1111000111  967
#define EMP_BIN_TO_DEC_1111001000  968
#define EMP_BIN_TO_DEC_1111001001  969
#define EMP_BIN_TO_DEC_1111001010  970
#define EMP_BIN_TO_DEC_1111001011  971
#define EMP_BIN_TO_DEC_1111001100  972
#define EMP_BIN_TO_DEC_1111001101  973
#define EMP_BIN_TO_DEC_1111001110  974
#define EMP_BIN_TO_DEC_1111001111  975
#define EMP_BIN_TO_DEC_1111010000  976
#define EMP_BIN_TO_DEC_1111010001  977
#define EMP_BIN_TO_DEC_1111010010  978
#define EMP_BIN_TO_DEC_1111010011  979
#define EMP_BIN_TO_DEC_1111010100  980
#define EMP_BIN_TO_DEC_1111010101  981
#define EMP_BIN_TO_DEC_1111010110  982
#define EMP_BIN_TO_DEC_1111010111  983
#define EMP_BIN_TO_DEC_1111011000  984
#define EMP_BIN_TO_DEC_1111011001  985
#define EMP_BIN_TO_DEC_1111011010  986
#define EMP_BIN_TO_DEC_1111011011  987
#define EMP_BIN_TO_DEC_1111011100  988
#define EMP_BIN_TO_DEC_1111011101  989
#define EMP_BIN_TO_DEC_1111011110  990
#define EMP_BIN_TO_DEC_1111011111  991
#define EMP_BIN_TO_DEC_1111100000  992
#define EMP_BIN_TO_DEC_1111100001  993
#define EMP_BIN_TO_DEC_1111100010  994
#define EMP_BIN_TO_DEC_1111100011  995
#define EMP_BIN_TO_DEC_1111100100  996
#define EMP_BIN_TO_DEC_1111100101  997
#define EMP_BIN_TO_DEC_1111100110  998
#define EMP_BIN_TO_DEC_1111100111  999
#define EMP_BIN_TO_DEC_1111101000  1000
#define EMP_BIN_TO_DEC_1111101001  1001
#define EMP_BIN_TO_DEC_1111101010  1002
#define EMP_BIN_TO_DEC_1111101011  1003
#define EMP_BIN_TO_DEC_1111101100  1004
#define EMP_BIN_TO_DEC_1111101101  1005
#define EMP_BIN_TO_DEC_1111101110  1006
#define EMP_BIN_TO_DEC_1111101111  1007
#define EMP_BIN_TO_DEC_1111110000  1008
#define EMP_BIN_TO_DEC_1111110001  1009
#define EMP_BIN_TO_DEC_1111110010  1010
#define EMP_BIN_TO_DEC_1111110011  1011
#define EMP_BIN_TO_DEC_1111110100  1012
#define EMP_BIN_TO_DEC_1111110101  1013
#define EMP_BIN_TO_DEC_1111110110  1014
#define EMP_BIN_TO_DEC_1111110111  1015
#define EMP_BIN_TO_DEC_1111111000  1016
#define EMP_BIN_TO_DEC_1111111001  1017
#define EMP_BIN_TO_DEC_1111111010  1018
#define EMP_BIN_TO_DEC_1111111011  1019
#define EMP_BIN_TO_DEC_1111111100  1020
#define EMP_BIN_TO_DEC_1111111101  1021
#define EMP_BIN_TO_DEC_1111111110  1022
#define EMP_BIN_TO_DEC_1111111111  1023


// Pre-define some simple multiplication
#define EMP_MATH_VAL_TIMES_0(A) 0
#define EMP_MATH_VAL_TIMES_1(A) A

#define EMP_ADD_ARG_IF_VAL_0(A)
#define EMP_ADD_ARG_IF_VAL_1(A) , A

#define EMP_MATH_BIN_TIMES_0(A0,A1,A2,A3,A4,A5,A6,A7,A8,A9) 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
#define EMP_MATH_BIN_TIMES_1(A0,A1,A2,A3,A4,A5,A6,A7,A8,A9) A0, A1, A2, A3, A4, A5, A6, A7, A8, A9


// Now, convert to SUM format.
#define EMP_BIN_TO_SUM(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9) EMP_MATH_VAL_TIMES_ ## A0(512), \
    EMP_MATH_VAL_TIMES_##A1(256), EMP_MATH_VAL_TIMES_##A2(128), EMP_MATH_VAL_TIMES_##A3(64),   \
    EMP_MATH_VAL_TIMES_##A4(32),  EMP_MATH_VAL_TIMES_##A5(16),  EMP_MATH_VAL_TIMES_##A6(8),    \
    EMP_MATH_VAL_TIMES_##A7(4),   EMP_MATH_VAL_TIMES_##A8(2),   EMP_MATH_VAL_TIMES_##A9(1)

// Now, convert to PACK format.
#define EMP_BIN_TO_PACK(...) \
  (EMP_EVAL( EMP_BIN_TO_PACK_POP EMP_EMPTY() ( EMP_BIN_TO_PACK_IMPL(__VA_ARGS__) ) ))
#define EMP_BIN_TO_PACK_POP(A, ...) __VA_ARGS__
#define EMP_BIN_TO_PACK_IMPL(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9)    \
  ~ EMP_ADD_ARG_IF_VAL_##A0(512)EMP_ADD_ARG_IF_VAL_##A1(256)EMP_ADD_ARG_IF_VAL_##A2(128)EMP_ADD_ARG_IF_VAL_##A3(64)EMP_ADD_ARG_IF_VAL_##A4(32)EMP_ADD_ARG_IF_VAL_##A5(16)EMP_ADD_ARG_IF_VAL_##A6(8)EMP_ADD_ARG_IF_VAL_##A7(4)EMP_ADD_ARG_IF_VAL_##A8(2)EMP_ADD_ARG_IF_VAL_##A9(1)



#define EMP_DEC_TO_SUM(A) EMP_BIN_TO_SUM EMP_EMPTY() ( EMP_DEC_TO_BIN(A) )
#define EMP_DEC_TO_PACK(A) EMP_BIN_TO_PACK EMP_EMPTY() ( EMP_DEC_TO_BIN(A) )


// Pre-define simple comparisons & boolean logic
#define EMP_MATH_BIT_EQU_00 1
#define EMP_MATH_BIT_EQU_01 0
#define EMP_MATH_BIT_EQU_10 0
#define EMP_MATH_BIT_EQU_11 1

#define EMP_MATH_BIT_LESS_00 0
#define EMP_MATH_BIT_LESS_01 1
#define EMP_MATH_BIT_LESS_10 0
#define EMP_MATH_BIT_LESS_11 0

#define EMP_MATH_BIT_AND_00 0
#define EMP_MATH_BIT_AND_01 0
#define EMP_MATH_BIT_AND_10 0
#define EMP_MATH_BIT_AND_11 1

#define EMP_MATH_BIT_OR_00 0
#define EMP_MATH_BIT_OR_01 1
#define EMP_MATH_BIT_OR_10 1
#define EMP_MATH_BIT_OR_11 1

#define EMP_MATH_BIT_NAND_00 1
#define EMP_MATH_BIT_NAND_01 1
#define EMP_MATH_BIT_NAND_10 1
#define EMP_MATH_BIT_NAND_11 0

#define EMP_MATH_BIT_NOR_00 1
#define EMP_MATH_BIT_NOR_01 0
#define EMP_MATH_BIT_NOR_10 0
#define EMP_MATH_BIT_NOR_11 0

#define EMP_MATH_BIT_XOR_00 0
#define EMP_MATH_BIT_XOR_01 1
#define EMP_MATH_BIT_XOR_10 1
#define EMP_MATH_BIT_XOR_11 0

#define EMP_BOOL_IMPL(OP, X, Y) EMP_MATH_BIT_ ## OP ## _ ## X ## Y

#define EMP_NOT(X) EMP_BOOL_IMPL(NOR, X, 0)
#define EMP_BIT_EQU(X, Y) EMP_BOOL_IMPL(EQU, X, Y)
#define EMP_AND(X, Y) EMP_BOOL_IMPL(AND, X, Y)
#define EMP_OR(X, Y) EMP_BOOL_IMPL(OR, X, Y)
#define EMP_NAND(X, Y) EMP_BOOL_IMPL(NAND, X, Y)
#define EMP_NOR(X, Y) EMP_BOOL_IMPL(NOR, X, Y)
#define EMP_XOR(X, Y) EMP_BOOL_IMPL(XOR, X, Y)
#define EMP_BIT_LESS(X, Y) EMP_BOOL_IMPL(LESS, X, Y)
#define EMP_BIT_GTR(X, Y) EMP_BOOL_IMPL(LESS, Y, X)

/// @endcond

/// EMP_IF examines the first argument; if it's 0, it resolves to the third argument, otherwise
/// it resolves to the second argument.  We do this by converting a 0 into two arguments, with
/// F being the second one.  Anything else stays as one argument, and the default second is T
#define EMP_IF(TEST, T, F) EMP_IF_impl(TEST, T, F)

/// @cond MACROS

#define EMP_IF_impl(TEST, T, F) EMP_EVAL( EMP_IF_impl_get_2 EMP_EMPTY() ( EMP_IF_impl_##TEST(F), T, ~) )
#define EMP_IF_impl_get_2(A, B, ...) B
#define EMP_IF_impl_0(A) ~, A

/// @endcond

/// Tools to handle comparisons
/// This macro will create a list of comparisons only when difference occur, and then always
/// grab the first one.  If the values are equal, the result will be an X.
#define EMP_COMPARE(VAL_A, VAL_B)                                       \
  EMP_EVAL( EMP_COMPARE_IMPL( EMP_DEC_TO_BIN(VAL_A), EMP_DEC_TO_BIN(VAL_B) ) )

/// @cond MACROS

#define EMP_COMPARE_IMPL(...) EMP_COMPARE_BIN_IMPL( __VA_ARGS__ )

#define EMP_COMPARE_BIN(...) EMP_COMPARE_BIN_IMPL EMP_EMPTY() (__VA_ARGS__)
#define EMP_COMPARE_BIN_IMPL(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9,    \
                             B0, B1, B2, B3, B4, B5, B6, B7, B8, B9)    \
  EMP_COMPARE_get_first( EMP_COMPARE_BITS(A0,B0) EMP_COMPARE_BITS(A1,B1) EMP_COMPARE_BITS(A2,B2) EMP_COMPARE_BITS(A3,B3) EMP_COMPARE_BITS(A4,B4) EMP_COMPARE_BITS(A5,B5) EMP_COMPARE_BITS(A6,B6) EMP_COMPARE_BITS(A7,B7) EMP_COMPARE_BITS(A8,B8) EMP_COMPARE_BITS(A9,B9) X, ~ )

#define EMP_COMPARE_get_first(...) EMP_COMPARE_get_first_IMPL(__VA_ARGS__)
#define EMP_COMPARE_get_first_IMPL(A, ...) A
#define EMP_COMPARE_result_00
#define EMP_COMPARE_result_01 B,
#define EMP_COMPARE_result_10 A,
#define EMP_COMPARE_result_11
#define EMP_COMPARE_BITS(BIT_A, BIT_B) EMP_COMPARE_BITS_IMPL(BIT_A, BIT_B)
#define EMP_COMPARE_BITS_IMPL(BIT_A, BIT_B) EMP_COMPARE_result_ ## BIT_A ## BIT_B

#define EMP_COMPARE_FUN(VAL_A, VAL_B, OUT_A, OUT_B, OUT_X) \
  EMP_COMPARE_FUN_impl( EMP_COMPARE(VAL_A, VAL_B), OUT_A, OUT_B, OUT_X )

#define EMP_COMPARE_FUN_impl(RESULT, OUT_A, OUT_B, OUT_X) \
  EMP_COMPARE_FUN_impl2(RESULT, OUT_A, OUT_B, OUT_X)

#define EMP_COMPARE_FUN_impl2(RESULT, OUT_A, OUT_B, OUT_X) \
  EMP_COMPARE_FUN_impl_##RESULT(OUT_A, OUT_B, OUT_X)

#define EMP_COMPARE_FUN_impl_A(OUT_A, OUT_B, OUT_X) OUT_A
#define EMP_COMPARE_FUN_impl_B(OUT_A, OUT_B, OUT_X) OUT_B
#define EMP_COMPARE_FUN_impl_X(OUT_A, OUT_B, OUT_X) OUT_X

#define EMP_EQU(A,B) EMP_COMPARE_FUN(A,B,0,0,1)
#define EMP_LESS(A,B) EMP_COMPARE_FUN(A,B,0,1,0)
#define EMP_LESS_EQU(A,B) EMP_COMPARE_FUN(A,B,0,1,1)
#define EMP_GTR(A,B) EMP_COMPARE_FUN(A,B,1,0,0)
#define EMP_GTR_EQU(A,B) EMP_COMPARE_FUN(A,B,1,0,1)
#define EMP_NEQU(A,B) EMP_COMPARE_FUN(A,B,1,1,0)

// Possible bit values during computation are:
// 0 or 1 (normal bits)
// 2 (after an ADD)
// N (for -1; after a SUB)


#define EMP_MATH_COUNT_BITS(A,B) EMP_MATH_COUNT_BITS_IMPL(A,B)
#define EMP_MATH_COUNT_BITS_IMPL(A,B) EMP_MATH_COUNT_BITS_ ## A ## B
#define EMP_MATH_COUNT_BITS_00 0
#define EMP_MATH_COUNT_BITS_01 1
#define EMP_MATH_COUNT_BITS_10 1
#define EMP_MATH_COUNT_BITS_11 2
#define EMP_MATH_COUNT_BITS_0N N
#define EMP_MATH_COUNT_BITS_1N 0

#define EMP_MATH_DIFF_BITS(A,B) EMP_MATH_DIFF_BITS_IMPL(A,B)
#define EMP_MATH_DIFF_BITS_IMPL(A,B) EMP_MATH_DIFF_BITS_ ## A ## B
#define EMP_MATH_DIFF_BITS_00 0
#define EMP_MATH_DIFF_BITS_01 N
#define EMP_MATH_DIFF_BITS_10 1
#define EMP_MATH_DIFF_BITS_11 0

#define EMP_MATH_GET_CARRY(A) EMP_MATH_GET_CARRY_IMPL(A)
#define EMP_MATH_GET_CARRY_IMPL(A) EMP_MATH_GET_CARRY_ ## A
#define EMP_MATH_GET_CARRY_0 0
#define EMP_MATH_GET_CARRY_1 0
#define EMP_MATH_GET_CARRY_2 1
#define EMP_MATH_GET_CARRY_N N

#define EMP_MATH_CLEAR_CARRY(A) EMP_MATH_CLEAR_CARRY_IMPL(A)
#define EMP_MATH_CLEAR_CARRY_IMPL(A) EMP_MATH_CLEAR_CARRY_ ## A
#define EMP_MATH_CLEAR_CARRY_0 0
#define EMP_MATH_CLEAR_CARRY_1 1
#define EMP_MATH_CLEAR_CARRY_2 0
#define EMP_MATH_CLEAR_CARRY_N 1

// In order to handle carrys, we need to run RESTORE_BIN once per bit (-1)

#define EMP_MATH_RESTORE_BIN(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9)  \
  EMP_MATH_RESTORE_BIN_1(                                             \
    EMP_MATH_COUNT_BITS(EMP_MATH_CLEAR_CARRY(A0), EMP_MATH_GET_CARRY(A1)), \
    EMP_MATH_COUNT_BITS(EMP_MATH_CLEAR_CARRY(A1), EMP_MATH_GET_CARRY(A2)), \
    EMP_MATH_COUNT_BITS(EMP_MATH_CLEAR_CARRY(A2), EMP_MATH_GET_CARRY(A3)), \
    EMP_MATH_COUNT_BITS(EMP_MATH_CLEAR_CARRY(A3), EMP_MATH_GET_CARRY(A4)), \
    EMP_MATH_COUNT_BITS(EMP_MATH_CLEAR_CARRY(A4), EMP_MATH_GET_CARRY(A5)), \
    EMP_MATH_COUNT_BITS(EMP_MATH_CLEAR_CARRY(A5), EMP_MATH_GET_CARRY(A6)), \
    EMP_MATH_COUNT_BITS(EMP_MATH_CLEAR_CARRY(A6), EMP_MATH_GET_CARRY(A7)), \
    EMP_MATH_COUNT_BITS(EMP_MATH_CLEAR_CARRY(A7), EMP_MATH_GET_CARRY(A8)), \
    EMP_MATH_COUNT_BITS(EMP_MATH_CLEAR_CARRY(A8), EMP_MATH_GET_CARRY(A9)), \
    EMP_MATH_CLEAR_CARRY(A9)                                               \
  )

#define EMP_MATH_RESTORE_BIN_1(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9)  \
  EMP_MATH_RESTORE_BIN_2(                                             \
    EMP_MATH_COUNT_BITS(EMP_MATH_CLEAR_CARRY(A0), EMP_MATH_GET_CARRY(A1)), \
    EMP_MATH_COUNT_BITS(EMP_MATH_CLEAR_CARRY(A1), EMP_MATH_GET_CARRY(A2)), \
    EMP_MATH_COUNT_BITS(EMP_MATH_CLEAR_CARRY(A2), EMP_MATH_GET_CARRY(A3)), \
    EMP_MATH_COUNT_BITS(EMP_MATH_CLEAR_CARRY(A3), EMP_MATH_GET_CARRY(A4)), \
    EMP_MATH_COUNT_BITS(EMP_MATH_CLEAR_CARRY(A4), EMP_MATH_GET_CARRY(A5)), \
    EMP_MATH_COUNT_BITS(EMP_MATH_CLEAR_CARRY(A5), EMP_MATH_GET_CARRY(A6)), \
    EMP_MATH_COUNT_BITS(EMP_MATH_CLEAR_CARRY(A6), EMP_MATH_GET_CARRY(A7)), \
    EMP_MATH_COUNT_BITS(EMP_MATH_CLEAR_CARRY(A7), EMP_MATH_GET_CARRY(A8)), \
    EMP_MATH_CLEAR_CARRY(A8),                                              \
    A9                                                                     \
  )

#define EMP_MATH_RESTORE_BIN_2(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9)   \
  EMP_MATH_RESTORE_BIN_3(                                             \
    EMP_MATH_COUNT_BITS(EMP_MATH_CLEAR_CARRY(A0), EMP_MATH_GET_CARRY(A1)), \
    EMP_MATH_COUNT_BITS(EMP_MATH_CLEAR_CARRY(A1), EMP_MATH_GET_CARRY(A2)), \
    EMP_MATH_COUNT_BITS(EMP_MATH_CLEAR_CARRY(A2), EMP_MATH_GET_CARRY(A3)), \
    EMP_MATH_COUNT_BITS(EMP_MATH_CLEAR_CARRY(A3), EMP_MATH_GET_CARRY(A4)), \
    EMP_MATH_COUNT_BITS(EMP_MATH_CLEAR_CARRY(A4), EMP_MATH_GET_CARRY(A5)), \
    EMP_MATH_COUNT_BITS(EMP_MATH_CLEAR_CARRY(A5), EMP_MATH_GET_CARRY(A6)), \
    EMP_MATH_COUNT_BITS(EMP_MATH_CLEAR_CARRY(A6), EMP_MATH_GET_CARRY(A7)), \
    EMP_MATH_CLEAR_CARRY(A7),                                           \
    A8,                                                                 \
    A9                                                                     \
  )

#define EMP_MATH_RESTORE_BIN_3(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9)   \
  EMP_MATH_RESTORE_BIN_4(                                             \
    EMP_MATH_COUNT_BITS(EMP_MATH_CLEAR_CARRY(A0), EMP_MATH_GET_CARRY(A1)), \
    EMP_MATH_COUNT_BITS(EMP_MATH_CLEAR_CARRY(A1), EMP_MATH_GET_CARRY(A2)), \
    EMP_MATH_COUNT_BITS(EMP_MATH_CLEAR_CARRY(A2), EMP_MATH_GET_CARRY(A3)), \
    EMP_MATH_COUNT_BITS(EMP_MATH_CLEAR_CARRY(A3), EMP_MATH_GET_CARRY(A4)), \
    EMP_MATH_COUNT_BITS(EMP_MATH_CLEAR_CARRY(A4), EMP_MATH_GET_CARRY(A5)), \
    EMP_MATH_COUNT_BITS(EMP_MATH_CLEAR_CARRY(A5), EMP_MATH_GET_CARRY(A6)), \
    EMP_MATH_CLEAR_CARRY(A6),                                         \
    A7,                                                               \
    A8,                                                                 \
    A9                                                                     \
  )

#define EMP_MATH_RESTORE_BIN_4(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9)   \
  EMP_MATH_RESTORE_BIN_5(                                             \
    EMP_MATH_COUNT_BITS(EMP_MATH_CLEAR_CARRY(A0), EMP_MATH_GET_CARRY(A1)), \
    EMP_MATH_COUNT_BITS(EMP_MATH_CLEAR_CARRY(A1), EMP_MATH_GET_CARRY(A2)), \
    EMP_MATH_COUNT_BITS(EMP_MATH_CLEAR_CARRY(A2), EMP_MATH_GET_CARRY(A3)), \
    EMP_MATH_COUNT_BITS(EMP_MATH_CLEAR_CARRY(A3), EMP_MATH_GET_CARRY(A4)), \
    EMP_MATH_COUNT_BITS(EMP_MATH_CLEAR_CARRY(A4), EMP_MATH_GET_CARRY(A5)), \
    EMP_MATH_CLEAR_CARRY(A5),                                         \
    A6,                                                               \
    A7,                                                               \
    A8,                                                                 \
    A9                                                                     \
  )

#define EMP_MATH_RESTORE_BIN_5(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9)   \
  EMP_MATH_RESTORE_BIN_6(                                             \
    EMP_MATH_COUNT_BITS(EMP_MATH_CLEAR_CARRY(A0), EMP_MATH_GET_CARRY(A1)), \
    EMP_MATH_COUNT_BITS(EMP_MATH_CLEAR_CARRY(A1), EMP_MATH_GET_CARRY(A2)), \
    EMP_MATH_COUNT_BITS(EMP_MATH_CLEAR_CARRY(A2), EMP_MATH_GET_CARRY(A3)), \
    EMP_MATH_COUNT_BITS(EMP_MATH_CLEAR_CARRY(A3), EMP_MATH_GET_CARRY(A4)), \
    EMP_MATH_CLEAR_CARRY(A4),                                         \
    A5,                                                               \
    A6,                                                               \
    A7,                                                               \
    A8,                                                                 \
    A9                                                                     \
  )

#define EMP_MATH_RESTORE_BIN_6(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9)   \
  EMP_MATH_RESTORE_BIN_7(                                             \
    EMP_MATH_COUNT_BITS(EMP_MATH_CLEAR_CARRY(A0), EMP_MATH_GET_CARRY(A1)), \
    EMP_MATH_COUNT_BITS(EMP_MATH_CLEAR_CARRY(A1), EMP_MATH_GET_CARRY(A2)), \
    EMP_MATH_COUNT_BITS(EMP_MATH_CLEAR_CARRY(A2), EMP_MATH_GET_CARRY(A3)), \
    EMP_MATH_CLEAR_CARRY(A3),                                         \
    A4,                                                               \
    A5,                                                               \
    A6,                                                               \
    A7,                                                               \
    A8,                                                                 \
    A9                                                                     \
  )

#define EMP_MATH_RESTORE_BIN_7(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9)   \
  EMP_MATH_RESTORE_BIN_8(                                             \
    EMP_MATH_COUNT_BITS(EMP_MATH_CLEAR_CARRY(A0), EMP_MATH_GET_CARRY(A1)), \
    EMP_MATH_COUNT_BITS(EMP_MATH_CLEAR_CARRY(A1), EMP_MATH_GET_CARRY(A2)), \
    EMP_MATH_CLEAR_CARRY(A2),                                           \
    A3,                                                                 \
    A4,                                                                 \
    A5,                                                                 \
    A6,                                                                 \
    A7,                                                                 \
    A8,                                                                 \
    A9                                                                     \
  )

#define EMP_MATH_RESTORE_BIN_8(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9)   \
  EMP_MATH_RESTORE_BIN_9(                                             \
    EMP_MATH_COUNT_BITS(EMP_MATH_CLEAR_CARRY(A0), EMP_MATH_GET_CARRY(A1)), \
    EMP_MATH_CLEAR_CARRY(A1),                                           \
    A2,                                                                 \
    A3,                                                                 \
    A4,                                                                 \
    A5,                                                                 \
    A6,                                                                 \
    A7,                                                                 \
    A8,                                                                 \
    A9                                                                     \
  )

#define EMP_MATH_RESTORE_BIN_9(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9)     \
    EMP_MATH_CLEAR_CARRY(A0),                                         \
    A1,                                                               \
    A2,                                                               \
    A3,                                                               \
    A4,                                                               \
    A5,                                                               \
    A6,                                                               \
    A7,                                                               \
    A8,                                                               \
    A9

/// @endcond

///////////////////////////
//
//  Math Operations!!!
//

// --- Shifting ---
/// @cond MACROS
#define EMP_SHIFTL_BIN(A0,A1,A2,A3,A4,A5,A6,A7,A8,A9) A1, A2, A3, A4, A5, A6, A7, A8, A9, 0
#define EMP_SHIFTR_BIN(A0,A1,A2,A3,A4,A5,A6,A7,A8,A9) 0, A0, A1, A2, A3, A4, A5, A6, A7, A8

#define EMP_SHIFTL_BIN_0(A0,A1,A2,A3,A4,A5,A6,A7,A8,A9) A0, A1, A2, A3, A4, A5, A6, A7, A8, A9
#define EMP_SHIFTR_BIN_0(A0,A1,A2,A3,A4,A5,A6,A7,A8,A9) A0, A1, A2, A3, A4, A5, A6, A7, A8, A9

#define EMP_SHIFTL_BIN_1(A0,A1,A2,A3,A4,A5,A6,A7,A8,A9) A1, A2, A3, A4, A5, A6, A7, A8, A9, 0
#define EMP_SHIFTR_BIN_1(A0,A1,A2,A3,A4,A5,A6,A7,A8,A9) 0, A0, A1, A2, A3, A4, A5, A6, A7, A8

#define EMP_SHIFTL_BIN_2(A0,A1,A2,A3,A4,A5,A6,A7,A8,A9) A2, A3, A4, A5, A6, A7, A8, A9, 0, 0
#define EMP_SHIFTR_BIN_2(A0,A1,A2,A3,A4,A5,A6,A7,A8,A9) 0, 0, A0, A1, A2, A3, A4, A5, A6, A7

#define EMP_SHIFTL_BIN_3(A0,A1,A2,A3,A4,A5,A6,A7,A8,A9) A3, A4, A5, A6, A7, A8, A9, 0, 0, 0
#define EMP_SHIFTR_BIN_3(A0,A1,A2,A3,A4,A5,A6,A7,A8,A9) 0, 0, 0, A0, A1, A2, A3, A4, A5, A6

#define EMP_SHIFTL_BIN_4(A0,A1,A2,A3,A4,A5,A6,A7,A8,A9) A4, A5, A6, A7, A8, A9, 0, 0, 0, 0
#define EMP_SHIFTR_BIN_4(A0,A1,A2,A3,A4,A5,A6,A7,A8,A9) 0, 0, 0, 0, A0, A1, A2, A3, A4, A5

#define EMP_SHIFTL_BIN_5(A0,A1,A2,A3,A4,A5,A6,A7,A8,A9) A5, A6, A7, A8, A9, 0, 0, 0, 0, 0
#define EMP_SHIFTR_BIN_5(A0,A1,A2,A3,A4,A5,A6,A7,A8,A9) 0, 0, 0, 0, 0, A0, A1, A2, A3, A4

#define EMP_SHIFTL_BIN_6(A0,A1,A2,A3,A4,A5,A6,A7,A8,A9) A6, A7, A8, A9, 0, 0, 0, 0, 0, 0
#define EMP_SHIFTR_BIN_6(A0,A1,A2,A3,A4,A5,A6,A7,A8,A9) 0, 0, 0, 0, 0, 0, A0, A1, A2, A3

#define EMP_SHIFTL_BIN_7(A0,A1,A2,A3,A4,A5,A6,A7,A8,A9) A7, A8, A9, 0, 0, 0, 0, 0, 0, 0
#define EMP_SHIFTR_BIN_7(A0,A1,A2,A3,A4,A5,A6,A7,A8,A9) 0, 0, 0, 0, 0, 0, 0, A0, A1, A2

#define EMP_SHIFTL_BIN_8(A0,A1,A2,A3,A4,A5,A6,A7,A8,A9) A8, A9, 0, 0, 0, 0, 0, 0, 0, 0
#define EMP_SHIFTR_BIN_8(A0,A1,A2,A3,A4,A5,A6,A7,A8,A9) 0, 0, 0, 0, 0, 0, 0, 0, A0, A1

#define EMP_SHIFTL_BIN_9(A0,A1,A2,A3,A4,A5,A6,A7,A8,A9) A9, 0, 0, 0, 0, 0, 0, 0, 0, 0
#define EMP_SHIFTR_BIN_9(A0,A1,A2,A3,A4,A5,A6,A7,A8,A9) 0, 0, 0, 0, 0, 0, 0, 0, 0, A0

#define EMP_SHIFTL_BIN_10(A0,A1,A2,A3,A4,A5,A6,A7,A8,A9) 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
#define EMP_SHIFTR_BIN_10(A0,A1,A2,A3,A4,A5,A6,A7,A8,A9) 0, 0, 0, 0, 0, 0, 0, 0, 0, 0

/// @endcond

/// Shift A left (i.e. multiply by 2)
#define EMP_SHIFTL(A) EMP_SHIFTL_IMPL( EMP_DEC_TO_BIN(A) )
/// @cond MACROS
#define EMP_SHIFTL_IMPL(...) EMP_BIN_TO_DEC( EMP_SHIFTL_BIN( __VA_ARGS__ ) )
/// @endcond

/// Shift A left by X
#define EMP_SHIFTL_X(X, A) EMP_SHIFTL_X_IMPL( X, EMP_DEC_TO_BIN(A) )
/// @cond MACROS
#define EMP_SHIFTL_X_IMPL(X, ...) EMP_BIN_TO_DEC( EMP_SHIFTL_BIN_##X( __VA_ARGS__ ) )
/// @endcond

/// Shift A  (i.e. divide by 2)
#define EMP_SHIFTR(A) EMP_SHIFTR_IMPL( EMP_DEC_TO_BIN(A) )
/// @cond MACROS
#define EMP_SHIFTR_IMPL(...) EMP_BIN_TO_DEC( EMP_SHIFTR_BIN( __VA_ARGS__ ) )
/// @endcond

/// Shift A right by X
#define EMP_SHIFTR_X(X, A) EMP_SHIFTR_X_IMPL( X, EMP_DEC_TO_BIN(A) )
/// @cond MACROS
#define EMP_SHIFTR_X_IMPL(X, ...) EMP_BIN_TO_DEC( EMP_EVAL(EMP_SHIFTR_BIN_##X( __VA_ARGS__ )) )
/// @endcond

// --- Addition ---
/// @cond MACROS
#define EMP_ADD_BIN(...) EMP_ADD_BIN_IMPL EMP_EMPTY() (__VA_ARGS__)

#define EMP_ADD_BIN_IMPL(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9,        \
                         B0, B1, B2, B3, B4, B5, B6, B7, B8, B9)        \
  EMP_MATH_RESTORE_BIN( EMP_MATH_COUNT_BITS(A0, B0), EMP_MATH_COUNT_BITS(A1, B1), \
                        EMP_MATH_COUNT_BITS(A2, B2), EMP_MATH_COUNT_BITS(A3, B3), \
                        EMP_MATH_COUNT_BITS(A4, B4), EMP_MATH_COUNT_BITS(A5, B5), \
                        EMP_MATH_COUNT_BITS(A6, B6), EMP_MATH_COUNT_BITS(A7, B7), \
                        EMP_MATH_COUNT_BITS(A8, B8), EMP_MATH_COUNT_BITS(A9, B9)  \
                        )

#define EMP_ADD_BIN_4(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9,        \
                      B0, B1, B2, B3, B4, B5, B6, B7, B8, B9, ...)   \
  EMP_ADD_BIN( EMP_ADD_BIN(A0,A1,A2,A3,A4,A5,A6,A7,A8,A9,B0,B1,B2,B3,B4,B5,B6,B7,B8,B9), \
               EMP_ADD_BIN(__VA_ARGS__) )

#define EMP_ADD_BIN_8(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9,        \
                      B0, B1, B2, B3, B4, B5, B6, B7, B8, B9,        \
                      C0, C1, C2, C3, C4, C5, C6, C7, C8, C9,        \
                      D0, D1, D2, D3, D4, D5, D6, D7, D8, D9, ...)   \
  EMP_ADD_BIN( EMP_ADD_BIN_4(A0,A1,A2,A3,A4,A5,A6,A7,A8,A9,B0,B1,B2,B3,B4,B5,B6,B7,B8,B9,  \
                             C0,C1,C2,C3,C4,C5,C6,C7,C8,C9,D0,D1,D2,D3,D4,D5,D6,D7,D8,D9), \
               EMP_ADD_BIN_4(__VA_ARGS__) )

#define EMP_ADD_BIN_10(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9,          \
                       B0, B1, B2, B3, B4, B5, B6, B7, B8, B9, ...)     \
  EMP_ADD_BIN( EMP_ADD_BIN(A0,A1,A2,A3,A4,A5,A6,A7,A8,A9,B0,B1,B2,B3,B4,B5,B6,B7,B8,B9), \
               EMP_ADD_BIN_8(__VA_ARGS__) )
/// @endcond

/// Add A to B
#define EMP_ADD(A, B) EMP_EVAL( EMP_ADD_IMPL( EMP_DEC_TO_BIN(A), EMP_DEC_TO_BIN(B) ) )

/// @cond MACROS
#define EMP_ADD_IMPL(...) EMP_BIN_TO_DEC( EMP_ADD_BIN_IMPL( __VA_ARGS__ ) )

#define EMP_ADD_10(A,B,C,D,E,F,G,H,I,J) EMP_EVAL( EMP_ADD_10_IMPL( EMP_DEC_TO_BIN(A), EMP_DEC_TO_BIN(B), EMP_DEC_TO_BIN(C), EMP_DEC_TO_BIN(D), EMP_DEC_TO_BIN(E), EMP_DEC_TO_BIN(F), EMP_DEC_TO_BIN(G), EMP_DEC_TO_BIN(H), EMP_DEC_TO_BIN(I), EMP_DEC_TO_BIN(J) ) )
#define EMP_ADD_10_IMPL(...) EMP_BIN_TO_DEC( EMP_ADD_BIN_10( __VA_ARGS__ ) )
/// @endcond

// --- Subtraction ---

/// @cond MACROS
#define EMP_SUB_BIN(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9,             \
                    B0, B1, B2, B3, B4, B5, B6, B7, B8, B9)             \
  EMP_MATH_RESTORE_BIN( EMP_MATH_DIFF_BITS(A0, B0), EMP_MATH_DIFF_BITS(A1, B1), \
                        EMP_MATH_DIFF_BITS(A2, B2), EMP_MATH_DIFF_BITS(A3, B3), \
                        EMP_MATH_DIFF_BITS(A4, B4), EMP_MATH_DIFF_BITS(A5, B5), \
                        EMP_MATH_DIFF_BITS(A6, B6), EMP_MATH_DIFF_BITS(A7, B7), \
                        EMP_MATH_DIFF_BITS(A8, B8), EMP_MATH_DIFF_BITS(A9, B9)  \
                        )
/// @endcond
/// Combute A - B
#define EMP_SUB(A, B) EMP_EVAL( EMP_SUB_IMPL( EMP_DEC_TO_BIN(A), EMP_DEC_TO_BIN(B) ) )

/// @cond MACROS
#define EMP_SUB_IMPL(...) EMP_BIN_TO_DEC( EMP_SUB_BIN( __VA_ARGS__ ) )
/// @endcond

// --- Increment and Decrement ---
/// Increment A
#define EMP_INC(A) EMP_ADD(A, 1)
/// Decrement A
#define EMP_DEC(A) EMP_SUB(A, 1)
/// Cut A in half
#define EMP_HALF(A) EMP_SHIFTR(A)


// --- Multiply ---
/// @cond MACROS
#define EMP_MULT_BIN(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9,   \
                     B0, B1, B2, B3, B4, B5, B6, B7, B8, B9)   \
  EMP_ADD_BIN_10 EMP_EMPTY() (                                          \
                 EMP_MATH_BIN_TIMES_ ## B9 (A0, A1, A2, A3, A4, A5, A6, A7, A8, A9), \
                 EMP_MATH_BIN_TIMES_ ## B8 (A1, A2, A3, A4, A5, A6, A7, A8, A9,  0), \
                 EMP_MATH_BIN_TIMES_ ## B7 (A2, A3, A4, A5, A6, A7, A8, A9,  0,  0), \
                 EMP_MATH_BIN_TIMES_ ## B6 (A3, A4, A5, A6, A7, A8, A9,  0,  0,  0), \
                 EMP_MATH_BIN_TIMES_ ## B5 (A4, A5, A6, A7, A8, A9,  0,  0,  0,  0), \
                 EMP_MATH_BIN_TIMES_ ## B4 (A5, A6, A7, A8, A9,  0,  0,  0,  0,  0), \
                 EMP_MATH_BIN_TIMES_ ## B3 (A6, A7, A8, A9,  0,  0,  0,  0,  0,  0), \
                 EMP_MATH_BIN_TIMES_ ## B2 (A7, A8, A9,  0,  0,  0,  0,  0,  0,  0), \
                 EMP_MATH_BIN_TIMES_ ## B1 (A8, A9,  0,  0,  0,  0,  0,  0,  0,  0), \
                 EMP_MATH_BIN_TIMES_ ## B0 (A9,  0,  0,  0,  0,  0,  0,  0,  0,  0) \
  )
/// @endcond
/// Compute A * B
#define EMP_MULT(A, B) EMP_EVAL( EMP_MULT_IMPL( EMP_DEC_TO_BIN(A), EMP_DEC_TO_BIN(B) ) )

/// @cond MACROS
#define EMP_MULT_IMPL(...) EMP_BIN_TO_DEC EMP_EMPTY() ( EMP_MULT_BIN( __VA_ARGS__ ) )
/// @endcond

// --- Bit Manipulation ---
/// Use bit magic to count the number of 1s in the binary representation of A
#define EMP_COUNT_ONES(A) EMP_ADD_10 EMP_EMPTY() ( EMP_DEC_TO_BIN_ ## A )

// --- Log2 ---
/// @cond MACROS
#define EMP_LOG2_BIN(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9) EMP_IF(A0, 10, EMP_IF(A1, 9, EMP_IF(A2, 8, EMP_IF(A3, 7, EMP_IF(A4, 6, EMP_IF(A5, 5, EMP_IF(A6, 4, EMP_IF(A7, 3, EMP_IF(A8, 2, EMP_IF(A9, 1, 0) ) ) ) ) ) ) ) ) )
/// @endcond
/// Compute log base 2 of A
#define EMP_LOG2(A) EMP_EVAL( EMP_LOG2_IMPL( EMP_DEC_TO_BIN(A) ) )

/// @cond MACROS
#define EMP_LOG2_IMPL(...) EMP_LOG2_BIN( __VA_ARGS__)
/// @endcond

// --- Division ---

/// @cond MACROS
#define EMP_DIV_start(B) EMP_SUB(10, EMP_LOG2(B))
/// @endcond
/// Compute A / B
#define EMP_DIV(A, B) EMP_IF( EMP_EQU(B,0), DIV_BY_ZERO_ERROR,          \
                              EMP_DIV_impl(EMP_DIV_start(B), A,B) )

/// @cond MACROS
#define EMP_DIV_impl(START, A, B) EMP_DIV_implB(START, A, B)
#define EMP_DIV_implB(START, A, B) EMP_DIV_impl_ ## START(A, EMP_SHIFTL_X(START, B) )


#define EMP_DIV_impl_0(A, B) EMP_IF( EMP_LESS(A, B), 0, 1)
#define EMP_DIV_impl_1(A, B) EMP_IF( EMP_LESS(A, B),                    \
                                     EMP_DIV_impl_0(A, EMP_SHIFTR(B)),  \
                                     EMP_ADD(2, EMP_DIV_impl_0( EMP_SUB(A,B), EMP_SHIFTR(B) )) )
#define EMP_DIV_impl_2(A, B) EMP_IF( EMP_LESS(A, B),                    \
                                     EMP_DIV_impl_1(A, EMP_SHIFTR(B)),  \
                                     EMP_ADD(4, EMP_DIV_impl_1( EMP_SUB(A,B), EMP_SHIFTR(B) )) )
#define EMP_DIV_impl_3(A, B) EMP_IF( EMP_LESS(A, B),                    \
                                     EMP_DIV_impl_2(A, EMP_SHIFTR(B)),  \
                                     EMP_ADD(8, EMP_DIV_impl_2( EMP_SUB(A,B), EMP_SHIFTR(B) )) )
#define EMP_DIV_impl_4(A, B) EMP_IF( EMP_LESS(A, B),                    \
                                     EMP_DIV_impl_3(A, EMP_SHIFTR(B)),  \
                                     EMP_ADD(16, EMP_DIV_impl_3( EMP_SUB(A,B), EMP_SHIFTR(B) )) )
#define EMP_DIV_impl_5(A, B) EMP_IF( EMP_LESS(A, B),                    \
                                     EMP_DIV_impl_4(A, EMP_SHIFTR(B)),  \
                                     EMP_ADD(32, EMP_DIV_impl_4( EMP_SUB(A,B), EMP_SHIFTR(B) )) )
#define EMP_DIV_impl_6(A, B) EMP_IF( EMP_LESS(A, B),                    \
                                     EMP_DIV_impl_5(A, EMP_SHIFTR(B)),  \
                                     EMP_ADD(64, EMP_DIV_impl_5( EMP_SUB(A,B), EMP_SHIFTR(B) )) )
#define EMP_DIV_impl_7(A, B) EMP_IF( EMP_LESS(A, B),                    \
                                     EMP_DIV_impl_6(A, EMP_SHIFTR(B)),  \
                                     EMP_ADD(128, EMP_DIV_impl_6( EMP_SUB(A,B), EMP_SHIFTR(B) )) )
#define EMP_DIV_impl_8(A, B) EMP_IF( EMP_LESS(A, B),                    \
                                     EMP_DIV_impl_7(A, EMP_SHIFTR(B)),  \
                                     EMP_ADD(256, EMP_DIV_impl_7( EMP_SUB(A,B), EMP_SHIFTR(B) )) )
#define EMP_DIV_impl_9(A, B) EMP_IF( EMP_LESS(A, B),                    \
                                     EMP_DIV_impl_8(A, EMP_SHIFTR(B)),  \
                                     EMP_ADD(512, EMP_DIV_impl_8( EMP_SUB(A,B), EMP_SHIFTR(B) )) )

/// @endcond

// --- Modulus ---
/// Compute A % B
#define EMP_MOD(A, B) EMP_MOD_impl(A, B, EMP_DIV(A, B))

/// @cond MACROS

#define EMP_MOD_impl(A, B, D) EMP_MOD_impl2( A, EMP_MULT(D, B))
#define EMP_MOD_impl2(A, M) EMP_SUB(A, M)

/// @endcond

#endif
