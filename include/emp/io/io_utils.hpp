/*
 *  This file is part of Empirical, https://github.com/devosoft/Empirical
 *  Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  date: 2025
*/
/**
 *  @file
 *  @brief Tools to simplify IO, such as with a command line
 *  @note Status: BETA
 *
 */

#ifndef EMP_IO_IO_UTILS_HPP_INCLUDE
#define EMP_IO_IO_UTILS_HPP_INCLUDE

#include <iostream>

#ifdef _WIN32
  #include <conio.h>
  namespace emp {
    static constexpr bool WINDOWS_IO = true;
  }
#else
  #include <termios.h>
  #include <unistd.h>
  namespace emp {
    static constexpr bool WINDOWS_IO = false;
  }
#endif

namespace emp {

  char GetIOChar() {
    #ifdef _WIN32
      return _getch();
    #else
      termios old_term;
      char ch;
      tcgetattr(STDIN_FILENO, &old_term);
      termios new_term = old_term;
      new_term.c_lflag &= ~(ICANON | ECHO);
      tcsetattr(STDIN_FILENO, TCSANOW, &new_term);
      read(STDIN_FILENO, &ch, 1);
      tcsetattr(STDIN_FILENO, TCSANOW, &old_term);
      return ch;
    #endif
  }

}

#endif // #ifndef EMP_IO_IO_UTILS_HPP_INCLUDE
 