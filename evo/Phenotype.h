//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  This file maintains the basic information for a Phenotype.
//
//  All of the phenotype metrics must be established at compile time and tools are provided
//  to easily do so.

#ifndef EMP_EVO_PHENOTYPE_H
#define EMP_EVO_PHENOTYPE_H

namespace emp {

  class Phenotype {
  public:

    class TraitBase {
    protected:
      std::string name;         // What is this trait called?
      std::string type;         // What type is this trait?
      std::string default_val;  // What value should an injected organism have?
      std::string desc;         // Brief description of trait

      // How should this trait be initialized at birth?
      //  INIT_DEFAULT - always use the default_val
      //  INIT_INHERIT - use the parent's initial value (and track it!)
      //  INIT_DIVIDE  - use the parent's final value on divide.
      enum INIT_TYPE { INIT_DEFAULT, INIT_INHERIT, INHERIT_DIVIDE };
      INIT_TYPE init;

      bool track_prev; // Should the previous value be tracked?
    public:
      TraitInfo(const std::string _name, const std::string _type,
                   const std::string _d_val, const std::string _desc)
        : name(_name), type(_type), default_val(_d_val), desc(_desc)
      { ; }
      virtual ~TraitInfo() { ; }

      const std::string & GetName() const { return name; }
      const std::string & GetType() const { return type; }
      const std::string & GetDefault() const { return default_val; }
      const std::string & GetDescription() const { return desc; }

      TraitInfo & SetName(const std::string & _in) { name = _in; return *this; }
      TraitInfo & SetType(const std::string & _in) { type = _in; return *this; }
      TraitInfo & SetDefault(const std::string & _in) { default_val = _in; return *this; }
      TraitInfo & SetDescription(const std::string & _in) { desc = _in; return *this; }

      virtual std::string GetValue() const = 0;
      virtual std::string GetLiteralValue() const = 0;
      virtual TraitInfo & SetValue(const std::string & in_val, std::stringstream & warnings) = 0;
    }; // end TraitBase

    // We need type-specific versions on this class to manage variables
    template <class VAR_TYPE> class Trait : public TraitBase {
    protected:
      VAR_TYPE & entry_ref;    // @CAO Should this be an index into an associated vector (or array?) so that all Trait descriptions can be static?
    public:
      Trait(const std::string _name, const std::string _type,
                   const std::string _d_val, const std::string _desc,
                   VAR_TYPE & _ref)
        : TraitBase(_name, _type, _d_val, _desc), entry_ref(_ref) { ; }
      ~Trait() { ; }

      std::string GetValue() const override { std::stringstream ss; ss << entry_ref; return ss.str(); }
      std::string GetLiteralValue() const override { return to_literal(entry_ref); }
      TraitBase & SetValue(const std::string & in_val, std::stringstream & warnings) override {
        (void) warnings;
        std::stringstream ss; ss << in_val; ss >> entry_ref; return *this;
      }
    };

  };


}

#endif
