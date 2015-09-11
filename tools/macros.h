#ifndef EMP_MACROS_H
#define EMP_MACROS_H

//////////////////////////////////////////////////////////////////////////////////////////
//
//  Generally useful macros that can perform cools tricks.  As with all macros, use only
//  after careful exclusion of alternative approaches!
//
//  The Macros to highlight here are:
//
//  ===== String Handling and Printing =====
//  EMP_PRINT_RESULT(A) will print to std::cout both the string that was passed into the
//       macro and what that string evaluates to.
//
//  EMP_STRINGIFY(...) converts all arguments into a single string (including commas).
//  EMP_STRINGIFY_EACH(...) converts each argument into a string (leaving commas).
//
//  ===== Managing variadic arguments =====
//  EMP_COUNT_ARGS(...) will return the number of arguments in the __VA_ARGS__
//
//  EMP_GET_ARG_*(...) replace * with number and will return the arg at that position.
//
//  EMP_GET_ODD_ARGS(...) will return all arguments at odd positions (1,3,5,7, etc.)
//  EMP_GET_EVEN_ARGS(...) will return all arguments at odd positions (2,4,6,8, etc.)
//  EMP_REVERSE_ARGS(...) Reverse the order of arguments passed in.
//
//  ===== Argument Manipulation and Formatting =====
//  EMP_MERGE(...) merge all arguments (after conversion) into a single unit.
//  EMP_WRAP_EACH(W, ...) will run macro W on each of the other args and concatinate them.
//  EMP_LAYOUT(W, P, ...) Similar to EMP_WRAP_EACH, but puts a P between each arg pair.
//  EMP_WRAP_ARGS(W, ...) Similar to EMP_WRAP_EACH, but puts a COMMA between each arg pair.
//  EMP_WRAP_ARG_PAIRS(W, ...) Similar to EMP_WRAP_ARGS, but passes pairs of args into W.
//
//  ===== Simple Math =====
//  EMP_INC_x resolves to x+1 (for a number in the place of x)
//  EMP_HALF_x resolves to x/2
//
//  Development Notes:
//  * We need to fix who we handle macros that covert inputs to comma-separated results,
//    from those that merge them all together.  One option is to have comma-separated the
//    default and then have an EMP_REMOVE_COMMAS (or somesuch)
//  * EMP_TYPES_TO_ARGS (not yet list above) is poorly name.  Maybe EMP_DECLARE_ARGS?
//  * It would be useful to have EMP_WRAP_WITH_ID which passes in the position ID as
//    the second argument.  This would allow us to, for example, redo EMP_TYPES_TO_ARGS.
//

#define EMP_COMMA ,

// EMP_STRINGIFY takes any input, processes macros, and puts the result in quotes.
#define EMP_STRINGIFY(...) EMP_STRINGIFY_IMPL(__VA_ARGS__)
#define EMP_STRINGIFY_IMPL(...) #__VA_ARGS__

#define EMP_PRINT_RESULT_IMPL(STREAM, LHS, RHS) STREAM << "[[" << LHS << "]] = [[" << RHS << "]]" << std::endl
#define EMP_PRINT_RESULT_TO(STREAM, A) EMP_PRINT_RESULT_IMPL(STREAM, #A, A)
#define EMP_PRINT_RESULT(A) EMP_PRINT_RESULT_IMPL(std::cout, #A, A)

#define EMP_GET_ARG_1(A1, ...) A1
#define EMP_GET_ARG_2(A1, A2, ...) A2
#define EMP_GET_ARG_3(A1, A2, A3, ...) A3
#define EMP_GET_ARG_4(A1, A2, A3, A4, ...) A4
#define EMP_GET_ARG_5(A1, A2, A3, A4, A5, ...) A5
#define EMP_GET_ARG_6(A1, A2, A3, A4, A5, A6, ...) A6
#define EMP_GET_ARG_7(A, ...) EMP_GET_ARG_6(__VA_ARGS__)
#define EMP_GET_ARG_8(A, ...) EMP_GET_ARG_7(__VA_ARGS__)
#define EMP_GET_ARG_9(A, ...) EMP_GET_ARG_8(__VA_ARGS__)
#define EMP_GET_ARG_10(A, ...) EMP_GET_ARG_9(__VA_ARGS__)
#define EMP_GET_ARG_11(A, ...) EMP_GET_ARG_10(__VA_ARGS__)
#define EMP_GET_ARG_12(A, ...) EMP_GET_ARG_11(__VA_ARGS__)
#define EMP_GET_ARG_13(A, ...) EMP_GET_ARG_12(__VA_ARGS__)
#define EMP_GET_ARG_14(A, ...) EMP_GET_ARG_13(__VA_ARGS__)
#define EMP_GET_ARG_15(A, ...) EMP_GET_ARG_14(__VA_ARGS__)
#define EMP_GET_ARG_16(A, ...) EMP_GET_ARG_15(__VA_ARGS__)
#define EMP_GET_ARG_17(A, ...) EMP_GET_ARG_16(__VA_ARGS__)
#define EMP_GET_ARG_18(A, ...) EMP_GET_ARG_17(__VA_ARGS__)
#define EMP_GET_ARG_19(A, ...) EMP_GET_ARG_18(__VA_ARGS__)
#define EMP_GET_ARG_20(A, ...) EMP_GET_ARG_19(__VA_ARGS__)
#define EMP_GET_ARG_21(A, ...) EMP_GET_ARG_20(__VA_ARGS__)
#define EMP_GET_ARG_22(A, ...) EMP_GET_ARG_21(__VA_ARGS__)
#define EMP_GET_ARG_23(A, ...) EMP_GET_ARG_22(__VA_ARGS__)
#define EMP_GET_ARG_24(A, ...) EMP_GET_ARG_23(__VA_ARGS__)
#define EMP_GET_ARG_25(A, ...) EMP_GET_ARG_24(__VA_ARGS__)
#define EMP_GET_ARG_26(A, ...) EMP_GET_ARG_25(__VA_ARGS__)
#define EMP_GET_ARG_27(A, ...) EMP_GET_ARG_26(__VA_ARGS__)
#define EMP_GET_ARG_28(A, ...) EMP_GET_ARG_27(__VA_ARGS__)
#define EMP_GET_ARG_29(A, ...) EMP_GET_ARG_28(__VA_ARGS__)
#define EMP_GET_ARG_30(A, ...) EMP_GET_ARG_29(__VA_ARGS__)
#define EMP_GET_ARG_31(A, ...) EMP_GET_ARG_30(__VA_ARGS__)
#define EMP_GET_ARG_32(A, ...) EMP_GET_ARG_31(__VA_ARGS__)
#define EMP_GET_ARG_33(A, ...) EMP_GET_ARG_32(__VA_ARGS__)
#define EMP_GET_ARG_34(A, ...) EMP_GET_ARG_33(__VA_ARGS__)
#define EMP_GET_ARG_35(A, ...) EMP_GET_ARG_34(__VA_ARGS__)
#define EMP_GET_ARG_36(A, ...) EMP_GET_ARG_35(__VA_ARGS__)
#define EMP_GET_ARG_37(A, ...) EMP_GET_ARG_36(__VA_ARGS__)
#define EMP_GET_ARG_38(A, ...) EMP_GET_ARG_37(__VA_ARGS__)
#define EMP_GET_ARG_39(A, ...) EMP_GET_ARG_38(__VA_ARGS__)
#define EMP_GET_ARG_40(A, ...) EMP_GET_ARG_39(__VA_ARGS__)
#define EMP_GET_ARG_41(A, ...) EMP_GET_ARG_40(__VA_ARGS__)
#define EMP_GET_ARG_42(A, ...) EMP_GET_ARG_41(__VA_ARGS__)
#define EMP_GET_ARG_43(A, ...) EMP_GET_ARG_42(__VA_ARGS__)
#define EMP_GET_ARG_44(A, ...) EMP_GET_ARG_43(__VA_ARGS__)
#define EMP_GET_ARG_45(A, ...) EMP_GET_ARG_44(__VA_ARGS__)
#define EMP_GET_ARG_46(A, ...) EMP_GET_ARG_45(__VA_ARGS__)
#define EMP_GET_ARG_47(A, ...) EMP_GET_ARG_46(__VA_ARGS__)
#define EMP_GET_ARG_48(A, ...) EMP_GET_ARG_47(__VA_ARGS__)
#define EMP_GET_ARG_49(A, ...) EMP_GET_ARG_48(__VA_ARGS__)
#define EMP_GET_ARG_50(A, ...) EMP_GET_ARG_49(__VA_ARGS__)
#define EMP_GET_ARG_51(A, ...) EMP_GET_ARG_50(__VA_ARGS__)
#define EMP_GET_ARG_52(A, ...) EMP_GET_ARG_51(__VA_ARGS__)
#define EMP_GET_ARG_53(A, ...) EMP_GET_ARG_52(__VA_ARGS__)
#define EMP_GET_ARG_54(A, ...) EMP_GET_ARG_53(__VA_ARGS__)
#define EMP_GET_ARG_55(A, ...) EMP_GET_ARG_54(__VA_ARGS__)
#define EMP_GET_ARG_56(A, ...) EMP_GET_ARG_55(__VA_ARGS__)
#define EMP_GET_ARG_57(A, ...) EMP_GET_ARG_56(__VA_ARGS__)
#define EMP_GET_ARG_58(A, ...) EMP_GET_ARG_57(__VA_ARGS__)
#define EMP_GET_ARG_59(A, ...) EMP_GET_ARG_58(__VA_ARGS__)
#define EMP_GET_ARG_60(A, ...) EMP_GET_ARG_59(__VA_ARGS__)
#define EMP_GET_ARG_61(A, ...) EMP_GET_ARG_60(__VA_ARGS__)
#define EMP_GET_ARG_62(A, ...) EMP_GET_ARG_61(__VA_ARGS__)
#define EMP_GET_ARG_63(A, ...) EMP_GET_ARG_62(__VA_ARGS__)
#define EMP_GET_ARG_64(A, ...) EMP_GET_ARG_63(__VA_ARGS__)

