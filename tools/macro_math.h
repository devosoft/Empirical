#ifndef EMP_MACRO_MATH_H
#define EMP_MACRO_MATH_H

//////////////////////////////////////////////////////////////////////////////////////////
//
//  These macros build a pre-processor calculator system.
//
//  Working macros include:
//   EMP_INC(A)    : converts to result of A+1
//   EMP_DEC(A)    : converts to result of A-1
//   EMP_SHIFTL(A) : converts to result of A*2
//   EMP_SHIFTR(A) : converts to result of A/2
//
//   EMP_ADD(A,B)  : converts to result of A+B
//   EMP_SUB(A,B)  : converts to result of A-B
//   EMP_MULT(A,B) : converts to result of A*B
//
//  The core idea behind these macros is that we can use brute-force to convert numbers
//  to binary, but once there we can easily perform math on them, or convert them to
//  yet other forms.
//
//  Representations include:
//   DEC - Standard decimal values (e.g., 91)
//   BIN - Binary numbers, with bits separated by commas (e.g.  0,  1, 0,  1, 1, 0, 1, 1 )
//   SUM - Like BIN, but stored as zero or magnitude.    (e.g., 0, 64, 0, 16, 8, 0, 2, 1
//   HEX - Hexidecimal representation (e.g., 0x5B)  [todo]
//

#define EMP_EMPTY

#define EMP_DEC_TO_BIN(VAL) EMP_DEC_TO_BIN_ ## VAL

