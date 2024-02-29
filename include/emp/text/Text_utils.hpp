/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2024
 *
 *  @file Text_utils.hpp
 *  @brief Helper functions for building Text objects with different encodings.
 *  @note Status: ALPHA
 *
 */

#ifndef EMP_TEXT_TEXT_UTILS_HPP_INCLUDE
#define EMP_TEXT_TEXT_UTILS_HPP_INCLUDE

#include <map>
#include <type_traits>
#include <unordered_map>

#include "../base/assert.hpp"
#include "../base/notify.hpp"
#include "../bits/BitVector.hpp"
#include "../datastructs/map_utils.hpp"
#include "../tools/String.hpp"

#include "_TextCharRef.hpp"

namespace emp {

  // Information about 
  struct TextSymbolInfo {
    String html_tag;    ///< Tag to use to represent this symbol in HTML
    char placeholder;   ///< A single character that can be used as a placeholder
    String name;        ///< General symbol name (usually based on HTML name)
    size_t id;          ///< Unicode ID number of symbol
    String unicode;     ///< Unicode sequence for symbol (usually two characters)
    String desc;        ///< English description of what symbol looks like
  };

  /// A mapping of allowed symbol names to placeholder characters.
  static const auto & GetTextSymbolNames() {
    static std::set<TextSymbolInfo> symbol_map;
    if (symbol_map.size() == 0) {

      // === UNICODE Latin-1 Supplement ===

      // Punctuation & Symbols
      symbol_map.emplace("&nbsp;",    ' ',  "nbsp",      160,  " ",   "Non-breaking space");
      symbol_map.emplace("&iexcl;",   '!',  "iexcl",     161,  "¡",   "Inverted exclamation point");
      symbol_map.emplace("&cent;",    'c',  "cent",      162,  "¢",   "Cent");
      symbol_map.emplace("&pound;",   'L',  "pound",     163,  "£",   "Pound");
      symbol_map.emplace("&curren;",  '$',  "curren",    164,  "¤",   "Currency");
      symbol_map.emplace("&yen;",     'Y',  "yen",       165,  "¥",   "Yen");
      symbol_map.emplace("&brvbar;",  '|',  "brvbar",    166,  "¦",   "Broken vertical bar");
      symbol_map.emplace("&sect;",    'S',  "sect",      167,  "§",   "Section");
      symbol_map.emplace("&uml;",     '~',  "uml",       168,  "¨",   "Spacing diaeresis");
      symbol_map.emplace("&copy;",    'c',  "copy",      169,  "©",   "Copyright");
      symbol_map.emplace("&ordf;",    'a',  "ordf",      170,  "ª",   "Feminine ordinal indicator");
      symbol_map.emplace("&laquo;",   '<',  "laquo",     171,  "«",   "Opening/Left angle quotation mark");
      symbol_map.emplace("&not;",     '~',  "not",       172,  "¬",   "Negation");
      symbol_map.emplace("&shy ",     '-',  "shy",       173,  "­-",   "Soft hyphen");
      symbol_map.emplace("&reg;",     'R',  "reg",       174,  "®",   "Registered trademark");
      symbol_map.emplace("&macr;",    '-',  "macr",      175,  "¯",   "Spacing macron");
      symbol_map.emplace("&deg;",     'o',  "deg",       176,  "°",   "Degree");
      symbol_map.emplace("&plusmn;",  '+',  "plusmn",    177,  "±",   "Plus or minus");
      symbol_map.emplace("&sup2;",    '2',  "sup2",      178,  "²",   "Superscript 2");
      symbol_map.emplace("&sup3;",    '3',  "sup3",      179,  "³",   "Superscript 3");
      symbol_map.emplace("&acute;",   '\'', "acute",     180,  "´",   "Spacing acute");
      symbol_map.emplace("&micro;",   'u', "micro",      181,  "µ",   "Micro");
      symbol_map.emplace("&para;",    'P', "para",       182,  "¶",   "Paragraph");
      symbol_map.emplace("&dot;",     '.', "dot",        182,  "·",   "Dot");
      symbol_map.emplace("&cedil;",   ',', "cedil",      184,  "¸",   "Spacing cedilla");
      symbol_map.emplace("&sup1;",    '1', "sup1",       185,  "¹",   "Superscript 1");
      symbol_map.emplace("&ordm;",    'o', "ordm",       186,  "º",   "Masculine ordinal indicator");
      symbol_map.emplace("&raquo;",   '>', "raquo",      187,  "»",   "Closing/Right angle quotation mark");
      symbol_map.emplace("&frac14;",  '/', "frac14",     188,  "¼",   "Fraction 1/4");
      symbol_map.emplace("&frac12;",  '/', "frac12",     189,  "½",   "Fraction 1/2");
      symbol_map.emplace("&frac34;",  '/', "frac34",     190,  "¾",   "Fraction 3/4");
      symbol_map.emplace("&iquest;",  '?', "iquest",     191,  "¿",   "Inverted question mark");

      // Uppercase letters
      symbol_map.emplace("&Agrave;",  'A', "Agrave",     192,  "À",   "Capital a with grave accent");
      symbol_map.emplace("&Aacute;",  'A', "Aacute",     193,  "Á",   "Capital a with acute accent");
      symbol_map.emplace("&Acirc;",   'A', "Acirc",      194,  "Â",   "Capital a with circumflex accent");
      symbol_map.emplace("&Atilde;",  'A', "Atilde",     195,  "Ã",   "Capital a with tilde");
      symbol_map.emplace("&Auml;",    'A', "Auml",       196,  "Ä",   "Capital a with umlaut");
      symbol_map.emplace("&Aring;",   'A', "Aring",      197,  "Å",   "Capital a with ring");
      symbol_map.emplace("&AElig;",   'A', "AElig",      198,  "Æ",   "Capital ae");
      symbol_map.emplace("&Ccedil;",  'C', "Ccedil",     199,  "Ç",   "Capital c with cedilla");
      symbol_map.emplace("&Egrave;",  'E', "Egrave",     200,  "È",   "Capital e with grave accent");
      symbol_map.emplace("&Eacute;",  'E', "Eacute",     201,  "É",   "Capital e with acute accent");
      symbol_map.emplace("&Ecirc;",   'E', "Ecirc",      202,  "Ê",   "Capital e with circumflex accent");
      symbol_map.emplace("&Euml;",    'E', "Euml",       203,  "Ë",   "Capital e with umlaut");
      symbol_map.emplace("&Igrave;",  'I', "Igrave",     204,  "Ì",   "Capital i with grave accent");
      symbol_map.emplace("&Iacute;",  'I', "Iacute",     205,  "Í",   "Capital i with accute accent");
      symbol_map.emplace("&Icirc;",   'I', "Icirc",      206,  "Î",   "Capital i with circumflex accent");
      symbol_map.emplace("&Iuml;",    'I', "Iuml",       207,  "Ï",   "Capital i with umlaut");
      symbol_map.emplace("&ETH;",     'D', "ETH",        208,  "Ð",   "Capital eth (Icelandic)");
      symbol_map.emplace("&Ntilde;",  'N', "Ntilde",     209,  "Ñ",   "Capital n with tilde");
      symbol_map.emplace("&Ograve;",  'O', "Ograve",     210,  "Ò",   "Capital o with grave accent");
      symbol_map.emplace("&Oacute;",  'O', "Oacute",     211,  "Ó",   "Capital o with accute accent");
      symbol_map.emplace("&Ocirc;",   'O', "Ocirc",      212,  "Ô",   "Capital o with circumflex accent");
      symbol_map.emplace("&Otilde;",  'O', "Otilde",     213,  "Õ",   "Capital o with tilde");
      symbol_map.emplace("&Ouml;",    'O', "Ouml",       214,  "Ö",   "Capital o with umlaut");
      symbol_map.emplace("&times;",   'x', "times",      215,  "×",   "Multiplication");
      symbol_map.emplace("&Oslash;",  'O', "Oslash",     216,  "Ø",   "Capital o with slash");
      symbol_map.emplace("&Ugrave;",  'U', "Ugrave",     217,  "Ù",   "Capital u with grave accent");
      symbol_map.emplace("&Uacute;",  'U', "Uacute",     218,  "Ú",   "Capital u with acute accent");
      symbol_map.emplace("&Ucirc;",   'U', "Ucirc",      219,  "Û",   "Capital u with circumflex accent");
      symbol_map.emplace("&Uuml;",    'U', "Uuml",       220,  "Ü",   "Capital u with umlaut");
      symbol_map.emplace("&Yacute;",  'Y', "Yacute",     221,  "Ý",   "Capital y with acute accent");
      symbol_map.emplace("&THORN;",   'P', "THORN",      222,  "Þ",   "Capital thorn (Icelandic)");

      // Lowercase letters
      symbol_map.emplace("&szlig;",   'B', "szlig",      223,  "ß",   "Lowercase sharp s (German)");
      symbol_map.emplace("&agrave;",  'a', "agrave",     224,  "à",   "Lowercase a with grave accent");
      symbol_map.emplace("&aacute;",  'a', "aacute",     225,  "á",   "Lowercase a with acute accent");
      symbol_map.emplace("&acirc;",   'a', "acirc",      226,  "â",   "Lowercase a with circumflex accent");
      symbol_map.emplace("&atilde;",  'a', "atilde",     227,  "ã",   "Lowercase a with tilde");
      symbol_map.emplace("&auml;",    'a', "auml",       228,  "ä",   "Lowercase a with umlaut");
      symbol_map.emplace("&aring;",   'a', "aring",      229,  "å",   "Lowercase a with ring");
      symbol_map.emplace("&aelig;",   'a', "aelig",      230,  "æ",   "Lowercase ae");
      symbol_map.emplace("&ccedil;",  'c', "ccedil",     231,  "ç",   "Lowercase c with cedilla");
      symbol_map.emplace("&egrave;",  'e', "egrave",     232,  "è",   "Lowercase e with grave accent");
      symbol_map.emplace("&eacute;",  'e', "eacute",     233,  "é",   "Lowercase e with acute accent");
      symbol_map.emplace("&ecirc;",   'e', "ecirc",      234,  "ê",   "Lowercase e with circumflex accent");
      symbol_map.emplace("&euml;",    'e', "euml",       235,  "ë",   "Lowercase e with umlaut");
      symbol_map.emplace("&igrave;",  'i', "igrave",     236,  "ì",   "Lowercase i with grave accent");
      symbol_map.emplace("&iacute;",  'i', "iacute",     237,  "í",   "Lowercase i with acute accent");
      symbol_map.emplace("&icirc;",   'i', "icirc",      238,  "î",   "Lowercase i with circumflex accent");
      symbol_map.emplace("&iuml;",    'i', "iuml",       239,  "ï",   "Lowercase i with umlaut");
      symbol_map.emplace("&eth;",     'o', "eth",        240,  "ð",   "Lowercase eth (Icelandic)");
      symbol_map.emplace("&ntilde;",  'n', "ntilde",     241,  "ñ",   "Lowercase n with tilde");
      symbol_map.emplace("&ograve;",  'o', "ograve",     242,  "ò",   "Lowercase o with grave accent");
      symbol_map.emplace("&oacute;",  'o', "oacute",     243,  "ó",   "Lowercase o with acute accent");
      symbol_map.emplace("&ocirc;",   'o', "ocirc",      244,  "ô",   "Lowercase o with circumflex accent");
      symbol_map.emplace("&otilde;",  'o', "otilde",     245,  "õ",   "Lowercase o with tilde");
      symbol_map.emplace("&ouml;",    'o', "ouml",       246,  "ö",   "Lowercase o with umlaut");
      symbol_map.emplace("&divide;",  '/', "divide",     247,  "÷",   "Divide");
      symbol_map.emplace("&oslash;",  'o', "oslash",     248,  "ø",   "Lowercase o with slash");
      symbol_map.emplace("&ugrave;",  'u', "ugrave",     249,  "ù",   "Lowercase u with grave accent");
      symbol_map.emplace("&uacute;",  'u', "uacute",     250,  "ú",   "Lowercase u with acute accent");
      symbol_map.emplace("&ucirc;",   'u', "ucirc",      251,  "û",   "Lowercase u with circumflex accent");
      symbol_map.emplace("&uuml;",    'u', "uuml",       252,  "ü",   "Lowercase u with umlaut");
      symbol_map.emplace("&yacute;",  'y', "yacute",     253,  "ý",   "Lowercase y with acute accent");
      symbol_map.emplace("&thorn;",   'p', "thorn",      254,  "þ",   "Lowercase thorn (Icelandic)");
      symbol_map.emplace("&yuml;",    'y', "yuml",       255,  "ÿ",   "Lowercase y with umlaut");

      // === UNICODE Latin Extended-A ===

      // European latin
      symbol_map.emplace("&Amacr;",   'A', "Amacr",      256,  "Ā",   "Latin capital letter a with macron");
      symbol_map.emplace("&amacr;",   'a', "amacr",      257,  "ā",   "Latin small letter a with macron");
      symbol_map.emplace("&Abreve;",  'A', "Abreve",     258,  "Ă",   "Latin capital letter a with breve");
      symbol_map.emplace("&abreve;",  'a', "abreve",     259,  "ă",   "Latin small letter a with breve");
      symbol_map.emplace("&Aogon;",   'A', "Aogon",      260,  "Ą",   "Latin capital letter a with ogonek");
      symbol_map.emplace("&aogon;",   'a', "aogon",      261,  "ą",   "Latin small letter a with ogonek");
      symbol_map.emplace("&Cacute;",  'C', "Cacute",     262,  "Ć",   "Latin capital letter c with acute");
      symbol_map.emplace("&cacute;",  'c', "cacute",     263,  "ć",   "Latin small letter c with acute");
      symbol_map.emplace("&Ccirc;",   'C', "Ccirc",      264,  "Ĉ",   "Latin capital letter c with circumflex");
      symbol_map.emplace("&ccirc;",   'c', "ccirc",      265,  "ĉ",   "Latin small letter c with circumflex");
      symbol_map.emplace("&Cdot;",    'C', "Cdot",       266,  "Ċ",   "Latin capital letter c with dot above");
      symbol_map.emplace("&cdot;",    'c', "cdot",       267,  "ċ",   "Latin small letter c with dot above");
      symbol_map.emplace("&Ccaron;",  'C', "Ccaron",     268,  "Č",   "Latin capital letter c with caron");
      symbol_map.emplace("&ccaron;",  'c', "ccaron",     269,  "č",   "Latin small letter c with caron");
      symbol_map.emplace("&Dcaron;",  'D', "Dcaron",     270,  "Ď",   "Latin capital letter d with caron");
      symbol_map.emplace("&dcaron;",  'd', "dcaron",     271,  "ď",   "Latin small letter d with caron");
      symbol_map.emplace("&Dstrok;",  'D', "Dstrok",     272,  "Đ",   "Latin capital letter d with stroke");
      symbol_map.emplace("&dstrok;",  'd', "dstrok",     273,  "đ",   "Latin small letter d with stroke");
      symbol_map.emplace("&Emacr;",   'E', "Emacr",      274,  "Ē",   "Latin capital letter e with macron");
      symbol_map.emplace("&emacr;",   'e', "emacr",      275,  "ē",   "Latin small letter e with macron");
      symbol_map.emplace("&Ebreve;",  'E', "Ebreve",     276,  "Ĕ",   "Latin capital letter e with breve");
      symbol_map.emplace("&ebreve;",  'e', "ebreve",     277,  "ĕ",   "Latin small letter e with breve");
      symbol_map.emplace("&Edot;",    'E', "Edot",       278,  "Ė",   "Latin capital letter e with dot above");
      symbol_map.emplace("&edot;",    'e', "edot",       279,  "ė",   "Latin small letter e with dot above");
      symbol_map.emplace("&Eogon;",   'E', "Eogon",      280,  "Ę",   "Latin capital letter e with ogonek");
      symbol_map.emplace("&eogon;",   'e', "eogon",      281,  "ę",   "Latin small letter e with ogonek");
      symbol_map.emplace("&Ecaron;",  'E', "Ecaron",     282,  "Ě",   "Latin capital letter e with caron");
      symbol_map.emplace("&ecaron;",  'e', "ecaron",     283,  "ě",   "Latin small letter e with caron");
      symbol_map.emplace("&Gcirc;",   'G', "Gcirc",      284,  "Ĝ",   "Latin capital letter g with circumflex");
      symbol_map.emplace("&gcirc;",   'g', "gcirc",      285,  "ĝ",   "Latin small letter g with circumflex");
      symbol_map.emplace("&Gbreve;",  'G', "Gbreve",     286,  "Ğ",   "Latin capital letter g with breve");
      symbol_map.emplace("&gbreve;",  'g', "gbreve",     287,  "ğ",   "Latin small letter g with breve");
      symbol_map.emplace("&Gdot;",    'G', "Gdot",       288,  "Ġ",   "Latin capital letter g with dot above");
      symbol_map.emplace("&gdot;",    'g', "gdot",       289,  "ġ",   "Latin small letter g with dot above");
      symbol_map.emplace("&Gcedil;",  'G', "Gcedil",     290,  "Ģ",   "Latin capital letter g with cedilla");
      symbol_map.emplace("&gcedil;",  'g', "gcedil",     291,  "ģ",   "Latin small letter g with cedilla");
      symbol_map.emplace("&Hcirc;",   'H', "Hcirc",      292,  "Ĥ",   "Latin capital letter h with circumflex");
      symbol_map.emplace("&hcirc;",   'h', "hcirc",      293,  "ĥ",   "Latin small letter h with circumflex");
      symbol_map.emplace("&Hstrok;",  'H', "Hstrok",     294,  "Ħ",   "Latin capital letter h with stroke");
      symbol_map.emplace("&hstrok;",  'h', "hstrok",     295,  "ħ",   "Latin small letter h with stroke");
      symbol_map.emplace("&Itilde;",  'I', "Itilde",     296,  "Ĩ",   "Latin capital letter I with tilde");
      symbol_map.emplace("&itilde;",  'i', "itilde",     297,  "ĩ",   "Latin small letter I with tilde");
      symbol_map.emplace("&Imacr;",   'I', "Imacr",      298,  "Ī",   "Latin capital letter I with macron");
      symbol_map.emplace("&imacr;",   'i', "imacr",      299,  "ī",   "Latin small letter I with macron");
      symbol_map.emplace("&Ibreve;",  'I', "Ibreve",     300,  "Ĭ",   "Latin capital letter I with breve");
      symbol_map.emplace("&ibreve;",  'i', "ibreve",     301,  "ĭ",   "Latin small letter I with breve");
      symbol_map.emplace("&Iogon;",   'I', "Iogon",      302,  "Į",   "Latin capital letter I with ogonek");
      symbol_map.emplace("&iogon;",   'i', "iogon",      303,  "į",   "Latin small letter I with ogonek");
      symbol_map.emplace("&Idot;",    'I', "Idot",       304,  "İ",   "Latin capital letter I with dot above");
      symbol_map.emplace("&imath;",   'i', "imath",      305,  "ı",   "Latin small letter dotless I");
      symbol_map.emplace("&IJlig;",   'I', "IJlig",      306,  "Ĳ",   "Latin capital ligature ij");
      symbol_map.emplace("&ijlig;",   'i', "ijlig",      307,  "ĳ",   "Latin small ligature ij");
      symbol_map.emplace("&Jcirc;",   'J', "Jcirc",      308,  "Ĵ",   "Latin capital letter j with circumflex");
      symbol_map.emplace("&jcirc;",   'j', "jcirc",      309,  "ĵ",   "Latin small letter j with circumflex");
      symbol_map.emplace("&Kcedil;",  'K', "Kcedil",     310,  "Ķ",   "Latin capital letter k with cedilla");
      symbol_map.emplace("&kcedil;",  'k', "kcedil",     311,  "ķ",   "Latin small letter k with cedilla");
      symbol_map.emplace("&kgreen;",  'k', "kgreen",     312,  "ĸ",   "Latin small letter kra");
      symbol_map.emplace("&Lacute;",  'L', "Lacute",     313,  "Ĺ",   "Latin capital letter l with acute");
      symbol_map.emplace("&lacute;",  'l', "lacute",     314,  "ĺ",   "Latin small letter l with acute");
      symbol_map.emplace("&Lcedil;",  'L', "Lcedil",     315,  "Ļ",   "Latin capital letter l with cedilla");
      symbol_map.emplace("&lcedil;",  'l', "lcedil",     316,  "ļ",   "Latin small letter l with cedilla");
      symbol_map.emplace("&Lcaron;",  'L', "Lcaron",     317,  "Ľ",   "Latin capital letter l with caron");
      symbol_map.emplace("&lcaron;",  'l', "lcaron",     318,  "ľ",   "Latin small letter l with caron");
      symbol_map.emplace("&Lmidot;",  'L', "Lmidot",     319,  "Ŀ",   "Latin capital letter l with middle dot");
      symbol_map.emplace("&lmidot;",  'l', "lmidot",     320,  "ŀ",   "Latin small letter l with middle dot");
      symbol_map.emplace("&Lstrok;",  'L', "Lstrok",     321,  "Ł",   "Latin capital letter l with stroke");
      symbol_map.emplace("&lstrok;",  'l', "lstrok",     322,  "ł",   "Latin small letter l with stroke");
      symbol_map.emplace("&Nacute;",  'N', "Nacute",     323,  "Ń",   "Latin capital letter n with acute");
      symbol_map.emplace("&nacute;",  'n', "nacute",     324,  "ń",   "Latin small letter n with acute");
      symbol_map.emplace("&Ncedil;",  'N', "Ncedil",     325,  "Ņ",   "Latin capital letter n with cedilla");
      symbol_map.emplace("&ncedil;",  'n', "ncedil",     326,  "ņ",   "Latin small letter n with cedilla");
      symbol_map.emplace("&Ncaron;",  'N', "Ncaron",     327,  "Ň",   "Latin capital letter n with caron");
      symbol_map.emplace("&ncaron;",  'n', "ncaron",     328,  "ň",   "Latin small letter n with caron");
      symbol_map.emplace("&napos;",   'n', "napos",      329,  "ŉ",   "Latin small letter n preceded by apostrophe"); // DEPRECATED!
      symbol_map.emplace("&ENG;",     'N', "ENG",        330,  "Ŋ",   "Latin capital letter eng");
      symbol_map.emplace("&eng;",     'n', "eng",        331,  "ŋ",   "Latin small letter eng");
      symbol_map.emplace("&Omacr;",   'O', "Omacr",      332,  "Ō",   "Latin capital letter o with macron");
      symbol_map.emplace("&omacr;",   'o', "omacr",      333,  "ō",   "Latin small letter o with macron");
      symbol_map.emplace("&Obreve;",  'O', "Obreve",     334,  "Ŏ",   "Latin capital letter o with breve");
      symbol_map.emplace("&obreve;",  'o', "obreve",     335,  "ŏ",   "Latin small letter o with breve");
      symbol_map.emplace("&Odblac;",  'O', "Odblac",     336,  "Ő",   "Latin capital letter o with double acute");
      symbol_map.emplace("&odblac;",  'o', "odblac",     337,  "ő",   "Latin small letter o with double acute");
      symbol_map.emplace("&OElig;",   'O', "OElig",      338,  "Œ",   "Uppercase ligature OE");
      symbol_map.emplace("&oelig;",   'o', "oelig",      339,  "œ",   "Lowercase ligature OE");
      symbol_map.emplace("&Racute;",  'R', "Racute",     340,  "Ŕ",   "Latin capital letter r with acute");
      symbol_map.emplace("&racute;",  'r', "racute",     341,  "ŕ",   "Latin small letter r with acute");
      symbol_map.emplace("&Rcedil;",  'R', "Rcedil",     342,  "Ŗ",   "Latin capital letter r with cedilla");
      symbol_map.emplace("&rcedil;",  'r', "rcedil",     343,  "ŗ",   "Latin small letter r with cedilla");
      symbol_map.emplace("&Rcaron;",  'R', "Rcaron",     344,  "Ř",   "Latin capital letter r with caron");
      symbol_map.emplace("&rcaron;",  'r', "rcaron",     345,  "ř",   "Latin small letter r with caron");
      symbol_map.emplace("&Sacute;",  'S', "Sacute",     346,  "Ś",   "Latin capital letter s with acute");
      symbol_map.emplace("&sacute;",  's', "sacute",     347,  "ś",   "Latin small letter s with acute");
      symbol_map.emplace("&Scirc;",   'S', "Scirc",      348,  "Ŝ",   "Latin capital letter s with circumflex");
      symbol_map.emplace("&scirc;",   's', "scirc",      349,  "ŝ",   "Latin small letter s with circumflex");
      symbol_map.emplace("&Scedil;",  'S', "Scedil",     350,  "Ş",   "Latin capital letter s with cedilla");
      symbol_map.emplace("&scedil;",  's', "scedil",     351,  "ş",   "Latin small letter s with cedilla");
      symbol_map.emplace("&Scaron;",  'S', "Scaron",     352,  "Š",   "Uppercase S with caron");
      symbol_map.emplace("&scaron;",  's', "scaron",     353,  "š",   "Lowercase S with caron");
      symbol_map.emplace("&Tcedil;",  'T', "Tcedil",     354,  "Ţ",   "Latin capital letter t with cedilla");
      symbol_map.emplace("&tcedil;",  't', "tcedil",     355,  "ţ",   "Latin small letter t with cedilla");
      symbol_map.emplace("&Tcaron;",  'T', "Tcaron",     356,  "Ť",   "Latin capital letter t with caron");
      symbol_map.emplace("&tcaron;",  't', "tcaron",     357,  "ť",   "Latin small letter t with caron");
      symbol_map.emplace("&Tstrok;",  'T', "Tstrok",     358,  "Ŧ",   "Latin capital letter t with stroke");
      symbol_map.emplace("&tstrok;",  't', "tstrok",     359,  "ŧ",   "Latin small letter t with stroke");
      symbol_map.emplace("&Utilde;",  'U', "Utilde",     360,  "Ũ",   "Latin capital letter u with tilde");
      symbol_map.emplace("&utilde;",  'u', "utilde",     361,  "ũ",   "Latin small letter u with tilde");
      symbol_map.emplace("&Umacr;",   'U', "Umacr",      362,  "Ū",   "Latin capital letter u with macron");
      symbol_map.emplace("&umacr;",   'u', "umacr",      363,  "ū",   "Latin small letter u with macron");
      symbol_map.emplace("&Ubreve;",  'U', "Ubreve",     364,  "Ŭ",   "Latin capital letter u with breve");
      symbol_map.emplace("&ubreve;",  'u', "ubreve",     365,  "ŭ",   "Latin small letter u with breve");
      symbol_map.emplace("&Uring;",   'U', "Uring",      366,  "Ů",   "Latin capital letter u with ring above");
      symbol_map.emplace("&uring;",   'u', "uring",      367,  "ů",   "Latin small letter u with ring above");
      symbol_map.emplace("&Udblac;",  'U', "Udblac",     368,  "Ű",   "Latin capital letter u with double acute");
      symbol_map.emplace("&udblac;",  'u', "udblac",     369,  "ű",   "Latin small letter u with double acute");
      symbol_map.emplace("&Uogon;",   'U', "Uogon",      370,  "Ų",   "Latin capital letter u with ogonek");
      symbol_map.emplace("&uogon;",   'u', "uogon",      371,  "ų",   "Latin small letter u with ogonek");
      symbol_map.emplace("&Wcirc;",   'W', "Wcirc",      372,  "Ŵ",   "Latin capital letter w with circumflex");
      symbol_map.emplace("&wcirc;",   'w', "wcirc",      373,  "ŵ",   "Latin small letter w with circumflex");
      symbol_map.emplace("&Ycirc;",   'Y', "Ycirc",      374,  "Ŷ",   "Latin capital letter y with circumflex");
      symbol_map.emplace("&ycirc;",   'y', "ycirc",      375,  "ŷ",   "Latin small letter y with circumflex");
      symbol_map.emplace("&Yuml;",    'Y', "Yuml",       376,  "Ÿ",   "Capital Y with diaeres");
      symbol_map.emplace("&Zacute;",  'Z', "Zacute",     377,  "Ź",   "Latin Capital Letter Z with acute");
      symbol_map.emplace("&zacute;",  'z', "zacute",     378,  "ź",   "Latin Small Letter Z with acute");
      symbol_map.emplace("&Zdot;",    'Z', "Zdot",       379,  "Ż",   "Latin Capital Letter Z with dot above");
      symbol_map.emplace("&zdot;",    'z', "zdot",       380,  "ż",   "Latin Small Letter Z with dot above");
      symbol_map.emplace("&Zcaron;",  'Z', "Zcaron",     381,  "Ž",   "Latin Capital Letter Z with caron");
      symbol_map.emplace("&zcaron;",  'z', "zcaron",     382,  "ž",   "Latin Small Letter Z with caron");
      symbol_map.emplace("",          's', "longfs",     383,  "ſ",   "Latin Small Letter long S");

      // === Latin Extended-B ===
      // (skipping most 348-591)

      symbol_map.emplace("&fnof;",    'f', "fnof",       402,  "ƒ",   "Lowercase with hook");

      // === IPA Extensions ===
      // (skipping 591-687; Latin repeats?)

      // === Spacing modifier letters ===
      // (skipping most 688-767; mostly small version of other symbols for spacing?)
      symbol_map.emplace("&circ;",    '^', "circ",       710,  "ˆ",   "Circumflex accent");
      symbol_map.emplace("&tilde;",   '~', "tilde",      732,  "˜",   "Tilde");

      // === Combining marks ===
      // (skipping 768-879)

      // === Greek and Coptic ===
      // (880-1023)
      symbol_map.emplace("&Alpha;",   'A', "Alpha",      913,  "Α",   "Alpha");
      symbol_map.emplace("&Beta;",    'B', "Beta",       914,  "Β",   "Beta");
      symbol_map.emplace("&Gamma;",   'G', "Gamma",      915,  "Γ",   "Gamma");
      symbol_map.emplace("&Delta;",   'D', "Delta",      916,  "Δ",   "Delta");
      symbol_map.emplace("&Epsilon;", 'E', "Epsilon",    917,  "Ε",   "Epsilon");
      symbol_map.emplace("&Zeta;",    'Z', "Zeta",       918,  "Ζ",   "Zeta");
      symbol_map.emplace("&Eta;",     'H', "Eta",        919,  "Η",   "Eta");
      symbol_map.emplace("&Theta;",   'T', "Theta",      920,  "Θ",   "Theta");
      symbol_map.emplace("&Iota;",    'I', "Iota",       921,  "Ι",   "Iota");
      symbol_map.emplace("&Kappa;",   'K', "Kappa",      922,  "Κ",   "Kappa");
      symbol_map.emplace("&Lambda;",  'L', "Lambda",     923,  "Λ",   "Lambda");
      symbol_map.emplace("&Mu;",      '<', "Mu",         924,  "Μ",   "Mu");
      symbol_map.emplace("&Nu;",      'N', "Nu",         925,  "Ν",   "Nu");
      symbol_map.emplace("&Xi;",      'X', "Xi",         926,  "Ξ",   "Xi");
      symbol_map.emplace("&Omicron;", 'O', "Omicron",    927,  "Ο",   "Omicron");
      symbol_map.emplace("&Pi;",      'P', "Pi",         928,  "Π",   "Pi");
      symbol_map.emplace("&Rho;",     'R', "Rho",        929,  "Ρ",   "Rho");
      symbol_map.emplace("&Sigma;",   'S', "Sigma",      931,  "Σ",   "Sigma");
      symbol_map.emplace("&Tau;",     'T', "Tau",        932,  "Τ",   "Tau");
      symbol_map.emplace("&Upsilon;", 'Y', "Upsilon",    933,  "Υ",   "Upsilon");
      symbol_map.emplace("&Phi;",     'P', "Phi",        934,  "Φ",   "Phi");
      symbol_map.emplace("&Chi;",     'X', "Chi",        935,  "Χ",   "Chi");
      symbol_map.emplace("&Psi;",     'W', "Psi",        936,  "Ψ",   "Psi");
      symbol_map.emplace("&Omega;",   'O', "Omega",      937,  "Ω",   "Omega");
      symbol_map.emplace("&alpha;",   'a', "alpha",      945,  "α",   "alpha");
      symbol_map.emplace("&beta;",    'b', "beta",       946,  "β",   "beta");
      symbol_map.emplace("&gamma;",   'g', "gamma",      947,  "γ",   "gamma");
      symbol_map.emplace("&delta;",   'd', "delta",      948,  "δ",   "delta");
      symbol_map.emplace("&epsilon;", 'e', "epsilon",    949,  "ε",   "epsilon");
      symbol_map.emplace("&zeta;",    'z', "zeta",       950,  "ζ",   "zeta");
      symbol_map.emplace("&eta;",     'n', "eta",        951,  "η",   "eta");
      symbol_map.emplace("&theta;",   't', "theta",      952,  "θ",   "theta");
      symbol_map.emplace("&iota;",    'i', "iota",       953,  "ι",   "iota");
      symbol_map.emplace("&kappa;",   'k', "kappa",      954,  "κ",   "kappa");
      symbol_map.emplace("&lambda;",  'l', "lambda",     955,  "λ",   "lambda");
      symbol_map.emplace("&mu;",      'u', "mu",         956,  "μ",   "mu");
      symbol_map.emplace("&nu;",      'v', "nu",         957,  "ν",   "nu");
      symbol_map.emplace("&xi;",      'x', "xi",         958,  "ξ",   "xi");
      symbol_map.emplace("&omicron;", 'o', "omicron",    959,  "ο",   "omicron");
      symbol_map.emplace("&pi;",      'p', "pi",         960,  "π",   "pi");
      symbol_map.emplace("&rho;",     'p', "rho",        961,  "ρ",   "rho");
      symbol_map.emplace("&sigmaf;",  'c', "sigmaf",     962,  "ς",   "sigmaf");
      symbol_map.emplace("&sigma;",   's', "sigma",      963,  "σ",   "sigma");
      symbol_map.emplace("&tau;",     't', "tau",        964,  "τ",   "tau");
      symbol_map.emplace("&upsilon;", 'u', "upsilon",    965,  "υ",   "upsilon");
      symbol_map.emplace("&phi;",     'p', "phi",        966,  "φ",   "phi");
      symbol_map.emplace("&chi;",     'x', "chi",        967,  "χ",   "chi");
      symbol_map.emplace("&psi;",     's', "psi",        968,  "ψ",   "psi");
      symbol_map.emplace("&omega;",   'w', "omega",      969,  "ω",   "omega");
      symbol_map.emplace("&thetasym;", 'T', "thetasym",  977,  "ϑ",   "Theta symbol");
      symbol_map.emplace("&upsih;",   'U', "upsih",      978,  "ϒ",   "Upsilon symbol");
      symbol_map.emplace("&piv;",     'O', "piv",        982,  "ϖ",   "Pi symbol");

      // === Higher ID Symbols ===

      symbol_map.emplace("&ensp;",    ' ', "ensp",       8194, " ",   "En space");
      symbol_map.emplace("&emsp;",    ' ', "emsp",       8195, " ",   "Em space");
      symbol_map.emplace("&thinsp;",  ' ', "thinsp",     8201, " ",   "Thin space");
      symbol_map.emplace("&zwnj;",    ' ', "zwnj",       8204, ":",   "Zero width non-joiner");
      symbol_map.emplace("&zwj;",     ' ', "zwj",        8205, ":",   "Zero width joiner");
      symbol_map.emplace("&lrm;",     '?', "lrm",        8206, ":",   "Left-to-right mark");
      symbol_map.emplace("&rlm;",     '?', "rlm",        8207, ":",   "Right-to-left mark");
      symbol_map.emplace("&ndash;",   '-', "ndash",      8211, "–",   "En dash");
      symbol_map.emplace("&mdash;",   '-', "mdash",      8212, "—",   "Em dash");
      symbol_map.emplace("&lsquo;",   ' ', "lsquo",      8216, "‘",   "Left single quotation mark");
      symbol_map.emplace("&rsquo;",   ' ', "rsquo",      8217, "’",   "Right single quotation mark");
      symbol_map.emplace("&sbquo;",   ' ', "sbquo",      8218, "‚",   "Single low-9 quotation mark");
      symbol_map.emplace("&ldquo;",   ' ', "ldquo",      8220, "“",   "Left double quotation mark");
      symbol_map.emplace("&rdquo;",   ' ', "rdquo",      8221, "”",   "Right double quotation mark");
      symbol_map.emplace("&bdquo;",   ' ', "bdquo",      8222, "„",   "Double low-9 quotation mark");
      symbol_map.emplace("&dagger;",  ' ', "dagger",     8224, "†",   "Dagger");
      symbol_map.emplace("&Dagger;",  ' ', "Dagger",     8225, "‡",   "Double dagger");
      symbol_map.emplace("&bull;",    ' ', "bull",       8226, "•",   "Bullet");
      symbol_map.emplace("&hellip;",  ' ', "hellip",     8230, "…",   "Horizontal ellipsis");
      symbol_map.emplace("&permil;",  ' ', "permil",     8240, "‰",   "Per mille");
      symbol_map.emplace("&prime;",   ' ', "prime",      8242, "′",   "Minutes (Degrees)");
      symbol_map.emplace("&Prime;",   ' ', "Prime",      8243, "″",   "Seconds (Degrees)");
      symbol_map.emplace("&lsaquo;",  ' ', "lsaquo",     8249, "‹",   "Single left angle quotation");
      symbol_map.emplace("&rsaquo;",  ' ', "rsaquo",     8250, "›",   "Single right angle quotation");
      symbol_map.emplace("&oline;",   ' ', "oline",      8254, "‾",   "Overline");
      symbol_map.emplace("&euro;",    ' ', "euro",       8364, "€",   "Euro");
      symbol_map.emplace("&trade;",   ' ', "trade",      8482, "™",   "Trademark");
      symbol_map.emplace("&larr;",    ' ', "larr",       8592, "←",   "Left arrow");
      symbol_map.emplace("&uarr;",    ' ', "uarr",       8593, "↑",   "Up arrow");
      symbol_map.emplace("&rarr;",    ' ', "rarr",       8594, "→",   "Right arrow");
      symbol_map.emplace("&darr;",    ' ', "darr",       8595, "↓",   "Down arrow");
      symbol_map.emplace("&harr;",    ' ', "harr",       8596, "↔",   "Left right arrow");
      symbol_map.emplace("&crarr;",   ' ', "crarr",      8629, "↵",   "Carriage return arrow");
      symbol_map.emplace("&forall;",  ' ', "forall",     8704, "∀",   "For all");
      symbol_map.emplace("&part;",    ' ', "part",       8706, "∂",   "Part");
      symbol_map.emplace("&exist;",   ' ', "exist",      8707, "∃",   "Exist");
      symbol_map.emplace("&empty;",   ' ', "empty",      8709, "∅",   "Empty");
      symbol_map.emplace("&nabla;",   ' ', "nabla",      8711, "∇",   "Nabla");
      symbol_map.emplace("&isin;",    ' ', "isin",       8712, "∈",   "Is in");
      symbol_map.emplace("&notin;",   ' ', "notin",      8713, "∉",   "Not in");
      symbol_map.emplace("&ni;",      ' ', "ni",         8715, "∋",   "Ni");
      symbol_map.emplace("&prod;",    ' ', "prod",       8719, "∏",   "Product");
      symbol_map.emplace("&sum;",     ' ', "sum",        8721, "∑",   "Sum");
      symbol_map.emplace("&minus;",   ' ', "minus",      8722, "−",   "Minus");
      symbol_map.emplace("&lowast;",  ' ', "lowast",     8727, "∗",   "Asterisk (Lowast)");
      symbol_map.emplace("&radic;",   ' ', "radic",      8730, "√",   "Square root");
      symbol_map.emplace("&prop;",    ' ', "prop",       8733, "∝",   "Proportional to");
      symbol_map.emplace("&infin;",   ' ', "infin",      8734, "∞",   "Infinity");
      symbol_map.emplace("&ang;",     ' ', "ang",        8736, "∠",   "Angle");
      symbol_map.emplace("&and;",     ' ', "and",        8743, "∧",   "And");
      symbol_map.emplace("&or;",      ' ', "or",         8744, "∨",   "Or");
      symbol_map.emplace("&cap;",     ' ', "cap",        8745, "∩",   "Cap");
      symbol_map.emplace("&cup;",     ' ', "cup",        8746, "∪",   "Cup");
      symbol_map.emplace("&int;",     ' ', "int",        8747, "∫",   "Integral");
      symbol_map.emplace("&there4;",  ' ', "there4",     8756, "∴",   "Therefore");
      symbol_map.emplace("&sim;",     ' ', "sim",        8764, "∼",   "Similar to");
      symbol_map.emplace("&cong;",    ' ', "cong",       8773, "≅",   "Congurent to");
      symbol_map.emplace("&asymp;",   ' ', "asymp",      8776, "≈",   "Almost equal");
      symbol_map.emplace("&ne;",      ' ', "ne",         8800, "≠",   "Not equal");
      symbol_map.emplace("&equiv;",   ' ', "equiv",      8801, "≡",   "Equivalent");
      symbol_map.emplace("&le;",      ' ', "le",         8804, "≤",   "Less or equal");
      symbol_map.emplace("&ge;",      ' ', "ge",         8805, "≥",   "Greater or equal");
      symbol_map.emplace("&sub;",     ' ', "sub",        8834, "⊂",   "Subset of");
      symbol_map.emplace("&sup;",     ' ', "sup",        8835, "⊃",   "Superset of");
      symbol_map.emplace("&nsub;",    ' ', "nsub",       8836, "⊄",   "Not subset of");
      symbol_map.emplace("&sube;",    ' ', "sube",       8838, "⊆",   "Subset or equal");
      symbol_map.emplace("&supe;",    ' ', "supe",       8839, "⊇",   "Superset or equal");
      symbol_map.emplace("&oplus;",   ' ', "oplus",      8853, "⊕",   "Circled plus");
      symbol_map.emplace("&otimes;",  ' ', "otimes",     8855, "⊗",   "Circled times");
      symbol_map.emplace("&perp;",    ' ', "perp",       8869, "⊥",   "Perpendicular");
      symbol_map.emplace("&sdot;",    ' ', "sdot",       8901, "⋅",   "Dot operator");
      symbol_map.emplace("&lceil;",   ' ', "lceil",      8968, "⌈",   "Left ceiling");
      symbol_map.emplace("&rceil;",   ' ', "rceil",      8969, "⌉",   "Right ceiling");
      symbol_map.emplace("&lfloor;",  ' ', "lfloor",     8970, "⌊",   "Left floor");
      symbol_map.emplace("&rfloor;",  ' ', "rfloor",     8971, "⌋",   "Right floor");
      symbol_map.emplace("&loz;",     ' ', "loz",        9674, "◊",   "Lozenge");
      symbol_map.emplace("&spades;",  ' ', "spades",     9824, "♠",   "Spade");
      symbol_map.emplace("&clubs;",   ' ', "clubs",      9827, "♣",   "Club");
      symbol_map.emplace("&hearts;",  ' ', "hearts",     9829, "♥",   "Heart");
      symbol_map.emplace("&diams;",   ' ', "diams",      9830, "♦",   "Diamond");
    }
    return symbol_map;
  }

