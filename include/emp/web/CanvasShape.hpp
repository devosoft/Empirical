/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2015-2018
 *
 *  @file  Canvas.hpp
 *  @brief Manage an HTML canvas object.
 *
 */


#ifndef EMP_WEB_CANVAS_SHAPE_HPP
#define EMP_WEB_CANVAS_SHAPE_HPP

#ifdef __EMSCRIPTEN__

#include "_EmscriptenCanvasShape.hpp"

// https://stackoverflow.com/questions/4102351/test-for-empty-macro-definition/4127073#4127073
#elif __has_include(<SFML/Graphics.hpp>) && (!defined(EMP_HAS_X11) || (EMP_HAS_X11 == 1)) 

#include "_NativeCanvasShape.hpp"

#else  // Must have SFML and environment variable DISPLAY set
static_assert(false, "Can't compile Canvas features without SFML available.");

#endif // #ifdef __EMSCRIPTEN__

#endif // #ifndef EMP_WEB_CANVAS_HPP