#define EMP_DEC_TO_BIN_0    0, 0, 0, 0, 0, 0, 0, 0
#define EMP_DEC_TO_BIN_1    0, 0, 0, 0, 0, 0, 0, 1
#define EMP_DEC_TO_BIN_2    0, 0, 0, 0, 0, 0, 1, 0
#define EMP_DEC_TO_BIN_3    0, 0, 0, 0, 0, 0, 1, 1
#define EMP_DEC_TO_BIN_4    0, 0, 0, 0, 0, 1, 0, 0
#define EMP_DEC_TO_BIN_5    0, 0, 0, 0, 0, 1, 0, 1
#define EMP_DEC_TO_BIN_6    0, 0, 0, 0, 0, 1, 1, 0
#define EMP_DEC_TO_BIN_7    0, 0, 0, 0, 0, 1, 1, 1
#define EMP_DEC_TO_BIN_8    0, 0, 0, 0, 1, 0, 0, 0
#define EMP_DEC_TO_BIN_9    0, 0, 0, 0, 1, 0, 0, 1
#define EMP_DEC_TO_BIN_10   0, 0, 0, 0, 1, 0, 1, 0
#define EMP_DEC_TO_BIN_11   0, 0, 0, 0, 1, 0, 1, 1
#define EMP_DEC_TO_BIN_12   0, 0, 0, 0, 1, 1, 0, 0
#define EMP_DEC_TO_BIN_13   0, 0, 0, 0, 1, 1, 0, 1
#define EMP_DEC_TO_BIN_14   0, 0, 0, 0, 1, 1, 1, 0
#define EMP_DEC_TO_BIN_15   0, 0, 0, 0, 1, 1, 1, 1
#define EMP_DEC_TO_BIN_16   0, 0, 0, 1, 0, 0, 0, 0
#define EMP_DEC_TO_BIN_17   0, 0, 0, 1, 0, 0, 0, 1
#define EMP_DEC_TO_BIN_18   0, 0, 0, 1, 0, 0, 1, 0
#define EMP_DEC_TO_BIN_19   0, 0, 0, 1, 0, 0, 1, 1
#define EMP_DEC_TO_BIN_20   0, 0, 0, 1, 0, 1, 0, 0
#define EMP_DEC_TO_BIN_21   0, 0, 0, 1, 0, 1, 0, 1
#define EMP_DEC_TO_BIN_22   0, 0, 0, 1, 0, 1, 1, 0
#define EMP_DEC_TO_BIN_23   0, 0, 0, 1, 0, 1, 1, 1
#define EMP_DEC_TO_BIN_24   0, 0, 0, 1, 1, 0, 0, 0
#define EMP_DEC_TO_BIN_25   0, 0, 0, 1, 1, 0, 0, 1
#define EMP_DEC_TO_BIN_26   0, 0, 0, 1, 1, 0, 1, 0
#define EMP_DEC_TO_BIN_27   0, 0, 0, 1, 1, 0, 1, 1
#define EMP_DEC_TO_BIN_28   0, 0, 0, 1, 1, 1, 0, 0
#define EMP_DEC_TO_BIN_29   0, 0, 0, 1, 1, 1, 0, 1
#define EMP_DEC_TO_BIN_30   0, 0, 0, 1, 1, 1, 1, 0
#define EMP_DEC_TO_BIN_31   0, 0, 0, 1, 1, 1, 1, 1
#define EMP_DEC_TO_BIN_32   0, 0, 1, 0, 0, 0, 0, 0
#define EMP_DEC_TO_BIN_33   0, 0, 1, 0, 0, 0, 0, 1
#define EMP_DEC_TO_BIN_34   0, 0, 1, 0, 0, 0, 1, 0
#define EMP_DEC_TO_BIN_35   0, 0, 1, 0, 0, 0, 1, 1
#define EMP_DEC_TO_BIN_36   0, 0, 1, 0, 0, 1, 0, 0
#define EMP_DEC_TO_BIN_37   0, 0, 1, 0, 0, 1, 0, 1
#define EMP_DEC_TO_BIN_38   0, 0, 1, 0, 0, 1, 1, 0
#define EMP_DEC_TO_BIN_39   0, 0, 1, 0, 0, 1, 1, 1
#define EMP_DEC_TO_BIN_40   0, 0, 1, 0, 1, 0, 0, 0
#define EMP_DEC_TO_BIN_41   0, 0, 1, 0, 1, 0, 0, 1
#define EMP_DEC_TO_BIN_42   0, 0, 1, 0, 1, 0, 1, 0
#define EMP_DEC_TO_BIN_43   0, 0, 1, 0, 1, 0, 1, 1
#define EMP_DEC_TO_BIN_44   0, 0, 1, 0, 1, 1, 0, 0
#define EMP_DEC_TO_BIN_45   0, 0, 1, 0, 1, 1, 0, 1
#define EMP_DEC_TO_BIN_46   0, 0, 1, 0, 1, 1, 1, 0
#define EMP_DEC_TO_BIN_47   0, 0, 1, 0, 1, 1, 1, 1
#define EMP_DEC_TO_BIN_48   0, 0, 1, 1, 0, 0, 0, 0
#define EMP_DEC_TO_BIN_49   0, 0, 1, 1, 0, 0, 0, 1
#define EMP_DEC_TO_BIN_50   0, 0, 1, 1, 0, 0, 1, 0
#define EMP_DEC_TO_BIN_51   0, 0, 1, 1, 0, 0, 1, 1
#define EMP_DEC_TO_BIN_52   0, 0, 1, 1, 0, 1, 0, 0
#define EMP_DEC_TO_BIN_53   0, 0, 1, 1, 0, 1, 0, 1
#define EMP_DEC_TO_BIN_54   0, 0, 1, 1, 0, 1, 1, 0
#define EMP_DEC_TO_BIN_55   0, 0, 1, 1, 0, 1, 1, 1
#define EMP_DEC_TO_BIN_56   0, 0, 1, 1, 1, 0, 0, 0
#define EMP_DEC_TO_BIN_57   0, 0, 1, 1, 1, 0, 0, 1
#define EMP_DEC_TO_BIN_58   0, 0, 1, 1, 1, 0, 1, 0
#define EMP_DEC_TO_BIN_59   0, 0, 1, 1, 1, 0, 1, 1
#define EMP_DEC_TO_BIN_60   0, 0, 1, 1, 1, 1, 0, 0
#define EMP_DEC_TO_BIN_61   0, 0, 1, 1, 1, 1, 0, 1
#define EMP_DEC_TO_BIN_62   0, 0, 1, 1, 1, 1, 1, 0
#define EMP_DEC_TO_BIN_63   0, 0, 1, 1, 1, 1, 1, 1
#define EMP_DEC_TO_BIN_64   0, 1, 0, 0, 0, 0, 0, 0
#define EMP_DEC_TO_BIN_65   0, 1, 0, 0, 0, 0, 0, 1
#define EMP_DEC_TO_BIN_66   0, 1, 0, 0, 0, 0, 1, 0
#define EMP_DEC_TO_BIN_67   0, 1, 0, 0, 0, 0, 1, 1
#define EMP_DEC_TO_BIN_68   0, 1, 0, 0, 0, 1, 0, 0
#define EMP_DEC_TO_BIN_69   0, 1, 0, 0, 0, 1, 0, 1
#define EMP_DEC_TO_BIN_70   0, 1, 0, 0, 0, 1, 1, 0
#define EMP_DEC_TO_BIN_71   0, 1, 0, 0, 0, 1, 1, 1
#define EMP_DEC_TO_BIN_72   0, 1, 0, 0, 1, 0, 0, 0
#define EMP_DEC_TO_BIN_73   0, 1, 0, 0, 1, 0, 0, 1
#define EMP_DEC_TO_BIN_74   0, 1, 0, 0, 1, 0, 1, 0
#define EMP_DEC_TO_BIN_75   0, 1, 0, 0, 1, 0, 1, 1
#define EMP_DEC_TO_BIN_76   0, 1, 0, 0, 1, 1, 0, 0
#define EMP_DEC_TO_BIN_77   0, 1, 0, 0, 1, 1, 0, 1
#define EMP_DEC_TO_BIN_78   0, 1, 0, 0, 1, 1, 1, 0
#define EMP_DEC_TO_BIN_79   0, 1, 0, 0, 1, 1, 1, 1
#define EMP_DEC_TO_BIN_80   0, 1, 0, 1, 0, 0, 0, 0
#define EMP_DEC_TO_BIN_81   0, 1, 0, 1, 0, 0, 0, 1
#define EMP_DEC_TO_BIN_82   0, 1, 0, 1, 0, 0, 1, 0
#define EMP_DEC_TO_BIN_83   0, 1, 0, 1, 0, 0, 1, 1
#define EMP_DEC_TO_BIN_84   0, 1, 0, 1, 0, 1, 0, 0
#define EMP_DEC_TO_BIN_85   0, 1, 0, 1, 0, 1, 0, 1
#define EMP_DEC_TO_BIN_86   0, 1, 0, 1, 0, 1, 1, 0
#define EMP_DEC_TO_BIN_87   0, 1, 0, 1, 0, 1, 1, 1
#define EMP_DEC_TO_BIN_88   0, 1, 0, 1, 1, 0, 0, 0
#define EMP_DEC_TO_BIN_89   0, 1, 0, 1, 1, 0, 0, 1
#define EMP_DEC_TO_BIN_90   0, 1, 0, 1, 1, 0, 1, 0
#define EMP_DEC_TO_BIN_91   0, 1, 0, 1, 1, 0, 1, 1
#define EMP_DEC_TO_BIN_92   0, 1, 0, 1, 1, 1, 0, 0
#define EMP_DEC_TO_BIN_93   0, 1, 0, 1, 1, 1, 0, 1
#define EMP_DEC_TO_BIN_94   0, 1, 0, 1, 1, 1, 1, 0
#define EMP_DEC_TO_BIN_95   0, 1, 0, 1, 1, 1, 1, 1
#define EMP_DEC_TO_BIN_96   0, 1, 1, 0, 0, 0, 0, 0
#define EMP_DEC_TO_BIN_97   0, 1, 1, 0, 0, 0, 0, 1
#define EMP_DEC_TO_BIN_98   0, 1, 1, 0, 0, 0, 1, 0
#define EMP_DEC_TO_BIN_99   0, 1, 1, 0, 0, 0, 1, 1
#define EMP_DEC_TO_BIN_100  0, 1, 1, 0, 0, 1, 0, 0
#define EMP_DEC_TO_BIN_101  0, 1, 1, 0, 0, 1, 0, 1
#define EMP_DEC_TO_BIN_102  0, 1, 1, 0, 0, 1, 1, 0
#define EMP_DEC_TO_BIN_103  0, 1, 1, 0, 0, 1, 1, 1
#define EMP_DEC_TO_BIN_104  0, 1, 1, 0, 1, 0, 0, 0
#define EMP_DEC_TO_BIN_105  0, 1, 1, 0, 1, 0, 0, 1
#define EMP_DEC_TO_BIN_106  0, 1, 1, 0, 1, 0, 1, 0
#define EMP_DEC_TO_BIN_107  0, 1, 1, 0, 1, 0, 1, 1
#define EMP_DEC_TO_BIN_108  0, 1, 1, 0, 1, 1, 0, 0
#define EMP_DEC_TO_BIN_109  0, 1, 1, 0, 1, 1, 0, 1
#define EMP_DEC_TO_BIN_110  0, 1, 1, 0, 1, 1, 1, 0
#define EMP_DEC_TO_BIN_111  0, 1, 1, 0, 1, 1, 1, 1
#define EMP_DEC_TO_BIN_112  0, 1, 1, 1, 0, 0, 0, 0
#define EMP_DEC_TO_BIN_113  0, 1, 1, 1, 0, 0, 0, 1
#define EMP_DEC_TO_BIN_114  0, 1, 1, 1, 0, 0, 1, 0
#define EMP_DEC_TO_BIN_115  0, 1, 1, 1, 0, 0, 1, 1
#define EMP_DEC_TO_BIN_116  0, 1, 1, 1, 0, 1, 0, 0
#define EMP_DEC_TO_BIN_117  0, 1, 1, 1, 0, 1, 0, 1
#define EMP_DEC_TO_BIN_118  0, 1, 1, 1, 0, 1, 1, 0
#define EMP_DEC_TO_BIN_119  0, 1, 1, 1, 0, 1, 1, 1
#define EMP_DEC_TO_BIN_120  0, 1, 1, 1, 1, 0, 0, 0
#define EMP_DEC_TO_BIN_121  0, 1, 1, 1, 1, 0, 0, 1
#define EMP_DEC_TO_BIN_122  0, 1, 1, 1, 1, 0, 1, 0
#define EMP_DEC_TO_BIN_123  0, 1, 1, 1, 1, 0, 1, 1
#define EMP_DEC_TO_BIN_124  0, 1, 1, 1, 1, 1, 0, 0
#define EMP_DEC_TO_BIN_125  0, 1, 1, 1, 1, 1, 0, 1
#define EMP_DEC_TO_BIN_126  0, 1, 1, 1, 1, 1, 1, 0
#define EMP_DEC_TO_BIN_127  0, 1, 1, 1, 1, 1, 1, 1
#define EMP_DEC_TO_BIN_128  1, 0, 0, 0, 0, 0, 0, 0
#define EMP_DEC_TO_BIN_129  1, 0, 0, 0, 0, 0, 0, 1
#define EMP_DEC_TO_BIN_130  1, 0, 0, 0, 0, 0, 1, 0
#define EMP_DEC_TO_BIN_131  1, 0, 0, 0, 0, 0, 1, 1
#define EMP_DEC_TO_BIN_132  1, 0, 0, 0, 0, 1, 0, 0
#define EMP_DEC_TO_BIN_133  1, 0, 0, 0, 0, 1, 0, 1
#define EMP_DEC_TO_BIN_134  1, 0, 0, 0, 0, 1, 1, 0
#define EMP_DEC_TO_BIN_135  1, 0, 0, 0, 0, 1, 1, 1
#define EMP_DEC_TO_BIN_136  1, 0, 0, 0, 1, 0, 0, 0
#define EMP_DEC_TO_BIN_137  1, 0, 0, 0, 1, 0, 0, 1
#define EMP_DEC_TO_BIN_138  1, 0, 0, 0, 1, 0, 1, 0
#define EMP_DEC_TO_BIN_139  1, 0, 0, 0, 1, 0, 1, 1
#define EMP_DEC_TO_BIN_140  1, 0, 0, 0, 1, 1, 0, 0
#define EMP_DEC_TO_BIN_141  1, 0, 0, 0, 1, 1, 0, 1
#define EMP_DEC_TO_BIN_142  1, 0, 0, 0, 1, 1, 1, 0
#define EMP_DEC_TO_BIN_143  1, 0, 0, 0, 1, 1, 1, 1
#define EMP_DEC_TO_BIN_144  1, 0, 0, 1, 0, 0, 0, 0
#define EMP_DEC_TO_BIN_145  1, 0, 0, 1, 0, 0, 0, 1
#define EMP_DEC_TO_BIN_146  1, 0, 0, 1, 0, 0, 1, 0
#define EMP_DEC_TO_BIN_147  1, 0, 0, 1, 0, 0, 1, 1
#define EMP_DEC_TO_BIN_148  1, 0, 0, 1, 0, 1, 0, 0
#define EMP_DEC_TO_BIN_149  1, 0, 0, 1, 0, 1, 0, 1
#define EMP_DEC_TO_BIN_150  1, 0, 0, 1, 0, 1, 1, 0
#define EMP_DEC_TO_BIN_151  1, 0, 0, 1, 0, 1, 1, 1
#define EMP_DEC_TO_BIN_152  1, 0, 0, 1, 1, 0, 0, 0
#define EMP_DEC_TO_BIN_153  1, 0, 0, 1, 1, 0, 0, 1
#define EMP_DEC_TO_BIN_154  1, 0, 0, 1, 1, 0, 1, 0
#define EMP_DEC_TO_BIN_155  1, 0, 0, 1, 1, 0, 1, 1
#define EMP_DEC_TO_BIN_156  1, 0, 0, 1, 1, 1, 0, 0
#define EMP_DEC_TO_BIN_157  1, 0, 0, 1, 1, 1, 0, 1
#define EMP_DEC_TO_BIN_158  1, 0, 0, 1, 1, 1, 1, 0
#define EMP_DEC_TO_BIN_159  1, 0, 0, 1, 1, 1, 1, 1
#define EMP_DEC_TO_BIN_160  1, 0, 1, 0, 0, 0, 0, 0
#define EMP_DEC_TO_BIN_161  1, 0, 1, 0, 0, 0, 0, 1
#define EMP_DEC_TO_BIN_162  1, 0, 1, 0, 0, 0, 1, 0
#define EMP_DEC_TO_BIN_163  1, 0, 1, 0, 0, 0, 1, 1
#define EMP_DEC_TO_BIN_164  1, 0, 1, 0, 0, 1, 0, 0
#define EMP_DEC_TO_BIN_165  1, 0, 1, 0, 0, 1, 0, 1
#define EMP_DEC_TO_BIN_166  1, 0, 1, 0, 0, 1, 1, 0
#define EMP_DEC_TO_BIN_167  1, 0, 1, 0, 0, 1, 1, 1
#define EMP_DEC_TO_BIN_168  1, 0, 1, 0, 1, 0, 0, 0
#define EMP_DEC_TO_BIN_169  1, 0, 1, 0, 1, 0, 0, 1
#define EMP_DEC_TO_BIN_170  1, 0, 1, 0, 1, 0, 1, 0
#define EMP_DEC_TO_BIN_171  1, 0, 1, 0, 1, 0, 1, 1
#define EMP_DEC_TO_BIN_172  1, 0, 1, 0, 1, 1, 0, 0
#define EMP_DEC_TO_BIN_173  1, 0, 1, 0, 1, 1, 0, 1
#define EMP_DEC_TO_BIN_174  1, 0, 1, 0, 1, 1, 1, 0
#define EMP_DEC_TO_BIN_175  1, 0, 1, 0, 1, 1, 1, 1
#define EMP_DEC_TO_BIN_176  1, 0, 1, 1, 0, 0, 0, 0
#define EMP_DEC_TO_BIN_177  1, 0, 1, 1, 0, 0, 0, 1
#define EMP_DEC_TO_BIN_178  1, 0, 1, 1, 0, 0, 1, 0
#define EMP_DEC_TO_BIN_179  1, 0, 1, 1, 0, 0, 1, 1
#define EMP_DEC_TO_BIN_180  1, 0, 1, 1, 0, 1, 0, 0
#define EMP_DEC_TO_BIN_181  1, 0, 1, 1, 0, 1, 0, 1
#define EMP_DEC_TO_BIN_182  1, 0, 1, 1, 0, 1, 1, 0
#define EMP_DEC_TO_BIN_183  1, 0, 1, 1, 0, 1, 1, 1
#define EMP_DEC_TO_BIN_184  1, 0, 1, 1, 1, 0, 0, 0
#define EMP_DEC_TO_BIN_185  1, 0, 1, 1, 1, 0, 0, 1
#define EMP_DEC_TO_BIN_186  1, 0, 1, 1, 1, 0, 1, 0
#define EMP_DEC_TO_BIN_187  1, 0, 1, 1, 1, 0, 1, 1
#define EMP_DEC_TO_BIN_188  1, 0, 1, 1, 1, 1, 0, 0
#define EMP_DEC_TO_BIN_189  1, 0, 1, 1, 1, 1, 0, 1
#define EMP_DEC_TO_BIN_190  1, 0, 1, 1, 1, 1, 1, 0
#define EMP_DEC_TO_BIN_191  1, 0, 1, 1, 1, 1, 1, 1
#define EMP_DEC_TO_BIN_192  1, 1, 0, 0, 0, 0, 0, 0
#define EMP_DEC_TO_BIN_193  1, 1, 0, 0, 0, 0, 0, 1
#define EMP_DEC_TO_BIN_194  1, 1, 0, 0, 0, 0, 1, 0
#define EMP_DEC_TO_BIN_195  1, 1, 0, 0, 0, 0, 1, 1
#define EMP_DEC_TO_BIN_196  1, 1, 0, 0, 0, 1, 0, 0
#define EMP_DEC_TO_BIN_197  1, 1, 0, 0, 0, 1, 0, 1
#define EMP_DEC_TO_BIN_198  1, 1, 0, 0, 0, 1, 1, 0
#define EMP_DEC_TO_BIN_199  1, 1, 0, 0, 0, 1, 1, 1
#define EMP_DEC_TO_BIN_200  1, 1, 0, 0, 1, 0, 0, 0
#define EMP_DEC_TO_BIN_201  1, 1, 0, 0, 1, 0, 0, 1
#define EMP_DEC_TO_BIN_202  1, 1, 0, 0, 1, 0, 1, 0
#define EMP_DEC_TO_BIN_203  1, 1, 0, 0, 1, 0, 1, 1
#define EMP_DEC_TO_BIN_204  1, 1, 0, 0, 1, 1, 0, 0
#define EMP_DEC_TO_BIN_205  1, 1, 0, 0, 1, 1, 0, 1
#define EMP_DEC_TO_BIN_206  1, 1, 0, 0, 1, 1, 1, 0
#define EMP_DEC_TO_BIN_207  1, 1, 0, 0, 1, 1, 1, 1
#define EMP_DEC_TO_BIN_208  1, 1, 0, 1, 0, 0, 0, 0
#define EMP_DEC_TO_BIN_209  1, 1, 0, 1, 0, 0, 0, 1
#define EMP_DEC_TO_BIN_210  1, 1, 0, 1, 0, 0, 1, 0
#define EMP_DEC_TO_BIN_211  1, 1, 0, 1, 0, 0, 1, 1
#define EMP_DEC_TO_BIN_212  1, 1, 0, 1, 0, 1, 0, 0
#define EMP_DEC_TO_BIN_213  1, 1, 0, 1, 0, 1, 0, 1
#define EMP_DEC_TO_BIN_214  1, 1, 0, 1, 0, 1, 1, 0
#define EMP_DEC_TO_BIN_215  1, 1, 0, 1, 0, 1, 1, 1
#define EMP_DEC_TO_BIN_216  1, 1, 0, 1, 1, 0, 0, 0
#define EMP_DEC_TO_BIN_217  1, 1, 0, 1, 1, 0, 0, 1
#define EMP_DEC_TO_BIN_218  1, 1, 0, 1, 1, 0, 1, 0
#define EMP_DEC_TO_BIN_219  1, 1, 0, 1, 1, 0, 1, 1
#define EMP_DEC_TO_BIN_220  1, 1, 0, 1, 1, 1, 0, 0
#define EMP_DEC_TO_BIN_221  1, 1, 0, 1, 1, 1, 0, 1
#define EMP_DEC_TO_BIN_222  1, 1, 0, 1, 1, 1, 1, 0
#define EMP_DEC_TO_BIN_223  1, 1, 0, 1, 1, 1, 1, 1
#define EMP_DEC_TO_BIN_224  1, 1, 1, 0, 0, 0, 0, 0
#define EMP_DEC_TO_BIN_225  1, 1, 1, 0, 0, 0, 0, 1
#define EMP_DEC_TO_BIN_226  1, 1, 1, 0, 0, 0, 1, 0
#define EMP_DEC_TO_BIN_227  1, 1, 1, 0, 0, 0, 1, 1
#define EMP_DEC_TO_BIN_228  1, 1, 1, 0, 0, 1, 0, 0
#define EMP_DEC_TO_BIN_229  1, 1, 1, 0, 0, 1, 0, 1
#define EMP_DEC_TO_BIN_230  1, 1, 1, 0, 0, 1, 1, 0
#define EMP_DEC_TO_BIN_231  1, 1, 1, 0, 0, 1, 1, 1
#define EMP_DEC_TO_BIN_232  1, 1, 1, 0, 1, 0, 0, 0
#define EMP_DEC_TO_BIN_233  1, 1, 1, 0, 1, 0, 0, 1
#define EMP_DEC_TO_BIN_234  1, 1, 1, 0, 1, 0, 1, 0
#define EMP_DEC_TO_BIN_235  1, 1, 1, 0, 1, 0, 1, 1
#define EMP_DEC_TO_BIN_236  1, 1, 1, 0, 1, 1, 0, 0
#define EMP_DEC_TO_BIN_237  1, 1, 1, 0, 1, 1, 0, 1
#define EMP_DEC_TO_BIN_238  1, 1, 1, 0, 1, 1, 1, 0
#define EMP_DEC_TO_BIN_239  1, 1, 1, 0, 1, 1, 1, 1
#define EMP_DEC_TO_BIN_240  1, 1, 1, 1, 0, 0, 0, 0
#define EMP_DEC_TO_BIN_241  1, 1, 1, 1, 0, 0, 0, 1
#define EMP_DEC_TO_BIN_242  1, 1, 1, 1, 0, 0, 1, 0
#define EMP_DEC_TO_BIN_243  1, 1, 1, 1, 0, 0, 1, 1
#define EMP_DEC_TO_BIN_244  1, 1, 1, 1, 0, 1, 0, 0
#define EMP_DEC_TO_BIN_245  1, 1, 1, 1, 0, 1, 0, 1
#define EMP_DEC_TO_BIN_246  1, 1, 1, 1, 0, 1, 1, 0
#define EMP_DEC_TO_BIN_247  1, 1, 1, 1, 0, 1, 1, 1
#define EMP_DEC_TO_BIN_248  1, 1, 1, 1, 1, 0, 0, 0
#define EMP_DEC_TO_BIN_249  1, 1, 1, 1, 1, 0, 0, 1
#define EMP_DEC_TO_BIN_250  1, 1, 1, 1, 1, 0, 1, 0
#define EMP_DEC_TO_BIN_251  1, 1, 1, 1, 1, 0, 1, 1
#define EMP_DEC_TO_BIN_252  1, 1, 1, 1, 1, 1, 0, 0
#define EMP_DEC_TO_BIN_253  1, 1, 1, 1, 1, 1, 0, 1
#define EMP_DEC_TO_BIN_254  1, 1, 1, 1, 1, 1, 1, 0
#define EMP_DEC_TO_BIN_255  1, 1, 1, 1, 1, 1, 1, 1

