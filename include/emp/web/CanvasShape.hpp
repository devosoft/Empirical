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

#else // #ifdef __EMSCRIPTEN__

#include "_NativeCanvasShape.hpp"

#endif // #ifdef __EMSCRIPTEN__

#endif // #ifndef EMP_WEB_CANVAS_HPP