#define EMP_COUNT_ARGS(...) EMP_GET_ARG_64(__VA_ARGS__, 63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49, 48, 47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)

#define EMP_GET_ODD_ARGS(...) EMP_ASSEMBLE_MACRO(EMP_GET_ODD_ARGS_, EMP_COUNT_ARGS(__VA_ARGS__), __VA_ARGS__)
#define EMP_GET_ODD_ARGS_1(A) A
#define EMP_GET_ODD_ARGS_2(A, B) A
#define EMP_GET_ODD_ARGS_3(A, B, ...) A, EMP_GET_ODD_ARGS_1(__VA_ARGS__)
#define EMP_GET_ODD_ARGS_4(A, B, ...) A, EMP_GET_ODD_ARGS_2(__VA_ARGS__)
#define EMP_GET_ODD_ARGS_5(A, B, ...) A, EMP_GET_ODD_ARGS_3(__VA_ARGS__)
#define EMP_GET_ODD_ARGS_6(A, B, ...) A, EMP_GET_ODD_ARGS_4(__VA_ARGS__)
#define EMP_GET_ODD_ARGS_7(A, B, ...) A, EMP_GET_ODD_ARGS_5(__VA_ARGS__)
#define EMP_GET_ODD_ARGS_8(A, B, ...) A, EMP_GET_ODD_ARGS_6(__VA_ARGS__)
#define EMP_GET_ODD_ARGS_9(A, B, ...) A, EMP_GET_ODD_ARGS_7(__VA_ARGS__)
#define EMP_GET_ODD_ARGS_10(A, B, ...) A, EMP_GET_ODD_ARGS_8(__VA_ARGS__)
#define EMP_GET_ODD_ARGS_11(A, B, ...) A, EMP_GET_ODD_ARGS_9(__VA_ARGS__)
#define EMP_GET_ODD_ARGS_12(A, B, ...) A, EMP_GET_ODD_ARGS_10(__VA_ARGS__)
#define EMP_GET_ODD_ARGS_13(A, B, ...) A, EMP_GET_ODD_ARGS_11(__VA_ARGS__)
#define EMP_GET_ODD_ARGS_14(A, B, ...) A, EMP_GET_ODD_ARGS_12(__VA_ARGS__)
#define EMP_GET_ODD_ARGS_15(A, B, ...) A, EMP_GET_ODD_ARGS_13(__VA_ARGS__)
#define EMP_GET_ODD_ARGS_16(A, B, ...) A, EMP_GET_ODD_ARGS_14(__VA_ARGS__)
#define EMP_GET_ODD_ARGS_17(A, B, ...) A, EMP_GET_ODD_ARGS_15(__VA_ARGS__)
#define EMP_GET_ODD_ARGS_18(A, B, ...) A, EMP_GET_ODD_ARGS_16(__VA_ARGS__)
#define EMP_GET_ODD_ARGS_19(A, B, ...) A, EMP_GET_ODD_ARGS_17(__VA_ARGS__)
#define EMP_GET_ODD_ARGS_20(A, B, ...) A, EMP_GET_ODD_ARGS_18(__VA_ARGS__)
#define EMP_GET_ODD_ARGS_21(A, B, ...) A, EMP_GET_ODD_ARGS_19(__VA_ARGS__)
#define EMP_GET_ODD_ARGS_22(A, B, ...) A, EMP_GET_ODD_ARGS_20(__VA_ARGS__)
#define EMP_GET_ODD_ARGS_23(A, B, ...) A, EMP_GET_ODD_ARGS_21(__VA_ARGS__)
#define EMP_GET_ODD_ARGS_24(A, B, ...) A, EMP_GET_ODD_ARGS_22(__VA_ARGS__)
#define EMP_GET_ODD_ARGS_25(A, B, ...) A, EMP_GET_ODD_ARGS_23(__VA_ARGS__)
#define EMP_GET_ODD_ARGS_26(A, B, ...) A, EMP_GET_ODD_ARGS_24(__VA_ARGS__)
#define EMP_GET_ODD_ARGS_27(A, B, ...) A, EMP_GET_ODD_ARGS_25(__VA_ARGS__)
#define EMP_GET_ODD_ARGS_28(A, B, ...) A, EMP_GET_ODD_ARGS_26(__VA_ARGS__)
#define EMP_GET_ODD_ARGS_29(A, B, ...) A, EMP_GET_ODD_ARGS_27(__VA_ARGS__)
#define EMP_GET_ODD_ARGS_30(A, B, ...) A, EMP_GET_ODD_ARGS_28(__VA_ARGS__)
#define EMP_GET_ODD_ARGS_31(A, B, ...) A, EMP_GET_ODD_ARGS_29(__VA_ARGS__)
#define EMP_GET_ODD_ARGS_32(A, B, ...) A, EMP_GET_ODD_ARGS_30(__VA_ARGS__)
#define EMP_GET_ODD_ARGS_33(A, B, ...) A, EMP_GET_ODD_ARGS_31(__VA_ARGS__)
#define EMP_GET_ODD_ARGS_34(A, B, ...) A, EMP_GET_ODD_ARGS_32(__VA_ARGS__)
#define EMP_GET_ODD_ARGS_35(A, B, ...) A, EMP_GET_ODD_ARGS_33(__VA_ARGS__)
#define EMP_GET_ODD_ARGS_36(A, B, ...) A, EMP_GET_ODD_ARGS_34(__VA_ARGS__)
#define EMP_GET_ODD_ARGS_37(A, B, ...) A, EMP_GET_ODD_ARGS_35(__VA_ARGS__)
#define EMP_GET_ODD_ARGS_38(A, B, ...) A, EMP_GET_ODD_ARGS_36(__VA_ARGS__)
#define EMP_GET_ODD_ARGS_39(A, B, ...) A, EMP_GET_ODD_ARGS_37(__VA_ARGS__)
#define EMP_GET_ODD_ARGS_40(A, B, ...) A, EMP_GET_ODD_ARGS_38(__VA_ARGS__)
#define EMP_GET_ODD_ARGS_41(A, B, ...) A, EMP_GET_ODD_ARGS_39(__VA_ARGS__)
#define EMP_GET_ODD_ARGS_42(A, B, ...) A, EMP_GET_ODD_ARGS_40(__VA_ARGS__)
#define EMP_GET_ODD_ARGS_43(A, B, ...) A, EMP_GET_ODD_ARGS_41(__VA_ARGS__)
#define EMP_GET_ODD_ARGS_44(A, B, ...) A, EMP_GET_ODD_ARGS_42(__VA_ARGS__)
#define EMP_GET_ODD_ARGS_45(A, B, ...) A, EMP_GET_ODD_ARGS_43(__VA_ARGS__)
#define EMP_GET_ODD_ARGS_46(A, B, ...) A, EMP_GET_ODD_ARGS_44(__VA_ARGS__)
#define EMP_GET_ODD_ARGS_47(A, B, ...) A, EMP_GET_ODD_ARGS_45(__VA_ARGS__)
#define EMP_GET_ODD_ARGS_48(A, B, ...) A, EMP_GET_ODD_ARGS_46(__VA_ARGS__)
#define EMP_GET_ODD_ARGS_49(A, B, ...) A, EMP_GET_ODD_ARGS_47(__VA_ARGS__)
#define EMP_GET_ODD_ARGS_50(A, B, ...) A, EMP_GET_ODD_ARGS_48(__VA_ARGS__)
#define EMP_GET_ODD_ARGS_51(A, B, ...) A, EMP_GET_ODD_ARGS_49(__VA_ARGS__)
#define EMP_GET_ODD_ARGS_52(A, B, ...) A, EMP_GET_ODD_ARGS_50(__VA_ARGS__)
#define EMP_GET_ODD_ARGS_53(A, B, ...) A, EMP_GET_ODD_ARGS_51(__VA_ARGS__)
#define EMP_GET_ODD_ARGS_54(A, B, ...) A, EMP_GET_ODD_ARGS_52(__VA_ARGS__)
#define EMP_GET_ODD_ARGS_55(A, B, ...) A, EMP_GET_ODD_ARGS_53(__VA_ARGS__)
#define EMP_GET_ODD_ARGS_56(A, B, ...) A, EMP_GET_ODD_ARGS_54(__VA_ARGS__)
#define EMP_GET_ODD_ARGS_57(A, B, ...) A, EMP_GET_ODD_ARGS_55(__VA_ARGS__)
#define EMP_GET_ODD_ARGS_58(A, B, ...) A, EMP_GET_ODD_ARGS_56(__VA_ARGS__)
#define EMP_GET_ODD_ARGS_59(A, B, ...) A, EMP_GET_ODD_ARGS_57(__VA_ARGS__)
#define EMP_GET_ODD_ARGS_60(A, B, ...) A, EMP_GET_ODD_ARGS_58(__VA_ARGS__)
#define EMP_GET_ODD_ARGS_61(A, B, ...) A, EMP_GET_ODD_ARGS_59(__VA_ARGS__)
#define EMP_GET_ODD_ARGS_62(A, B, ...) A, EMP_GET_ODD_ARGS_60(__VA_ARGS__)
#define EMP_GET_ODD_ARGS_63(A, B, ...) A, EMP_GET_ODD_ARGS_61(__VA_ARGS__)


