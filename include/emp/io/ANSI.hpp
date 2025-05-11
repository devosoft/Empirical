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
    static constexpr char Reset[] = "\033[0m";
    static constexpr char Bold[] = "\033[1m";
    static constexpr char Faint[] = "\033[2m";
    static constexpr char Italic[] = "\033[3m";
    static constexpr char Underline[] = "\033[4m";
    static constexpr char SlowBlink[] = "\033[5m";
    static constexpr char Blink[] = "\033[6m";
    static constexpr char Reverse[] = "\033[7m";
    static constexpr char Strike[] = "\033[9m";

    static constexpr char NoBold[] = "\033[22m";      // Normal intensity; also not faint.
    static constexpr char NoItalic[] = "\033[23m";
    static constexpr char NoUnderline[] = "\033[24m";
    static constexpr char NoBlink[] = "\033[25m";
    static constexpr char NoReverse[] = "\033[27m";
    static constexpr char NoStrike[] = "\033[29m";

    static constexpr char Black[] = "\033[30m";
    static constexpr char Red[] = "\033[31m";
    static constexpr char Green[] = "\033[32m";
    static constexpr char Yellow[] = "\033[33m";
    static constexpr char Blue[] = "\033[34m";
    static constexpr char Magenta[] = "\033[35m";
    static constexpr char Cyan[] = "\033[36m";
    static constexpr char White[] = "\033[37m";
    static constexpr char DefaultColor[] = "\033[39m";

    static constexpr char BlackBG[] = "\033[40m";
    static constexpr char RedBG[] = "\033[41m";
    static constexpr char GreenBG[] = "\033[42m";
    static constexpr char YellowBG[] = "\033[43m";
    static constexpr char BlueBG[] = "\033[44m";
    static constexpr char MagentaBG[] = "\033[45m";
    static constexpr char CyanBG[] = "\033[46m";
    static constexpr char WhiteBG[] = "\033[47m";
    static constexpr char DefaultBGColor[] = "\033[49m";

    static constexpr char BrightBlack[] = "\033[90m";
    static constexpr char BrightRed[] = "\033[91m";
    static constexpr char BrightGreen[] = "\033[92m";
    static constexpr char BrightYellow[] = "\033[93m";
    static constexpr char BrightBlue[] = "\033[94m";
    static constexpr char BrightMagenta[] = "\033[95m";
    static constexpr char BrightCyan[] = "\033[96m";
    static constexpr char BrightWhite[] = "\033[97m";

    static constexpr char BrightBlackBG[] = "\033[100m";
    static constexpr char BrightRedBG[] = "\033[101m";
    static constexpr char BrightGreenBG[] = "\033[102m";
    static constexpr char BrightYellowBG[] = "\033[103m";
    static constexpr char BrightBlueBG[] = "\033[104m";
    static constexpr char BrightMagentaBG[] = "\033[105m";
    static constexpr char BrightCyanBG[] = "\033[106m";
    static constexpr char BrightWhiteBG[] = "\033[107m";

    inline std::string Apply(const char * start, std::string_view sv, const char * end) {
      return std::string(start).append(sv).append(end);
    }

    std::string MakeBold(std::string_view sv) { return Apply(Bold, sv, NoBold); }
    std::string MakeFaint(std::string_view sv) { return Apply(Faint, sv, NoBold); }
    std::string MakeItalic(std::string_view sv) { return Apply(Italic, sv, NoItalic); }
    std::string MakeUnderline(std::string_view sv) { return Apply(Underline, sv, NoUnderline); }
    std::string MakeSlowBlink(std::string_view sv) { return Apply(SlowBlink, sv, NoBlink); }
    std::string MakeBlink(std::string_view sv) { return Apply(Blink, sv, NoBlink); }
    std::string MakeReverse(std::string_view sv) { return Apply(Reverse, sv, NoReverse); }
    std::string MakeStrike(std::string_view sv) { return Apply(Strike, sv, NoStrike); }

    std::string MakeColor(std::string_view sv, const char * color) {
      return Apply(color, sv, DefaultColor);
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

    std::string MakeBGColor(std::string_view sv, const char * color_id) {
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
  }

}

#endif // EMP_TOOLS_ANSI_HPP_INCLUDE