/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2020
 *
 *  @file  Color.hpp
 *  @brief Represent RGBA color, parseable from CSS strings.
 *
 *  Adapted largely from https://github.com/mapbox/css-color-parser-cpp/.
 */


#ifndef EMP_WEB_COLOR_HPP
#define EMP_WEB_COLOR_HPP

#include <algorithm>
#include <cstdint>
#include <sstream>
#include <string>
#include "Canvas.hpp"


#include "../base/vector.hpp"
#include "../geometry/Circle2D.hpp"
#include "../tools/string_utils.hpp"
#include "color_logic_utils.hpp"
#include "color_map.hpp"

namespace emp {
namespace web {

  /// Represents RGBA color.
  class Color {
    private:
        unsigned char r{}, g{}, b{};
        float a{ 1.0f };
        std::string cached_css_str = "";

        #ifdef EMP_HAS_SFML
        sf::Color cached_sf_color;
        #endif
    public:
        Color() = default;

        Color(const unsigned char r_, const unsigned char g_, const unsigned char b_, const float a_)
        : r(r_), g(g_), b(b_), a(std::clamp(a_, 0.f, 1.f))
        {
            cached_css_str = emp::ColorRGB(r, g, b, a);
            #ifdef EMP_HAS_SFML
            cached_sf_color = sf::Color(r, g, b, a * 255);;
            #endif
        }

        // defined out of body
        Color( const std::string& css_str );

        /// == comparison operator
        /// \param rhs Right hand Color to compare to
        bool operator==( const Color& rhs ) const {
            return std::tuple{
                r, g, b, a
            } == std::tuple{
                rhs.r, rhs.g, rhs.b, rhs.a
            };
        }

        /// Convert to a css string
        const std::string& ToString() {
            return cached_css_str;
        }
        #ifdef EMP_HAS_SFML
        const sf::Color& ToSfColor() {
            return cached_sf_color;
        }
        #endif

        //implicit conversion
        //operator std::string() const { return cached_css_str; }

        //implicit conversion
        operator const std::string&() const { return cached_css_str; }

        bool operator!=( const Color& rhs ) const { return !operator==( rhs ); }

        #ifdef EMP_HAS_SFML
        operator const sf::Color&() const { return cached_sf_color; }
        #endif
    private:
        static Color ParseColor(const std::string& str);
        static Color ParseABC(const std::string& str);
        static Color ParseRGB(const std::string& str, const std::string& format_name);
        static Color ParseHSL(const std::string& str, const std::string& format_name);
  };

  namespace color_impl {

