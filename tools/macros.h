#ifndef EMP_MACROS_H
#define EMP_MACROS_H

//////////////////////////////////////////////////////////////////////////////////////////
//
//  Generally useful macros that can perform cools tricks.  As with all macros, use only
//  after careful exclusion of alternative approaches!
//

#define EMP_COMMA ,

// EMP_STRINGIFY takes any input, processes macros, and puts the result in quotes.
#define EMP_STRINGIFY(A) EMP_STRINGIFY_IMPL(A)
#define EMP_STRINGIFY_IMPL(...) #__VA_ARGS__

#define EMP_COMMA_MERGE(A,B) A,B

#define EMP_GET_ARG_1(A1, ...) A1
#define EMP_GET_ARG_2(A1, A2, ...) A2
#define EMP_GET_ARG_3(A1, A2, A3, ...) A3
#define EMP_GET_ARG_4(A1, A2, A3, A4, ...) A4
#define EMP_GET_ARG_5(A1, A2, A3, A4, A5, ...) A5
#define EMP_GET_ARG_6(A1, A2, A3, A4, A5, A6, ...) A6
#define EMP_GET_ARG_7(A1, A2, A3, A4, A5, A6, A7, ...) A7
#define EMP_GET_ARG_8(A1, A2, A3, A4, A5, A6, A7, A8, ...) A8
#define EMP_GET_ARG_9(A1, A2, A3, A4, A5, A6, A7, A8, A9, ...) A9
#define EMP_GET_ARG_10(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, ...) A10
#define EMP_GET_ARG_20(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15, A16, A17, A18, A19, A20, ...) A20
#define EMP_GET_ARG_30(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15, A16, A17, A18, A19, A20, A21, A22, A23, A24, A25, A26, A27, A28, A29, A30, ...) A30
#define EMP_GET_ARG_40(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15, A16, A17, A18, A19, A20, A21, A22, A23, A24, A25, A26, A27, A28, A29, A30, A31, A32, A33, A34, A35, A36, A37, A38, A39, A40, ...) A40
#define EMP_GET_ARG_50(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15, A16, A17, A18, A19, A20, A21, A22, A23, A24, A25, A26, A27, A28, A29, A30, A31, A32, A33, A34, A35, A36, A37, A38, A39, A40, A41, A42, A43, A44, A45, A46, A47, A48, A49, A50, ...) A50
#define EMP_GET_ARG_60(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15, A16, A17, A18, A19, A20, A21, A22, A23, A24, A25, A26, A27, A28, A29, A30, A31, A32, A33, A34, A35, A36, A37, A38, A39, A40, A41, A42, A43, A44, A45, A46, A47, A48, A49, A50, A51, A52, A53, A54, A55, A56, A57, A58, A59, A60, ...) A60
#define EMP_GET_ARG_64(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15, A16, A17, A18, A19, A20, A21, A22, A23, A24, A25, A26, A27, A28, A29, A30, A31, A32, A33, A34, A35, A36, A37, A38, A39, A40, A41, A42, A43, A44, A45, A46, A47, A48, A49, A50, A51, A52, A53, A54, A55, A56, A57, A58, A59, A60, A61, A62, A63, A64, ...) A64

#define EMP_COUNT_ARGS(...) EMP_GET_ARG_64(__VA_ARGS__, 63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49, 48, 47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)

// Enable an arbitrary number of arguments to be merged AFTER being processed!
#define EMP_MERGE(...) EMP_ASSEMBLE_MACRO(EMP_MERGE_, EMP_COUNT_ARGS(__VA_ARGS__), __VA_ARGS__)
#define EMP_MERGE_1(A1) A1
#define EMP_MERGE_2(A1,A2) A1 ## A2
#define EMP_MERGE_3(A1,A2,A3) A1 ## A2 ## A3
#define EMP_MERGE_4(A1,A2,A3,A4) A1 ## A2 ## A3 ## A4
#define EMP_MERGE_5(A1,A2,A3,A4,A5) A1 ## A2 ## A3 ## A4 ## A5
#define EMP_MERGE_6(A1,A2,A3,A4,A5,A6) A1 ## A2 ## A3 ## A4 ## A5 ## A6
#define EMP_MERGE_7(A1,A2,A3,A4,A5,A6,A7) A1 ## A2 ## A3 ## A4 ## A5 ## A6 ## A7

#define EMP_ADD_SPACER(SPACER, ...) EMP_ASSEMBLE_MACRO(EMP_ADD_SPACER_, EMP_COUNT_ARGS(__VA_ARGS__), SPACER, __VA_ARGS__)