#define EMP_GET_EVEN_ARGS(...) EMP_ASSEMBLE_MACRO(EMP_GET_EVEN_ARGS_, EMP_COUNT_ARGS(__VA_ARGS__), __VA_ARGS__)
#define EMP_GET_EVEN_ARGS_1(A)
#define EMP_GET_EVEN_ARGS_2(A, B) B
#define EMP_GET_EVEN_ARGS_3(A, B, C) B
#define EMP_GET_EVEN_ARGS_4(A, B, ...) B, EMP_GET_EVEN_ARGS_2(__VA_ARGS__)
#define EMP_GET_EVEN_ARGS_5(A, B, ...) B, EMP_GET_EVEN_ARGS_3(__VA_ARGS__)
#define EMP_GET_EVEN_ARGS_6(A, B, ...) B, EMP_GET_EVEN_ARGS_4(__VA_ARGS__)
#define EMP_GET_EVEN_ARGS_7(A, B, ...) B, EMP_GET_EVEN_ARGS_5(__VA_ARGS__)
#define EMP_GET_EVEN_ARGS_8(A, B, ...) B, EMP_GET_EVEN_ARGS_6(__VA_ARGS__)
#define EMP_GET_EVEN_ARGS_9(A, B, ...) B, EMP_GET_EVEN_ARGS_7(__VA_ARGS__)
#define EMP_GET_EVEN_ARGS_10(A, B, ...) B, EMP_GET_EVEN_ARGS_8(__VA_ARGS__)
#define EMP_GET_EVEN_ARGS_11(A, B, ...) B, EMP_GET_EVEN_ARGS_9(__VA_ARGS__)
#define EMP_GET_EVEN_ARGS_12(A, B, ...) B, EMP_GET_EVEN_ARGS_10(__VA_ARGS__)
#define EMP_GET_EVEN_ARGS_13(A, B, ...) B, EMP_GET_EVEN_ARGS_11(__VA_ARGS__)
#define EMP_GET_EVEN_ARGS_14(A, B, ...) B, EMP_GET_EVEN_ARGS_12(__VA_ARGS__)
#define EMP_GET_EVEN_ARGS_15(A, B, ...) B, EMP_GET_EVEN_ARGS_13(__VA_ARGS__)
#define EMP_GET_EVEN_ARGS_16(A, B, ...) B, EMP_GET_EVEN_ARGS_14(__VA_ARGS__)
#define EMP_GET_EVEN_ARGS_17(A, B, ...) B, EMP_GET_EVEN_ARGS_15(__VA_ARGS__)
#define EMP_GET_EVEN_ARGS_18(A, B, ...) B, EMP_GET_EVEN_ARGS_16(__VA_ARGS__)
#define EMP_GET_EVEN_ARGS_19(A, B, ...) B, EMP_GET_EVEN_ARGS_17(__VA_ARGS__)
#define EMP_GET_EVEN_ARGS_20(A, B, ...) B, EMP_GET_EVEN_ARGS_18(__VA_ARGS__)
#define EMP_GET_EVEN_ARGS_21(A, B, ...) B, EMP_GET_EVEN_ARGS_19(__VA_ARGS__)
#define EMP_GET_EVEN_ARGS_22(A, B, ...) B, EMP_GET_EVEN_ARGS_20(__VA_ARGS__)
#define EMP_GET_EVEN_ARGS_23(A, B, ...) B, EMP_GET_EVEN_ARGS_21(__VA_ARGS__)
#define EMP_GET_EVEN_ARGS_24(A, B, ...) B, EMP_GET_EVEN_ARGS_22(__VA_ARGS__)
#define EMP_GET_EVEN_ARGS_25(A, B, ...) B, EMP_GET_EVEN_ARGS_23(__VA_ARGS__)
#define EMP_GET_EVEN_ARGS_26(A, B, ...) B, EMP_GET_EVEN_ARGS_24(__VA_ARGS__)
#define EMP_GET_EVEN_ARGS_27(A, B, ...) B, EMP_GET_EVEN_ARGS_25(__VA_ARGS__)
#define EMP_GET_EVEN_ARGS_28(A, B, ...) B, EMP_GET_EVEN_ARGS_26(__VA_ARGS__)
#define EMP_GET_EVEN_ARGS_29(A, B, ...) B, EMP_GET_EVEN_ARGS_27(__VA_ARGS__)
#define EMP_GET_EVEN_ARGS_30(A, B, ...) B, EMP_GET_EVEN_ARGS_28(__VA_ARGS__)
#define EMP_GET_EVEN_ARGS_31(A, B, ...) B, EMP_GET_EVEN_ARGS_29(__VA_ARGS__)
#define EMP_GET_EVEN_ARGS_32(A, B, ...) B, EMP_GET_EVEN_ARGS_30(__VA_ARGS__)
#define EMP_GET_EVEN_ARGS_33(A, B, ...) B, EMP_GET_EVEN_ARGS_31(__VA_ARGS__)
#define EMP_GET_EVEN_ARGS_34(A, B, ...) B, EMP_GET_EVEN_ARGS_32(__VA_ARGS__)
#define EMP_GET_EVEN_ARGS_35(A, B, ...) B, EMP_GET_EVEN_ARGS_33(__VA_ARGS__)
#define EMP_GET_EVEN_ARGS_36(A, B, ...) B, EMP_GET_EVEN_ARGS_34(__VA_ARGS__)
#define EMP_GET_EVEN_ARGS_37(A, B, ...) B, EMP_GET_EVEN_ARGS_35(__VA_ARGS__)
#define EMP_GET_EVEN_ARGS_38(A, B, ...) B, EMP_GET_EVEN_ARGS_36(__VA_ARGS__)
#define EMP_GET_EVEN_ARGS_39(A, B, ...) B, EMP_GET_EVEN_ARGS_37(__VA_ARGS__)
#define EMP_GET_EVEN_ARGS_40(A, B, ...) B, EMP_GET_EVEN_ARGS_38(__VA_ARGS__)
#define EMP_GET_EVEN_ARGS_41(A, B, ...) B, EMP_GET_EVEN_ARGS_39(__VA_ARGS__)
#define EMP_GET_EVEN_ARGS_42(A, B, ...) B, EMP_GET_EVEN_ARGS_40(__VA_ARGS__)
#define EMP_GET_EVEN_ARGS_43(A, B, ...) B, EMP_GET_EVEN_ARGS_41(__VA_ARGS__)
#define EMP_GET_EVEN_ARGS_44(A, B, ...) B, EMP_GET_EVEN_ARGS_42(__VA_ARGS__)
#define EMP_GET_EVEN_ARGS_45(A, B, ...) B, EMP_GET_EVEN_ARGS_43(__VA_ARGS__)
#define EMP_GET_EVEN_ARGS_46(A, B, ...) B, EMP_GET_EVEN_ARGS_44(__VA_ARGS__)
#define EMP_GET_EVEN_ARGS_47(A, B, ...) B, EMP_GET_EVEN_ARGS_45(__VA_ARGS__)
#define EMP_GET_EVEN_ARGS_48(A, B, ...) B, EMP_GET_EVEN_ARGS_46(__VA_ARGS__)
#define EMP_GET_EVEN_ARGS_49(A, B, ...) B, EMP_GET_EVEN_ARGS_47(__VA_ARGS__)
#define EMP_GET_EVEN_ARGS_50(A, B, ...) B, EMP_GET_EVEN_ARGS_48(__VA_ARGS__)
#define EMP_GET_EVEN_ARGS_51(A, B, ...) B, EMP_GET_EVEN_ARGS_49(__VA_ARGS__)
#define EMP_GET_EVEN_ARGS_52(A, B, ...) B, EMP_GET_EVEN_ARGS_50(__VA_ARGS__)
#define EMP_GET_EVEN_ARGS_53(A, B, ...) B, EMP_GET_EVEN_ARGS_51(__VA_ARGS__)
#define EMP_GET_EVEN_ARGS_54(A, B, ...) B, EMP_GET_EVEN_ARGS_52(__VA_ARGS__)
#define EMP_GET_EVEN_ARGS_55(A, B, ...) B, EMP_GET_EVEN_ARGS_53(__VA_ARGS__)
#define EMP_GET_EVEN_ARGS_56(A, B, ...) B, EMP_GET_EVEN_ARGS_54(__VA_ARGS__)
#define EMP_GET_EVEN_ARGS_57(A, B, ...) B, EMP_GET_EVEN_ARGS_55(__VA_ARGS__)
#define EMP_GET_EVEN_ARGS_58(A, B, ...) B, EMP_GET_EVEN_ARGS_56(__VA_ARGS__)
#define EMP_GET_EVEN_ARGS_59(A, B, ...) B, EMP_GET_EVEN_ARGS_57(__VA_ARGS__)
#define EMP_GET_EVEN_ARGS_60(A, B, ...) B, EMP_GET_EVEN_ARGS_58(__VA_ARGS__)
#define EMP_GET_EVEN_ARGS_61(A, B, ...) B, EMP_GET_EVEN_ARGS_59(__VA_ARGS__)
#define EMP_GET_EVEN_ARGS_62(A, B, ...) B, EMP_GET_EVEN_ARGS_60(__VA_ARGS__)
#define EMP_GET_EVEN_ARGS_63(A, B, ...) B, EMP_GET_EVEN_ARGS_61(__VA_ARGS__)