#define EMP_BIN_TO_DEC(...) EMP_BIN_TO_DEC_IMPL(__VA_ARGS__)
#define EMP_BIN_TO_DEC_IMPL(B1,B2,B3,B4,B5,B6,B7,B8) \
  EMP_BIN_TO_DEC_ ## B1 ## B2 ## B3 ## B4 ## B5 ## B6 ## B7 ## B8

#define EMP_BIN_TO_DEC_00000000  0  
#define EMP_BIN_TO_DEC_00000001  1  
#define EMP_BIN_TO_DEC_00000010  2  
#define EMP_BIN_TO_DEC_00000011  3  
#define EMP_BIN_TO_DEC_00000100  4  
#define EMP_BIN_TO_DEC_00000101  5  
#define EMP_BIN_TO_DEC_00000110  6  
#define EMP_BIN_TO_DEC_00000111  7  
#define EMP_BIN_TO_DEC_00001000  8  
#define EMP_BIN_TO_DEC_00001001  9  
#define EMP_BIN_TO_DEC_00001010  10 
#define EMP_BIN_TO_DEC_00001011  11 
#define EMP_BIN_TO_DEC_00001100  12 
#define EMP_BIN_TO_DEC_00001101  13 
#define EMP_BIN_TO_DEC_00001110  14 
#define EMP_BIN_TO_DEC_00001111  15 
#define EMP_BIN_TO_DEC_00010000  16 
#define EMP_BIN_TO_DEC_00010001  17 
#define EMP_BIN_TO_DEC_00010010  18 
#define EMP_BIN_TO_DEC_00010011  19 
#define EMP_BIN_TO_DEC_00010100  20 
#define EMP_BIN_TO_DEC_00010101  21 
#define EMP_BIN_TO_DEC_00010110  22 
#define EMP_BIN_TO_DEC_00010111  23 
#define EMP_BIN_TO_DEC_00011000  24 
#define EMP_BIN_TO_DEC_00011001  25 
#define EMP_BIN_TO_DEC_00011010  26 
#define EMP_BIN_TO_DEC_00011011  27 
#define EMP_BIN_TO_DEC_00011100  28 
#define EMP_BIN_TO_DEC_00011101  29 
#define EMP_BIN_TO_DEC_00011110  30 
#define EMP_BIN_TO_DEC_00011111  31 
#define EMP_BIN_TO_DEC_00100000  32 
#define EMP_BIN_TO_DEC_00100001  33 
#define EMP_BIN_TO_DEC_00100010  34 
#define EMP_BIN_TO_DEC_00100011  35 
#define EMP_BIN_TO_DEC_00100100  36 
#define EMP_BIN_TO_DEC_00100101  37 
#define EMP_BIN_TO_DEC_00100110  38 
#define EMP_BIN_TO_DEC_00100111  39 
#define EMP_BIN_TO_DEC_00101000  40 
#define EMP_BIN_TO_DEC_00101001  41 
#define EMP_BIN_TO_DEC_00101010  42 
#define EMP_BIN_TO_DEC_00101011  43 
#define EMP_BIN_TO_DEC_00101100  44 
#define EMP_BIN_TO_DEC_00101101  45 
#define EMP_BIN_TO_DEC_00101110  46 
#define EMP_BIN_TO_DEC_00101111  47 
#define EMP_BIN_TO_DEC_00110000  48 
#define EMP_BIN_TO_DEC_00110001  49 
#define EMP_BIN_TO_DEC_00110010  50 
#define EMP_BIN_TO_DEC_00110011  51 
#define EMP_BIN_TO_DEC_00110100  52 
#define EMP_BIN_TO_DEC_00110101  53 
#define EMP_BIN_TO_DEC_00110110  54 
#define EMP_BIN_TO_DEC_00110111  55 
#define EMP_BIN_TO_DEC_00111000  56 
#define EMP_BIN_TO_DEC_00111001  57 
#define EMP_BIN_TO_DEC_00111010  58 
#define EMP_BIN_TO_DEC_00111011  59 
#define EMP_BIN_TO_DEC_00111100  60 
#define EMP_BIN_TO_DEC_00111101  61 
#define EMP_BIN_TO_DEC_00111110  62 
#define EMP_BIN_TO_DEC_00111111  63 
#define EMP_BIN_TO_DEC_01000000  64 
#define EMP_BIN_TO_DEC_01000001  65 
#define EMP_BIN_TO_DEC_01000010  66 
#define EMP_BIN_TO_DEC_01000011  67 
#define EMP_BIN_TO_DEC_01000100  68 
#define EMP_BIN_TO_DEC_01000101  69 
#define EMP_BIN_TO_DEC_01000110  70 
#define EMP_BIN_TO_DEC_01000111  71 
#define EMP_BIN_TO_DEC_01001000  72 
#define EMP_BIN_TO_DEC_01001001  73 
#define EMP_BIN_TO_DEC_01001010  74 
#define EMP_BIN_TO_DEC_01001011  75 
#define EMP_BIN_TO_DEC_01001100  76 
#define EMP_BIN_TO_DEC_01001101  77 
#define EMP_BIN_TO_DEC_01001110  78 
#define EMP_BIN_TO_DEC_01001111  79 
#define EMP_BIN_TO_DEC_01010000  80 
#define EMP_BIN_TO_DEC_01010001  81 
#define EMP_BIN_TO_DEC_01010010  82 
#define EMP_BIN_TO_DEC_01010011  83 
#define EMP_BIN_TO_DEC_01010100  84 
#define EMP_BIN_TO_DEC_01010101  85 
#define EMP_BIN_TO_DEC_01010110  86 
#define EMP_BIN_TO_DEC_01010111  87 
#define EMP_BIN_TO_DEC_01011000  88 
#define EMP_BIN_TO_DEC_01011001  89 
#define EMP_BIN_TO_DEC_01011010  90 
#define EMP_BIN_TO_DEC_01011011  91 
#define EMP_BIN_TO_DEC_01011100  92 
#define EMP_BIN_TO_DEC_01011101  93 
#define EMP_BIN_TO_DEC_01011110  94 
#define EMP_BIN_TO_DEC_01011111  95 
#define EMP_BIN_TO_DEC_01100000  96 
#define EMP_BIN_TO_DEC_01100001  97 
#define EMP_BIN_TO_DEC_01100010  98 
#define EMP_BIN_TO_DEC_01100011  99 
#define EMP_BIN_TO_DEC_01100100  100
#define EMP_BIN_TO_DEC_01100101  101
#define EMP_BIN_TO_DEC_01100110  102
#define EMP_BIN_TO_DEC_01100111  103
#define EMP_BIN_TO_DEC_01101000  104
#define EMP_BIN_TO_DEC_01101001  105
#define EMP_BIN_TO_DEC_01101010  106
#define EMP_BIN_TO_DEC_01101011  107
#define EMP_BIN_TO_DEC_01101100  108
#define EMP_BIN_TO_DEC_01101101  109
#define EMP_BIN_TO_DEC_01101110  110
#define EMP_BIN_TO_DEC_01101111  111
#define EMP_BIN_TO_DEC_01110000  112
#define EMP_BIN_TO_DEC_01110001  113
#define EMP_BIN_TO_DEC_01110010  114
#define EMP_BIN_TO_DEC_01110011  115
#define EMP_BIN_TO_DEC_01110100  116
#define EMP_BIN_TO_DEC_01110101  117
#define EMP_BIN_TO_DEC_01110110  118
#define EMP_BIN_TO_DEC_01110111  119
#define EMP_BIN_TO_DEC_01111000  120
#define EMP_BIN_TO_DEC_01111001  121
#define EMP_BIN_TO_DEC_01111010  122
#define EMP_BIN_TO_DEC_01111011  123
#define EMP_BIN_TO_DEC_01111100  124
#define EMP_BIN_TO_DEC_01111101  125
#define EMP_BIN_TO_DEC_01111110  126
#define EMP_BIN_TO_DEC_01111111  127
#define EMP_BIN_TO_DEC_10000000  128
#define EMP_BIN_TO_DEC_10000001  129
#define EMP_BIN_TO_DEC_10000010  130
#define EMP_BIN_TO_DEC_10000011  131
#define EMP_BIN_TO_DEC_10000100  132
#define EMP_BIN_TO_DEC_10000101  133
#define EMP_BIN_TO_DEC_10000110  134
#define EMP_BIN_TO_DEC_10000111  135
#define EMP_BIN_TO_DEC_10001000  136
#define EMP_BIN_TO_DEC_10001001  137
#define EMP_BIN_TO_DEC_10001010  138
#define EMP_BIN_TO_DEC_10001011  139
#define EMP_BIN_TO_DEC_10001100  140
#define EMP_BIN_TO_DEC_10001101  141
#define EMP_BIN_TO_DEC_10001110  142
#define EMP_BIN_TO_DEC_10001111  143
#define EMP_BIN_TO_DEC_10010000  144
#define EMP_BIN_TO_DEC_10010001  145
#define EMP_BIN_TO_DEC_10010010  146
#define EMP_BIN_TO_DEC_10010011  147
#define EMP_BIN_TO_DEC_10010100  148
#define EMP_BIN_TO_DEC_10010101  149
#define EMP_BIN_TO_DEC_10010110  150
#define EMP_BIN_TO_DEC_10010111  151
#define EMP_BIN_TO_DEC_10011000  152
#define EMP_BIN_TO_DEC_10011001  153
#define EMP_BIN_TO_DEC_10011010  154
#define EMP_BIN_TO_DEC_10011011  155
#define EMP_BIN_TO_DEC_10011100  156
#define EMP_BIN_TO_DEC_10011101  157
#define EMP_BIN_TO_DEC_10011110  158
#define EMP_BIN_TO_DEC_10011111  159
#define EMP_BIN_TO_DEC_10100000  160
#define EMP_BIN_TO_DEC_10100001  161
#define EMP_BIN_TO_DEC_10100010  162
#define EMP_BIN_TO_DEC_10100011  163
#define EMP_BIN_TO_DEC_10100100  164
#define EMP_BIN_TO_DEC_10100101  165
#define EMP_BIN_TO_DEC_10100110  166
#define EMP_BIN_TO_DEC_10100111  167
#define EMP_BIN_TO_DEC_10101000  168
#define EMP_BIN_TO_DEC_10101001  169
#define EMP_BIN_TO_DEC_10101010  170
#define EMP_BIN_TO_DEC_10101011  171
#define EMP_BIN_TO_DEC_10101100  172
#define EMP_BIN_TO_DEC_10101101  173
#define EMP_BIN_TO_DEC_10101110  174
#define EMP_BIN_TO_DEC_10101111  175
#define EMP_BIN_TO_DEC_10110000  176
#define EMP_BIN_TO_DEC_10110001  177
#define EMP_BIN_TO_DEC_10110010  178
#define EMP_BIN_TO_DEC_10110011  179
#define EMP_BIN_TO_DEC_10110100  180
#define EMP_BIN_TO_DEC_10110101  181
#define EMP_BIN_TO_DEC_10110110  182
#define EMP_BIN_TO_DEC_10110111  183
#define EMP_BIN_TO_DEC_10111000  184
#define EMP_BIN_TO_DEC_10111001  185
#define EMP_BIN_TO_DEC_10111010  186
#define EMP_BIN_TO_DEC_10111011  187
#define EMP_BIN_TO_DEC_10111100  188
#define EMP_BIN_TO_DEC_10111101  189
#define EMP_BIN_TO_DEC_10111110  190
#define EMP_BIN_TO_DEC_10111111  191
#define EMP_BIN_TO_DEC_11000000  192
#define EMP_BIN_TO_DEC_11000001  193
#define EMP_BIN_TO_DEC_11000010  194
#define EMP_BIN_TO_DEC_11000011  195
#define EMP_BIN_TO_DEC_11000100  196
#define EMP_BIN_TO_DEC_11000101  197
#define EMP_BIN_TO_DEC_11000110  198
#define EMP_BIN_TO_DEC_11000111  199
#define EMP_BIN_TO_DEC_11001000  200
#define EMP_BIN_TO_DEC_11001001  201
#define EMP_BIN_TO_DEC_11001010  202
#define EMP_BIN_TO_DEC_11001011  203
#define EMP_BIN_TO_DEC_11001100  204
#define EMP_BIN_TO_DEC_11001101  205
#define EMP_BIN_TO_DEC_11001110  206
#define EMP_BIN_TO_DEC_11001111  207
#define EMP_BIN_TO_DEC_11010000  208
#define EMP_BIN_TO_DEC_11010001  209
#define EMP_BIN_TO_DEC_11010010  210
#define EMP_BIN_TO_DEC_11010011  211
#define EMP_BIN_TO_DEC_11010100  212
#define EMP_BIN_TO_DEC_11010101  213
#define EMP_BIN_TO_DEC_11010110  214
#define EMP_BIN_TO_DEC_11010111  215
#define EMP_BIN_TO_DEC_11011000  216
#define EMP_BIN_TO_DEC_11011001  217
#define EMP_BIN_TO_DEC_11011010  218
#define EMP_BIN_TO_DEC_11011011  219
#define EMP_BIN_TO_DEC_11011100  220
#define EMP_BIN_TO_DEC_11011101  221
#define EMP_BIN_TO_DEC_11011110  222
#define EMP_BIN_TO_DEC_11011111  223
#define EMP_BIN_TO_DEC_11100000  224
#define EMP_BIN_TO_DEC_11100001  225
#define EMP_BIN_TO_DEC_11100010  226
#define EMP_BIN_TO_DEC_11100011  227
#define EMP_BIN_TO_DEC_11100100  228
#define EMP_BIN_TO_DEC_11100101  229
#define EMP_BIN_TO_DEC_11100110  230
#define EMP_BIN_TO_DEC_11100111  231
#define EMP_BIN_TO_DEC_11101000  232
#define EMP_BIN_TO_DEC_11101001  233
#define EMP_BIN_TO_DEC_11101010  234
#define EMP_BIN_TO_DEC_11101011  235
#define EMP_BIN_TO_DEC_11101100  236
#define EMP_BIN_TO_DEC_11101101  237
#define EMP_BIN_TO_DEC_11101110  238
#define EMP_BIN_TO_DEC_11101111  239
#define EMP_BIN_TO_DEC_11110000  240
#define EMP_BIN_TO_DEC_11110001  241
#define EMP_BIN_TO_DEC_11110010  242
#define EMP_BIN_TO_DEC_11110011  243
#define EMP_BIN_TO_DEC_11110100  244
#define EMP_BIN_TO_DEC_11110101  245
#define EMP_BIN_TO_DEC_11110110  246
#define EMP_BIN_TO_DEC_11110111  247
#define EMP_BIN_TO_DEC_11111000  248
#define EMP_BIN_TO_DEC_11111001  249
#define EMP_BIN_TO_DEC_11111010  250
#define EMP_BIN_TO_DEC_11111011  251
#define EMP_BIN_TO_DEC_11111100  252
#define EMP_BIN_TO_DEC_11111101  253
#define EMP_BIN_TO_DEC_11111110  254
#define EMP_BIN_TO_DEC_11111111  255