    struct NamedColor { const std::string name; const Color color; };
    const NamedColor namedColors[] = {
        { "aliceblue", { 240, 248, 255, 1 } },
        { "antiquewhite", { 250, 235, 215, 1 } },
        { "aqua", { 0, 255, 255, 1 } },
        { "aquamarine", { 127, 255, 212, 1 } },
        { "azure", { 240, 255, 255, 1 } },
        { "beige", { 245, 245, 220, 1 } },
        { "bisque", { 255, 228, 196, 1 } },
        { "black", { 0, 0, 0, 1 } },
        { "blanchedalmond", { 255, 235, 205, 1 } },
        { "blue", { 0, 0, 255, 1 } },
        { "blueviolet", { 138, 43, 226, 1 } },
        { "brown", { 165, 42, 42, 1 } },
        { "burlywood", { 222, 184, 135, 1 } },
        { "cadetblue", { 95, 158, 160, 1 } },
        { "chartreuse", { 127, 255, 0, 1 } },
        { "chocolate", { 210, 105, 30, 1 } },
        { "coral", { 255, 127, 80, 1 } },
        { "cornflowerblue", { 100, 149, 237, 1 } },
        { "cornsilk", { 255, 248, 220, 1 } },
        { "crimson", { 220, 20, 60, 1 } },
        { "cyan", { 0, 255, 255, 1 } },
        { "darkblue", { 0, 0, 139, 1 } },
        { "darkcyan", { 0, 139, 139, 1 } },
        { "darkgoldenrod", { 184, 134, 11, 1 } },
        { "darkgray", { 169, 169, 169, 1 } },
        { "darkgreen", { 0, 100, 0, 1 } },
        { "darkgrey", { 169, 169, 169, 1 } },
        { "darkkhaki", { 189, 183, 107, 1 } },
        { "darkmagenta", { 139, 0, 139, 1 } },
        { "darkolivegreen", { 85, 107, 47, 1 } },
        { "darkorange", { 255, 140, 0, 1 } },
        { "darkorchid", { 153, 50, 204, 1 } },
        { "darkred", { 139, 0, 0, 1 } },
        { "darksalmon", { 233, 150, 122, 1 } },
        { "darkseagreen", { 143, 188, 143, 1 } },
        { "darkslateblue", { 72, 61, 139, 1 } },
        { "darkslategray", { 47, 79, 79, 1 } },
        { "darkslategrey", { 47, 79, 79, 1 } },
        { "darkturquoise", { 0, 206, 209, 1 } },
        { "darkviolet", { 148, 0, 211, 1 } },
        { "deeppink", { 255, 20, 147, 1 } },
        { "deepskyblue", { 0, 191, 255, 1 } },
        { "dimgray", { 105, 105, 105, 1 } },
        { "dimgrey", { 105, 105, 105, 1 } },
        { "dodgerblue", { 30, 144, 255, 1 } },
        { "firebrick", { 178, 34, 34, 1 } },
        { "floralwhite", { 255, 250, 240, 1 } },
        { "forestgreen", { 34, 139, 34, 1 } },
        { "fuchsia", { 255, 0, 255, 1 } },
        { "gainsboro", { 220, 220, 220, 1 } },
        { "ghostwhite", { 248, 248, 255, 1 } },
        { "gold", { 255, 215, 0, 1 } },
        { "goldenrod", { 218, 165, 32, 1 } },
        { "gray", { 128, 128, 128, 1 } },
        { "green", { 0, 128, 0, 1 } },
        { "greenyellow", { 173, 255, 47, 1 } },
        { "grey", { 128, 128, 128, 1 } },
        { "honeydew", { 240, 255, 240, 1 } },
        { "hotpink", { 255, 105, 180, 1 } },
        { "indianred", { 205, 92, 92, 1 } },
        { "indigo", { 75, 0, 130, 1 } },
        { "ivory", { 255, 255, 240, 1 } },
        { "khaki", { 240, 230, 140, 1 } },
        { "lavender", { 230, 230, 250, 1 } },
        { "lavenderblush", { 255, 240, 245, 1 } },
        { "lawngreen", { 124, 252, 0, 1 } },
        { "lemonchiffon", { 255, 250, 205, 1 } },
        { "lightblue", { 173, 216, 230, 1 } },
        { "lightcoral", { 240, 128, 128, 1 } },
        { "lightcyan", { 224, 255, 255, 1 } },
        { "lightgoldenrodyellow", { 250, 250, 210, 1 } },
        { "lightgray", { 211, 211, 211, 1 } },
        { "lightgreen", { 144, 238, 144, 1 } },
        { "lightgrey", { 211, 211, 211, 1 } },
        { "lightpink", { 255, 182, 193, 1 } },
        { "lightsalmon", { 255, 160, 122, 1 } },
        { "lightseagreen", { 32, 178, 170, 1 } },
        { "lightskyblue", { 135, 206, 250, 1 } },
        { "lightslategray", { 119, 136, 153, 1 } },
        { "lightslategrey", { 119, 136, 153, 1 } },
        { "lightsteelblue", { 176, 196, 222, 1 } },
        { "lightyellow", { 255, 255, 224, 1 } },
        { "lime", { 0, 255, 0, 1 } },
        { "limegreen", { 50, 205, 50, 1 } },
        { "linen", { 250, 240, 230, 1 } },
        { "magenta", { 255, 0, 255, 1 } },
        { "maroon", { 128, 0, 0, 1 } },
        { "mediumaquamarine", { 102, 205, 170, 1 } },
        { "mediumblue", { 0, 0, 205, 1 } },
        { "mediumorchid", { 186, 85, 211, 1 } },
        { "mediumpurple", { 147, 112, 219, 1 } },
        { "mediumseagreen", { 60, 179, 113, 1 } },
        { "mediumslateblue", { 123, 104, 238, 1 } },
        { "mediumspringgreen", { 0, 250, 154, 1 } },
        { "mediumturquoise", { 72, 209, 204, 1 } },
        { "mediumvioletred", { 199, 21, 133, 1 } },
        { "midnightblue", { 25, 25, 112, 1 } },
        { "mintcream", { 245, 255, 250, 1 } },
        { "mistyrose", { 255, 228, 225, 1 } },
        { "moccasin", { 255, 228, 181, 1 } },
        { "navajowhite", { 255, 222, 173, 1 } },
        { "navy", { 0, 0, 128, 1 } },
        { "oldlace", { 253, 245, 230, 1 } },
        { "olive", { 128, 128, 0, 1 } },
        { "olivedrab", { 107, 142, 35, 1 } },
        { "orange", { 255, 165, 0, 1 } },
        { "orangered", { 255, 69, 0, 1 } },
        { "orchid", { 218, 112, 214, 1 } },
        { "palegoldenrod", { 238, 232, 170, 1 } },
        { "palegreen", { 152, 251, 152, 1 } },
        { "paleturquoise", { 175, 238, 238, 1 } },
        { "palevioletred", { 219, 112, 147, 1 } },
        { "papayawhip", { 255, 239, 213, 1 } },
        { "peachpuff", { 255, 218, 185, 1 } },
        { "peru", { 205, 133, 63, 1 } },
        { "pink", { 255, 192, 203, 1 } },
        { "plum", { 221, 160, 221, 1 } },
        { "powderblue", { 176, 224, 230, 1 } },
        { "purple", { 128, 0, 128, 1 } },
        { "red", { 255, 0, 0, 1 } },
        { "rosybrown", { 188, 143, 143, 1 } },
        { "royalblue", { 65, 105, 225, 1 } },
        { "saddlebrown", { 139, 69, 19, 1 } },
        { "salmon", { 250, 128, 114, 1 } },
        { "sandybrown", { 244, 164, 96, 1 } },
        { "seagreen", { 46, 139, 87, 1 } },
        { "seashell", { 255, 245, 238, 1 } },
        { "sienna", { 160, 82, 45, 1 } },
        { "silver", { 192, 192, 192, 1 } },
        { "skyblue", { 135, 206, 235, 1 } },
        { "slateblue", { 106, 90, 205, 1 } },
        { "slategray", { 112, 128, 144, 1 } },
        { "slategrey", { 112, 128, 144, 1 } },
        { "snow", { 255, 250, 250, 1 } },
        { "springgreen", { 0, 255, 127, 1 } },
        { "steelblue", { 70, 130, 180, 1 } },
        { "tan", { 210, 180, 140, 1 } },
        { "teal", { 0, 128, 128, 1 } },
        { "thistle", { 216, 191, 216, 1 } },
        { "tomato", { 255, 99, 71, 1 } },
        { "transparent", { 0, 0, 0, 0 } },
        { "turquoise", { 64, 224, 208, 1 } },
        { "violet", { 238, 130, 238, 1 } },
        { "wheat", { 245, 222, 179, 1 } },
        { "white", { 255, 255, 255, 1 } },
        { "whitesmoke", { 245, 245, 245, 1 } },
        { "yellow", { 255, 255, 0, 1 } },
        { "yellowgreen", { 154, 205, 50, 1 } }
    };

