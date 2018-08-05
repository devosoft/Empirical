/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2018
 *
 *  @file  ControllerBase.h
 *  @brief Base class for all controller types.
 *
 *  Controllers are generic forms of agent brains.
 */

#ifndef MABE_CONTROLLER_BASE_H
#define MABE_CONTROLLER_BASE_H

namespace mabe {

  class ControllerBase {
  private:
  public:
    ControllerBase() { ; }
    virtual ~ControllerBase() { ; }

    virtual std::string GetClassName() const = 0;
  };

}

#endif

