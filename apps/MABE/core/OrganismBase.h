/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2018
 *
 *  @file  OrganismBase.h
 *  @brief Base class for all organisms in MABE
 *
 *  This file details all of the basic functionality that all organisms MUST have, providing
 *  reasonable defaults when such are possible.
 * 
 *  These include a required Clone() member function to produce a duplicate of an organism.
 */

#ifndef MABE_ORGANISM_BASE_H
#define MABE_ORGANISM_BASE_H

#include "data/DataMap.h"

#include "OrganismTypeBase.h"

namespace mabe {

  class OrganismBase {
  protected:
    emp::Ptr<OrganismTypeBase> type_ptr;  ///< Pointer to information about this type of organism.
    OrgDataBlob org_data;

  public:
    OrganismBase(emp::Ptr<OrganismTypeBase> _type_ptr)
    : type_ptr(_type_ptr)
    , org_data(type_ptr->GetDefaultDataBlob())
    {
      type_ptr->IncCount();
    }
    OrganismBase(const OrganismBase & in_org) : type_ptr(in_org.type_ptr) { type_ptr->IncCount(); }
    virtual ~OrganismBase() { type_ptr->DecCount(); }

    emp::Ptr<OrganismTypeBase> GetTypePtr() const { return type_ptr; }
    OrgDataBlob & GetData() { return org_data; }
    const OrgDataBlob & GetData() const { return org_data; }

    virtual emp::Ptr<OrganismBase> Clone() = 0;
  };

}

#endif