    /// Clamp a value to an integer 0-255
    /// @param i The value to clamp
    template <typename T>
    uint8_t clamp_css_byte(T i) {  // Clamp to integer 0 .. 255.
        i = ::round(i);  // Seems to be what Chrome does (vs truncation).
        return uint8_t(std::clamp(i,T(0),T(255)));
    }

    /// Clamp to float 0.0 .. 1.0.
    /// @param f The value to clamp
    template <typename T>
    float clamp_css_float(T f) {
        return std::clamp(static_cast<float>(f), 0.f, 1.f);
    }

    /// Convert a string to float
    /// @param str The string to convert
    float parse_float(const std::string& str) {
        return strtof(str.c_str(), nullptr);
    }

    /// Convert a string to integer in the given base
    /// @param str The string to convert
    /// @param base The base the string is in
    int64_t parse_int(const std::string& str, const uint8_t base = 10) {
        return strtoll(str.c_str(), nullptr, base);
    }

    /// Convert a string to an integer or percentage
    /// @param str The string to convert
    uint8_t parse_css_int(const std::string& str) {  // int or percentage.
        if (str.length() && str.back() == '%') {
            return clamp_css_byte(parse_float(str) / 100.0f * 255.0f);
        } else {
            return clamp_css_byte(parse_int(str));
        }
    }
    /// Convert a string to a float or percentage
    /// @param str The string to convert
    float parse_css_float(const std::string& str) {  // float or percentage.
        if (str.length() && str.back() == '%') {
            return clamp_css_float(parse_float(str) / 100.0f);
        } else {
            return clamp_css_float(parse_float(str));
        }
    }
    /// Convert floats from hsl to rgb value
    /// @param m1 Value calculated in ParseHSl
    /// @param m2 Value calculated in ParseHSl
    /// @param h Value calculated in ParseHSl
    float css_hue_to_rgb(const float m1, const float m2, float h) {
        if (h < 0.0f) {
            h += 1.0f;
        } else if (h > 1.0f) {
            h -= 1.0f;
        } else if (h * 6.0f < 1.0f) {
            return m1 + (m2 - m1) * h * 6.0f;
        } else if (h * 2.0f < 1.0f) {
            return m2;
        }
        else if (h * 3.0f < 2.0f) {
            return m1 + (m2 - m1) * (2.0f / 3.0f - h) * 6.0f;
        }
        return m1;
    }
    /// Split a string into a vector of strings
    /// @param s The string to split
    /// @param delim The delimiter
    emp::vector<std::string> split(const std::string& s, char delim) {
        emp::vector<std::string> elems;
        std::stringstream ss(s);
        std::string item;
        while (std::getline(ss, item, delim)) {
            elems.push_back(item);
        }
        return elems;
    }

  }

