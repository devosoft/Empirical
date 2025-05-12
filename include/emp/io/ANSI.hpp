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
    inline constexpr char Reset[] = "\033[0m";
    inline constexpr char Bold[] = "\033[1m";
    inline constexpr char Faint[] = "\033[2m";
    inline constexpr char Italic[] = "\033[3m";
    inline constexpr char Underline[] = "\033[4m";
    inline constexpr char SlowBlink[] = "\033[5m";
    inline constexpr char Blink[] = "\033[6m";
    inline constexpr char Reverse[] = "\033[7m";
    inline constexpr char Strike[] = "\033[9m";

    inline constexpr char NoBold[] = "\033[22m";      // Normal intensity; also not faint.
    inline constexpr char NoItalic[] = "\033[23m";
    inline constexpr char NoUnderline[] = "\033[24m";
    inline constexpr char NoBlink[] = "\033[25m";
    inline constexpr char NoReverse[] = "\033[27m";
    inline constexpr char NoStrike[] = "\033[29m";

    inline constexpr char Black[] = "\033[30m";
    inline constexpr char Red[] = "\033[31m";
    inline constexpr char Green[] = "\033[32m";
    inline constexpr char Yellow[] = "\033[33m";
    inline constexpr char Blue[] = "\033[34m";
    inline constexpr char Magenta[] = "\033[35m";
    inline constexpr char Cyan[] = "\033[36m";
    inline constexpr char White[] = "\033[37m";
    inline constexpr char DefaultColor[] = "\033[39m";

    inline constexpr char BlackBG[] = "\033[40m";
    inline constexpr char RedBG[] = "\033[41m";
    inline constexpr char GreenBG[] = "\033[42m";
    inline constexpr char YellowBG[] = "\033[43m";
    inline constexpr char BlueBG[] = "\033[44m";
    inline constexpr char MagentaBG[] = "\033[45m";
    inline constexpr char CyanBG[] = "\033[46m";
    inline constexpr char WhiteBG[] = "\033[47m";
    inline constexpr char DefaultBGColor[] = "\033[49m";

    inline constexpr char BrightBlack[] = "\033[90m";
    inline constexpr char BrightRed[] = "\033[91m";
    inline constexpr char BrightGreen[] = "\033[92m";
    inline constexpr char BrightYellow[] = "\033[93m";
    inline constexpr char BrightBlue[] = "\033[94m";
    inline constexpr char BrightMagenta[] = "\033[95m";
    inline constexpr char BrightCyan[] = "\033[96m";
    inline constexpr char BrightWhite[] = "\033[97m";

    inline constexpr char BrightBlackBG[] = "\033[100m";
    inline constexpr char BrightRedBG[] = "\033[101m";
    inline constexpr char BrightGreenBG[] = "\033[102m";
    inline constexpr char BrightYellowBG[] = "\033[103m";
    inline constexpr char BrightBlueBG[] = "\033[104m";
    inline constexpr char BrightMagentaBG[] = "\033[105m";
    inline constexpr char BrightCyanBG[] = "\033[106m";
    inline constexpr char BrightWhiteBG[] = "\033[107m";

    [[nodiscard]] inline std::string Apply(const char * start, std::string_view sv, const char * end) {
      return std::string(start).append(sv).append(end);
    }

    [[nodiscard]] inline std::string MakeBold(std::string_view sv) { return Apply(Bold, sv, NoBold); }
    [[nodiscard]] inline std::string MakeFaint(std::string_view sv) { return Apply(Faint, sv, NoBold); }
    [[nodiscard]] inline std::string MakeItalic(std::string_view sv) { return Apply(Italic, sv, NoItalic); }
    [[nodiscard]] inline std::string MakeUnderline(std::string_view sv) { return Apply(Underline, sv, NoUnderline); }
    [[nodiscard]] inline std::string MakeSlowBlink(std::string_view sv) { return Apply(SlowBlink, sv, NoBlink); }
    [[nodiscard]] inline std::string MakeBlink(std::string_view sv) { return Apply(Blink, sv, NoBlink); }
    [[nodiscard]] inline std::string MakeReverse(std::string_view sv) { return Apply(Reverse, sv, NoReverse); }
    [[nodiscard]] inline std::string MakeStrike(std::string_view sv) { return Apply(Strike, sv, NoStrike); }

    [[nodiscard]] inline std::string MakeColor(std::string_view sv, const char * color) {
      return Apply(color, sv, DefaultColor);
    }
    [[nodiscard]] inline std::string MakeBlack(std::string_view sv) { return MakeColor(sv, Black); }
    [[nodiscard]] inline std::string MakeRed(std::string_view sv) { return MakeColor(sv, Red); }
    [[nodiscard]] inline std::string MakeGreen(std::string_view sv) { return MakeColor(sv, Green); }
    [[nodiscard]] inline std::string MakeYellow(std::string_view sv) { return MakeColor(sv, Yellow); }
    [[nodiscard]] inline std::string MakeBlue(std::string_view sv) { return MakeColor(sv, Blue); }
    [[nodiscard]] inline std::string MakeMagenta(std::string_view sv) { return MakeColor(sv, Magenta); }
    [[nodiscard]] inline std::string MakeCyan(std::string_view sv) { return MakeColor(sv, Cyan); }
    [[nodiscard]] inline std::string MakeWhite(std::string_view sv) { return MakeColor(sv, White); }

    [[nodiscard]] inline std::string MakeBrightBlack(std::string_view sv) { return MakeColor(sv, BrightBlack); }
    [[nodiscard]] inline std::string MakeBrightRed(std::string_view sv) { return MakeColor(sv, BrightRed); }
    [[nodiscard]] inline std::string MakeBrightGreen(std::string_view sv) { return MakeColor(sv, BrightGreen); }
    [[nodiscard]] inline std::string MakeBrightYellow(std::string_view sv) { return MakeColor(sv, BrightYellow); }
    [[nodiscard]] inline std::string MakeBrightBlue(std::string_view sv) { return MakeColor(sv, BrightBlue); }
    [[nodiscard]] inline std::string MakeBrightMagenta(std::string_view sv) { return MakeColor(sv, BrightMagenta); }
    [[nodiscard]] inline std::string MakeBrightCyan(std::string_view sv) { return MakeColor(sv, BrightCyan); }
    [[nodiscard]] inline std::string MakeBrightWhite(std::string_view sv) { return MakeColor(sv, BrightWhite); }

    [[nodiscard]] inline std::string MakeBGColor(std::string_view sv, const char * color_id) {
      return Apply(color_id, sv, DefaultBGColor);
    }
    [[nodiscard]] inline std::string MakeBlackBG(std::string_view sv) { return MakeBGColor(sv, BlackBG); }
    [[nodiscard]] inline std::string MakeRedBG(std::string_view sv) { return MakeBGColor(sv, RedBG); }
    [[nodiscard]] inline std::string MakeGreenBG(std::string_view sv) { return MakeBGColor(sv, GreenBG); }
    [[nodiscard]] inline std::string MakeYellowBG(std::string_view sv) { return MakeBGColor(sv, YellowBG); }
    [[nodiscard]] inline std::string MakeBlueBG(std::string_view sv) { return MakeBGColor(sv, BlueBG); }
    [[nodiscard]] inline std::string MakeMagentaBG(std::string_view sv) { return MakeBGColor(sv, MagentaBG); }
    [[nodiscard]] inline std::string MakeCyanBG(std::string_view sv) { return MakeBGColor(sv, CyanBG); }
    [[nodiscard]] inline std::string MakeWhiteBG(std::string_view sv) { return MakeBGColor(sv, WhiteBG); }

    [[nodiscard]] inline std::string MakeBrightBlackBG(std::string_view sv) { return MakeBGColor(sv, BrightBlackBG); }
    [[nodiscard]] inline std::string MakeBrightRedBG(std::string_view sv) { return MakeBGColor(sv, BrightRedBG); }
    [[nodiscard]] inline std::string MakeBrightGreenBG(std::string_view sv) { return MakeBGColor(sv, BrightGreenBG); }
    [[nodiscard]] inline std::string MakeBrightYellowBG(std::string_view sv) { return MakeBGColor(sv, BrightYellowBG); }
    [[nodiscard]] inline std::string MakeBrightBlueBG(std::string_view sv) { return MakeBGColor(sv, BrightBlueBG); }
    [[nodiscard]] inline std::string MakeBrightMagentaBG(std::string_view sv) { return MakeBGColor(sv, BrightMagentaBG); }
    [[nodiscard]] inline std::string MakeBrightCyanBG(std::string_view sv) { return MakeBGColor(sv, BrightCyanBG); }
    [[nodiscard]] inline std::string MakeBrightWhiteBG(std::string_view sv) { return MakeBGColor(sv, BrightWhiteBG); }
  }

}

#endif // EMP_TOOLS_ANSI_HPP_INCLUDE