// Enable an arbitrary number of arguments (well, up to 10) to be merged AFTER being processed!
#define EMP_MERGE(...) EMP_ASSEMBLE_MACRO(EMP_MERGE_, EMP_COUNT_ARGS(__VA_ARGS__), __VA_ARGS__)
#define EMP_MERGE_1(A1) A1
#define EMP_MERGE_2(A1,A2) A1 ## A2
#define EMP_MERGE_3(A1,A2,A3) A1 ## A2 ## A3
#define EMP_MERGE_4(A1,A2,A3,A4) A1 ## A2 ## A3 ## A4
#define EMP_MERGE_5(A1,A2,A3,A4,A5) A1 ## A2 ## A3 ## A4 ## A5
#define EMP_MERGE_6(A1,A2,A3,A4,A5,A6) A1 ## A2 ## A3 ## A4 ## A5 ## A6
#define EMP_MERGE_7(A1,A2,A3,A4,A5,A6,A7) A1 ## A2 ## A3 ## A4 ## A5 ## A6 ## A7
#define EMP_MERGE_8(A1,A2,A3,A4,A5,A6,A7,A8) A1 ## A2 ## A3 ## A4 ## A5 ## A6 ## A7 ## A8
#define EMP_MERGE_9(A1,A2,A3,A4,A5,A6,A7,A8,A9) A1 ## A2 ## A3 ## A4 ## A5 ## A6 ## A7 ## A8 ## A9
#define EMP_MERGE_10(A1,A2,A3,A4,A5,A6,A7,A8,A9,A10) A1 ## A2 ## A3 ## A4 ## A5 ## A6 ## A7 ## A8 ## A9 ## A10


// EMP_WRAP_EACH takes a wrapper macro and a variable set of arguments,
// then applied to wrapper macro to each argument in order.
#define EMP_WRAP_EACH(W, ...) EMP_ASSEMBLE_MACRO(EMP_LAYOUT_, EMP_COUNT_ARGS(__VA_ARGS__), W, , __VA_ARGS__)

// EMP_LAYOUT takes a wrapper macro and padding information, wraps each argument in the macro
// and then spaces them out with the padding.
// W = Wrapper macro name, P = Padding between results
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


#define EMP_WRAP_ARGS(W, ...) EMP_ASSEMBLE_MACRO(EMP_WRAP_ARGS_, EMP_COUNT_ARGS(__VA_ARGS__), W, __VA_ARGS__)
#define EMP_WRAP_ARGS_1(W, A) W(A)
#define EMP_WRAP_ARGS_2(W, A, ...) W(A), EMP_WRAP_ARGS_1(W, __VA_ARGS__)
#define EMP_WRAP_ARGS_3(W, A, ...) W(A), EMP_WRAP_ARGS_2(W, __VA_ARGS__)
#define EMP_WRAP_ARGS_4(W, A, ...) W(A), EMP_WRAP_ARGS_3(W, __VA_ARGS__)
#define EMP_WRAP_ARGS_5(W, A, ...) W(A), EMP_WRAP_ARGS_4(W, __VA_ARGS__)
#define EMP_WRAP_ARGS_6(W, A, ...) W(A), EMP_WRAP_ARGS_5(W, __VA_ARGS__)
#define EMP_WRAP_ARGS_7(W, A, ...) W(A), EMP_WRAP_ARGS_6(W, __VA_ARGS__)
#define EMP_WRAP_ARGS_8(W, A, ...) W(A), EMP_WRAP_ARGS_7(W, __VA_ARGS__)
#define EMP_WRAP_ARGS_9(W, A, ...) W(A), EMP_WRAP_ARGS_8(W, __VA_ARGS__)
#define EMP_WRAP_ARGS_10(W, A, ...) W(A), EMP_WRAP_ARGS_9(W, __VA_ARGS__)
#define EMP_WRAP_ARGS_11(W, A, ...) W(A), EMP_WRAP_ARGS_10(W, __VA_ARGS__)
#define EMP_WRAP_ARGS_12(W, A, ...) W(A), EMP_WRAP_ARGS_11(W, __VA_ARGS__)
#define EMP_WRAP_ARGS_13(W, A, ...) W(A), EMP_WRAP_ARGS_12(W, __VA_ARGS__)
#define EMP_WRAP_ARGS_14(W, A, ...) W(A), EMP_WRAP_ARGS_13(W, __VA_ARGS__)
#define EMP_WRAP_ARGS_15(W, A, ...) W(A), EMP_WRAP_ARGS_14(W, __VA_ARGS__)
#define EMP_WRAP_ARGS_16(W, A, ...) W(A), EMP_WRAP_ARGS_15(W, __VA_ARGS__)
#define EMP_WRAP_ARGS_17(W, A, ...) W(A), EMP_WRAP_ARGS_16(W, __VA_ARGS__)
#define EMP_WRAP_ARGS_18(W, A, ...) W(A), EMP_WRAP_ARGS_17(W, __VA_ARGS__)
#define EMP_WRAP_ARGS_19(W, A, ...) W(A), EMP_WRAP_ARGS_18(W, __VA_ARGS__)
#define EMP_WRAP_ARGS_20(W, A, ...) W(A), EMP_WRAP_ARGS_19(W, __VA_ARGS__)
#define EMP_WRAP_ARGS_21(W, A, ...) W(A), EMP_WRAP_ARGS_20(W, __VA_ARGS__)
#define EMP_WRAP_ARGS_22(W, A, ...) W(A), EMP_WRAP_ARGS_21(W, __VA_ARGS__)
#define EMP_WRAP_ARGS_23(W, A, ...) W(A), EMP_WRAP_ARGS_22(W, __VA_ARGS__)
#define EMP_WRAP_ARGS_24(W, A, ...) W(A), EMP_WRAP_ARGS_23(W, __VA_ARGS__)
#define EMP_WRAP_ARGS_25(W, A, ...) W(A), EMP_WRAP_ARGS_24(W, __VA_ARGS__)
#define EMP_WRAP_ARGS_26(W, A, ...) W(A), EMP_WRAP_ARGS_25(W, __VA_ARGS__)
#define EMP_WRAP_ARGS_27(W, A, ...) W(A), EMP_WRAP_ARGS_26(W, __VA_ARGS__)
#define EMP_WRAP_ARGS_28(W, A, ...) W(A), EMP_WRAP_ARGS_27(W, __VA_ARGS__)
#define EMP_WRAP_ARGS_29(W, A, ...) W(A), EMP_WRAP_ARGS_28(W, __VA_ARGS__)
#define EMP_WRAP_ARGS_30(W, A, ...) W(A), EMP_WRAP_ARGS_29(W, __VA_ARGS__)
#define EMP_WRAP_ARGS_31(W, A, ...) W(A), EMP_WRAP_ARGS_30(W, __VA_ARGS__)
#define EMP_WRAP_ARGS_32(W, A, ...) W(A), EMP_WRAP_ARGS_31(W, __VA_ARGS__)
#define EMP_WRAP_ARGS_33(W, A, ...) W(A), EMP_WRAP_ARGS_32(W, __VA_ARGS__)
#define EMP_WRAP_ARGS_34(W, A, ...) W(A), EMP_WRAP_ARGS_33(W, __VA_ARGS__)
#define EMP_WRAP_ARGS_35(W, A, ...) W(A), EMP_WRAP_ARGS_34(W, __VA_ARGS__)
#define EMP_WRAP_ARGS_36(W, A, ...) W(A), EMP_WRAP_ARGS_35(W, __VA_ARGS__)
#define EMP_WRAP_ARGS_37(W, A, ...) W(A), EMP_WRAP_ARGS_36(W, __VA_ARGS__)
#define EMP_WRAP_ARGS_38(W, A, ...) W(A), EMP_WRAP_ARGS_37(W, __VA_ARGS__)
#define EMP_WRAP_ARGS_39(W, A, ...) W(A), EMP_WRAP_ARGS_38(W, __VA_ARGS__)
#define EMP_WRAP_ARGS_40(W, A, ...) W(A), EMP_WRAP_ARGS_39(W, __VA_ARGS__)
#define EMP_WRAP_ARGS_41(W, A, ...) W(A), EMP_WRAP_ARGS_40(W, __VA_ARGS__)
#define EMP_WRAP_ARGS_42(W, A, ...) W(A), EMP_WRAP_ARGS_41(W, __VA_ARGS__)
#define EMP_WRAP_ARGS_43(W, A, ...) W(A), EMP_WRAP_ARGS_42(W, __VA_ARGS__)
#define EMP_WRAP_ARGS_44(W, A, ...) W(A), EMP_WRAP_ARGS_43(W, __VA_ARGS__)
#define EMP_WRAP_ARGS_45(W, A, ...) W(A), EMP_WRAP_ARGS_44(W, __VA_ARGS__)
#define EMP_WRAP_ARGS_46(W, A, ...) W(A), EMP_WRAP_ARGS_45(W, __VA_ARGS__)
#define EMP_WRAP_ARGS_47(W, A, ...) W(A), EMP_WRAP_ARGS_46(W, __VA_ARGS__)
#define EMP_WRAP_ARGS_48(W, A, ...) W(A), EMP_WRAP_ARGS_47(W, __VA_ARGS__)
#define EMP_WRAP_ARGS_49(W, A, ...) W(A), EMP_WRAP_ARGS_48(W, __VA_ARGS__)
#define EMP_WRAP_ARGS_50(W, A, ...) W(A), EMP_WRAP_ARGS_49(W, __VA_ARGS__)
#define EMP_WRAP_ARGS_51(W, A, ...) W(A), EMP_WRAP_ARGS_50(W, __VA_ARGS__)
#define EMP_WRAP_ARGS_52(W, A, ...) W(A), EMP_WRAP_ARGS_51(W, __VA_ARGS__)
#define EMP_WRAP_ARGS_53(W, A, ...) W(A), EMP_WRAP_ARGS_52(W, __VA_ARGS__)
#define EMP_WRAP_ARGS_54(W, A, ...) W(A), EMP_WRAP_ARGS_53(W, __VA_ARGS__)
#define EMP_WRAP_ARGS_55(W, A, ...) W(A), EMP_WRAP_ARGS_54(W, __VA_ARGS__)
#define EMP_WRAP_ARGS_56(W, A, ...) W(A), EMP_WRAP_ARGS_55(W, __VA_ARGS__)
#define EMP_WRAP_ARGS_57(W, A, ...) W(A), EMP_WRAP_ARGS_56(W, __VA_ARGS__)
#define EMP_WRAP_ARGS_58(W, A, ...) W(A), EMP_WRAP_ARGS_57(W, __VA_ARGS__)
#define EMP_WRAP_ARGS_59(W, A, ...) W(A), EMP_WRAP_ARGS_58(W, __VA_ARGS__)
#define EMP_WRAP_ARGS_60(W, A, ...) W(A), EMP_WRAP_ARGS_59(W, __VA_ARGS__)
#define EMP_WRAP_ARGS_61(W, A, ...) W(A), EMP_WRAP_ARGS_60(W, __VA_ARGS__)
#define EMP_WRAP_ARGS_62(W, A, ...) W(A), EMP_WRAP_ARGS_61(W, __VA_ARGS__)
#define EMP_WRAP_ARGS_63(W, A, ...) W(A), EMP_WRAP_ARGS_62(W, __VA_ARGS__)