  /// A mapping of html tags to the emphatic style that they represent.
  /// Since the mapping between HTML and Emphatic text encodings are useful in multiple contexts,
  /// include easy access to them here.
  static const auto & GetTextStyleMap_FromHTML() {
    static std::map<String, String> html_map;
    if (html_map.size() == 0) {
      html_map["b"] = "bold";
      html_map["big"] = "big";
      html_map["code"] = "code";
      html_map["del"] = "strikethrough";
      html_map["i"] = "italic";
      html_map["small"] = "small";
      html_map["sub"] = "subscript";
      html_map["sup"] = "superscript";
      html_map["u"] = "underline";
      html_map["h1"] = "header1";
      html_map["h2"] = "header2";
      html_map["h3"] = "header3";
      html_map["h4"] = "header4";
      html_map["h5"] = "header5";
      html_map["h6"] = "header6";
    }
    return html_map;
  }

  /// Request a map of html tags to the emphatic style that they represent.
  /// Since the mapping between HTML and Emphatic text encodings are useful in multiple contexts,
  /// include easy access to them here.
  static const auto & GetTextStyleMap_ToHTML() {
    using map_t = std::map<String, String>;
    static map_t out_map = emp::flip_map<String, String, map_t>(GetTextStyleMap_FromHTML());
    return out_map;
  }
}

#endif // #ifndef EMP_TEXT_TEXT_UTILS_HPP_INCLUDE
