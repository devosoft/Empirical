#ifndef EMP_CONFIG_H
#define EMP_CONFIG_H

//////////////////////////////////////////////////////////////////////////////////////////
//
//  This file defines a master configuration option cConfig, whose values can be loaded
//  at runtime or else set as constant values throughout the code.
//


#include <map>
#include <ostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>

#include "string_utils.h"

namespace emp {

  // If EMP_CONFIG_FILE is set at compile time, the filename given will be used to define
  // configuration options.  Otherwise "config_opts.h" must exist.
#ifndef EMP_CONFIG_FILE
#define EMP_CONFIG_FILE "config_opts.h"
#endif
  
#define EMP_CONFIG_GROUP(NAME, DESC)
  
  // Master configuration class.
  class cConfig {
  private:
    class cConfigEntry {
    protected:
      std::string name;
      std::string type;
      std::string default_val;
      std::string desc;
    public:
      cConfigEntry(const std::string _name, const std::string _type,
                   const std::string _d_val, const std::string _desc)
        : name(_name), type(_type), default_val(_d_val), desc(_desc)
      { ; }
      virtual ~cConfigEntry() { ; }
      
      const std::string & GetName() const { return name; }
      const std::string & GetType() const { return type; }
      const std::string & GetDefault() const { return default_val; }
      const std::string & GetDescription() const { return desc; }
      
      cConfigEntry & SetName(const std::string & _in) { name = _in; return *this; }
      cConfigEntry & SetType(const std::string & _in) { type = _in; return *this; }
      cConfigEntry & SetDefault(const std::string & _in) { default_val = _in; return *this; }
      cConfigEntry & SetDescription(const std::string & _in) { desc = _in; return *this; }
      
      virtual std::string GetValue() = 0;
      virtual cConfigEntry & SetValue(const std::string & in_val) = 0;
    };
    
    // We need type-specific versions on this class to manage variables
    template <class VAR_TYPE> class tConfigEntry : public cConfigEntry {
    private:
      VAR_TYPE & entry_ref;
    public:
      tConfigEntry(const std::string _name, const std::string _type,
                   const std::string _d_val, const std::string _desc,
                   VAR_TYPE & _ref)
        : cConfigEntry(_name, _type, _d_val, _desc), entry_ref(_ref) { ; }
      ~tConfigEntry() { ; }
      
      std::string GetValue() { std::stringstream ss; ss << entry_ref; return ss.str(); }
      cConfigEntry & SetValue(const std::string & in_val) {
        std::stringstream ss; ss << in_val; ss >> entry_ref; return *this;
      }
    };
    
    class cConfigGroup {
    private:
      std::string m_name;
      std::string m_desc;
      std::vector<cConfigEntry *> entry_set;
    public:
      cConfigGroup(const std::string & _name, const std::string & _desc)
        : m_name(_name), m_desc(_desc)
      { ; }
      ~cConfigGroup() { ; }
      
      void Add(cConfigEntry * new_entry) { entry_set.push_back(new_entry); }
      void Save(std::ostream & out) {
        // Print header information with the group name.
        out << "### " << m_name << " ###" << std::endl;
        // @CAO Print group description.
        
        const int entry_count = entry_set.size();
        std::vector<std::string> setting_info(entry_count);
        int max_length = 0;

        // Loop through once to figure out non-comment output
        for (int i = 0; i < entry_count; i++) {
          setting_info[i] = entry_set[i]->GetName();
          setting_info[i] += " ";
          setting_info[i] += entry_set[i]->GetValue();
          if (max_length < (int) setting_info[i].size()) max_length = (int) setting_info[i].size();
        }

        max_length += 2;
        for (int i = 0; i < entry_count; i++) {
          out << setting_info[i];

          // Break the description up over multiple lines.
          std::vector<std::string> desc_lines;
          slice_string(entry_set[i]->GetDescription(), desc_lines);

          int start_col = (int) setting_info[i].size();
          for (int comment_line = 0; comment_line < (int) desc_lines.size(); comment_line++) {
            for (int ws = start_col; ws < max_length; ws++) out << ' ';
            out << "# " << desc_lines[comment_line] << std::endl;
            start_col = 0;
          }
        }

        out << std::endl; // Skip a line after each group.
      }
    };
    
    // Private member variables
    std::map<std::string, cConfigEntry *> m_var_map;
    std::string m_version_id;
    std::vector<cConfigGroup *> group_set;
    
  public:
    // Place all of the config variables here.
#define EMP_CONFIG_VAR(NAME, TYPE, DEFAULT, DESC) TYPE NAME;
#include EMP_CONFIG_FILE
#undef EMP_CONFIG_VAR
    
    cConfig(const std::string & in_version = "")
      : m_version_id(in_version)
        // Setup inital values for all variables.
#define EMP_CONFIG_VAR(NAME, TYPE, DEFAULT, DESC) , NAME(DEFAULT)
#include EMP_CONFIG_FILE
#undef EMP_CONFIG_VAR
    { 
      group_set.push_back(new cConfigGroup("Default", "Default group"));
      
      // Build a map to information about each variable.
#define EMP_CONFIG_VAR(NAME, TYPE, DEFAULT, DESC)                       \
      m_var_map[#NAME] = new tConfigEntry<TYPE>(#NAME, #TYPE, #DEFAULT, DESC, NAME); \
      group_set.back()->Add(m_var_map[#NAME]);
#include EMP_CONFIG_FILE
#undef EMP_CONFIG_VAR
    }
    
    ~cConfig() {
      // @CAO Delete all entries in the var_map
      ;
    }

    // Generate a text representation (typically a file) for the state of cConfig
    void Save(std::ostream & out) {
      // @CAO Start by printing some header information.
      
      // Next print each group and it's information.
      for (auto it = group_set.begin(); it != group_set.end(); it++) {
        (*it)->Save(out);
      }
    }
    
    // If a string is passed into Save, treat it as a filename.
    void Save(std::string filename) {
      std::ofstream out(filename);
      Save(out);
      out.close();
    }
    
    // @CAO Don't think we need accessors anymore...
    /*
    // Build Get Accessors
    #define EMP_CONFIG_VAR(NAME, TYPE, DEFAULT, DESC) const TYPE & Get_ ## NAME() const { return NAME; }
    #include EMP_CONFIG_FILE
    #undef EMP_CONFIG_VAR
    
    // Build Set Accessors (doesn't use references, so could be faster...)
    #define EMP_CONFIG_VAR(NAME, TYPE, DEFAULT, DESC) cConfig & Set_ ## NAME(TYPE _in) { NAME = _in; return *this; }
    #include EMP_CONFIG_FILE
    #undef EMP_CONFIG_VAR
    */

  };
};

#endif

