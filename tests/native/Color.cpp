#define CATCH_CONFIG_MAIN
#include "third-party/Catch/single_include/catch2/catch.hpp"


#include "emp/web/Canvas.hpp"

#ifdef EMP_HAS_SFML

TEST_CASE("Test Color Construction", "[native]") {
    unsigned char r = 255;
    unsigned char g = 255;
    unsigned char b = 255;
    emp::web::Color white(r, g, b, 1);

    // Named color
    const std::string css_str1 = "white";
    emp::web::Color white2(css_str1);
    REQUIRE(white == white2);
    emp::web::Color notWhite("plum");
    REQUIRE(notWhite != white2);

    /// ABC format
    const std::string css_str2 = "#fff";
    emp::web::Color white3(css_str2);
    REQUIRE(white == white3);
    emp::web::Color notWhite2("#5f5");
    REQUIRE(notWhite2 != white3);

    const std::string css_str3 = "#ffffff";
    emp::web::Color white4(css_str3);
    REQUIRE(white == white4);
    emp::web::Color notWhite3("#bbbbbb");
    REQUIRE(notWhite3 != white4);

    /// RGB format
    const std::string css_str4 = "rgba(255,255,255,1)";
    emp::web::Color white5(css_str4);
    REQUIRE(white == white5);
    emp::web::Color notWhite4("rgba(221,21,0,1)");
    REQUIRE(notWhite4 != white5);

    const std::string css_str5 = "rgb(255,255,255)";
    emp::web::Color white6(css_str5);
    REQUIRE(white == white6);
    emp::web::Color notWhite5("rgb(221,21,0)");
    REQUIRE(notWhite5 != white6);

    REQUIRE(white.ToString() == white5.ToString());
    REQUIRE(css_str4 == white5.ToString());
    REQUIRE(notWhite4.ToString() == "rgba(221,21,0,1)");

    /// HSL format
    const std::string css_str6 = "hsla(100%,100%,100%,1)";
    emp::web::Color white7(css_str6);
    REQUIRE(white == white7);
    emp::web::Color notWhite6("hsla(50%,21%,0%,1)");
    REQUIRE(notWhite6 != white7);

    const std::string css_str7 = "hsl(100%,100%,100%)";
    emp::web::Color white8(css_str7);
    REQUIRE(white == white8);
    emp::web::Color notWhite7("hsl(50%,21%,0%)");
    REQUIRE(notWhite7 != white8);

    std::string impl_string = white8;
    REQUIRE(impl_string != css_str7);

    const std::string impl_string2 = white7;
    REQUIRE(impl_string2 != css_str6);
}

#endif