// Pre-define some simple multiplication
#define EMP_MATH_VAL_TIMES_0(A) 0
#define EMP_MATH_VAL_TIMES_1(A) A

#define EMP_MATH_BIN_TIMES_0(A1, A2, A3, A4, A5, A6, A7, A8)  0, 0, 0, 0, 0, 0, 0, 0
#define EMP_MATH_BIN_TIMES_1(A1, A2, A3, A4, A5, A6, A7, A8)  A1, A2, A3, A4, A5, A6, A7, A8


// Now, convert to SUM format.
#define EMP_BIN_TO_SUM(A1, A2, A3, A4, A5, A6, A7, A8) EMP_MATH_VAL_TIMES_ ## A1(128),     \
    EMP_MATH_VAL_TIMES_##A2(64), EMP_MATH_VAL_TIMES_##A3(32), EMP_MATH_VAL_TIMES_##A4(16), \
    EMP_MATH_VAL_TIMES_##A5(8),  EMP_MATH_VAL_TIMES_##A6(4),  EMP_MATH_VAL_TIMES_##A7(2),  \
    EMP_MATH_VAL_TIMES_##A8(1)

#define EMP_DEC_TO_SUM(A) EMP_BIN_TO_SUM EMP_EMPTY ( EMP_DEC_TO_BIN(A) )

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