#define EMP_WRAP_ARG_PAIRS(W, ...) EMP_ASSEMBLE_MACRO(EMP_WRAP_ARG_PAIRS_, EMP_COUNT_ARGS(__VA_ARGS__), W, __VA_ARGS__)
#define EMP_WRAP_ARG_PAIRS_2(W, A1, A2) W(A1, A2)
#define EMP_WRAP_ARG_PAIRS_4(W, A1, A2, ...) W(A1, A2), EMP_WRAP_ARG_PAIRS_2(W, __VA_ARGS__)
#define EMP_WRAP_ARG_PAIRS_6(W, A1, A2, ...) W(A1, A2), EMP_WRAP_ARG_PAIRS_4(W, __VA_ARGS__)
#define EMP_WRAP_ARG_PAIRS_8(W, A1, A2, ...) W(A1, A2), EMP_WRAP_ARG_PAIRS_6(W, __VA_ARGS__)
#define EMP_WRAP_ARG_PAIRS_10(W, A1, A2, ...) W(A1, A2), EMP_WRAP_ARG_PAIRS_8(W, __VA_ARGS__) 
#define EMP_WRAP_ARG_PAIRS_12(W, A1, A2, ...) W(A1, A2), EMP_WRAP_ARG_PAIRS_10(W, __VA_ARGS__)
#define EMP_WRAP_ARG_PAIRS_14(W, A1, A2, ...) W(A1, A2), EMP_WRAP_ARG_PAIRS_12(W, __VA_ARGS__)
#define EMP_WRAP_ARG_PAIRS_16(W, A1, A2, ...) W(A1, A2), EMP_WRAP_ARG_PAIRS_14(W, __VA_ARGS__)
#define EMP_WRAP_ARG_PAIRS_18(W, A1, A2, ...) W(A1, A2), EMP_WRAP_ARG_PAIRS_16(W, __VA_ARGS__)
#define EMP_WRAP_ARG_PAIRS_20(W, A1, A2, ...) W(A1, A2), EMP_WRAP_ARG_PAIRS_18(W, __VA_ARGS__)
#define EMP_WRAP_ARG_PAIRS_22(W, A1, A2, ...) W(A1, A2), EMP_WRAP_ARG_PAIRS_20(W, __VA_ARGS__)
#define EMP_WRAP_ARG_PAIRS_24(W, A1, A2, ...) W(A1, A2), EMP_WRAP_ARG_PAIRS_22(W, __VA_ARGS__)
#define EMP_WRAP_ARG_PAIRS_26(W, A1, A2, ...) W(A1, A2), EMP_WRAP_ARG_PAIRS_24(W, __VA_ARGS__)
#define EMP_WRAP_ARG_PAIRS_28(W, A1, A2, ...) W(A1, A2), EMP_WRAP_ARG_PAIRS_26(W, __VA_ARGS__)
#define EMP_WRAP_ARG_PAIRS_30(W, A1, A2, ...) W(A1, A2), EMP_WRAP_ARG_PAIRS_28(W, __VA_ARGS__)
#define EMP_WRAP_ARG_PAIRS_32(W, A1, A2, ...) W(A1, A2), EMP_WRAP_ARG_PAIRS_30(W, __VA_ARGS__)
#define EMP_WRAP_ARG_PAIRS_34(W, A1, A2, ...) W(A1, A2), EMP_WRAP_ARG_PAIRS_32(W, __VA_ARGS__)
#define EMP_WRAP_ARG_PAIRS_36(W, A1, A2, ...) W(A1, A2), EMP_WRAP_ARG_PAIRS_34(W, __VA_ARGS__)
#define EMP_WRAP_ARG_PAIRS_38(W, A1, A2, ...) W(A1, A2), EMP_WRAP_ARG_PAIRS_36(W, __VA_ARGS__)
#define EMP_WRAP_ARG_PAIRS_40(W, A1, A2, ...) W(A1, A2), EMP_WRAP_ARG_PAIRS_38(W, __VA_ARGS__)
#define EMP_WRAP_ARG_PAIRS_42(W, A1, A2, ...) W(A1, A2), EMP_WRAP_ARG_PAIRS_40(W, __VA_ARGS__)
#define EMP_WRAP_ARG_PAIRS_44(W, A1, A2, ...) W(A1, A2), EMP_WRAP_ARG_PAIRS_42(W, __VA_ARGS__)
#define EMP_WRAP_ARG_PAIRS_46(W, A1, A2, ...) W(A1, A2), EMP_WRAP_ARG_PAIRS_44(W, __VA_ARGS__)
#define EMP_WRAP_ARG_PAIRS_48(W, A1, A2, ...) W(A1, A2), EMP_WRAP_ARG_PAIRS_46(W, __VA_ARGS__)
#define EMP_WRAP_ARG_PAIRS_50(W, A1, A2, ...) W(A1, A2), EMP_WRAP_ARG_PAIRS_48(W, __VA_ARGS__)
#define EMP_WRAP_ARG_PAIRS_52(W, A1, A2, ...) W(A1, A2), EMP_WRAP_ARG_PAIRS_50(W, __VA_ARGS__)
#define EMP_WRAP_ARG_PAIRS_54(W, A1, A2, ...) W(A1, A2), EMP_WRAP_ARG_PAIRS_52(W, __VA_ARGS__)
#define EMP_WRAP_ARG_PAIRS_56(W, A1, A2, ...) W(A1, A2), EMP_WRAP_ARG_PAIRS_54(W, __VA_ARGS__)
#define EMP_WRAP_ARG_PAIRS_58(W, A1, A2, ...) W(A1, A2), EMP_WRAP_ARG_PAIRS_56(W, __VA_ARGS__)
#define EMP_WRAP_ARG_PAIRS_60(W, A1, A2, ...) W(A1, A2), EMP_WRAP_ARG_PAIRS_58(W, __VA_ARGS__)
#define EMP_WRAP_ARG_PAIRS_62(W, A1, A2, ...) W(A1, A2), EMP_WRAP_ARG_PAIRS_60(W, __VA_ARGS__)
#define EMP_WRAP_ARG_PAIRS_64(W, A1, A2, ...) W(A1, A2), EMP_WRAP_ARG_PAIRS_62(W, __VA_ARGS__)
#define EMP_WRAP_ARG_PAIRS_66(W, A1, A2, ...) W(A1, A2), EMP_WRAP_ARG_PAIRS_64(W, __VA_ARGS__)
#define EMP_WRAP_ARG_PAIRS_68(W, A1, A2, ...) W(A1, A2), EMP_WRAP_ARG_PAIRS_66(W, __VA_ARGS__)
#define EMP_WRAP_ARG_PAIRS_70(W, A1, A2, ...) W(A1, A2), EMP_WRAP_ARG_PAIRS_68(W, __VA_ARGS__)
#define EMP_WRAP_ARG_PAIRS_72(W, A1, A2, ...) W(A1, A2), EMP_WRAP_ARG_PAIRS_70(W, __VA_ARGS__)
#define EMP_WRAP_ARG_PAIRS_74(W, A1, A2, ...) W(A1, A2), EMP_WRAP_ARG_PAIRS_72(W, __VA_ARGS__)
#define EMP_WRAP_ARG_PAIRS_76(W, A1, A2, ...) W(A1, A2), EMP_WRAP_ARG_PAIRS_74(W, __VA_ARGS__)
#define EMP_WRAP_ARG_PAIRS_78(W, A1, A2, ...) W(A1, A2), EMP_WRAP_ARG_PAIRS_76(W, __VA_ARGS__)
#define EMP_WRAP_ARG_PAIRS_80(W, A1, A2, ...) W(A1, A2), EMP_WRAP_ARG_PAIRS_78(W, __VA_ARGS__)
#define EMP_WRAP_ARG_PAIRS_82(W, A1, A2, ...) W(A1, A2), EMP_WRAP_ARG_PAIRS_80(W, __VA_ARGS__)
#define EMP_WRAP_ARG_PAIRS_84(W, A1, A2, ...) W(A1, A2), EMP_WRAP_ARG_PAIRS_82(W, __VA_ARGS__)
#define EMP_WRAP_ARG_PAIRS_86(W, A1, A2, ...) W(A1, A2), EMP_WRAP_ARG_PAIRS_84(W, __VA_ARGS__)
#define EMP_WRAP_ARG_PAIRS_88(W, A1, A2, ...) W(A1, A2), EMP_WRAP_ARG_PAIRS_86(W, __VA_ARGS__)
#define EMP_WRAP_ARG_PAIRS_90(W, A1, A2, ...) W(A1, A2), EMP_WRAP_ARG_PAIRS_88(W, __VA_ARGS__)
#define EMP_WRAP_ARG_PAIRS_92(W, A1, A2, ...) W(A1, A2), EMP_WRAP_ARG_PAIRS_90(W, __VA_ARGS__)
#define EMP_WRAP_ARG_PAIRS_94(W, A1, A2, ...) W(A1, A2), EMP_WRAP_ARG_PAIRS_92(W, __VA_ARGS__)
#define EMP_WRAP_ARG_PAIRS_96(W, A1, A2, ...) W(A1, A2), EMP_WRAP_ARG_PAIRS_94(W, __VA_ARGS__)
#define EMP_WRAP_ARG_PAIRS_98(W, A1, A2, ...) W(A1, A2), EMP_WRAP_ARG_PAIRS_96(W, __VA_ARGS__)
#define EMP_WRAP_ARG_PAIRS_100(W, A1, A2, ...) W(A1, A2), EMP_WRAP_ARG_PAIRS_98(W, __VA_ARGS__)
#define EMP_WRAP_ARG_PAIRS_102(W, A1, A2, ...) W(A1, A2), EMP_WRAP_ARG_PAIRS_100(W, __VA_ARGS__)
#define EMP_WRAP_ARG_PAIRS_104(W, A1, A2, ...) W(A1, A2), EMP_WRAP_ARG_PAIRS_102(W, __VA_ARGS__)
#define EMP_WRAP_ARG_PAIRS_106(W, A1, A2, ...) W(A1, A2), EMP_WRAP_ARG_PAIRS_104(W, __VA_ARGS__)
#define EMP_WRAP_ARG_PAIRS_108(W, A1, A2, ...) W(A1, A2), EMP_WRAP_ARG_PAIRS_106(W, __VA_ARGS__)
#define EMP_WRAP_ARG_PAIRS_110(W, A1, A2, ...) W(A1, A2), EMP_WRAP_ARG_PAIRS_108(W, __VA_ARGS__)
#define EMP_WRAP_ARG_PAIRS_112(W, A1, A2, ...) W(A1, A2), EMP_WRAP_ARG_PAIRS_110(W, __VA_ARGS__)
#define EMP_WRAP_ARG_PAIRS_114(W, A1, A2, ...) W(A1, A2), EMP_WRAP_ARG_PAIRS_112(W, __VA_ARGS__)
#define EMP_WRAP_ARG_PAIRS_116(W, A1, A2, ...) W(A1, A2), EMP_WRAP_ARG_PAIRS_114(W, __VA_ARGS__)
#define EMP_WRAP_ARG_PAIRS_118(W, A1, A2, ...) W(A1, A2), EMP_WRAP_ARG_PAIRS_116(W, __VA_ARGS__)
#define EMP_WRAP_ARG_PAIRS_120(W, A1, A2, ...) W(A1, A2), EMP_WRAP_ARG_PAIRS_118(W, __VA_ARGS__)
#define EMP_WRAP_ARG_PAIRS_122(W, A1, A2, ...) W(A1, A2), EMP_WRAP_ARG_PAIRS_120(W, __VA_ARGS__)
#define EMP_WRAP_ARG_PAIRS_124(W, A1, A2, ...) W(A1, A2), EMP_WRAP_ARG_PAIRS_122(W, __VA_ARGS__)
#define EMP_WRAP_ARG_PAIRS_126(W, A1, A2, ...) W(A1, A2), EMP_WRAP_ARG_PAIRS_124(W, __VA_ARGS__)
#define EMP_WRAP_ARG_PAIRS_128(W, A1, A2, ...) W(A1, A2), EMP_WRAP_ARG_PAIRS_126(W, __VA_ARGS__)