#define EMP_WRAP_EACH(W, ...) EMP_ASSEMBLE_MACRO(EMP_WRAP_EACH_, EMP_COUNT_ARGS(__VA_ARGS__), W, __VA_ARGS__)
#define EMP_WRAP_EACH_1(W, A1) W(A1)
#define EMP_WRAP_EACH_2(W, A1, A2) W(A1) W(A2)
#define EMP_WRAP_EACH_3(W, A1, A2, A3) W(A1) W(A2) W(A3)
#define EMP_WRAP_EACH_4(W, A1, A2, A3, A4) W(A1) W(A2) W(A3) W(A4)
#define EMP_WRAP_EACH_5(W, A1, A2, A3, A4, A5) W(A1) W(A2) W(A3) W(A4) W(A5)
#define EMP_WRAP_EACH_6(W, A1, A2, A3, A4, A5, A6) W(A1) W(A2) W(A3) W(A4) W(A5) W(A6)
#define EMP_WRAP_EACH_7(W, A1, A2, A3, A4, A5, A6, A7) W(A1) W(A2) W(A3) W(A4) W(A5) W(A6) W(A7)
#define EMP_WRAP_EACH_8(W, A1, A2, A3, A4, A5, A6, A7, A8) W(A1) W(A2) W(A3) W(A4) W(A5) W(A6) W(A7) W(A8)
#define EMP_WRAP_EACH_9(W, A1, A2, A3, A4, A5, A6, A7, A8, A9) W(A1) W(A2) W(A3) W(A4) W(A5) W(A6) W(A7) W(A8) W(A9)
#define EMP_WRAP_EACH_10(W, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10) W(A1) W(A2) W(A3) W(A4) W(A5) W(A6) W(A7) W(A8) W(A9) W(A10)
#define EMP_WRAP_EACH_11(W, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11) W(A1) W(A2) W(A3) W(A4) W(A5) W(A6) W(A7) W(A8) W(A9) W(A10) W(A11)
#define EMP_WRAP_EACH_12(W, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12) W(A1) W(A2) W(A3) W(A4) W(A5) W(A6) W(A7) W(A8) W(A9) W(A10) W(A11) W(A12)
#define EMP_WRAP_EACH_13(W, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13) W(A1) W(A2) W(A3) W(A4) W(A5) W(A6) W(A7) W(A8) W(A9) W(A10) W(A11) W(A12) W(A13)
#define EMP_WRAP_EACH_14(W, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14) W(A1) W(A2) W(A3) W(A4) W(A5) W(A6) W(A7) W(A8) W(A9) W(A10) W(A11) W(A12) W(A13) W(A14)
#define EMP_WRAP_EACH_15(W, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15) W(A1) W(A2) W(A3) W(A4) W(A5) W(A6) W(A7) W(A8) W(A9) W(A10) W(A11) W(A12) W(A13) W(A14) W(A15)
#define EMP_WRAP_EACH_16(W, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15, A16) W(A1) W(A2) W(A3) W(A4) W(A5) W(A6) W(A7) W(A8) W(A9) W(A10) W(A11) W(A12) W(A13) W(A14) W(A15) W(A16)
#define EMP_WRAP_EACH_17(W, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15, A16, A17) W(A1) W(A2) W(A3) W(A4) W(A5) W(A6) W(A7) W(A8) W(A9) W(A10) W(A11) W(A12) W(A13) W(A14) W(A15) W(A16) W(A17)
#define EMP_WRAP_EACH_18(W, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15, A16, A17, A18) W(A1) W(A2) W(A3) W(A4) W(A5) W(A6) W(A7) W(A8) W(A9) W(A10) W(A11) W(A12) W(A13) W(A14) W(A15) W(A16) W(A17) W(A18)
#define EMP_WRAP_EACH_19(W, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15, A16, A17, A18, A19) W(A1) W(A2) W(A3) W(A4) W(A5) W(A6) W(A7) W(A8) W(A9) W(A10) W(A11) W(A12) W(A13) W(A14) W(A15) W(A16) W(A17) W(A18) W(A19)




// Setup a generic method of calling a specific version of a macro based on argument count.
#define EMP_ASSEMBLE_IMPL(BASE, ARG_COUNT) BASE ## ARG_COUNT
#define EMP_ASSEMBLE_MACRO(BASE, ARG_COUNT, ...) EMP_ASSEMBLE_IMPL(BASE, ARG_COUNT) (__VA_ARGS__)

#endif