#define EMP_MATH_RESTORE_BIN(A1, A2, A3, A4, A5, A6, A7, A8)          \
  EMP_MATH_RESTORE_BIN_2(                                             \
    EMP_MATH_COUNT_BITS(EMP_MATH_CLEAR_CARRY(A1), EMP_MATH_GET_CARRY(A2)), \
    EMP_MATH_COUNT_BITS(EMP_MATH_CLEAR_CARRY(A2), EMP_MATH_GET_CARRY(A3)), \
    EMP_MATH_COUNT_BITS(EMP_MATH_CLEAR_CARRY(A3), EMP_MATH_GET_CARRY(A4)), \
    EMP_MATH_COUNT_BITS(EMP_MATH_CLEAR_CARRY(A4), EMP_MATH_GET_CARRY(A5)), \
    EMP_MATH_COUNT_BITS(EMP_MATH_CLEAR_CARRY(A5), EMP_MATH_GET_CARRY(A6)), \
    EMP_MATH_COUNT_BITS(EMP_MATH_CLEAR_CARRY(A6), EMP_MATH_GET_CARRY(A7)), \
    EMP_MATH_COUNT_BITS(EMP_MATH_CLEAR_CARRY(A7), EMP_MATH_GET_CARRY(A8)), \
    EMP_MATH_CLEAR_CARRY(A8)                                          \
  )

