/*
 *  This file is part of Empirical, https://github.com/devosoft/Empirical
 *  Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  date: 2025
*/
/**
 *  @file
 *  @brief Simple functions to manipulate strings.
 *  Status: RELEASE
 */

#ifndef EMP_TOOLS_ANSI_HPP_INCLUDE
#define EMP_TOOLS_ANSI_HPP_INCLUDE

#include <string>
#include <string_view>

namespace emp {

  namespace ANSI {
    static constexpr size_t Reset = 0;
    static constexpr size_t Bold = 1;
    static constexpr size_t Faint = 2;
    static constexpr size_t Italic = 3;
    static constexpr size_t Underline = 4;
    static constexpr size_t SlowBlink = 5;
    static constexpr size_t Blink = 6;
    static constexpr size_t Reverse = 7;
    static constexpr size_t Strike = 9;

    static constexpr size_t NoBold = 22;      // Normal intensity; also not faint.
    static constexpr size_t NoItalic = 23;
    static constexpr size_t NoUnderline = 24;
    static constexpr size_t NoBlink = 25;
    static constexpr size_t NoReverse = 27;
    static constexpr size_t NoStrike = 29;

    static constexpr size_t Black = 30;
    static constexpr size_t Red = 31;
    static constexpr size_t Green = 32;
    static constexpr size_t Yellow = 33;
    static constexpr size_t Blue = 34;
    static constexpr size_t Magenta = 35;
    static constexpr size_t Cyan = 36;
    static constexpr size_t White = 37;
    static constexpr size_t DefaultColor = 39;

    static constexpr size_t BlackBG = 40;
    static constexpr size_t RedBG = 41;
    static constexpr size_t GreenBG = 42;
    static constexpr size_t YellowBG = 43;
    static constexpr size_t BlueBG = 44;
    static constexpr size_t MagentaBG = 45;
    static constexpr size_t CyanBG = 46;
    static constexpr size_t WhiteBG = 47;
    static constexpr size_t DefaultBGColor = 49;

    static constexpr size_t BrightBlack = 90;
    static constexpr size_t BrightRed = 91;
    static constexpr size_t BrightGreen = 92;
    static constexpr size_t BrightYellow = 93;
    static constexpr size_t BrightBlue = 94;
    static constexpr size_t BrightMagenta = 95;
    static constexpr size_t BrightCyan = 96;
    static constexpr size_t BrightWhite = 97;

    static constexpr size_t BrightBlackBG = 100;
    static constexpr size_t BrightRedBG = 101;
    static constexpr size_t BrightGreenBG = 102;
    static constexpr size_t BrightYellowBG = 103;
    static constexpr size_t BrightBlueBG = 104;
    static constexpr size_t BrightMagentaBG = 105;
    static constexpr size_t BrightCyanBG = 106;
    static constexpr size_t BrightWhiteBG = 107;

    inline std::string Tag(size_t id) {
      return std::string("\033[") + std::to_string(id) + 'm';
    }

    inline std::string Apply(size_t start_id, std::string_view sv, size_t end_id) {
      return Tag(start_id).append(sv) + Tag(end_id);
    }

    std::string MakeBold(std::string_view sv) { return Apply(Bold, sv, NoBold); }
    std::string MakeFaint(std::string_view sv) { return Apply(Faint, sv, NoBold); }
    std::string MakeItalic(std::string_view sv) { return Apply(Italic, sv, NoItalic); }
    std::string MakeUnderline(std::string_view sv) { return Apply(Underline, sv, NoUnderline); }
    std::string MakeSlowBlink(std::string_view sv) { return Apply(SlowBlink, sv, NoBlink); }
    std::string MakeBlink(std::string_view sv) { return Apply(Blink, sv, NoBlink); }
    std::string MakeReverse(std::string_view sv) { return Apply(Reverse, sv, NoReverse); }
    std::string MakeStrike(std::string_view sv) { return Apply(Strike, sv, NoStrike); }

    std::string MakeColor(std::string_view sv, size_t color_id) {
      return Apply(color_id, sv, DefaultColor);
    }
    std::string MakeBlack(std::string_view sv) { return MakeColor(sv, Black); }
    std::string MakeRed(std::string_view sv) { return MakeColor(sv, Red); }
    std::string MakeGreen(std::string_view sv) { return MakeColor(sv, Green); }
    std::string MakeYellow(std::string_view sv) { return MakeColor(sv, Yellow); }
    std::string MakeBlue(std::string_view sv) { return MakeColor(sv, Blue); }
    std::string MakeMagenta(std::string_view sv) { return MakeColor(sv, Magenta); }
    std::string MakeCyan(std::string_view sv) { return MakeColor(sv, Cyan); }
    std::string MakeWhite(std::string_view sv) { return MakeColor(sv, White); }

    std::string MakeBrightBlack(std::string_view sv) { return MakeColor(sv, BrightBlack); }
    std::string MakeBrightRed(std::string_view sv) { return MakeColor(sv, BrightRed); }
    std::string MakeBrightGreen(std::string_view sv) { return MakeColor(sv, BrightGreen); }
    std::string MakeBrightYellow(std::string_view sv) { return MakeColor(sv, BrightYellow); }
    std::string MakeBrightBlue(std::string_view sv) { return MakeColor(sv, BrightBlue); }
    std::string MakeBrightMagenta(std::string_view sv) { return MakeColor(sv, BrightMagenta); }
    std::string MakeBrightCyan(std::string_view sv) { return MakeColor(sv, BrightCyan); }
    std::string MakeBrightWhite(std::string_view sv) { return MakeColor(sv, BrightWhite); }

    std::string MakeBGColor(std::string_view sv, size_t color_id) {
      return Apply(color_id, sv, DefaultBGColor);
    }
    std::string MakeBlackBG(std::string_view sv) { return MakeBGColor(sv, BlackBG); }
    std::string MakeRedBG(std::string_view sv) { return MakeBGColor(sv, RedBG); }
    std::string MakeGreenBG(std::string_view sv) { return MakeBGColor(sv, GreenBG); }
    std::string MakeYellowBG(std::string_view sv) { return MakeBGColor(sv, YellowBG); }
    std::string MakeBlueBG(std::string_view sv) { return MakeBGColor(sv, BlueBG); }
    std::string MakeMagentaBG(std::string_view sv) { return MakeBGColor(sv, MagentaBG); }
    std::string MakeCyanBG(std::string_view sv) { return MakeBGColor(sv, CyanBG); }
    std::string MakeWhiteBG(std::string_view sv) { return MakeBGColor(sv, WhiteBG); }

    std::string MakeBrightBlackBG(std::string_view sv) { return MakeBGColor(sv, BrightBlackBG); }
    std::string MakeBrightRedBG(std::string_view sv) { return MakeBGColor(sv, BrightRedBG); }
    std::string MakeBrightGreenBG(std::string_view sv) { return MakeBGColor(sv, BrightGreenBG); }
    std::string MakeBrightYellowBG(std::string_view sv) { return MakeBGColor(sv, BrightYellowBG); }
    std::string MakeBrightBlueBG(std::string_view sv) { return MakeBGColor(sv, BrightBlueBG); }
    std::string MakeBrightMagentaBG(std::string_view sv) { return MakeBGColor(sv, BrightMagentaBG); }
    std::string MakeBrightCyanBG(std::string_view sv) { return MakeBGColor(sv, BrightCyanBG); }
    std::string MakeBrightWhiteBG(std::string_view sv) { return MakeBGColor(sv, BrightWhiteBG); }
  };

}

#endif // EMP_TOOLS_ANSI_HPP_INCLUDE