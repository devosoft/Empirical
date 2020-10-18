/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2018
 *
 *  @file  Organism.h
 *  @brief Class to manage all organisms in MABE
 *
 *  This file details a base class all of the basic functionality that all organisms MUST have,
 *  providing reasonable defaults when such are possible, plus a template for building an organism
 *  with a brain and a genome.
 * 
 *  These include a required Clone() member function to produce a duplicate of an organism.
 */

#ifndef MABE_ORGANISM_H
#define MABE_ORGANISM_H

#include "data/DataMap.h"

#include "BrainWrapper.h"
#include "GenomeWrapper.h"
#include "OrganismInterface.h"

namespace mabe {

  /// An OrganismInterface helps connect it to the environment and manages communications.
  /// OrganismInterfaceBase provides a consistent interface for unknown organism types.
  class OrganismInterfaceBase {
  protected:
  public:
  };

  class OrganismBase {
  protected:
    emp::Ptr<OrganismInterfaceBase> type_ptr;  ///< Pointer to information about this type of organism.
    emp::DataMap<double,std::string> org_data;

  public:
    OrganismBase(emp::Ptr<OrganismInterfaceBase> _type_ptr)
    : type_ptr(_type_ptr)
    , org_data(type_ptr->GetDefaultDataBlob())
    {
      type_ptr->IncCount();
    }
    OrganismBase(const OrganismBase & in_org) : type_ptr(in_org.type_ptr) { type_ptr->IncCount(); }
    virtual ~OrganismBase() { type_ptr->DecCount(); }

    emp::Ptr<OrganismInterfaceBase> GetInterfacePtr() const { return type_ptr; }
    OrgDataBlob & GetData() { return org_data; }
    const OrgDataBlob & GetData() const { return org_data; }

    virtual emp::Ptr<OrganismBase> Clone() = 0;
  };

}

#endif