#define EMP_MATH_RESTORE_BIN_2(A1, A2, A3, A4, A5, A6, A7, A8)        \
  EMP_MATH_RESTORE_BIN_3(                                             \
    EMP_MATH_COUNT_BITS(EMP_MATH_CLEAR_CARRY(A1), EMP_MATH_GET_CARRY(A2)), \
    EMP_MATH_COUNT_BITS(EMP_MATH_CLEAR_CARRY(A2), EMP_MATH_GET_CARRY(A3)), \
    EMP_MATH_COUNT_BITS(EMP_MATH_CLEAR_CARRY(A3), EMP_MATH_GET_CARRY(A4)), \
    EMP_MATH_COUNT_BITS(EMP_MATH_CLEAR_CARRY(A4), EMP_MATH_GET_CARRY(A5)), \
    EMP_MATH_COUNT_BITS(EMP_MATH_CLEAR_CARRY(A5), EMP_MATH_GET_CARRY(A6)), \
    EMP_MATH_COUNT_BITS(EMP_MATH_CLEAR_CARRY(A6), EMP_MATH_GET_CARRY(A7)), \
    EMP_MATH_CLEAR_CARRY(A7),                                         \
    A8                                                                \
  )

#define EMP_MATH_RESTORE_BIN_3(A1, A2, A3, A4, A5, A6, A7, A8)        \
  EMP_MATH_RESTORE_BIN_4(                                             \
    EMP_MATH_COUNT_BITS(EMP_MATH_CLEAR_CARRY(A1), EMP_MATH_GET_CARRY(A2)), \
    EMP_MATH_COUNT_BITS(EMP_MATH_CLEAR_CARRY(A2), EMP_MATH_GET_CARRY(A3)), \
    EMP_MATH_COUNT_BITS(EMP_MATH_CLEAR_CARRY(A3), EMP_MATH_GET_CARRY(A4)), \
    EMP_MATH_COUNT_BITS(EMP_MATH_CLEAR_CARRY(A4), EMP_MATH_GET_CARRY(A5)), \
    EMP_MATH_COUNT_BITS(EMP_MATH_CLEAR_CARRY(A5), EMP_MATH_GET_CARRY(A6)), \
    EMP_MATH_CLEAR_CARRY(A6),                                         \
    A7,                                                               \
    A8                                                                \
  )

