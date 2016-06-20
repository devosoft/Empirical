// This file is part of Empirical, https://github.com/mercere99/Empirical/, and is
// Copyright (C) Michigan State University, 2015. It is licensed
// under the MIT Software license; see doc/LICENSE

#ifndef JS_OBJECT_STUCT_H
#define JS_OBJECT_STUCT_H

//////////////////////////////////////////////////////////////////////////////
//
//  If C++ is going to let you use Javascript-style objects (with multiple
//  properties), it needs to know what variables and types are available.
//  That will necessarily require some definition on the part of the user.
//  This header is the place for the user to do that.
//
//  The tuple struct builder macro takes type,variable-name pairs. If variable
//  names are chosen to match the names of properties of a type of object in
//  Javascript, JSWrap will be able to wrap functions that contain variables of
//  type JSDataObject.
//
//  Currently, in order for JSWrap to work with these structs, you also need
//  to tell the compiler the number of variables in the tuple struct by
//  #defining DATA_OBJECT_SIZE to be that number.
//
//  Development notes:
//  * It would be great if we could do away with this whole header

#include "../tools/tuple_struct.h"

struct JSDataObject{
  //constexpr const static int n_fields = 3;
  EMP_BUILD_INTROSPECTIVE_TUPLE( int, val,
				   std::string, word,
				   float, val2,
                   int, x,
                   int, y
				   )



};
#endif