  /// Color constructor
  /// @param css_str The string to construct a color from
  Color::Color( const std::string& css_str ) {
    std::string str = css_str;

    // Remove all whitespace, not compliant, but should just be more accepting.
    emp::remove_whitespace(str);

    // Convert to lowercase.
    emp::to_lower(str);

    // Ensure sorted and find named color match with lower_bound
    emp_assert(std::is_sorted(
      std::begin(color_impl::namedColors), std::end(color_impl::namedColors),
      [](const auto& left, const auto& right){
          return left.name < right.name;
      }
    ));

    const auto namedColor = std::lower_bound(
      std::begin(color_impl::namedColors), std::end(color_impl::namedColors), str,
      [](const auto& left, const auto& right){
          return left.name < right;
      }
    );

    if (
      namedColor != std::end(color_impl::namedColors)
      && namedColor->name == str
    ) *this = namedColor->color;
    else *this = ParseColor(str);

  }

    /// Handle all color detection and parsing from the string
    /// @param str The color string to be parsed
    Color Color::ParseColor(const std::string& str) {
        // #abc and #abc123 syntax.
        if (DetectSyntaxABC(str)) {
            return ParseABC(str);
        }

        const size_t open_paren = str.find_first_of( '(' );
        if (open_paren != std::string::npos) {
            const std::string format_name = str.substr(0, open_paren);

            if (format_name == "rgba" || format_name == "rgb") {
                return ParseRGB(str, format_name);

            } else if (format_name == "hsla" || format_name == "hsl") {
                return ParseHSL(str, format_name);
            }
        }
        emp_assert( false, str);
        __builtin_unreachable();
  }