#define EMP_REVERSE_ARGS(...) EMP_ASSEMBLE_MACRO(EMP_REVERSE_ARGS_, EMP_COUNT_ARGS(__VA_ARGS__), __VA_ARGS__)
#define EMP_REVERSE_ARGS_1(A) A
#define EMP_REVERSE_ARGS_2(A, ...) EMP_REVERSE_ARGS_1(__VA_ARGS__), A
#define EMP_REVERSE_ARGS_3(A, ...) EMP_REVERSE_ARGS_2(__VA_ARGS__), A
#define EMP_REVERSE_ARGS_4(A, ...) EMP_REVERSE_ARGS_3(__VA_ARGS__), A
#define EMP_REVERSE_ARGS_5(A, ...) EMP_REVERSE_ARGS_4(__VA_ARGS__), A
#define EMP_REVERSE_ARGS_6(A, ...) EMP_REVERSE_ARGS_5(__VA_ARGS__), A
#define EMP_REVERSE_ARGS_7(A, ...) EMP_REVERSE_ARGS_6(__VA_ARGS__), A
#define EMP_REVERSE_ARGS_8(A, ...) EMP_REVERSE_ARGS_7(__VA_ARGS__), A
#define EMP_REVERSE_ARGS_9(A, ...) EMP_REVERSE_ARGS_8(__VA_ARGS__), A
#define EMP_REVERSE_ARGS_10(A, ...) EMP_REVERSE_ARGS_9(__VA_ARGS__), A
#define EMP_REVERSE_ARGS_11(A, ...) EMP_REVERSE_ARGS_10(__VA_ARGS__), A
#define EMP_REVERSE_ARGS_12(A, ...) EMP_REVERSE_ARGS_11(__VA_ARGS__), A
#define EMP_REVERSE_ARGS_13(A, ...) EMP_REVERSE_ARGS_12(__VA_ARGS__), A
#define EMP_REVERSE_ARGS_14(A, ...) EMP_REVERSE_ARGS_13(__VA_ARGS__), A
#define EMP_REVERSE_ARGS_15(A, ...) EMP_REVERSE_ARGS_14(__VA_ARGS__), A
#define EMP_REVERSE_ARGS_16(A, ...) EMP_REVERSE_ARGS_15(__VA_ARGS__), A
#define EMP_REVERSE_ARGS_17(A, ...) EMP_REVERSE_ARGS_16(__VA_ARGS__), A
#define EMP_REVERSE_ARGS_18(A, ...) EMP_REVERSE_ARGS_17(__VA_ARGS__), A
#define EMP_REVERSE_ARGS_19(A, ...) EMP_REVERSE_ARGS_18(__VA_ARGS__), A
#define EMP_REVERSE_ARGS_20(A, ...) EMP_REVERSE_ARGS_19(__VA_ARGS__), A
#define EMP_REVERSE_ARGS_21(A, ...) EMP_REVERSE_ARGS_20(__VA_ARGS__), A
#define EMP_REVERSE_ARGS_22(A, ...) EMP_REVERSE_ARGS_21(__VA_ARGS__), A
#define EMP_REVERSE_ARGS_23(A, ...) EMP_REVERSE_ARGS_22(__VA_ARGS__), A
#define EMP_REVERSE_ARGS_24(A, ...) EMP_REVERSE_ARGS_23(__VA_ARGS__), A
#define EMP_REVERSE_ARGS_25(A, ...) EMP_REVERSE_ARGS_24(__VA_ARGS__), A
#define EMP_REVERSE_ARGS_26(A, ...) EMP_REVERSE_ARGS_25(__VA_ARGS__), A
#define EMP_REVERSE_ARGS_27(A, ...) EMP_REVERSE_ARGS_26(__VA_ARGS__), A
#define EMP_REVERSE_ARGS_28(A, ...) EMP_REVERSE_ARGS_27(__VA_ARGS__), A
#define EMP_REVERSE_ARGS_29(A, ...) EMP_REVERSE_ARGS_28(__VA_ARGS__), A
#define EMP_REVERSE_ARGS_30(A, ...) EMP_REVERSE_ARGS_29(__VA_ARGS__), A
#define EMP_REVERSE_ARGS_31(A, ...) EMP_REVERSE_ARGS_30(__VA_ARGS__), A
#define EMP_REVERSE_ARGS_32(A, ...) EMP_REVERSE_ARGS_31(__VA_ARGS__), A
#define EMP_REVERSE_ARGS_33(A, ...) EMP_REVERSE_ARGS_32(__VA_ARGS__), A
#define EMP_REVERSE_ARGS_34(A, ...) EMP_REVERSE_ARGS_33(__VA_ARGS__), A
#define EMP_REVERSE_ARGS_35(A, ...) EMP_REVERSE_ARGS_34(__VA_ARGS__), A
#define EMP_REVERSE_ARGS_36(A, ...) EMP_REVERSE_ARGS_35(__VA_ARGS__), A
#define EMP_REVERSE_ARGS_37(A, ...) EMP_REVERSE_ARGS_36(__VA_ARGS__), A
#define EMP_REVERSE_ARGS_38(A, ...) EMP_REVERSE_ARGS_37(__VA_ARGS__), A
#define EMP_REVERSE_ARGS_39(A, ...) EMP_REVERSE_ARGS_38(__VA_ARGS__), A
#define EMP_REVERSE_ARGS_40(A, ...) EMP_REVERSE_ARGS_39(__VA_ARGS__), A
#define EMP_REVERSE_ARGS_41(A, ...) EMP_REVERSE_ARGS_40(__VA_ARGS__), A
#define EMP_REVERSE_ARGS_42(A, ...) EMP_REVERSE_ARGS_41(__VA_ARGS__), A
#define EMP_REVERSE_ARGS_43(A, ...) EMP_REVERSE_ARGS_42(__VA_ARGS__), A
#define EMP_REVERSE_ARGS_44(A, ...) EMP_REVERSE_ARGS_43(__VA_ARGS__), A
#define EMP_REVERSE_ARGS_45(A, ...) EMP_REVERSE_ARGS_44(__VA_ARGS__), A
#define EMP_REVERSE_ARGS_46(A, ...) EMP_REVERSE_ARGS_45(__VA_ARGS__), A
#define EMP_REVERSE_ARGS_47(A, ...) EMP_REVERSE_ARGS_46(__VA_ARGS__), A
#define EMP_REVERSE_ARGS_48(A, ...) EMP_REVERSE_ARGS_47(__VA_ARGS__), A
#define EMP_REVERSE_ARGS_49(A, ...) EMP_REVERSE_ARGS_48(__VA_ARGS__), A
#define EMP_REVERSE_ARGS_50(A, ...) EMP_REVERSE_ARGS_49(__VA_ARGS__), A
#define EMP_REVERSE_ARGS_51(A, ...) EMP_REVERSE_ARGS_50(__VA_ARGS__), A
#define EMP_REVERSE_ARGS_52(A, ...) EMP_REVERSE_ARGS_51(__VA_ARGS__), A
#define EMP_REVERSE_ARGS_53(A, ...) EMP_REVERSE_ARGS_52(__VA_ARGS__), A
#define EMP_REVERSE_ARGS_54(A, ...) EMP_REVERSE_ARGS_53(__VA_ARGS__), A
#define EMP_REVERSE_ARGS_55(A, ...) EMP_REVERSE_ARGS_54(__VA_ARGS__), A
#define EMP_REVERSE_ARGS_56(A, ...) EMP_REVERSE_ARGS_55(__VA_ARGS__), A
#define EMP_REVERSE_ARGS_57(A, ...) EMP_REVERSE_ARGS_56(__VA_ARGS__), A
#define EMP_REVERSE_ARGS_58(A, ...) EMP_REVERSE_ARGS_57(__VA_ARGS__), A
#define EMP_REVERSE_ARGS_59(A, ...) EMP_REVERSE_ARGS_58(__VA_ARGS__), A
#define EMP_REVERSE_ARGS_60(A, ...) EMP_REVERSE_ARGS_59(__VA_ARGS__), A
#define EMP_REVERSE_ARGS_61(A, ...) EMP_REVERSE_ARGS_60(__VA_ARGS__), A
#define EMP_REVERSE_ARGS_62(A, ...) EMP_REVERSE_ARGS_61(__VA_ARGS__), A
#define EMP_REVERSE_ARGS_63(A, ...) EMP_REVERSE_ARGS_62(__VA_ARGS__), A
#define EMP_REVERSE_ARGS_64(A, ...) EMP_REVERSE_ARGS_63(__VA_ARGS__), A

