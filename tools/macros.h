#ifndef EMP_MACROS_H
#define EMP_MACROS_H

//////////////////////////////////////////////////////////////////////////////////////////
//
//  Generally useful macros that can perform cools tricks.  As with all macros, use only
//  after careful exclusion of alternative approaches!
//

// EMP_STRINGIFY takes any input, processes macros, and puts the result in quotes.
#define EMP_STRINGIFY(A) EMP_STRINGIFY_IMPL(A)
#define EMP_STRINGIFY_IMPL(...) #__VA_ARGS__

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


// EMP_WRAP_EACH takes a wrapper macro and a variable set of arguments,
// then applied to wrapper macro to each argument in order.
#define EMP_WRAP_EACH(W, ...) EMP_ASSEMBLE_MACRO(EMP_LAYOUT_, EMP_COUNT_ARGS(__VA_ARGS__), W, , __VA_ARGS__)

// EMP_LAYOUT takes a wrapper macro and padding information, wraps each argument in the macro
// and then spaces them out with the padding.
#define EMP_LAYOUT(W, P, ...) EMP_ASSEMBLE_MACRO(EMP_LAYOUT_, EMP_COUNT_ARGS(__VA_ARGS__), W, P, __VA_ARGS__)
#define EMP_LAYOUT_0(W, P)
#define EMP_LAYOUT_1(W, P, A) W(A)
#define EMP_LAYOUT_2(W, P, A, ...) W(A) P EMP_LAYOUT_1(W, P, __VA_ARGS__)
#define EMP_LAYOUT_3(W, P, A, ...) W(A) P EMP_LAYOUT_2(W, P, __VA_ARGS__)
#define EMP_LAYOUT_4(W, P, A, ...) W(A) P EMP_LAYOUT_3(W, P, __VA_ARGS__)
#define EMP_LAYOUT_5(W, P, A, ...) W(A) P EMP_LAYOUT_4(W, P, __VA_ARGS__)
#define EMP_LAYOUT_6(W, P, A, ...) W(A) P EMP_LAYOUT_5(W, P, __VA_ARGS__)
#define EMP_LAYOUT_7(W, P, A, ...) W(A) P EMP_LAYOUT_6(W, P, __VA_ARGS__)
#define EMP_LAYOUT_8(W, P, A, ...) W(A) P EMP_LAYOUT_7(W, P, __VA_ARGS__)
#define EMP_LAYOUT_9(W, P, A, ...) W(A) P EMP_LAYOUT_8(W, P, __VA_ARGS__)
#define EMP_LAYOUT_10(W, P, A, ...) W(A) P EMP_LAYOUT_9(W, P, __VA_ARGS__)
#define EMP_LAYOUT_11(W, P, A, ...) W(A) P EMP_LAYOUT_10(W, P, __VA_ARGS__)
#define EMP_LAYOUT_12(W, P, A, ...) W(A) P EMP_LAYOUT_11(W, P, __VA_ARGS__)
#define EMP_LAYOUT_13(W, P, A, ...) W(A) P EMP_LAYOUT_12(W, P, __VA_ARGS__)
#define EMP_LAYOUT_14(W, P, A, ...) W(A) P EMP_LAYOUT_13(W, P, __VA_ARGS__)
#define EMP_LAYOUT_15(W, P, A, ...) W(A) P EMP_LAYOUT_14(W, P, __VA_ARGS__)
#define EMP_LAYOUT_16(W, P, A, ...) W(A) P EMP_LAYOUT_15(W, P, __VA_ARGS__)
#define EMP_LAYOUT_17(W, P, A, ...) W(A) P EMP_LAYOUT_16(W, P, __VA_ARGS__)
#define EMP_LAYOUT_18(W, P, A, ...) W(A) P EMP_LAYOUT_17(W, P, __VA_ARGS__)
#define EMP_LAYOUT_19(W, P, A, ...) W(A) P EMP_LAYOUT_18(W, P, __VA_ARGS__)
#define EMP_LAYOUT_20(W, P, A, ...) W(A) P EMP_LAYOUT_19(W, P, __VA_ARGS__)
#define EMP_LAYOUT_21(W, P, A, ...) W(A) P EMP_LAYOUT_20(W, P, __VA_ARGS__)
#define EMP_LAYOUT_22(W, P, A, ...) W(A) P EMP_LAYOUT_21(W, P, __VA_ARGS__)
#define EMP_LAYOUT_23(W, P, A, ...) W(A) P EMP_LAYOUT_22(W, P, __VA_ARGS__)
#define EMP_LAYOUT_24(W, P, A, ...) W(A) P EMP_LAYOUT_23(W, P, __VA_ARGS__)
#define EMP_LAYOUT_25(W, P, A, ...) W(A) P EMP_LAYOUT_24(W, P, __VA_ARGS__)
#define EMP_LAYOUT_26(W, P, A, ...) W(A) P EMP_LAYOUT_25(W, P, __VA_ARGS__)
#define EMP_LAYOUT_27(W, P, A, ...) W(A) P EMP_LAYOUT_26(W, P, __VA_ARGS__)
#define EMP_LAYOUT_28(W, P, A, ...) W(A) P EMP_LAYOUT_27(W, P, __VA_ARGS__)
#define EMP_LAYOUT_29(W, P, A, ...) W(A) P EMP_LAYOUT_28(W, P, __VA_ARGS__)
#define EMP_LAYOUT_30(W, P, A, ...) W(A) P EMP_LAYOUT_29(W, P, __VA_ARGS__)
#define EMP_LAYOUT_31(W, P, A, ...) W(A) P EMP_LAYOUT_30(W, P, __VA_ARGS__)
#define EMP_LAYOUT_32(W, P, A, ...) W(A) P EMP_LAYOUT_31(W, P, __VA_ARGS__)
#define EMP_LAYOUT_33(W, P, A, ...) W(A) P EMP_LAYOUT_32(W, P, __VA_ARGS__)
#define EMP_LAYOUT_34(W, P, A, ...) W(A) P EMP_LAYOUT_33(W, P, __VA_ARGS__)
#define EMP_LAYOUT_35(W, P, A, ...) W(A) P EMP_LAYOUT_34(W, P, __VA_ARGS__)
#define EMP_LAYOUT_36(W, P, A, ...) W(A) P EMP_LAYOUT_35(W, P, __VA_ARGS__)
#define EMP_LAYOUT_37(W, P, A, ...) W(A) P EMP_LAYOUT_36(W, P, __VA_ARGS__)
#define EMP_LAYOUT_38(W, P, A, ...) W(A) P EMP_LAYOUT_37(W, P, __VA_ARGS__)
#define EMP_LAYOUT_39(W, P, A, ...) W(A) P EMP_LAYOUT_38(W, P, __VA_ARGS__)
#define EMP_LAYOUT_40(W, P, A, ...) W(A) P EMP_LAYOUT_39(W, P, __VA_ARGS__)
#define EMP_LAYOUT_41(W, P, A, ...) W(A) P EMP_LAYOUT_40(W, P, __VA_ARGS__)
#define EMP_LAYOUT_42(W, P, A, ...) W(A) P EMP_LAYOUT_41(W, P, __VA_ARGS__)
#define EMP_LAYOUT_43(W, P, A, ...) W(A) P EMP_LAYOUT_42(W, P, __VA_ARGS__)
#define EMP_LAYOUT_44(W, P, A, ...) W(A) P EMP_LAYOUT_43(W, P, __VA_ARGS__)
#define EMP_LAYOUT_45(W, P, A, ...) W(A) P EMP_LAYOUT_44(W, P, __VA_ARGS__)
#define EMP_LAYOUT_46(W, P, A, ...) W(A) P EMP_LAYOUT_45(W, P, __VA_ARGS__)
#define EMP_LAYOUT_47(W, P, A, ...) W(A) P EMP_LAYOUT_46(W, P, __VA_ARGS__)
#define EMP_LAYOUT_48(W, P, A, ...) W(A) P EMP_LAYOUT_47(W, P, __VA_ARGS__)
#define EMP_LAYOUT_49(W, P, A, ...) W(A) P EMP_LAYOUT_48(W, P, __VA_ARGS__)
#define EMP_LAYOUT_50(W, P, A, ...) W(A) P EMP_LAYOUT_49(W, P, __VA_ARGS__)
#define EMP_LAYOUT_51(W, P, A, ...) W(A) P EMP_LAYOUT_50(W, P, __VA_ARGS__)
#define EMP_LAYOUT_52(W, P, A, ...) W(A) P EMP_LAYOUT_51(W, P, __VA_ARGS__)
#define EMP_LAYOUT_53(W, P, A, ...) W(A) P EMP_LAYOUT_52(W, P, __VA_ARGS__)
#define EMP_LAYOUT_54(W, P, A, ...) W(A) P EMP_LAYOUT_53(W, P, __VA_ARGS__)
#define EMP_LAYOUT_55(W, P, A, ...) W(A) P EMP_LAYOUT_54(W, P, __VA_ARGS__)
#define EMP_LAYOUT_56(W, P, A, ...) W(A) P EMP_LAYOUT_55(W, P, __VA_ARGS__)
#define EMP_LAYOUT_57(W, P, A, ...) W(A) P EMP_LAYOUT_56(W, P, __VA_ARGS__)
#define EMP_LAYOUT_58(W, P, A, ...) W(A) P EMP_LAYOUT_57(W, P, __VA_ARGS__)
#define EMP_LAYOUT_59(W, P, A, ...) W(A) P EMP_LAYOUT_58(W, P, __VA_ARGS__)
#define EMP_LAYOUT_60(W, P, A, ...) W(A) P EMP_LAYOUT_59(W, P, __VA_ARGS__)
#define EMP_LAYOUT_61(W, P, A, ...) W(A) P EMP_LAYOUT_60(W, P, __VA_ARGS__)
#define EMP_LAYOUT_62(W, P, A, ...) W(A) P EMP_LAYOUT_61(W, P, __VA_ARGS__)
#define EMP_LAYOUT_63(W, P, A, ...) W(A) P EMP_LAYOUT_62(W, P, __VA_ARGS__)




// Setup a generic method of calling a specific version of a macro based on argument count.
#define EMP_ASSEMBLE_IMPL(BASE, ARG_COUNT) BASE ## ARG_COUNT
#define EMP_ASSEMBLE_MACRO(BASE, ARG_COUNT, ...) EMP_ASSEMBLE_IMPL(BASE, ARG_COUNT) (__VA_ARGS__)


#define EMP_COMMA ,


#endif