    /// Parse the ABC format color string
    /// @param str The color string ex) "#fff"
    Color Color::ParseABC(const std::string& str){
      if (str.length() == 4) {
            const int64_t iv = color_impl::parse_int(str.substr(1), 16);  // TODO(deanm): Stricter parsing.
            if (!(iv >= 0 && iv <= 0xfff)) {
                emp_assert( false, str );
                __builtin_unreachable();
            } else {
                return Color(
                    static_cast<uint8_t>(((iv & 0xf00) >> 4) | ((iv & 0xf00) >> 8)),
                    static_cast<uint8_t>((iv & 0xf0) | ((iv & 0xf0) >> 4)),
                    static_cast<uint8_t>((iv & 0xf) | ((iv & 0xf) << 4)),
                    1
                );
            }
        } else if (str.length() == 7) {
            const int64_t iv = color_impl::parse_int(str.substr(1), 16);  // TODO(deanm): Stricter parsing.
            if (!(iv >= 0 && iv <= 0xffffff)) {
              emp_assert( false, str );
              __builtin_unreachable();
            } else {
                return Color(
                    static_cast<uint8_t>((iv & 0xff0000) >> 16),
                    static_cast<uint8_t>((iv & 0xff00) >> 8),
                    static_cast<uint8_t>(iv & 0xff),
                    1
                );
            }
        }

        emp_assert( false, str );
        __builtin_unreachable();
  }

    /// Parse the RGB or RGBA format color string
    /// @param str The color string ex) "rgb(255,255,255)"
    /// @param format_name Format name rgba or rgb
    Color Color::ParseRGB(const std::string& str, const std::string& format_name){
        const size_t open_paren = str.find_first_of( '(' );
        const size_t close_paren = str.find_first_of( ')' );
        if (open_paren != std::string::npos && close_paren + 1 == str.length()) {
            const std::vector<std::string> color_tokens = color_impl::split(str.substr(open_paren + 1,
                close_paren - (open_paren + 1)), ',');
            float alpha = 1.0f;
            if (format_name == "rgba") {
                if (color_tokens.size() != 4) {
                    emp_assert( false, str );
                    __builtin_unreachable();
                }
                alpha = color_impl::parse_css_float(color_tokens.back());
            } else if (color_tokens.size() != 3) {
                    emp_assert( false, str );
                    __builtin_unreachable();
            }

            return Color(
                color_impl::parse_css_int(color_tokens[0]),
                color_impl::parse_css_int(color_tokens[1]),
                color_impl::parse_css_int(color_tokens[2]),
                alpha
            );
        }
        emp_assert( false, str );
        __builtin_unreachable();
    }

    /// Parse the HSL or HSLA format color string
    /// @param str The color string ex) "hsl(100%,100%,100%)"
    /// @param format_name Format name hsla or hsl
    Color Color::ParseHSL(const std::string& str, const std::string& format_name){
        const size_t open_paren = str.find_first_of( '(' );
        const size_t close_paren = str.find_first_of( ')' );
        if (close_paren + 1 == str.length()) {
            const std::vector<std::string> color_tokens = color_impl::split(str.substr(open_paren + 1,
                close_paren - (open_paren + 1)), ',');
            float alpha = 1.0f;
            if (format_name == "hsla") {
                if (color_tokens.size() != 4) {
                    emp_assert( false, str );
                    __builtin_unreachable();
                }
                alpha = color_impl::parse_css_float(color_tokens.back());
            } else if (color_tokens.size() != 3) {
                    emp_assert( false, str );
                    __builtin_unreachable();
            }

            float h = color_impl::parse_float(color_tokens[0]) / 360.0f;
            float i;
            // Normalize the hue to [0..1]
            h = std::modf(h, &i);

            // NOTE(deanm): According to the CSS spec s/l should only be
            // percentages, but we don't bother and let float or percentage.
            const float s = color_impl::parse_css_float(color_tokens[1]);
            const float l = color_impl::parse_css_float(color_tokens[2]);

            const float m2 = l <= 0.5f ? l * (s + 1.0f) : l + s - l * s;
            const float m1 = l * 2.0f - m2;

            return Color(
                color_impl::clamp_css_byte(color_impl::css_hue_to_rgb(m1, m2, h + 1.0f / 3.0f) * 255.0f),
                color_impl::clamp_css_byte(color_impl::css_hue_to_rgb(m1, m2, h) * 255.0f),
                color_impl::clamp_css_byte(color_impl::css_hue_to_rgb(m1, m2, h - 1.0f / 3.0f) * 255.0f),
                alpha
            );
        }
        emp_assert( false, str );
        __builtin_unreachable();
    }

}
}

#endif
