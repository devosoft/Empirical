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
    String name = "";        ///< General symbol name (usually based on HTML name)
    size_t id = 0;           ///< Unicode ID number of symbol
    String unicode = "";     ///< Unicode sequence for symbol (usually two characters)
    char placeholder = '\0'; ///< A single character that can be used as a placeholder
    String desc = "";        ///< English description of what symbol looks like
    String html_tag = "";    ///< Tag to use to represent this symbol in HTML

    TextSymbolInfo() = default;
    TextSymbolInfo(const TextSymbolInfo &)= default;
    TextSymbolInfo(TextSymbolInfo &&)= default;
    TextSymbolInfo(String name, size_t id, String unicode, char placeholder, String desc)
      : name(name), id(id), unicode(unicode), placeholder(placeholder), desc(desc) {
        html_tag = MakeString('&', name, ';');
      }

    auto operator<=>(const TextSymbolInfo &) const = default;
  };

  /// A mapping of allowed symbol names to placeholder characters.
  static const auto & GetTextSymbolNames() {
    static std::set<TextSymbolInfo> symbol_map;
    if (symbol_map.size() == 0) {

      // === UNICODE Latin-1 Supplement ===

      // Punctuation & Symbols
      symbol_map.emplace("nbsp",      160,  " ",  ' ',    "Non-breaking space");
      symbol_map.emplace("iexcl",     161,  "¡",  '!',    "Inverted exclamation point");
      symbol_map.emplace("cent",      162,  "¢",  'c',    "Cent");
      symbol_map.emplace("pound",     163,  "£",  'L',    "Pound");
      symbol_map.emplace("curren",    164,  "¤",  '$',    "Currency");
      symbol_map.emplace("yen",       165,  "¥",  'Y',    "Yen");
      symbol_map.emplace("brvbar",    166,  "¦",  '|',    "Broken vertical bar");
      symbol_map.emplace("sect",      167,  "§",  'S',    "Section");
      symbol_map.emplace("uml",       168,  "¨",  '~',    "Spacing diaeresis");
      symbol_map.emplace("copy",      169,  "©",  'c',    "Copyright");
      symbol_map.emplace("ordf",      170,  "ª",  'a',    "Feminine ordinal indicator");
      symbol_map.emplace("laquo",     171,  "«",  '<',    "Opening/Left angle quotation mark");
      symbol_map.emplace("not",       172,  "¬",  '~',    "Negation");
      symbol_map.emplace("shy",       173,  "­-",  '-',    "Soft hyphen");
      symbol_map.emplace("reg",       174,  "®",  'R',    "Registered trademark");
      symbol_map.emplace("macr",      175,  "¯",  '-',    "Spacing macron");
      symbol_map.emplace("deg",       176,  "°",  'o',    "Degree");
      symbol_map.emplace("plusmn",    177,  "±",  '+',    "Plus or minus");
      symbol_map.emplace("sup2",      178,  "²",  '2',    "Superscript 2");
      symbol_map.emplace("sup3",      179,  "³",  '3',    "Superscript 3");
      symbol_map.emplace("acute",     180,  "´",  '\'',   "Spacing acute");
      symbol_map.emplace("micro",     181,  "µ",  'u',    "Micro");
      symbol_map.emplace("para",      182,  "¶",  'P',    "Paragraph");
      symbol_map.emplace("dot",       182,  "·",  '.',    "Dot");
      symbol_map.emplace("cedil",     184,  "¸",  ',',    "Spacing cedilla");
      symbol_map.emplace("sup1",      185,  "¹",  '1',    "Superscript 1");
      symbol_map.emplace("ordm",      186,  "º",  'o',    "Masculine ordinal indicator");
      symbol_map.emplace("raquo",     187,  "»",  '>',    "Closing/Right angle quotation mark");
      symbol_map.emplace("frac14",    188,  "¼",  '/',    "Fraction 1/4");
      symbol_map.emplace("frac12",    189,  "½",  '/',    "Fraction 1/2");
      symbol_map.emplace("frac34",    190,  "¾",  '/',    "Fraction 3/4");
      symbol_map.emplace("iquest",    191,  "¿",  '?',    "Inverted question mark");

      // Uppercase letters
      symbol_map.emplace("Agrave",    192,  "À", 'A',  "Capital a with grave accent");
      symbol_map.emplace("Aacute",    193,  "Á", 'A',  "Capital a with acute accent");
      symbol_map.emplace("Acirc",     194,  "Â", 'A',  "Capital a with circumflex accent");
      symbol_map.emplace("Atilde",    195,  "Ã", 'A',  "Capital a with tilde");
      symbol_map.emplace("Auml",      196,  "Ä", 'A',  "Capital a with umlaut");
      symbol_map.emplace("Aring",     197,  "Å", 'A',  "Capital a with ring");
      symbol_map.emplace("AElig",     198,  "Æ", 'A',  "Capital ae");
      symbol_map.emplace("Ccedil",    199,  "Ç", 'C',  "Capital c with cedilla");
      symbol_map.emplace("Egrave",    200,  "È", 'E',  "Capital e with grave accent");
      symbol_map.emplace("Eacute",    201,  "É", 'E',  "Capital e with acute accent");
      symbol_map.emplace("Ecirc",     202,  "Ê", 'E',  "Capital e with circumflex accent");
      symbol_map.emplace("Euml",      203,  "Ë", 'E',  "Capital e with umlaut");
      symbol_map.emplace("Igrave",    204,  "Ì", 'I',  "Capital i with grave accent");
      symbol_map.emplace("Iacute",    205,  "Í", 'I',  "Capital i with accute accent");
      symbol_map.emplace("Icirc",     206,  "Î", 'I',  "Capital i with circumflex accent");
      symbol_map.emplace("Iuml",      207,  "Ï", 'I',  "Capital i with umlaut");
      symbol_map.emplace("ETH",       208,  "Ð", 'D',  "Capital eth (Icelandic)");
      symbol_map.emplace("Ntilde",    209,  "Ñ", 'N',  "Capital n with tilde");
      symbol_map.emplace("Ograve",    210,  "Ò", 'O',  "Capital o with grave accent");
      symbol_map.emplace("Oacute",    211,  "Ó", 'O',  "Capital o with accute accent");
      symbol_map.emplace("Ocirc",     212,  "Ô", 'O',  "Capital o with circumflex accent");
      symbol_map.emplace("Otilde",    213,  "Õ", 'O',  "Capital o with tilde");
      symbol_map.emplace("Ouml",      214,  "Ö", 'O',  "Capital o with umlaut");
      symbol_map.emplace("times",     215,  "×", 'x',  "Multiplication");
      symbol_map.emplace("Oslash",    216,  "Ø", 'O',  "Capital o with slash");
      symbol_map.emplace("Ugrave",    217,  "Ù", 'U',  "Capital u with grave accent");
      symbol_map.emplace("Uacute",    218,  "Ú", 'U',  "Capital u with acute accent");
      symbol_map.emplace("Ucirc",     219,  "Û", 'U',  "Capital u with circumflex accent");
      symbol_map.emplace("Uuml",      220,  "Ü", 'U',  "Capital u with umlaut");
      symbol_map.emplace("Yacute",    221,  "Ý", 'Y',  "Capital y with acute accent");
      symbol_map.emplace("THORN",     222,  "Þ", 'P',  "Capital thorn (Icelandic)");

      // Lowercase letters
      symbol_map.emplace("szlig",     223,  "ß", 'B',  "Lowercase sharp s (German)");
      symbol_map.emplace("agrave",    224,  "à", 'a',  "Lowercase a with grave accent");
      symbol_map.emplace("aacute",    225,  "á", 'a',  "Lowercase a with acute accent");
      symbol_map.emplace("acirc",     226,  "â", 'a',  "Lowercase a with circumflex accent");
      symbol_map.emplace("atilde",    227,  "ã", 'a',  "Lowercase a with tilde");
      symbol_map.emplace("auml",      228,  "ä", 'a',  "Lowercase a with umlaut");
      symbol_map.emplace("aring",     229,  "å", 'a',  "Lowercase a with ring");
      symbol_map.emplace("aelig",     230,  "æ", 'a',  "Lowercase ae");
      symbol_map.emplace("ccedil",    231,  "ç", 'c',  "Lowercase c with cedilla");
      symbol_map.emplace("egrave",    232,  "è", 'e',  "Lowercase e with grave accent");
      symbol_map.emplace("eacute",    233,  "é", 'e',  "Lowercase e with acute accent");
      symbol_map.emplace("ecirc",     234,  "ê", 'e',  "Lowercase e with circumflex accent");
      symbol_map.emplace("euml",      235,  "ë", 'e',  "Lowercase e with umlaut");
      symbol_map.emplace("igrave",    236,  "ì", 'i',  "Lowercase i with grave accent");
      symbol_map.emplace("iacute",    237,  "í", 'i',  "Lowercase i with acute accent");
      symbol_map.emplace("icirc",     238,  "î", 'i',  "Lowercase i with circumflex accent");
      symbol_map.emplace("iuml",      239,  "ï", 'i',  "Lowercase i with umlaut");
      symbol_map.emplace("eth",       240,  "ð", 'o',  "Lowercase eth (Icelandic)");
      symbol_map.emplace("ntilde",    241,  "ñ", 'n',  "Lowercase n with tilde");
      symbol_map.emplace("ograve",    242,  "ò", 'o',  "Lowercase o with grave accent");
      symbol_map.emplace("oacute",    243,  "ó", 'o',  "Lowercase o with acute accent");
      symbol_map.emplace("ocirc",     244,  "ô", 'o',  "Lowercase o with circumflex accent");
      symbol_map.emplace("otilde",    245,  "õ", 'o',  "Lowercase o with tilde");
      symbol_map.emplace("ouml",      246,  "ö", 'o',  "Lowercase o with umlaut");
      symbol_map.emplace("divide",    247,  "÷", '/',  "Divide");
      symbol_map.emplace("oslash",    248,  "ø", 'o',  "Lowercase o with slash");
      symbol_map.emplace("ugrave",    249,  "ù", 'u',  "Lowercase u with grave accent");
      symbol_map.emplace("uacute",    250,  "ú", 'u',  "Lowercase u with acute accent");
      symbol_map.emplace("ucirc",     251,  "û", 'u',  "Lowercase u with circumflex accent");
      symbol_map.emplace("uuml",      252,  "ü", 'u',  "Lowercase u with umlaut");
      symbol_map.emplace("yacute",    253,  "ý", 'y',  "Lowercase y with acute accent");
      symbol_map.emplace("thorn",     254,  "þ", 'p',  "Lowercase thorn (Icelandic)");
      symbol_map.emplace("yuml",      255,  "ÿ", 'y',  "Lowercase y with umlaut");

      // === UNICODE Latin Extended-A ===

      // European latin
      symbol_map.emplace("Amacr",     256,  "Ā", 'A',   "Latin capital letter a with macron");
      symbol_map.emplace("amacr",     257,  "ā", 'a',   "Latin small letter a with macron");
      symbol_map.emplace("Abreve",    258,  "Ă", 'A',   "Latin capital letter a with breve");
      symbol_map.emplace("abreve",    259,  "ă", 'a',   "Latin small letter a with breve");
      symbol_map.emplace("Aogon",     260,  "Ą", 'A',   "Latin capital letter a with ogonek");
      symbol_map.emplace("aogon",     261,  "ą", 'a',   "Latin small letter a with ogonek");
      symbol_map.emplace("Cacute",    262,  "Ć", 'C',   "Latin capital letter c with acute");
      symbol_map.emplace("cacute",    263,  "ć", 'c',   "Latin small letter c with acute");
      symbol_map.emplace("Ccirc",     264,  "Ĉ", 'C',   "Latin capital letter c with circumflex");
      symbol_map.emplace("ccirc",     265,  "ĉ", 'c',   "Latin small letter c with circumflex");
      symbol_map.emplace("Cdot",      266,  "Ċ", 'C',   "Latin capital letter c with dot above");
      symbol_map.emplace("cdot",      267,  "ċ", 'c',   "Latin small letter c with dot above");
      symbol_map.emplace("Ccaron",    268,  "Č", 'C',   "Latin capital letter c with caron");
      symbol_map.emplace("ccaron",    269,  "č", 'c',   "Latin small letter c with caron");
      symbol_map.emplace("Dcaron",    270,  "Ď", 'D',   "Latin capital letter d with caron");
      symbol_map.emplace("dcaron",    271,  "ď", 'd',   "Latin small letter d with caron");
      symbol_map.emplace("Dstrok",    272,  "Đ", 'D',   "Latin capital letter d with stroke");
      symbol_map.emplace("dstrok",    273,  "đ", 'd',   "Latin small letter d with stroke");
      symbol_map.emplace("Emacr",     274,  "Ē", 'E',   "Latin capital letter e with macron");
      symbol_map.emplace("emacr",     275,  "ē", 'e',   "Latin small letter e with macron");
      symbol_map.emplace("Ebreve",    276,  "Ĕ", 'E',   "Latin capital letter e with breve");
      symbol_map.emplace("ebreve",    277,  "ĕ", 'e',   "Latin small letter e with breve");
      symbol_map.emplace("Edot",      278,  "Ė", 'E',   "Latin capital letter e with dot above");
      symbol_map.emplace("edot",      279,  "ė", 'e',   "Latin small letter e with dot above");
      symbol_map.emplace("Eogon",     280,  "Ę", 'E',   "Latin capital letter e with ogonek");
      symbol_map.emplace("eogon",     281,  "ę", 'e',   "Latin small letter e with ogonek");
      symbol_map.emplace("Ecaron",    282,  "Ě", 'E',   "Latin capital letter e with caron");
      symbol_map.emplace("ecaron",    283,  "ě", 'e',   "Latin small letter e with caron");
      symbol_map.emplace("Gcirc",     284,  "Ĝ", 'G',   "Latin capital letter g with circumflex");
      symbol_map.emplace("gcirc",     285,  "ĝ", 'g',   "Latin small letter g with circumflex");
      symbol_map.emplace("Gbreve",    286,  "Ğ", 'G',   "Latin capital letter g with breve");
      symbol_map.emplace("gbreve",    287,  "ğ", 'g',   "Latin small letter g with breve");
      symbol_map.emplace("Gdot",      288,  "Ġ", 'G',   "Latin capital letter g with dot above");
      symbol_map.emplace("gdot",      289,  "ġ", 'g',   "Latin small letter g with dot above");
      symbol_map.emplace("Gcedil",    290,  "Ģ", 'G',   "Latin capital letter g with cedilla");
      symbol_map.emplace("gcedil",    291,  "ģ", 'g',   "Latin small letter g with cedilla");
      symbol_map.emplace("Hcirc",     292,  "Ĥ", 'H',   "Latin capital letter h with circumflex");
      symbol_map.emplace("hcirc",     293,  "ĥ", 'h',   "Latin small letter h with circumflex");
      symbol_map.emplace("Hstrok",    294,  "Ħ", 'H',   "Latin capital letter h with stroke");
      symbol_map.emplace("hstrok",    295,  "ħ", 'h',   "Latin small letter h with stroke");
      symbol_map.emplace("Itilde",    296,  "Ĩ", 'I',   "Latin capital letter I with tilde");
      symbol_map.emplace("itilde",    297,  "ĩ", 'i',   "Latin small letter I with tilde");
      symbol_map.emplace("Imacr",     298,  "Ī", 'I',   "Latin capital letter I with macron");
      symbol_map.emplace("imacr",     299,  "ī", 'i',   "Latin small letter I with macron");
      symbol_map.emplace("Ibreve",    300,  "Ĭ", 'I',   "Latin capital letter I with breve");
      symbol_map.emplace("ibreve",    301,  "ĭ", 'i',   "Latin small letter I with breve");
      symbol_map.emplace("Iogon",     302,  "Į", 'I',   "Latin capital letter I with ogonek");
      symbol_map.emplace("iogon",     303,  "į", 'i',   "Latin small letter I with ogonek");
      symbol_map.emplace("Idot",      304,  "İ", 'I',   "Latin capital letter I with dot above");
      symbol_map.emplace("imath",     305,  "ı", 'i',   "Latin small letter dotless I");
      symbol_map.emplace("IJlig",     306,  "Ĳ", 'I',   "Latin capital ligature ij");
      symbol_map.emplace("ijlig",     307,  "ĳ", 'i',   "Latin small ligature ij");
      symbol_map.emplace("Jcirc",     308,  "Ĵ", 'J',   "Latin capital letter j with circumflex");
      symbol_map.emplace("jcirc",     309,  "ĵ", 'j',   "Latin small letter j with circumflex");
      symbol_map.emplace("Kcedil",    310,  "Ķ", 'K',   "Latin capital letter k with cedilla");
      symbol_map.emplace("kcedil",    311,  "ķ", 'k',   "Latin small letter k with cedilla");
      symbol_map.emplace("kgreen",    312,  "ĸ", 'k',   "Latin small letter kra");
      symbol_map.emplace("Lacute",    313,  "Ĺ", 'L',   "Latin capital letter l with acute");
      symbol_map.emplace("lacute",    314,  "ĺ", 'l',   "Latin small letter l with acute");
      symbol_map.emplace("Lcedil",    315,  "Ļ", 'L',   "Latin capital letter l with cedilla");
      symbol_map.emplace("lcedil",    316,  "ļ", 'l',   "Latin small letter l with cedilla");
      symbol_map.emplace("Lcaron",    317,  "Ľ", 'L',   "Latin capital letter l with caron");
      symbol_map.emplace("lcaron",    318,  "ľ", 'l',   "Latin small letter l with caron");
      symbol_map.emplace("Lmidot",    319,  "Ŀ", 'L',   "Latin capital letter l with middle dot");
      symbol_map.emplace("lmidot",    320,  "ŀ", 'l',   "Latin small letter l with middle dot");
      symbol_map.emplace("Lstrok",    321,  "Ł", 'L',   "Latin capital letter l with stroke");
      symbol_map.emplace("lstrok",    322,  "ł", 'l',   "Latin small letter l with stroke");
      symbol_map.emplace("Nacute",    323,  "Ń", 'N',   "Latin capital letter n with acute");
      symbol_map.emplace("nacute",    324,  "ń", 'n',   "Latin small letter n with acute");
      symbol_map.emplace("Ncedil",    325,  "Ņ", 'N',   "Latin capital letter n with cedilla");
      symbol_map.emplace("ncedil",    326,  "ņ", 'n',   "Latin small letter n with cedilla");
      symbol_map.emplace("Ncaron",    327,  "Ň", 'N',   "Latin capital letter n with caron");
      symbol_map.emplace("ncaron",    328,  "ň", 'n',   "Latin small letter n with caron");
      symbol_map.emplace("napos",     329,  "ŉ", 'n',   "Latin small letter n preceded by apostrophe"); // DEPRECATED!
      symbol_map.emplace("ENG",       330,  "Ŋ", 'N',   "Latin capital letter eng");
      symbol_map.emplace("eng",       331,  "ŋ", 'n',   "Latin small letter eng");
      symbol_map.emplace("Omacr",     332,  "Ō", 'O',   "Latin capital letter o with macron");
      symbol_map.emplace("omacr",     333,  "ō", 'o',   "Latin small letter o with macron");
      symbol_map.emplace("Obreve",    334,  "Ŏ", 'O',   "Latin capital letter o with breve");
      symbol_map.emplace("obreve",    335,  "ŏ", 'o',   "Latin small letter o with breve");
      symbol_map.emplace("Odblac",    336,  "Ő", 'O',   "Latin capital letter o with double acute");
      symbol_map.emplace("odblac",    337,  "ő", 'o',   "Latin small letter o with double acute");
      symbol_map.emplace("OElig",     338,  "Œ", 'O',   "Uppercase ligature OE");
      symbol_map.emplace("oelig",     339,  "œ", 'o',   "Lowercase ligature OE");
      symbol_map.emplace("Racute",    340,  "Ŕ", 'R',   "Latin capital letter r with acute");
      symbol_map.emplace("racute",    341,  "ŕ", 'r',   "Latin small letter r with acute");
      symbol_map.emplace("Rcedil",    342,  "Ŗ", 'R',   "Latin capital letter r with cedilla");
      symbol_map.emplace("rcedil",    343,  "ŗ", 'r',   "Latin small letter r with cedilla");
      symbol_map.emplace("Rcaron",    344,  "Ř", 'R',   "Latin capital letter r with caron");
      symbol_map.emplace("rcaron",    345,  "ř", 'r',   "Latin small letter r with caron");
      symbol_map.emplace("Sacute",    346,  "Ś", 'S',   "Latin capital letter s with acute");
      symbol_map.emplace("sacute",    347,  "ś", 's',   "Latin small letter s with acute");
      symbol_map.emplace("Scirc",     348,  "Ŝ", 'S',   "Latin capital letter s with circumflex");
      symbol_map.emplace("scirc",     349,  "ŝ", 's',   "Latin small letter s with circumflex");
      symbol_map.emplace("Scedil",    350,  "Ş", 'S',   "Latin capital letter s with cedilla");
      symbol_map.emplace("scedil",    351,  "ş", 's',   "Latin small letter s with cedilla");
      symbol_map.emplace("Scaron",    352,  "Š", 'S',   "Uppercase S with caron");
      symbol_map.emplace("scaron",    353,  "š", 's',   "Lowercase S with caron");
      symbol_map.emplace("Tcedil",    354,  "Ţ", 'T',   "Latin capital letter t with cedilla");
      symbol_map.emplace("tcedil",    355,  "ţ", 't',   "Latin small letter t with cedilla");
      symbol_map.emplace("Tcaron",    356,  "Ť", 'T',   "Latin capital letter t with caron");
      symbol_map.emplace("tcaron",    357,  "ť", 't',   "Latin small letter t with caron");
      symbol_map.emplace("Tstrok",    358,  "Ŧ", 'T',   "Latin capital letter t with stroke");
      symbol_map.emplace("tstrok",    359,  "ŧ", 't',   "Latin small letter t with stroke");
      symbol_map.emplace("Utilde",    360,  "Ũ", 'U',   "Latin capital letter u with tilde");
      symbol_map.emplace("utilde",    361,  "ũ", 'u',   "Latin small letter u with tilde");
      symbol_map.emplace("Umacr",     362,  "Ū", 'U',   "Latin capital letter u with macron");
      symbol_map.emplace("umacr",     363,  "ū", 'u',   "Latin small letter u with macron");
      symbol_map.emplace("Ubreve",    364,  "Ŭ", 'U',   "Latin capital letter u with breve");
      symbol_map.emplace("ubreve",    365,  "ŭ", 'u',   "Latin small letter u with breve");
      symbol_map.emplace("Uring",     366,  "Ů", 'U',   "Latin capital letter u with ring above");
      symbol_map.emplace("uring",     367,  "ů", 'u',   "Latin small letter u with ring above");
      symbol_map.emplace("Udblac",    368,  "Ű", 'U',   "Latin capital letter u with double acute");
      symbol_map.emplace("udblac",    369,  "ű", 'u',   "Latin small letter u with double acute");
      symbol_map.emplace("Uogon",     370,  "Ų", 'U',   "Latin capital letter u with ogonek");
      symbol_map.emplace("uogon",     371,  "ų", 'u',   "Latin small letter u with ogonek");
      symbol_map.emplace("Wcirc",     372,  "Ŵ", 'W',   "Latin capital letter w with circumflex");
      symbol_map.emplace("wcirc",     373,  "ŵ", 'w',   "Latin small letter w with circumflex");
      symbol_map.emplace("Ycirc",     374,  "Ŷ", 'Y',   "Latin capital letter y with circumflex");
      symbol_map.emplace("ycirc",     375,  "ŷ", 'y',   "Latin small letter y with circumflex");
      symbol_map.emplace("Yuml",      376,  "Ÿ", 'Y',   "Capital Y with diaeres");
      symbol_map.emplace("Zacute",    377,  "Ź", 'Z',   "Latin Capital Letter Z with acute");
      symbol_map.emplace("zacute",    378,  "ź", 'z',   "Latin Small Letter Z with acute");
      symbol_map.emplace("Zdot",      379,  "Ż", 'Z',   "Latin Capital Letter Z with dot above");
      symbol_map.emplace("zdot",      380,  "ż", 'z',   "Latin Small Letter Z with dot above");
      symbol_map.emplace("Zcaron",    381,  "Ž", 'Z',   "Latin Capital Letter Z with caron");
      symbol_map.emplace("zcaron",    382,  "ž", 'z',   "Latin Small Letter Z with caron");
      symbol_map.emplace("longfs",    383,  "ſ", 's',   "Latin Small Letter long S");

      // === Latin Extended-B ===
      // (skipping most 348-591)

      symbol_map.emplace("fnof",      402,  "ƒ", 'f',   "Lowercase with hook");

      // === IPA Extensions ===
      // (skipping 591-687; Latin repeats?)

      // === Spacing modifier letters ===
      // (skipping most 688-767; mostly small version of other symbols for spacing?)
      symbol_map.emplace("circ",      710,  "ˆ",  '^',  "Circumflex accent");
      symbol_map.emplace("tilde",     732,  "˜",  '~',  "Tilde");

      // === Combining marks ===
      // (skipping 768-879)

      // === Greek and Coptic ===
      // (880-1023)
      symbol_map.emplace("Alpha",      913,  "Α", 'A',   "Alpha");
      symbol_map.emplace("Beta",       914,  "Β", 'B',   "Beta");
      symbol_map.emplace("Gamma",      915,  "Γ", 'G',   "Gamma");
      symbol_map.emplace("Delta",      916,  "Δ", 'D',   "Delta");
      symbol_map.emplace("Epsilon",    917,  "Ε", 'E',   "Epsilon");
      symbol_map.emplace("Zeta",       918,  "Ζ", 'Z',   "Zeta");
      symbol_map.emplace("Eta",        919,  "Η", 'H',   "Eta");
      symbol_map.emplace("Theta",      920,  "Θ", 'T',   "Theta");
      symbol_map.emplace("Iota",       921,  "Ι", 'I',   "Iota");
      symbol_map.emplace("Kappa",      922,  "Κ", 'K',   "Kappa");
      symbol_map.emplace("Lambda",     923,  "Λ", 'L',   "Lambda");
      symbol_map.emplace("Mu",         924,  "Μ", '<',   "Mu");
      symbol_map.emplace("Nu",         925,  "Ν", 'N',   "Nu");
      symbol_map.emplace("Xi",         926,  "Ξ", 'X',   "Xi");
      symbol_map.emplace("Omicron",    927,  "Ο", 'O',   "Omicron");
      symbol_map.emplace("Pi",         928,  "Π", 'P',   "Pi");
      symbol_map.emplace("Rho",        929,  "Ρ", 'R',   "Rho");
      symbol_map.emplace("Sigma",      931,  "Σ", 'S',   "Sigma");
      symbol_map.emplace("Tau",        932,  "Τ", 'T',   "Tau");
      symbol_map.emplace("Upsilon",    933,  "Υ", 'Y',   "Upsilon");
      symbol_map.emplace("Phi",        934,  "Φ", 'P',   "Phi");
      symbol_map.emplace("Chi",        935,  "Χ", 'X',   "Chi");
      symbol_map.emplace("Psi",        936,  "Ψ", 'W',   "Psi");
      symbol_map.emplace("Omega",      937,  "Ω", 'O',   "Omega");
      symbol_map.emplace("alpha",      945,  "α", 'a',   "alpha");
      symbol_map.emplace("beta",       946,  "β", 'b',   "beta");
      symbol_map.emplace("gamma",      947,  "γ", 'g',   "gamma");
      symbol_map.emplace("delta",      948,  "δ", 'd',   "delta");
      symbol_map.emplace("epsilon",    949,  "ε", 'e',   "epsilon");
      symbol_map.emplace("zeta",       950,  "ζ", 'z',   "zeta");
      symbol_map.emplace("eta",        951,  "η", 'n',   "eta");
      symbol_map.emplace("theta",      952,  "θ", 't',   "theta");
      symbol_map.emplace("iota",       953,  "ι", 'i',   "iota");
      symbol_map.emplace("kappa",      954,  "κ", 'k',   "kappa");
      symbol_map.emplace("lambda",     955,  "λ", 'l',   "lambda");
      symbol_map.emplace("mu",         956,  "μ", 'u',   "mu");
      symbol_map.emplace("nu",         957,  "ν", 'v',   "nu");
      symbol_map.emplace("xi",         958,  "ξ", 'x',   "xi");
      symbol_map.emplace("omicron",    959,  "ο", 'o',   "omicron");
      symbol_map.emplace("pi",         960,  "π", 'p',   "pi");
      symbol_map.emplace("rho",        961,  "ρ", 'p',   "rho");
      symbol_map.emplace("sigmaf",     962,  "ς", 'c',   "sigmaf");
      symbol_map.emplace("sigma",      963,  "σ", 's',   "sigma");
      symbol_map.emplace("tau",        964,  "τ", 't',   "tau");
      symbol_map.emplace("upsilon",    965,  "υ", 'u',   "upsilon");
      symbol_map.emplace("phi",        966,  "φ", 'p',   "phi");
      symbol_map.emplace("chi",        967,  "χ", 'x',   "chi");
      symbol_map.emplace("psi",        968,  "ψ", 's',   "psi");
      symbol_map.emplace("omega",      969,  "ω", 'w',   "omega");
      symbol_map.emplace("thetasym",   977,  "ϑ", 'T',   "Theta symbol");
      symbol_map.emplace("upsih",      978,  "ϒ", 'U',   "Upsilon symbol");
      symbol_map.emplace("piv",        982,  "ϖ", 'O',   "Pi symbol");

      // === Higher ID Symbols ===

      symbol_map.emplace("ensp",      8194,  " ",  ' ',   "En space");
      symbol_map.emplace("emsp",      8195,  " ",  ' ',   "Em space");
      symbol_map.emplace("thinsp",    8201,  " ",  ' ',   "Thin space");
      symbol_map.emplace("zwnj",      8204,  ":",  ' ',   "Zero width non-joiner");
      symbol_map.emplace("zwj",       8205,  ":",  ' ',   "Zero width joiner");
      symbol_map.emplace("lrm",       8206,  ":",  '?',   "Left-to-right mark");
      symbol_map.emplace("rlm",       8207,  ":",  '?',   "Right-to-left mark");
      symbol_map.emplace("ndash",     8211,  "–",  '-',   "En dash");
      symbol_map.emplace("mdash",     8212,  "—",  '-',   "Em dash");
      symbol_map.emplace("lsquo",     8216,  "‘",  ' ',   "Left single quotation mark");
      symbol_map.emplace("rsquo",     8217,  "’",  ' ',   "Right single quotation mark");
      symbol_map.emplace("sbquo",     8218,  "‚",  ' ',   "Single low-9 quotation mark");
      symbol_map.emplace("ldquo",     8220,  "“",  ' ',   "Left double quotation mark");
      symbol_map.emplace("rdquo",     8221,  "”",  ' ',   "Right double quotation mark");
      symbol_map.emplace("bdquo",     8222,  "„",  ' ',   "Double low-9 quotation mark");
      symbol_map.emplace("dagger",    8224,  "†",  ' ',   "Dagger");
      symbol_map.emplace("Dagger",    8225,  "‡",  ' ',   "Double dagger");
      symbol_map.emplace("bull",      8226,  "•",  ' ',   "Bullet");
      symbol_map.emplace("hellip",    8230,  "…",  ' ',   "Horizontal ellipsis");
      symbol_map.emplace("permil",    8240,  "‰",  ' ',   "Per mille");
      symbol_map.emplace("prime",     8242,  "′",  ' ',   "Minutes (Degrees)");
      symbol_map.emplace("Prime",     8243,  "″",  ' ',   "Seconds (Degrees)");
      symbol_map.emplace("lsaquo",    8249,  "‹",  ' ',   "Single left angle quotation");
      symbol_map.emplace("rsaquo",    8250,  "›",  ' ',   "Single right angle quotation");
      symbol_map.emplace("oline",     8254,  "‾",  ' ',   "Overline");
      symbol_map.emplace("euro",      8364,  "€",  ' ',   "Euro");
      symbol_map.emplace("trade",     8482,  "™",  ' ',   "Trademark");
      symbol_map.emplace("larr",      8592,  "←",  ' ',   "Left arrow");
      symbol_map.emplace("uarr",      8593,  "↑",  ' ',   "Up arrow");
      symbol_map.emplace("rarr",      8594,  "→",  ' ',   "Right arrow");
      symbol_map.emplace("darr",      8595,  "↓",  ' ',   "Down arrow");
      symbol_map.emplace("harr",      8596,  "↔",  ' ',   "Left right arrow");
      symbol_map.emplace("crarr",     8629,  "↵",  ' ',   "Carriage return arrow");
      symbol_map.emplace("forall",    8704,  "∀",  ' ',   "For all");
      symbol_map.emplace("part",      8706,  "∂",  ' ',   "Part");
      symbol_map.emplace("exist",     8707,  "∃",  ' ',   "Exist");
      symbol_map.emplace("empty",     8709,  "∅",  ' ',   "Empty");
      symbol_map.emplace("nabla",     8711,  "∇",  ' ',   "Nabla");
      symbol_map.emplace("isin",      8712,  "∈",  ' ',   "Is in");
      symbol_map.emplace("notin",     8713,  "∉",  ' ',   "Not in");
      symbol_map.emplace("ni",        8715,  "∋",  ' ',   "Ni");
      symbol_map.emplace("prod",      8719,  "∏",  ' ',   "Product");
      symbol_map.emplace("sum",       8721,  "∑",  ' ',   "Sum");
      symbol_map.emplace("minus",     8722,  "−",  ' ',   "Minus");
      symbol_map.emplace("lowast",    8727,  "∗",  ' ',   "Asterisk (Lowast)");
      symbol_map.emplace("radic",     8730,  "√",  ' ',   "Square root");
      symbol_map.emplace("prop",      8733,  "∝",  ' ',   "Proportional to");
      symbol_map.emplace("infin",     8734,  "∞",  ' ',   "Infinity");
      symbol_map.emplace("ang",       8736,  "∠",  ' ',   "Angle");
      symbol_map.emplace("and",       8743,  "∧",  ' ',   "And");
      symbol_map.emplace("or",        8744,  "∨",  ' ',   "Or");
      symbol_map.emplace("cap",       8745,  "∩",  ' ',   "Cap");
      symbol_map.emplace("cup",       8746,  "∪",  ' ',   "Cup");
      symbol_map.emplace("int",       8747,  "∫",  ' ',   "Integral");
      symbol_map.emplace("there4",    8756,  "∴",  ' ',   "Therefore");
      symbol_map.emplace("sim",       8764,  "∼",  ' ',   "Similar to");
      symbol_map.emplace("cong",      8773,  "≅",  ' ',   "Congurent to");
      symbol_map.emplace("asymp",     8776,  "≈",  ' ',   "Almost equal");
      symbol_map.emplace("ne",        8800,  "≠",  ' ',   "Not equal");
      symbol_map.emplace("equiv",     8801,  "≡",  ' ',   "Equivalent");
      symbol_map.emplace("le",        8804,  "≤",  ' ',   "Less or equal");
      symbol_map.emplace("ge",        8805,  "≥",  ' ',   "Greater or equal");
      symbol_map.emplace("sub",       8834,  "⊂",  ' ',   "Subset of");
      symbol_map.emplace("sup",       8835,  "⊃",  ' ',   "Superset of");
      symbol_map.emplace("nsub",      8836,  "⊄",  ' ',   "Not subset of");
      symbol_map.emplace("sube",      8838,  "⊆",  ' ',   "Subset or equal");
      symbol_map.emplace("supe",      8839,  "⊇",  ' ',   "Superset or equal");
      symbol_map.emplace("oplus",     8853,  "⊕",  ' ',   "Circled plus");
      symbol_map.emplace("otimes",    8855,  "⊗",  ' ',   "Circled times");
      symbol_map.emplace("perp",      8869,  "⊥",  ' ',   "Perpendicular");
      symbol_map.emplace("sdot",      8901,  "⋅",  ' ',   "Dot operator");
      symbol_map.emplace("lceil",     8968,  "⌈",  ' ',   "Left ceiling");
      symbol_map.emplace("rceil",     8969,  "⌉",  ' ',   "Right ceiling");
      symbol_map.emplace("lfloor",    8970,  "⌊",  ' ',   "Left floor");
      symbol_map.emplace("rfloor",    8971,  "⌋",  ' ',   "Right floor");
      symbol_map.emplace("loz",       9674,  "◊",  ' ',   "Lozenge");
      symbol_map.emplace("spades",    9824,  "♠",  ' ',   "Spade");
      symbol_map.emplace("clubs",     9827,  "♣",  ' ',   "Club");
      symbol_map.emplace("hearts",    9829,  "♥",  ' ',   "Heart");
      symbol_map.emplace("diams",     9830,  "♦",  ' ',   "Diamond");
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