#define EMP_MATH_RESTORE_BIN_4(A1, A2, A3, A4, A5, A6, A7, A8)        \
  EMP_MATH_RESTORE_BIN_5(                                             \
    EMP_MATH_COUNT_BITS(EMP_MATH_CLEAR_CARRY(A1), EMP_MATH_GET_CARRY(A2)), \
    EMP_MATH_COUNT_BITS(EMP_MATH_CLEAR_CARRY(A2), EMP_MATH_GET_CARRY(A3)), \
    EMP_MATH_COUNT_BITS(EMP_MATH_CLEAR_CARRY(A3), EMP_MATH_GET_CARRY(A4)), \
    EMP_MATH_COUNT_BITS(EMP_MATH_CLEAR_CARRY(A4), EMP_MATH_GET_CARRY(A5)), \
    EMP_MATH_CLEAR_CARRY(A5),                                         \
    A6,                                                               \
    A7,                                                               \
    A8                                                                \
  )

#define EMP_MATH_RESTORE_BIN_5(A1, A2, A3, A4, A5, A6, A7, A8)        \
  EMP_MATH_RESTORE_BIN_6(                                             \
    EMP_MATH_COUNT_BITS(EMP_MATH_CLEAR_CARRY(A1), EMP_MATH_GET_CARRY(A2)), \
    EMP_MATH_COUNT_BITS(EMP_MATH_CLEAR_CARRY(A2), EMP_MATH_GET_CARRY(A3)), \
    EMP_MATH_COUNT_BITS(EMP_MATH_CLEAR_CARRY(A3), EMP_MATH_GET_CARRY(A4)), \
    EMP_MATH_CLEAR_CARRY(A4),                                         \
    A5,                                                               \
    A6,                                                               \
    A7,                                                               \
    A8                                                                \
  )