#define EMP_TYPES_TO_ARGS(...) EMP_ASSEMBLE_MACRO(EMP_TYPES_TO_ARGS_, EMP_COUNT_ARGS(__VA_ARGS__), EMP_REVERSE_ARGS(__VA_ARGS__))
#define EMP_TYPES_TO_ARGS_1(A) A arg1
#define EMP_TYPES_TO_ARGS_2(A, ...) EMP_TYPES_TO_ARGS_1(__VA_ARGS__), A arg2
#define EMP_TYPES_TO_ARGS_3(A, ...) EMP_TYPES_TO_ARGS_2(__VA_ARGS__), A arg3
#define EMP_TYPES_TO_ARGS_4(A, ...) EMP_TYPES_TO_ARGS_3(__VA_ARGS__), A arg4
#define EMP_TYPES_TO_ARGS_5(A, ...) EMP_TYPES_TO_ARGS_4(__VA_ARGS__), A arg5
#define EMP_TYPES_TO_ARGS_6(A, ...) EMP_TYPES_TO_ARGS_5(__VA_ARGS__), A arg6
#define EMP_TYPES_TO_ARGS_7(A, ...) EMP_TYPES_TO_ARGS_6(__VA_ARGS__), A arg7
#define EMP_TYPES_TO_ARGS_8(A, ...) EMP_TYPES_TO_ARGS_7(__VA_ARGS__), A arg8
#define EMP_TYPES_TO_ARGS_9(A, ...) EMP_TYPES_TO_ARGS_8(__VA_ARGS__), A arg9
#define EMP_TYPES_TO_ARGS_10(A, ...) EMP_TYPES_TO_ARGS_9(__VA_ARGS__), A arg10
#define EMP_TYPES_TO_ARGS_11(A, ...) EMP_TYPES_TO_ARGS_10(__VA_ARGS__), A arg11
#define EMP_TYPES_TO_ARGS_12(A, ...) EMP_TYPES_TO_ARGS_11(__VA_ARGS__), A arg12
#define EMP_TYPES_TO_ARGS_13(A, ...) EMP_TYPES_TO_ARGS_12(__VA_ARGS__), A arg13
#define EMP_TYPES_TO_ARGS_14(A, ...) EMP_TYPES_TO_ARGS_13(__VA_ARGS__), A arg14
#define EMP_TYPES_TO_ARGS_15(A, ...) EMP_TYPES_TO_ARGS_14(__VA_ARGS__), A arg15
#define EMP_TYPES_TO_ARGS_16(A, ...) EMP_TYPES_TO_ARGS_15(__VA_ARGS__), A arg16
#define EMP_TYPES_TO_ARGS_17(A, ...) EMP_TYPES_TO_ARGS_16(__VA_ARGS__), A arg17
#define EMP_TYPES_TO_ARGS_18(A, ...) EMP_TYPES_TO_ARGS_17(__VA_ARGS__), A arg18
#define EMP_TYPES_TO_ARGS_19(A, ...) EMP_TYPES_TO_ARGS_18(__VA_ARGS__), A arg19
#define EMP_TYPES_TO_ARGS_20(A, ...) EMP_TYPES_TO_ARGS_19(__VA_ARGS__), A arg20
#define EMP_TYPES_TO_ARGS_21(A, ...) EMP_TYPES_TO_ARGS_20(__VA_ARGS__), A arg21
#define EMP_TYPES_TO_ARGS_22(A, ...) EMP_TYPES_TO_ARGS_21(__VA_ARGS__), A arg22
#define EMP_TYPES_TO_ARGS_23(A, ...) EMP_TYPES_TO_ARGS_22(__VA_ARGS__), A arg23
#define EMP_TYPES_TO_ARGS_24(A, ...) EMP_TYPES_TO_ARGS_23(__VA_ARGS__), A arg24
#define EMP_TYPES_TO_ARGS_25(A, ...) EMP_TYPES_TO_ARGS_24(__VA_ARGS__), A arg25
#define EMP_TYPES_TO_ARGS_26(A, ...) EMP_TYPES_TO_ARGS_25(__VA_ARGS__), A arg26
#define EMP_TYPES_TO_ARGS_27(A, ...) EMP_TYPES_TO_ARGS_26(__VA_ARGS__), A arg27
#define EMP_TYPES_TO_ARGS_28(A, ...) EMP_TYPES_TO_ARGS_27(__VA_ARGS__), A arg28
#define EMP_TYPES_TO_ARGS_29(A, ...) EMP_TYPES_TO_ARGS_28(__VA_ARGS__), A arg29
#define EMP_TYPES_TO_ARGS_30(A, ...) EMP_TYPES_TO_ARGS_29(__VA_ARGS__), A arg30
#define EMP_TYPES_TO_ARGS_31(A, ...) EMP_TYPES_TO_ARGS_30(__VA_ARGS__), A arg31
#define EMP_TYPES_TO_ARGS_32(A, ...) EMP_TYPES_TO_ARGS_31(__VA_ARGS__), A arg32
#define EMP_TYPES_TO_ARGS_33(A, ...) EMP_TYPES_TO_ARGS_32(__VA_ARGS__), A arg33
#define EMP_TYPES_TO_ARGS_34(A, ...) EMP_TYPES_TO_ARGS_33(__VA_ARGS__), A arg34
#define EMP_TYPES_TO_ARGS_35(A, ...) EMP_TYPES_TO_ARGS_34(__VA_ARGS__), A arg35
#define EMP_TYPES_TO_ARGS_36(A, ...) EMP_TYPES_TO_ARGS_35(__VA_ARGS__), A arg36
#define EMP_TYPES_TO_ARGS_37(A, ...) EMP_TYPES_TO_ARGS_36(__VA_ARGS__), A arg37
#define EMP_TYPES_TO_ARGS_38(A, ...) EMP_TYPES_TO_ARGS_37(__VA_ARGS__), A arg38
#define EMP_TYPES_TO_ARGS_39(A, ...) EMP_TYPES_TO_ARGS_38(__VA_ARGS__), A arg39
#define EMP_TYPES_TO_ARGS_40(A, ...) EMP_TYPES_TO_ARGS_39(__VA_ARGS__), A arg40
#define EMP_TYPES_TO_ARGS_41(A, ...) EMP_TYPES_TO_ARGS_40(__VA_ARGS__), A arg41
#define EMP_TYPES_TO_ARGS_42(A, ...) EMP_TYPES_TO_ARGS_41(__VA_ARGS__), A arg42
#define EMP_TYPES_TO_ARGS_43(A, ...) EMP_TYPES_TO_ARGS_42(__VA_ARGS__), A arg43
#define EMP_TYPES_TO_ARGS_44(A, ...) EMP_TYPES_TO_ARGS_43(__VA_ARGS__), A arg44
#define EMP_TYPES_TO_ARGS_45(A, ...) EMP_TYPES_TO_ARGS_44(__VA_ARGS__), A arg45
#define EMP_TYPES_TO_ARGS_46(A, ...) EMP_TYPES_TO_ARGS_45(__VA_ARGS__), A arg46
#define EMP_TYPES_TO_ARGS_47(A, ...) EMP_TYPES_TO_ARGS_46(__VA_ARGS__), A arg47
#define EMP_TYPES_TO_ARGS_48(A, ...) EMP_TYPES_TO_ARGS_47(__VA_ARGS__), A arg48
#define EMP_TYPES_TO_ARGS_49(A, ...) EMP_TYPES_TO_ARGS_48(__VA_ARGS__), A arg49
#define EMP_TYPES_TO_ARGS_50(A, ...) EMP_TYPES_TO_ARGS_49(__VA_ARGS__), A arg50
#define EMP_TYPES_TO_ARGS_51(A, ...) EMP_TYPES_TO_ARGS_50(__VA_ARGS__), A arg51
#define EMP_TYPES_TO_ARGS_52(A, ...) EMP_TYPES_TO_ARGS_51(__VA_ARGS__), A arg52
#define EMP_TYPES_TO_ARGS_53(A, ...) EMP_TYPES_TO_ARGS_52(__VA_ARGS__), A arg53
#define EMP_TYPES_TO_ARGS_54(A, ...) EMP_TYPES_TO_ARGS_53(__VA_ARGS__), A arg54
#define EMP_TYPES_TO_ARGS_55(A, ...) EMP_TYPES_TO_ARGS_54(__VA_ARGS__), A arg55
#define EMP_TYPES_TO_ARGS_56(A, ...) EMP_TYPES_TO_ARGS_55(__VA_ARGS__), A arg56
#define EMP_TYPES_TO_ARGS_57(A, ...) EMP_TYPES_TO_ARGS_56(__VA_ARGS__), A arg57
#define EMP_TYPES_TO_ARGS_58(A, ...) EMP_TYPES_TO_ARGS_57(__VA_ARGS__), A arg58
#define EMP_TYPES_TO_ARGS_59(A, ...) EMP_TYPES_TO_ARGS_58(__VA_ARGS__), A arg59
#define EMP_TYPES_TO_ARGS_60(A, ...) EMP_TYPES_TO_ARGS_59(__VA_ARGS__), A arg60
#define EMP_TYPES_TO_ARGS_61(A, ...) EMP_TYPES_TO_ARGS_60(__VA_ARGS__), A arg61
#define EMP_TYPES_TO_ARGS_62(A, ...) EMP_TYPES_TO_ARGS_61(__VA_ARGS__), A arg62
#define EMP_TYPES_TO_ARGS_63(A, ...) EMP_TYPES_TO_ARGS_62(__VA_ARGS__), A arg63


