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

  /// A mapping of allowed symbol names to placeholder characters.
  static const auto & GetTextSymbolNames() {
    static std::map<String, char> symbol_map;
    if (symbol_map.size() == 0) {
      // AddReplaceTag("&iexcl;",   '!', "symbol:iexcl");      // Inverted '!'
      // AddReplaceTag("&cent;",    'c', "symbol:cent");       // 162 ¢ : Cent
      // AddReplaceTag("&pound;",   'L', "symbol:pound");      // 163 £ : Pound
      // AddReplaceTag("&curren;",  '$', "symbol:curren");     // 164 ¤ : Currency
      // AddReplaceTag("&yen;",     'Y', "symbol:yen");        // 165 ¥ : Yen
      // AddReplaceTag("&brvbar;",  '|', "symbol:brvbar");     // 166 ¦ : Broken vertical bar
      // AddReplaceTag("&sect;",    'S', "symbol:sect");       // 167 § : Section
      // AddReplaceTag("&uml;",     '~', "symbol:uml");        // 168 ¨ : Spacing diaeresis
      // AddReplaceTag("&copy;",    'c', "symbol:copy");       // 169 © : Copyright
      // AddReplaceTag("&ordf;",    'a', "symbol:ordf");       // 170 ª : Feminine ordinal indicator
      // AddReplaceTag("&laquo;",   '<', "symbol:laquo");      // 171 « : Opening/Left angle quotation mark
      // AddReplaceTag("&not;",     '~', "symbol:not");        // 172 ¬ : Negation
      // AddReplaceTag("&shy ",     '-', "symbol:shy");        // 173 ­- : Soft hyphen
      // AddReplaceTag("&reg;",     'R', "symbol:reg");        // 174 ® : Registered trademark
      // AddReplaceTag("&macr;",    '-', "symbol:macr");       // 175 ¯ : Spacing macron
      // AddReplaceTag("&deg;",     'o', "symbol:deg");        // 176 ° : Degree
      // AddReplaceTag("&plusmn;",  '+', "symbol:plusmn");     // 177 ± : Plus or minus
      // AddReplaceTag("&sup2;",    '2', "symbol:sup2");       // 178 ² : Superscript 2
      // AddReplaceTag("&sup3;",    '3', "symbol:sup3");       // 179 ³ : Superscript 3
      // AddReplaceTag("&acute;",   '\'', "symbol:acute");     // 180 ´ : Spacing acute
      // AddReplaceTag("&micro;",   'u', "symbol:micro");      // 181 µ : Micro
      // AddReplaceTag("&para;",    'P', "symbol:para");       // 182 ¶ : Paragraph
      // AddReplaceTag("&dot;",     '.', "symbol:dot");        // 182 · : Dot
      // AddReplaceTag("&cedil;",   ',', "symbol:cedil");      // 184 ¸ : Spacing cedilla
      // AddReplaceTag("&sup1;",    '1', "symbol:sup1");       // 185 ¹ : Superscript 1
      // AddReplaceTag("&ordm;",    'o', "symbol:ordm");       // 186 º : Masculine ordinal indicator
      // AddReplaceTag("&raquo;",   '>', "symbol:raquo");      // 187 » : Closing/Right angle quotation mark
      // AddReplaceTag("&frac14;",  '/', "symbol:frac14");     // 188 ¼ : Fraction 1/4
      // AddReplaceTag("&frac12;",  '/', "symbol:frac12");     // 189 ½ : Fraction 1/2
      // AddReplaceTag("&frac34;",  '/', "symbol:frac34");     // 190 ¾ : Fraction 3/4
      // AddReplaceTag("&iquest;",  '?', "symbol:iquest");     // 191 ¿ : Inverted question mark
      // AddReplaceTag("&Agrave;",  'A', "symbol:Agrave");     // 192 À : Capital a with grave accent
      // AddReplaceTag("&Aacute;",  'A', "symbol:Aacute");     // 193 Á : Capital a with acute accent
      // AddReplaceTag("&Acirc;",   'A', "symbol:Acirc");      // 194 Â : Capital a with circumflex accent
      // AddReplaceTag("&Atilde;",  'A', "symbol:Atilde");     // 195 Ã : Capital a with tilde
      // AddReplaceTag("&Auml;",    'A', "symbol:Auml");       // 196 Ä : Capital a with umlaut
      // AddReplaceTag("&Aring;",   'A', "symbol:Aring");      // 197 Å : Capital a with ring
      // AddReplaceTag("&AElig;",   'A', "symbol:AElig");      // 198 Æ : Capital ae
      // AddReplaceTag("&Ccedil;",  'C', "symbol:Ccedil");     // 199 Ç : Capital c with cedilla
      // AddReplaceTag("&Egrave;",  'E', "symbol:Egrave");     // 200 È : Capital e with grave accent
      // AddReplaceTag("&Eacute;",  'E', "symbol:Eacute");     // 201 É : Capital e with acute accent
      // AddReplaceTag("&Ecirc;",   'E', "symbol:Ecirc");      // 202 Ê : Capital e with circumflex accent
      // AddReplaceTag("&Euml;",    'E', "symbol:Euml");       // 203 Ë : Capital e with umlaut
      // AddReplaceTag("&Igrave;",  'I', "symbol:Igrave");     // 204 Ì : Capital i with grave accent
      // AddReplaceTag("&Iacute;",  'I', "symbol:Iacute");     // 205 Í : Capital i with accute accent
      // AddReplaceTag("&Icirc;",   'I', "symbol:Icirc");      // 206 Î : Capital i with circumflex accent
      // AddReplaceTag("&Iuml;",    'I', "symbol:Iuml");       // 207 Ï : Capital i with umlaut
      // AddReplaceTag("&ETH;",     'D', "symbol:ETH");        // 208 Ð : Capital eth (Icelandic)
      // AddReplaceTag("&Ntilde;",  'N', "symbol:Ntilde");     // 209 Ñ : Capital n with tilde
      // AddReplaceTag("&Ograve;",  'O', "symbol:Ograve");     // 210 Ò : Capital o with grave accent
      // AddReplaceTag("&Oacute;",  'O', "symbol:Oacute");     // 211 Ó : Capital o with accute accent
      // AddReplaceTag("&Ocirc;",   'O', "symbol:Ocirc");      // 212 Ô : Capital o with circumflex accent
      // AddReplaceTag("&Otilde;",  'O', "symbol:Otilde");     // 213 Õ : Capital o with tilde
      // AddReplaceTag("&Ouml;",    'O', "symbol:Ouml");       // 214 Ö : Capital o with umlaut
      // AddReplaceTag("&times;",   'x', "symbol:times");      // 215 × : Multiplication
      // AddReplaceTag("&Oslash;",  'O', "symbol:Oslash");     // 216 Ø : Capital o with slash
      // AddReplaceTag("&Ugrave;",  'U', "symbol:Ugrave");     // 217 Ù : Capital u with grave accent
      // AddReplaceTag("&Uacute;",  'U', "symbol:Uacute");     // 218 Ú : Capital u with acute accent
      // AddReplaceTag("&Ucirc;",   'U', "symbol:Ucirc");      // 219 Û : Capital u with circumflex accent
      // AddReplaceTag("&Uuml;",    'U', "symbol:Uuml");       // 220 Ü : Capital u with umlaut
      // AddReplaceTag("&Yacute;",  'Y', "symbol:Yacute");     // 221 Ý : Capital y with acute accent
      // AddReplaceTag("&THORN;",   'P', "symbol:THORN");      // 222 Þ : Capital thorn (Icelandic)
      // AddReplaceTag("&szlig;",   'B', "symbol:szlig");      // 223 ß : Lowercase sharp s (German)
      // AddReplaceTag("&agrave;",  'a', "symbol:agrave");     // 224 à : Lowercase a with grave accent
      // AddReplaceTag("&aacute;",  'a', "symbol:aacute");     // 225 á : Lowercase a with acute accent
      // AddReplaceTag("&acirc;",   'a', "symbol:acirc");      // 226 â : Lowercase a with circumflex accent
      // AddReplaceTag("&atilde;",  'a', "symbol:atilde");     // 227 ã : Lowercase a with tilde
      // AddReplaceTag("&auml;",    'a', "symbol:auml");       // 228 ä : Lowercase a with umlaut
      // AddReplaceTag("&aring;",   'a', "symbol:aring");      // 229 å : Lowercase a with ring
      // AddReplaceTag("&aelig;",   'a', "symbol:aelig");      // 230 æ : Lowercase ae
      // AddReplaceTag("&ccedil;",  'c', "symbol:ccedil");     // 231 ç : Lowercase c with cedilla
      // AddReplaceTag("&egrave;",  'e', "symbol:egrave");     // 232 è : Lowercase e with grave accent
      // AddReplaceTag("&eacute;",  'e', "symbol:eacute");     // 233 é : Lowercase e with acute accent
      // AddReplaceTag("&ecirc;",   'e', "symbol:ecirc");      // 234 ê : Lowercase e with circumflex accent
      // AddReplaceTag("&euml;",    'e', "symbol:euml");       // 235 ë : Lowercase e with umlaut
      // AddReplaceTag("&igrave;",  'i', "symbol:igrave");     // 236 ì : Lowercase i with grave accent
      // AddReplaceTag("&iacute;",  'i', "symbol:iacute");     // 237 í : Lowercase i with acute accent
      // AddReplaceTag("&icirc;",   'i', "symbol:icirc");      // 238 î : Lowercase i with circumflex accent
      // AddReplaceTag("&iuml;",    'i', "symbol:iuml");       // 239 ï : Lowercase i with umlaut
      // AddReplaceTag("&eth;",     'o', "symbol:eth");        // 240 ð : Lowercase eth (Icelandic)
      // AddReplaceTag("&ntilde;",  'n', "symbol:ntilde");     // 241 ñ : Lowercase n with tilde
      // AddReplaceTag("&ograve;",  'o', "symbol:ograve");     // 242 ò : Lowercase o with grave accent
      // AddReplaceTag("&oacute;",  'o', "symbol:oacute");     // 243 ó : Lowercase o with acute accent
      // AddReplaceTag("&ocirc;",   'o', "symbol:ocirc");      // 244 ô : Lowercase o with circumflex accent
      // AddReplaceTag("&otilde;",  'o', "symbol:otilde");     // 245 õ : Lowercase o with tilde
      // AddReplaceTag("&ouml;",    'o', "symbol:ouml");       // 246 ö : Lowercase o with umlaut
      // AddReplaceTag("&divide;",  '/', "symbol:divide");     // 247 ÷ : Divide
      // AddReplaceTag("&oslash;",  'o', "symbol:oslash");     // 248 ø : Lowercase o with slash
      // AddReplaceTag("&ugrave;",  'u', "symbol:ugrave");     // 249 ù : Lowercase u with grave accent
      // AddReplaceTag("&uacute;",  'u', "symbol:uacute");     // 250 ú : Lowercase u with acute accent
      // AddReplaceTag("&ucirc;",   'u', "symbol:ucirc");      // 251 û : Lowercase u with circumflex accent
      // AddReplaceTag("&uuml;",    'u', "symbol:uuml");       // 252 ü : Lowercase u with umlaut
      // AddReplaceTag("&yacute;",  'y', "symbol:yacute");     // 253 ý : Lowercase y with acute accent
      // AddReplaceTag("&thorn;",   'p', "symbol:thorn");      // 254 þ : Lowercase thorn (Icelandic)
      // AddReplaceTag("&yuml;",    'y', "symbol:yuml");       // 255 ÿ : Lowercase y with umlaut
      // AddReplaceTag("&Amacr;",   'A', "symbol:Amacr");      // 256 Ā : Latin capital letter a with macron
      // AddReplaceTag("&amacr;",   'a', "symbol:amacr");      // 257 ā : Latin small letter a with macron
      // AddReplaceTag("&Abreve;",  'A', "symbol:Abreve");     // 258 Ă : Latin capital letter a with breve
      // AddReplaceTag("&abreve;",  'a', "symbol:abreve");     // 259 ă : Latin small letter a with breve
      // AddReplaceTag("&Aogon;",   'A', "symbol:Aogon");      // 260 Ą : Latin capital letter a with ogonek
      // AddReplaceTag("&aogon;",   'a', "symbol:aogon");      // 261 ą : Latin small letter a with ogonek
      // AddReplaceTag("&Cacute;",  'C', "symbol:Cacute");     // 262 Ć : Latin capital letter c with acute
      // AddReplaceTag("&cacute;",  'c', "symbol:cacute");     // 263 ć : Latin small letter c with acute
      // AddReplaceTag("&Ccirc;",   'C', "symbol:Ccirc");      // 264 Ĉ : Latin capital letter c with circumflex
      // AddReplaceTag("&ccirc;",   'c', "symbol:ccirc");      // 265 ĉ : Latin small letter c with circumflex
      // AddReplaceTag("&Cdot;",    'C', "symbol:Cdot");       // 266 Ċ : Latin capital letter c with dot above
      // AddReplaceTag("&cdot;",    'c', "symbol:cdot");       // 267 ċ : Latin small letter c with dot above
      // AddReplaceTag("&Ccaron;",  'C', "symbol:Ccaron");     // 268 Č : Latin capital letter c with caron
      // AddReplaceTag("&ccaron;",  'c', "symbol:ccaron");     // 269 č : Latin small letter c with caron
      // AddReplaceTag("&Dcaron;",  'D', "symbol:Dcaron");     // 270 Ď : Latin capital letter d with caron
      // AddReplaceTag("&dcaron;",  'd', "symbol:dcaron");     // 271 ď : Latin small letter d with caron
      // AddReplaceTag("&Dstrok;",  'D', "symbol:Dstrok");     // 272 Đ : Latin capital letter d with stroke
      // AddReplaceTag("&dstrok;",  'd', "symbol:dstrok");     // 273 đ : Latin small letter d with stroke
      // AddReplaceTag("&Emacr;",   'E', "symbol:Emacr");      // 274 Ē : Latin capital letter e with macron
      // AddReplaceTag("&emacr;",   'e', "symbol:emacr");      // 275 ē : Latin small letter e with macron
      // AddReplaceTag("&Ebreve;",  'E', "symbol:Ebreve");     // 276 Ĕ : Latin capital letter e with breve
      // AddReplaceTag("&ebreve;",  'e', "symbol:ebreve");     // 277 ĕ : Latin small letter e with breve
      // AddReplaceTag("&Edot;",    'E', "symbol:Edot");       // 278 Ė : Latin capital letter e with dot above
      // AddReplaceTag("&edot;",    'e', "symbol:edot");       // 279 ė : Latin small letter e with dot above
      // AddReplaceTag("&Eogon;",   'E', "symbol:Eogon");      // 280 Ę : Latin capital letter e with ogonek
      // AddReplaceTag("&eogon;",   'e', "symbol:eogon");      // 281 ę : Latin small letter e with ogonek
      // AddReplaceTag("&Ecaron;",  'E', "symbol:Ecaron");     // 282 Ě : Latin capital letter e with caron
      // AddReplaceTag("&ecaron;",  'e', "symbol:ecaron");     // 283 ě : Latin small letter e with caron
      // AddReplaceTag("&Gcirc;",   'G', "symbol:Gcirc");      // 284 Ĝ : Latin capital letter g with circumflex
      // AddReplaceTag("&gcirc;",   'g', "symbol:gcirc");      // 285 ĝ : Latin small letter g with circumflex
      // AddReplaceTag("&Gbreve;",  'G', "symbol:Gbreve");     // 286 Ğ : Latin capital letter g with breve
      // AddReplaceTag("&gbreve;",  'g', "symbol:gbreve");     // 287 ğ : Latin small letter g with breve
      // AddReplaceTag("&Gdot;",    'G', "symbol:Gdot");       // 288 Ġ : Latin capital letter g with dot above
      // AddReplaceTag("&gdot;",    'g', "symbol:gdot");       // 289 ġ : Latin small letter g with dot above
      // AddReplaceTag("&Gcedil;",  'G', "symbol:Gcedil");     // 290 Ģ : Latin capital letter g with cedilla
      // AddReplaceTag("&gcedil;",  'g', "symbol:gcedil");     // 291 ģ : Latin small letter g with cedilla
      // AddReplaceTag("&Hcirc;",   'H', "symbol:Hcirc");      // 292 Ĥ : Latin capital letter h with circumflex
      // AddReplaceTag("&hcirc;",   'h', "symbol:hcirc");      // 293 ĥ : Latin small letter h with circumflex
      // AddReplaceTag("&Hstrok;",  'H', "symbol:Hstrok");     // 294 Ħ : Latin capital letter h with stroke
      // AddReplaceTag("&hstrok;",  'h', "symbol:hstrok");     // 295 ħ : Latin small letter h with stroke
      // AddReplaceTag("&Itilde;",  'I', "symbol:Itilde");     // 296 Ĩ : Latin capital letter I with tilde
      // AddReplaceTag("&itilde;",  'i', "symbol:itilde");     // 297 ĩ : Latin small letter I with tilde
      // AddReplaceTag("&Imacr;",   'I', "symbol:Imacr");      // 298 Ī : Latin capital letter I with macron
      // AddReplaceTag("&imacr;",   'i', "symbol:imacr");      // 299 ī : Latin small letter I with macron
      // AddReplaceTag("&Ibreve;",  'I', "symbol:Ibreve");     // 300 Ĭ : Latin capital letter I with breve
      // AddReplaceTag("&ibreve;",  'i', "symbol:ibreve");     // 301 ĭ : Latin small letter I with breve
      // AddReplaceTag("&Iogon;",   'I', "symbol:Iogon");      // 302 Į : Latin capital letter I with ogonek
      // AddReplaceTag("&iogon;",   'i', "symbol:iogon");      // 303 į : Latin small letter I with ogonek
      // AddReplaceTag("&Idot;",    'I', "symbol:Idot");       // 304 İ : Latin capital letter I with dot above
      // AddReplaceTag("&imath;",   'i', "symbol:imath");      // 305 ı : #305; Latin small letter dotless I
      // AddReplaceTag("&IJlig;",   'I', "symbol:IJlig");      // 306 Ĳ : Latin capital ligature ij
      // AddReplaceTag("&ijlig;",   'i', "symbol:ijlig");      // 307 ĳ : Latin small ligature ij
      // AddReplaceTag("&Jcirc;",   'J', "symbol:Jcirc");      // 308 Ĵ : Latin capital letter j with circumflex
      // AddReplaceTag("&jcirc;",   'j', "symbol:jcirc");      // 309 ĵ : Latin small letter j with circumflex
      // AddReplaceTag("&Kcedil;",  'K', "symbol:Kcedil");     // 310 Ķ : Latin capital letter k with cedilla
      // AddReplaceTag("&kcedil;",  'k', "symbol:kcedil");     // 311 ķ : Latin small letter k with cedilla
      // AddReplaceTag("&kgreen;",  'k', "symbol:kgreen");     // 312 ĸ : Latin small letter kra
      // AddReplaceTag("&Lacute;",  'L', "symbol:Lacute");     // 313 Ĺ : Latin capital letter l with acute
      // AddReplaceTag("&lacute;",  'l', "symbol:lacute");     // 314 ĺ : Latin small letter l with acute
      // AddReplaceTag("&Lcedil;",  'L', "symbol:Lcedil");     // 315 Ļ : Latin capital letter l with cedilla
      // AddReplaceTag("&lcedil;",  'l', "symbol:lcedil");     // 316 ļ : Latin small letter l with cedilla
      // AddReplaceTag("&Lcaron;",  'L', "symbol:Lcaron");     // 317 Ľ : Latin capital letter l with caron
      // AddReplaceTag("&lcaron;",  'l', "symbol:lcaron");     // 318 ľ : Latin small letter l with caron
      // AddReplaceTag("&Lmidot;",  'L', "symbol:Lmidot");     // 319 Ŀ : Latin capital letter l with middle dot
      // AddReplaceTag("&lmidot;",  'l', "symbol:lmidot");     // 320 ŀ : Latin small letter l with middle dot
      // AddReplaceTag("&Lstrok;",  'L', "symbol:Lstrok");     // 321 Ł : Latin capital letter l with stroke
      // AddReplaceTag("&lstrok;",  'l', "symbol:lstrok");     // 322 ł : Latin small letter l with stroke
      // AddReplaceTag("&Nacute;",  'N', "symbol:Nacute");     // 323 Ń : Latin capital letter n with acute
      // AddReplaceTag("&nacute;",  'n', "symbol:nacute");     // 324 ń : Latin small letter n with acute
      // AddReplaceTag("&Ncedil;",  'N', "symbol:Ncedil");     // 325 Ņ : Latin capital letter n with cedilla
      // AddReplaceTag("&ncedil;",  'n', "symbol:ncedil");     // 326 ņ : Latin small letter n with cedilla
      // AddReplaceTag("&Ncaron;",  'N', "symbol:Ncaron");     // 327 Ň : Latin capital letter n with caron
      // AddReplaceTag("&ncaron;",  'n', "symbol:ncaron");     // 328 ň : Latin small letter n with caron
      // AddReplaceTag("&napos;",   'n', "symbol:napos");      // 329 ŉ : Latin small letter n preceded by apostrophe
      // AddReplaceTag("&ENG;",     'N', "symbol:ENG");        // 330 Ŋ : Latin capital letter eng
      // AddReplaceTag("&eng;",     'n', "symbol:eng");        // 331 ŋ : Latin small letter eng
      // AddReplaceTag("&Omacr;",   'O', "symbol:Omacr");      // 332 Ō : Latin capital letter o with macron
      // AddReplaceTag("&omacr;",   'o', "symbol:omacr");      // 333 ō : Latin small letter o with macron
      // AddReplaceTag("&Obreve;",  'O', "symbol:Obreve");     // 334 Ŏ : Latin capital letter o with breve
      // AddReplaceTag("&obreve;",  'o', "symbol:obreve");     // 335 ŏ : Latin small letter o with breve
      // AddReplaceTag("&Odblac;",  'O', "symbol:Odblac");     // 336 Ő : Latin capital letter o with double acute
      // AddReplaceTag("&odblac;",  'o', "symbol:odblac");     // 337 ő : Latin small letter o with double acute
      // AddReplaceTag("&OElig;",   'O', "symbol:OElig");      // 338 Œ : Uppercase ligature OE
      // AddReplaceTag("&oelig;",   'o', "symbol:oelig");      // 339 œ : Lowercase ligature OE
      // AddReplaceTag("&Racute;",  'R', "symbol:Racute");     // 340 Ŕ : Latin capital letter r with acute
      // AddReplaceTag("&racute;",  'r', "symbol:racute");     // 341 ŕ : Latin small letter r with acute
      // AddReplaceTag("&Rcedil;",  'R', "symbol:Rcedil");     // 342 Ŗ : Latin capital letter r with cedilla
      // AddReplaceTag("&rcedil;",  'r', "symbol:rcedil");     // 343 ŗ : Latin small letter r with cedilla
      // AddReplaceTag("&Rcaron;",  'R', "symbol:Rcaron");     // 344 Ř : Latin capital letter r with caron
      // AddReplaceTag("&rcaron;",  'r', "symbol:rcaron");     // 345 ř : Latin small letter r with caron
      // AddReplaceTag("&Sacute;",  'S', "symbol:Sacute");     // 346 Ś : Latin capital letter s with acute
      // AddReplaceTag("&sacute;",  's', "symbol:sacute");     // 347 ś : Latin small letter s with acute
      // AddReplaceTag("&Scirc;",   'S', "symbol:Scirc");      // 348 Ŝ : Latin capital letter s with circumflex
      // AddReplaceTag("&scirc;",   's', "symbol:scirc");      // 349 ŝ : Latin small letter s with circumflex
      // AddReplaceTag("&Scedil;",  'S', "symbol:Scedil");     // 350 Ş : Latin capital letter s with cedilla
      // AddReplaceTag("&scedil;",  's', "symbol:scedil");     // 351 ş : Latin small letter s with cedilla
      // AddReplaceTag("&Scaron;",  'S', "symbol:Scaron");     // 352 Š : Uppercase S with caron
      // AddReplaceTag("&scaron;",  's', "symbol:scaron");     // 353 š : Lowercase S with caron
      // AddReplaceTag("&Tcedil;",  'T', "symbol:Tcedil");     // 354 Ţ : Latin capital letter t with cedilla
      // AddReplaceTag("&tcedil;",  't', "symbol:tcedil");     // 355 ţ : Latin small letter t with cedilla
      // AddReplaceTag("&Tcaron;",  'T', "symbol:Tcaron");     // 356 Ť : Latin capital letter t with caron
      // AddReplaceTag("&tcaron;",  't', "symbol:tcaron");     // 357 ť : Latin small letter t with caron
      // AddReplaceTag("&Tstrok;",  'T', "symbol:Tstrok");     // 358 Ŧ : Latin capital letter t with stroke
      // AddReplaceTag("&tstrok;",  't', "symbol:tstrok");     // 359 ŧ : Latin small letter t with stroke
      // AddReplaceTag("&Utilde;",  'U', "symbol:Utilde");     // 360 Ũ : Latin capital letter u with tilde
      // AddReplaceTag("&utilde;",  'u', "symbol:utilde");     // 361 ũ : Latin small letter u with tilde
      // AddReplaceTag("&Umacr;",   'U', "symbol:Umacr");      // 362 Ū : Latin capital letter u with macron
      // AddReplaceTag("&umacr;",   'u', "symbol:umacr");      // 363 ū : Latin small letter u with macron
      // AddReplaceTag("&Ubreve;",  'U', "symbol:Ubreve");     // 364 Ŭ : Latin capital letter u with breve
      // AddReplaceTag("&ubreve;",  'u', "symbol:ubreve");     // 365 ŭ : Latin small letter u with breve
      // AddReplaceTag("&Uring;",   'U', "symbol:Uring");      // 366 Ů : Latin capital letter u with ring above
      // AddReplaceTag("&uring;",   'u', "symbol:uring");      // 367 ů : Latin small letter u with ring above
      // AddReplaceTag("&Udblac;",  'U', "symbol:Udblac");     // 368 Ű : Latin capital letter u with double acute
      // AddReplaceTag("&udblac;",  'u', "symbol:udblac");     // 369 ű : Latin small letter u with double acute
      // AddReplaceTag("&Uogon;",   'U', "symbol:Uogon");      // 370 Ų : Latin capital letter u with ogonek
      // AddReplaceTag("&uogon;",   'u', "symbol:uogon");      // 371 ų : Latin small letter u with ogonek
      // AddReplaceTag("&Wcirc;",   'W', "symbol:Wcirc");      // 372 Ŵ : Latin capital letter w with circumflex
      // AddReplaceTag("&wcirc;",   'w', "symbol:wcirc");      // 373 ŵ : Latin small letter w with circumflex
      // AddReplaceTag("&Ycirc;",   'Y', "symbol:Ycirc");      // 374 Ŷ : Latin capital letter y with circumflex
      // AddReplaceTag("&ycirc;",   'y', "symbol:ycirc");      // 375 ŷ : Latin small letter y with circumflex
      // AddReplaceTag("&Yuml;",    'Y', "symbol:Yuml");       // 376 Ÿ : Capital Y with diaeres
      // AddReplaceTag("&fnof;",    'f', "symbol:fnof");       // 402 ƒ : Lowercase with hook
      // AddReplaceTag("&circ;",    '^', "symbol:circ");       // 710 ˆ : Circumflex accent
      // AddReplaceTag("&tilde;",   '~', "symbol:tilde");      // 732 ˜ : Tilde
      // AddReplaceTag("&Alpha;",   'A', "symbol:Alpha");      // 913 Α : Alpha
      // AddReplaceTag("&Beta;",    'B', "symbol:Beta");       // 914 Β : Beta
      // AddReplaceTag("&Gamma;",   'G', "symbol:Gamma");      // 915 Γ : Gamma
      // AddReplaceTag("&Delta;",   'D', "symbol:Delta");      // 916 Δ : Delta
      // AddReplaceTag("&Epsilon;", 'E', "symbol:Epsilon");    // 917 Ε : Epsilon
      // AddReplaceTag("&Zeta;",    'Z', "symbol:Zeta");       // 918 Ζ : Zeta
      // AddReplaceTag("&Eta;",     'H', "symbol:Eta");        // 919 Η : Eta
      // AddReplaceTag("&Theta;",   'T', "symbol:Theta");      // 920 Θ : Theta
      // AddReplaceTag("&Iota;",    'I', "symbol:Iota");       // 921 Ι : Iota
      // AddReplaceTag("&Kappa;",   'K', "symbol:Kappa");      // 922 Κ : Kappa
      // AddReplaceTag("&Lambda;",  'L', "symbol:Lambda");     // 923 Λ : Lambda
      // AddReplaceTag("&Mu;",      '<', "symbol:Mu");         // 924 Μ : Mu
      // AddReplaceTag("&Nu;",      'N', "symbol:Nu");         // 925 Ν : Nu
      // AddReplaceTag("&Xi;",      'X', "symbol:Xi");         // 926 Ξ : Xi
      // AddReplaceTag("&Omicron;", 'O', "symbol:Omicron");    // 927 Ο : Omicron
      // AddReplaceTag("&Pi;",      'P', "symbol:Pi");         // 928 Π : Pi
      // AddReplaceTag("&Rho;",     'R', "symbol:Rho");        // 929 Ρ : Rho
      // AddReplaceTag("&Sigma;",   'S', "symbol:Sigma");      // 931 Σ : Sigma
      // AddReplaceTag("&Tau;",     'T', "symbol:Tau");        // 932 Τ : Tau
      // AddReplaceTag("&Upsilon;", 'Y', "symbol:Upsilon");    // 933 Υ : Upsilon
      // AddReplaceTag("&Phi;",     'P', "symbol:Phi");        // 934 Φ : Phi
      // AddReplaceTag("&Chi;",     'X', "symbol:Chi");        // 935 Χ : Chi
      // AddReplaceTag("&Psi;",     'W', "symbol:Psi");        // 936 Ψ : Psi
      // AddReplaceTag("&Omega;",   'O', "symbol:Omega");      // 937 Ω : Omega
      // AddReplaceTag("&alpha;",   'a', "symbol:alpha");      // 945 α : alpha
      // AddReplaceTag("&beta;",    'b', "symbol:beta");       // 946 β : beta
      // AddReplaceTag("&gamma;",   'g', "symbol:gamma");      // 947 γ : gamma
      // AddReplaceTag("&delta;",   'd', "symbol:delta");      // 948 δ : delta
      // AddReplaceTag("&epsilon;", 'e', "symbol:epsilon");    // 949 ε : epsilon
      // AddReplaceTag("&zeta;",    'z', "symbol:zeta");       // 950 ζ : zeta
      // AddReplaceTag("&eta;",     'n', "symbol:eta");        // 951 η : eta
      // AddReplaceTag("&theta;",   't', "symbol:theta");      // 952 θ : theta
      // AddReplaceTag("&iota;",    'i', "symbol:iota");       // 953 ι : iota
      // AddReplaceTag("&kappa;",   'k', "symbol:kappa");      // 954 κ : kappa
      // AddReplaceTag("&lambda;",  'l', "symbol:lambda");     // 955 λ : lambda
      // AddReplaceTag("&mu;",      'u', "symbol:mu");         // 956 μ : mu
      // AddReplaceTag("&nu;",      'v', "symbol:nu");         // 957 ν : nu
      // AddReplaceTag("&xi;",      'x', "symbol:xi");         // 958 ξ : xi
      // AddReplaceTag("&omicron;", 'o', "symbol:omicron");    // 959 ο : omicron
      // AddReplaceTag("&pi;",      'p', "symbol:pi");         // 960 π : pi
      // AddReplaceTag("&rho;",     'p', "symbol:rho");        // 961 ρ : rho
      // AddReplaceTag("&sigmaf;",  'c', "symbol:sigmaf");     // 962 ς : sigmaf
      // AddReplaceTag("&sigma;",   's', "symbol:sigma");      // 963 σ : sigma
      // AddReplaceTag("&tau;",     't', "symbol:tau");        // 964 τ : tau
      // AddReplaceTag("&upsilon;", 'u', "symbol:upsilon");    // 965 υ : upsilon
      // AddReplaceTag("&phi;",     'p', "symbol:phi");        // 966 φ : phi
      // AddReplaceTag("&chi;",     'x', "symbol:chi");        // 967 χ : chi
      // AddReplaceTag("&psi;",     's', "symbol:psi");        // 968 ψ : psi
      // AddReplaceTag("&omega;",   'w', "symbol:omega");      // 969 ω : omega
      // AddReplaceTag("&thetasym;", 'T', "symbol:thetasym");  // 977 ϑ : Theta symbol
      // AddReplaceTag("&upsih;",   'U', "symbol:upsih");      // 978 ϒ : Upsilon symbol
      // AddReplaceTag("&piv;",     'O', "symbol:piv");        // 982 ϖ : Pi symbol
      // AddReplaceTag("ensp",      ' ', "symbol:ensp");       // 8194 : En space
      // AddReplaceTag("emsp",      ' ', "symbol:emsp");       // 8195 : Em space
      // AddReplaceTag("&thinsp;",  ' ', "symbol:thinsp");     // 8201 : Thin space
      // AddReplaceTag("&zwnj;",    ' ', "symbol:zwnj");       // 8204 : Zero width non-joiner
      // AddReplaceTag("&zwj;",     ' ', "symbol:zwj");        // 8205 : Zero width joiner
      // AddReplaceTag("&lrm;",     '?', "symbol:lrm");        // 8206 : Left-to-right mark
      // AddReplaceTag("&rlm;",     '?', "symbol:rlm");        // 8207 : Right-to-left mark
      // AddReplaceTag("&ndash;",   '-', "symbol:ndash");      // 8211 – : En dash
      // AddReplaceTag("&mdash;",   '-', "symbol:mdash");      // 8212 — : Em dash
      // AddReplaceTag("&lsquo;",   ' ', "symbol:lsquo");      // 8216 ‘ : Left single quotation mark
      // AddReplaceTag("&rsquo;",   ' ', "symbol:rsquo");      // 8217 ’ : Right single quotation mark
      // AddReplaceTag("&sbquo;",   ' ', "symbol:sbquo");      // 8218 ‚ : Single low-9 quotation mark
      // AddReplaceTag("&ldquo;",   ' ', "symbol:ldquo");      // 8220 “ : Left double quotation mark
      // AddReplaceTag("&rdquo;",   ' ', "symbol:rdquo");      // 8221 ” : Right double quotation mark
      // AddReplaceTag("&bdquo;",   ' ', "symbol:bdquo");      // 8222 „ : Double low-9 quotation mark
      // AddReplaceTag("&dagger;",  ' ', "symbol:dagger");     // 8224 † : Dagger
      // AddReplaceTag("&Dagger;",  ' ', "symbol:Dagger");     // 8225 ‡ : Double dagger
      // AddReplaceTag("&bull;",    ' ', "symbol:bull");       // 8226 • : Bullet
      // AddReplaceTag("&hellip;",  ' ', "symbol:hellip");     // 8230 … : Horizontal ellipsis
      // AddReplaceTag("&permil;",  ' ', "symbol:permil");     // 8240 ‰ : Per mille
      // AddReplaceTag("&prime;",   ' ', "symbol:prime");      // 8242 ′ : Minutes (Degrees)
      // AddReplaceTag("&Prime;",   ' ', "symbol:Prime");      // 8243 ″ : Seconds (Degrees)
      // AddReplaceTag("&lsaquo;",  ' ', "symbol:lsaquo");     // 8249 ‹ : Single left angle quotation
      // AddReplaceTag("&rsaquo;",  ' ', "symbol:rsaquo");     // 8250 › : Single right angle quotation
      // AddReplaceTag("&oline;",   ' ', "symbol:oline");      // 8254 ‾ : Overline
      // AddReplaceTag("&euro;",    ' ', "symbol:euro");       // 8364 € : Euro
      // AddReplaceTag("&trade;",   ' ', "symbol:trade");      // 8482 ™ : Trademark
      // AddReplaceTag("&larr;",    ' ', "symbol:larr");       // 8592 ← : Left arrow
      // AddReplaceTag("&uarr;",    ' ', "symbol:uarr");       // 8593 ↑ : Up arrow
      // AddReplaceTag("&rarr;",    ' ', "symbol:rarr");       // 8594 → : Right arrow
      // AddReplaceTag("&darr;",    ' ', "symbol:darr");       // 8595 ↓ : Down arrow
      // AddReplaceTag("&harr;",    ' ', "symbol:harr");       // 8596 ↔ : Left right arrow
      // AddReplaceTag("&crarr;",   ' ', "symbol:crarr");      // 8629 ↵ : Carriage return arrow
      // AddReplaceTag("&forall;",  ' ', "symbol:forall");     // 8704 ∀ : For all
      // AddReplaceTag("&part;",    ' ', "symbol:part");       // 8706 ∂ : Part
      // AddReplaceTag("&exist;",   ' ', "symbol:exist");      // 8707 ∃ : Exist
      // AddReplaceTag("&empty;",   ' ', "symbol:empty");      // 8709 ∅ : Empty
      // AddReplaceTag("&nabla;",   ' ', "symbol:nabla");      // 8711 ∇ : Nabla
      // AddReplaceTag("&isin;",    ' ', "symbol:isin");       // 8712 ∈ : Is in
      // AddReplaceTag("&notin;",   ' ', "symbol:notin");      // 8713 ∉ : Not in
      // AddReplaceTag("&ni;",      ' ', "symbol:ni");         // 8715 ∋ : Ni
      // AddReplaceTag("&prod;",    ' ', "symbol:prod");       // 8719 ∏ : Product
      // AddReplaceTag("&sum;",     ' ', "symbol:sum");        // 8721 ∑ : Sum
      // AddReplaceTag("&minus;",   ' ', "symbol:minus");      // 8722 − : Minus
      // AddReplaceTag("&lowast;",  ' ', "symbol:lowast");     // 8727 ∗ : Asterisk (Lowast)
      // AddReplaceTag("&radic;",   ' ', "symbol:radic");      // 8730 √ : Square root
      // AddReplaceTag("&prop;",    ' ', "symbol:prop");       // 8733 ∝ : Proportional to
      // AddReplaceTag("&infin;",   ' ', "symbol:infin");      // 8734 ∞ : Infinity
      // AddReplaceTag("&ang;",     ' ', "symbol:ang");        // 8736 ∠ : Angle
      // AddReplaceTag("&and;",     ' ', "symbol:and");        // 8743 ∧ : And
      // AddReplaceTag("&or;",      ' ', "symbol:or");         // 8744 ∨ : Or
      // AddReplaceTag("&cap;",     ' ', "symbol:cap");        // 8745 ∩ : Cap
      // AddReplaceTag("&cup;",     ' ', "symbol:cup");        // 8746 ∪ : Cup
      // AddReplaceTag("&int;",     ' ', "symbol:int");        // 8747 ∫ : Integral
      // AddReplaceTag("&there4;",  ' ', "symbol:there4");     // 8756 ∴ : Therefore
      // AddReplaceTag("&sim;",     ' ', "symbol:sim");        // 8764 ∼ : Similar to
      // AddReplaceTag("&cong;",    ' ', "symbol:cong");       // 8773 ≅ : Congurent to
      // AddReplaceTag("&asymp;",   ' ', "symbol:asymp");      // 8776 ≈ : Almost equal
      // AddReplaceTag("&ne;",      ' ', "symbol:ne");         // 8800 ≠ : Not equal
      // AddReplaceTag("&equiv;",   ' ', "symbol:equiv");      // 8801 ≡ : Equivalent
      // AddReplaceTag("&le;",      ' ', "symbol:le");         // 8804 ≤ : Less or equal
      // AddReplaceTag("&ge;",      ' ', "symbol:ge");         // 8805 ≥ : Greater or equal
      // AddReplaceTag("&sub;",     ' ', "symbol:sub");        // 8834 ⊂ : Subset of
      // AddReplaceTag("&sup;",     ' ', "symbol:sup");        // 8835 ⊃ : Superset of
      // AddReplaceTag("&nsub;",    ' ', "symbol:nsub");       // 8836 ⊄ : Not subset of
      // AddReplaceTag("&sube;",    ' ', "symbol:sube");       // 8838 ⊆ : Subset or equal
      // AddReplaceTag("&supe;",    ' ', "symbol:supe");       // 8839 ⊇ : Superset or equal
      // AddReplaceTag("&oplus;",   ' ', "symbol:oplus");      // 8853 ⊕ : Circled plus
      // AddReplaceTag("&otimes;",  ' ', "symbol:otimes");     // 8855 ⊗ : Circled times
      // AddReplaceTag("&perp;",    ' ', "symbol:perp");       // 8869 ⊥ : Perpendicular
      // AddReplaceTag("&sdot;",    ' ', "symbol:sdot");       // 8901 ⋅ : Dot operator
      // AddReplaceTag("&lceil;",   ' ', "symbol:lceil");      // 8968 ⌈ : Left ceiling
      // AddReplaceTag("&rceil;",   ' ', "symbol:rceil");      // 8969 ⌉ : Right ceiling
      // AddReplaceTag("&lfloor;",  ' ', "symbol:lfloor");     // 8970 ⌊ : Left floor
      // AddReplaceTag("&rfloor;",  ' ', "symbol:rfloor");     // 8971 ⌋ : Right floor
      // AddReplaceTag("&loz;",     ' ', "symbol:loz");        // 9674 ◊ : Lozenge
      // AddReplaceTag("&spades;",  ' ', "symbol:spades");     // 9824 ♠ : Spade
      // AddReplaceTag("&clubs;",   ' ', "symbol:clubs");      // 9827 ♣ : Club
      // AddReplaceTag("&hearts;",  ' ', "symbol:hearts");     // 9829 ♥ : Heart
      // AddReplaceTag("&diams;",   ' ', "symbol:diams");      // 9830 ♦ : Diamond
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