#define EMP_MATH_RESTORE_BIN_6(A1, A2, A3, A4, A5, A6, A7, A8)        \
  EMP_MATH_RESTORE_BIN_7(                                             \
    EMP_MATH_COUNT_BITS(EMP_MATH_CLEAR_CARRY(A1), EMP_MATH_GET_CARRY(A2)), \
    EMP_MATH_COUNT_BITS(EMP_MATH_CLEAR_CARRY(A2), EMP_MATH_GET_CARRY(A3)), \
    EMP_MATH_CLEAR_CARRY(A3),                                         \
    A4,                                                               \
    A5,                                                               \
    A6,                                                               \
    A7,                                                               \
    A8                                                                \
  )

#define EMP_MATH_RESTORE_BIN_7(A1, A2, A3, A4, A5, A6, A7, A8)        \
  EMP_MATH_RESTORE_BIN_8(                                             \
    EMP_MATH_COUNT_BITS(EMP_MATH_CLEAR_CARRY(A1), EMP_MATH_GET_CARRY(A2)), \
    EMP_MATH_CLEAR_CARRY(A2),                                         \
    A3,                                                               \
    A4,                                                               \
    A5,                                                               \
    A6,                                                               \
    A7,                                                               \
    A8                                                                \
  )

#define EMP_MATH_RESTORE_BIN_8(A1, A2, A3, A4, A5, A6, A7, A8)        \
    EMP_MATH_CLEAR_CARRY(A1),                                         \
    A2,                                                               \
    A3,                                                               \
    A4,                                                               \
    A5,                                                               \
    A6,                                                               \
    A7,                                                               \
    A8                                                                \


///////////////////////////
//
//  Math Operations!!!
//

// --- Shifting ---

#define EMP_SHIFTL_BIN(A1, A2, A3, A4, A5, A6, A7, A8) A2, A3, A4, A5, A6, A7, A8, 0
#define EMP_SHIFTR_BIN(A1, A2, A3, A4, A5, A6, A7, A8) 0, A1, A2, A3, A4, A5, A6, A7

#define EMP_SHIFTL(A) EMP_SHIFTL_IMPL( EMP_DEC_TO_BIN(A) )
#define EMP_SHIFTL_IMPL(...) EMP_BIN_TO_DEC( EMP_SHIFTL_BIN( __VA_ARGS__ ) )

#define EMP_SHIFTR(A) EMP_SHIFTR_IMPL( EMP_DEC_TO_BIN(A) )
#define EMP_SHIFTR_IMPL(...) EMP_BIN_TO_DEC( EMP_SHIFTR_BIN( __VA_ARGS__ ) )


// --- Addition ---
#define EMP_ADD_BIN(...) EMP_ADD_BIN_IMPL(__VA_ARGS__)

#define EMP_ADD_BIN_IMPL(A1, A2, A3, A4, A5, A6, A7, A8,                \
                         B1, B2, B3, B4, B5, B6, B7, B8)                \
  EMP_MATH_RESTORE_BIN( EMP_MATH_COUNT_BITS(A1, B1), EMP_MATH_COUNT_BITS(A2, B2), \
                        EMP_MATH_COUNT_BITS(A3, B3), EMP_MATH_COUNT_BITS(A4, B4), \
                        EMP_MATH_COUNT_BITS(A5, B5), EMP_MATH_COUNT_BITS(A6, B6), \
                        EMP_MATH_COUNT_BITS(A7, B7), EMP_MATH_COUNT_BITS(A8, B8)  \
                        )

#define EMP_ADD(A, B) EMP_ADD_IMPL( EMP_DEC_TO_BIN(A), EMP_DEC_TO_BIN(B) )
#define EMP_ADD_IMPL(...) EMP_BIN_TO_DEC( EMP_ADD_BIN_IMPL( __VA_ARGS__ ) )

// --- Subtraction ---
#define EMP_SUB_BIN(A1, A2, A3, A4, A5, A6, A7, A8, \
                    B1, B2, B3, B4, B5, B6, B7, B8) \
  EMP_MATH_RESTORE_BIN( EMP_MATH_DIFF_BITS(A1, B1), EMP_MATH_DIFF_BITS(A2, B2), \
                        EMP_MATH_DIFF_BITS(A3, B3), EMP_MATH_DIFF_BITS(A4, B4), \
                        EMP_MATH_DIFF_BITS(A5, B5), EMP_MATH_DIFF_BITS(A6, B6), \
                        EMP_MATH_DIFF_BITS(A7, B7), EMP_MATH_DIFF_BITS(A8, B8)  \
                        )

#define EMP_SUB(A, B) EMP_SUB_IMPL( EMP_DEC_TO_BIN(A), EMP_DEC_TO_BIN(B) )
#define EMP_SUB_IMPL(...) EMP_BIN_TO_DEC( EMP_SUB_BIN( __VA_ARGS__ ) )

// --- Increment and Decrement ---
#define EMP_INC(A) EMP_ADD(A, 1)
#define EMP_DEC(A) EMP_SUB(A, 1)
#define EMP_HALF(A) EMP_SHIFTR(A)


// --- Multiply ---
#define EMP_MULT_BIN(A1, A2, A3, A4, A5, A6, A7, A8, \
                     B1, B2, B3, B4, B5, B6, B7, B8) \
EMP_ADD_BIN(                                         \
    EMP_ADD_BIN( EMP_ADD_BIN( EMP_MATH_BIN_TIMES_ ## B8 (A1, A2, A3, A4, A5, A6, A7, A8),     \
                              EMP_MATH_BIN_TIMES_ ## B7 (A2, A3, A4, A5, A6, A7, A8,  0) ),   \
                 EMP_ADD_BIN( EMP_MATH_BIN_TIMES_ ## B6 (A3, A4, A5, A6, A7, A8,  0,  0),     \
                              EMP_MATH_BIN_TIMES_ ## B5 (A4, A5, A6, A7, A8,  0,  0,  0) ) ), \
    EMP_ADD_BIN( EMP_ADD_BIN( EMP_MATH_BIN_TIMES_ ## B4 (A5, A6, A7, A8,  0,  0,  0,  0),     \
                              EMP_MATH_BIN_TIMES_ ## B3 (A6, A7, A8,  0,  0,  0,  0,  0) ),   \
                 EMP_ADD_BIN( EMP_MATH_BIN_TIMES_ ## B2 (A7, A8,  0,  0,  0,  0,  0,  0),     \
                              EMP_MATH_BIN_TIMES_ ## B1 (A8,  0,  0,  0,  0,  0,  0,  0) ) )  \
 )

#define EMP_MULT(A, B) EMP_MULT_IMPL( EMP_DEC_TO_BIN(A), EMP_DEC_TO_BIN(B) )
#define EMP_MULT_IMPL(...) EMP_BIN_TO_DEC( EMP_MULT_BIN( __VA_ARGS__ ) )

#endif