// Some basic math macros.  Since brute force is the only way to do math with macros...
#define EMP_INC(X) EMP_INC_ ## X
#define EMP_INC_0  1
#define EMP_INC_1  2
#define EMP_INC_2  3
#define EMP_INC_3  4
#define EMP_INC_4  5
#define EMP_INC_5  6
#define EMP_INC_6  7
#define EMP_INC_7  8
#define EMP_INC_8  9
#define EMP_INC_9  10
#define EMP_INC_10  11
#define EMP_INC_11  12
#define EMP_INC_12  13
#define EMP_INC_13  14
#define EMP_INC_14  15
#define EMP_INC_15  16
#define EMP_INC_16  17
#define EMP_INC_17  18
#define EMP_INC_18  19
#define EMP_INC_19  20
#define EMP_INC_20  21
#define EMP_INC_21  22
#define EMP_INC_22  23
#define EMP_INC_23  24
#define EMP_INC_24  25
#define EMP_INC_25  26
#define EMP_INC_26  27
#define EMP_INC_27  28
#define EMP_INC_28  29
#define EMP_INC_29  30
#define EMP_INC_30  31
#define EMP_INC_31  32
#define EMP_INC_32  33
#define EMP_INC_33  34
#define EMP_INC_34  35
#define EMP_INC_35  36
#define EMP_INC_36  37
#define EMP_INC_37  38
#define EMP_INC_38  39
#define EMP_INC_39  40
#define EMP_INC_40  41
#define EMP_INC_41  42
#define EMP_INC_42  43
#define EMP_INC_43  44
#define EMP_INC_44  45
#define EMP_INC_45  46
#define EMP_INC_46  47
#define EMP_INC_47  48
#define EMP_INC_48  49
#define EMP_INC_49  50
#define EMP_INC_50  51
#define EMP_INC_51  52
#define EMP_INC_52  53
#define EMP_INC_53  54
#define EMP_INC_54  55
#define EMP_INC_55  56
#define EMP_INC_56  57
#define EMP_INC_57  58
#define EMP_INC_58  59
#define EMP_INC_59  60
#define EMP_INC_60  61
#define EMP_INC_61  62
#define EMP_INC_62  63
#define EMP_INC_63  64

#define EMP_HALF(X) EMP_HALF_ ## X
#define EMP_HALF_0  0
#define EMP_HALF_1  0
#define EMP_HALF_2  1
#define EMP_HALF_3  1
#define EMP_HALF_4  2
#define EMP_HALF_5  2
#define EMP_HALF_6  3
#define EMP_HALF_7  3
#define EMP_HALF_8  4
#define EMP_HALF_9  4
#define EMP_HALF_10  5
#define EMP_HALF_11  5
#define EMP_HALF_12  6
#define EMP_HALF_13  6
#define EMP_HALF_14  7
#define EMP_HALF_15  7
#define EMP_HALF_16  8
#define EMP_HALF_17  8
#define EMP_HALF_18  9
#define EMP_HALF_19  9
#define EMP_HALF_20  10
#define EMP_HALF_21  10
#define EMP_HALF_22  11
#define EMP_HALF_23  11
#define EMP_HALF_24  12
#define EMP_HALF_25  12
#define EMP_HALF_26  13
#define EMP_HALF_27  13
#define EMP_HALF_28  14
#define EMP_HALF_29  14
#define EMP_HALF_30  15
#define EMP_HALF_31  15
#define EMP_HALF_32  16
#define EMP_HALF_33  16
#define EMP_HALF_34  17
#define EMP_HALF_35  17
#define EMP_HALF_36  18
#define EMP_HALF_37  18
#define EMP_HALF_38  19
#define EMP_HALF_39  19
#define EMP_HALF_40  20
#define EMP_HALF_41  20
#define EMP_HALF_42  21
#define EMP_HALF_43  21
#define EMP_HALF_44  22
#define EMP_HALF_45  22
#define EMP_HALF_46  23
#define EMP_HALF_47  23
#define EMP_HALF_48  24
#define EMP_HALF_49  24
#define EMP_HALF_50  25
#define EMP_HALF_51  25
#define EMP_HALF_52  26
#define EMP_HALF_53  26
#define EMP_HALF_54  27
#define EMP_HALF_55  27
#define EMP_HALF_56  28
#define EMP_HALF_57  28
#define EMP_HALF_58  29
#define EMP_HALF_59  29
#define EMP_HALF_60  30
#define EMP_HALF_61  30
#define EMP_HALF_62  31
#define EMP_HALF_63  31

//Individually stringifies each variable passed to it and returns them
//with commas in between.
#define EMP_STRINGIFY_EACH(...) EMP_WRAP_ARGS(EMP_STRINGIFY, __VA_ARGS__)


// Setup a generic method of calling a specific version of a macro based on argument count.
#define EMP_ASSEMBLE_IMPL(BASE, ARG_COUNT) BASE ## ARG_COUNT
#define EMP_ASSEMBLE_MACRO(BASE, ARG_COUNT, ...) EMP_ASSEMBLE_IMPL(BASE, ARG_COUNT) (__VA_ARGS__)

#endif
