#ifndef EMP_CONFIG_H
#define EMP_CONFIG_H

//////////////////////////////////////////////////////////////////////////////////////////
//
//  This file defines a master configuration option cConfig, whose values can be loaded
//  at runtime or else set as constant values throughout the code.
//
//  Assuming you have an emp::cConfig object called config, you can:
//
//  access a setting value:            config.SETTING_NAME()
//  adjust a setting value:            config.SETTING_NAME(new_value)
//  determine if a setting is locked:  config.SETTING_NAME_is_const()
//  lookup a setting dynamically:      config("SETTING_NAME")
//  adjust a setting dynamically:      config("SETTING_NAME", "new_value")
//
//  load settings from a stream:       config.Read(stream);
//  load settings from a file:         config.Read(filename);
//  save settings to a stream:         config.Write(stream);
//  save settings to a file:           config.Write(filename);
//
//  write settings macros to a stream: config.WriteMacros(stream);
//  write settings macros to a file:   config.WriteMacros(filename);
//

#include <map>
#include <ostream>
#include <fstream>
#include <string>
#include <sstream>
#include <unordered_set>
#include <vector>

#include "string_utils.h"
#include "functions.h"

namespace emp {

  // Master configuration class.
  class cConfig {
  private:
    class cConfigEntry {
    protected:
      std::string name;
      std::string type;
      std::string default_val;
      std::string desc;

      std::unordered_set<std::string> alias_set;
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

      cConfigEntry & AddAlias(const std::string & _in) { alias_set.insert(_in); return *this; }
      bool HasAlias(const std::string & _in) { return alias_set.find(_in) != alias_set.end(); }
      bool IsMatch(const std::string & _in) { return name == _in || HasAlias(_in); }
      const std::unordered_set<std::string> & GetAliases() { return alias_set; }
      
      virtual std::string GetValue() const = 0;
      virtual std::string GetLiteralValue() const = 0;
      virtual cConfigEntry & SetValue(const std::string & in_val) = 0;
      virtual bool IsConst() const = 0;
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
      
      std::string GetValue() const { std::stringstream ss; ss << entry_ref; return ss.str(); }
      std::string GetLiteralValue() const { return to_literal(entry_ref); }
      cConfigEntry & SetValue(const std::string & in_val) {
        std::stringstream ss; ss << in_val; ss >> entry_ref; return *this;
      }
      bool IsConst() const { return false; }
    };
    
    // We need a special entry type to represent constant values.
    template <class VAR_TYPE> class tConfigConstEntry : public cConfigEntry {
    public:
      tConfigConstEntry(const std::string _name, const std::string _type,
                   const std::string _d_val, const std::string _desc)
        : cConfigEntry(_name, _type, _d_val, _desc) { ; }
      ~tConfigConstEntry() { ; }
      
      std::string GetValue() const { return default_val; }
      std::string GetLiteralValue() const { return to_literal(default_val); }
      cConfigEntry & SetValue(const std::string & in_val) {
        // This is a constant setting.  If we are actually trying to change it, give a warning.
        if (in_val != GetValue()) {
          std::cerr << "WARNING: Trying to adjust locked setting '" 
                    << name << "' from '" << GetValue()
                    << "' to '" << in_val << "'. Ignoring." << std::endl;
        }
        return *this;
      }
      bool IsConst() const { return true; }
    };

    // A special entry for settings created during the run (only accissibly dynamically)
    class cConfigLiveEntry : public cConfigEntry {
    public:
      cConfigLiveEntry(const std::string _name, const std::string _type,
                       const std::string _d_val, const std::string _desc)
        : cConfigEntry(_name, _type, _d_val, _desc) { ; }
      ~cConfigLiveEntry() { ; }
      
      std::string GetValue() const { return default_val; }
      std::string GetLiteralValue() const { return to_literal(default_val); }
      cConfigEntry & SetValue(const std::string & in_val) { default_val = in_val; return *this; }
      bool IsConst() const { return false; }
    };
      
    // Entrys should be divided into groups
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
      
      int GetSize() const { return (int) entry_set.size(); }
      cConfigEntry * GetEntry(int id) { return entry_set[id]; }
      cConfigEntry * GetLastEntry() { return entry_set.back(); }

      void Add(cConfigEntry * new_entry) { entry_set.push_back(new_entry); }

      void Write(std::ostream & out) {
        // Print header information with the group name.
        out << "### " << m_name << " ###" << std::endl;
        // Print group description.
        std::vector<std::string> desc_lines;
        slice_string(m_desc, desc_lines);
        for (int comment_line = 0; comment_line < (int) desc_lines.size(); comment_line++) {
          out << "# " << desc_lines[comment_line] << std::endl;
        }
        out << std::endl;
        
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
          emp::slice_string(entry_set[i]->GetDescription(), desc_lines);

          int start_col = (int) setting_info[i].size();
          for (int comment_line = 0; comment_line < (int) desc_lines.size(); comment_line++) {
            for (int ws = start_col; ws < max_length; ws++) out << ' ';
            out << "# " << desc_lines[comment_line] << std::endl;
            start_col = 0;
          }
        }

        out << std::endl; // Skip a line after each group.
      }

      void WriteMacros(std::ostream & out) {
        // Print header information to register group.
        out << "EMP_CONFIG_GROUP(" << m_name << ", \"" << m_desc << "\")" << std::endl;

        // Loop through once to figure out non-comment output
        for (cConfigEntry * cur_entry : entry_set) {
          if (cur_entry->IsConst()) { out << "EMP_CONFIG_CONST("; }
          else { out << "EMP_CONFIG_VAR("; }
          out << cur_entry->GetName() << ", "
              << cur_entry->GetType() << ", "
            // << cur_entry->GetDefault() << ", "
            // << to_literal(cur_entry->GetValue()) << ", "
              << cur_entry->GetLiteralValue() << ", "
              << "\"" << cur_entry->GetDescription() << "\")"
              << std::endl;

          // Output aliases.
          const std::unordered_set<std::string> & alias_set = cur_entry->GetAliases();
          for (const std::string & cur_alias : alias_set) {
            out << "EMP_CONFIG_ALIAS(" << cur_alias << ")" << std::endl;
          }
        }

        out << std::endl; // Skip a line after each group.
      }
    };
    
    // Private member variables
    std::map<std::string, cConfigEntry *> m_var_map;
    std::string m_version_id;
    std::vector<cConfigGroup *> group_set;
    
    // Place all of the config private member variables here.
#define EMP_CONFIG_VAR(NAME, TYPE, DEFAULT, DESC) TYPE m_ ## NAME;
#include "config_include.h"
    
  public:
    cConfig(const std::string & in_version = "")
      : m_version_id(in_version)
        // Setup inital values for all variables.
#define EMP_CONFIG_VAR(NAME, TYPE, DEFAULT, DESC) , m_ ## NAME(DEFAULT)
#include "config_include.h"
    { 
      // Build a map to information about each variable.
#define EMP_CONFIG_VAR(NAME, TYPE, DEFAULT, DESC)                                          \
      m_var_map[#NAME] = new tConfigEntry<TYPE>(#NAME, #TYPE, #DEFAULT, DESC, m_ ## NAME); \
      group_set.back()->Add(m_var_map[#NAME]);
#define EMP_CONFIG_CONST(NAME, TYPE, VALUE, DESC)                                          \
      m_var_map[#NAME] = new tConfigConstEntry<TYPE>(#NAME, #TYPE, #VALUE, DESC);          \
      group_set.back()->Add(m_var_map[#NAME]);
#define EMP_CONFIG_GROUP(NAME, DESC) \
      group_set.push_back(new cConfigGroup(#NAME, DESC));
#include "config_include.h"
    }
    
    ~cConfig() {
      // Delete all entries in the var_map
      for (auto it = m_var_map.begin(); it != m_var_map.end(); it++) {
        delete it->second;
      }
    }

    std::string Get(const std::string & setting_name) {
      if (m_var_map.find(setting_name) == m_var_map.end()) {
        // This setting is not currently in the map!
        // @CAO Print warning?
        return "";
      }
      return m_var_map[setting_name]->GetValue();
    }

    cConfig & Set(const std::string & setting_name, const std::string & new_value,
                  const std::string & in_desc="") {
      if (m_var_map.find(setting_name) == m_var_map.end()) {
        // This setting is not currently in the map!  We should put it in.
        m_var_map[setting_name] =
          new cConfigLiveEntry(setting_name, "std::string", new_value, in_desc);
        group_set.back()->Add(m_var_map[setting_name]);
      }
      m_var_map[setting_name]->SetValue(new_value);
      return *this;
    }

    std::string operator()(const std::string & setting_name) { return Get(setting_name); }

    cConfig & operator()(const std::string & setting_name, const std::string & new_value) {      
      return Set(setting_name, new_value);
    }

    // Generate a text representation (typically a file) for the state of cConfig
    void Write(std::ostream & out) {
      // @CAO Start by printing some file header information?
      
      // Next print each group and it's information.
      for (auto it = group_set.begin(); it != group_set.end(); it++) {
        (*it)->Write(out);
      }
    }
    
    // If a string is passed into Write, treat it as a filename.
    void Write(std::string filename) {
      std::ofstream out(filename);
      Write(out);
      out.close();
    }

    // Generate a text representation (typically a file) for the state of cConfig
    void WriteMacros(std::ostream & out) {
      out << "/////////////////////////////////////////////////////////////////////////////////\n"
          << "//  This is an auto-generated file that defines a set of configuration options.\n"
          << "//  This file is read in mulitple times from config.h, each with different macro\n"
          << "//  definitions to generate correct, effecient code for the command below.\n"
          << "//\n"
          << "//  The available commands are:\n"
          << "//\n"
          << "//  EMP_CONFIG_GROUP(group name, group description string)\n"
          << "//   Start a new group of configuration options.  Group structure is preserved\n"
          << "//   when user-accessible configuration options are generated.\n"
          << "//\n"
          << "//  EMP_CONFIG_VAR(variable name, type, default value, description string)\n"
          << "//   Create a new setting in the cConfig object that can be easily accessed.\n"
          << "//\n"
          << "//  EMP_CONFIG_ALIAS(alias name)\n"
          << "//   Include an alias for the previous setting.  This command is useful to\n"
          << "//   maintain backward compatibility if names change in newer software versions.\n"
          << "//\n"
          << "//  EMP_CONFIG_CONST(variable name, type, fixed value, description string)\n"
          << "//   Create a new configuration constant that cannot be changed.  In practice,\n"
          << "//   allows broader optimizations in the code.\n"
          << std::endl;
      
      // Next print each group and it's information.
      for (auto it = group_set.begin(); it != group_set.end(); it++) {
        (*it)->WriteMacros(out);
      }
    }
    
    // If a string is passed into Write, treat it as a filename.
    void WriteMacros(std::string filename) {
      std::ofstream out(filename);
      WriteMacros(out);
      out.close();
    }

    // Read in from a text representation (typically a file) to set the state of cConfig.
    // Return success state.
    bool Read(std::istream & input) {
      // Load in the file one line at a time and process each line.
      std::string cur_line;

      // Loop through the file until eof is hit (does this work for other streams?)
      while (!input.eof()) {
        std::getline(input, cur_line);             // Get the current input line.
        cur_line = emp::string_pop(cur_line, '#'); // Deal with commments.
        emp::left_justify(cur_line);               // Clear out leading whitespace.
        if (cur_line == "") continue;              // Skip empty lines.

        std::string setting_name = emp::string_pop_word(cur_line);
        emp::right_justify(cur_line);

        Set(setting_name, cur_line);
      }
      return true;
    }

    bool Read(std::string filename) {
      std::ifstream in_file(filename);
      if (in_file.fail()) {
        std::cerr << "ERROR: Unable to open config file '" << filename << "'." << std::endl;
        return false;
      }
      bool success = Read(in_file);
      in_file.close();
      return success;
    }
    
    // Build Get and Set Accessors, as well as const check
#define EMP_CONFIG_VAR(NAME, TYPE, DEFAULT, DESC)                                 \
    inline const TYPE & NAME() const { return m_ ## NAME; }                       \
    const TYPE & NAME(const TYPE & _in) { m_ ## NAME = _in; return m_ ## NAME; }  \
    bool NAME ## _is_const() const { return false; }
#define EMP_CONFIG_CONST(NAME, TYPE, VALUE, DESC)                                 \
    inline TYPE NAME() const { return VALUE; }                                    \
    TYPE NAME(const TYPE & _in) {                                                 \
      std::cerr << "WARNING: Trying to set const '" << #NAME                      \
                << "'.  Ignoring." << std::endl;                                  \
      return VALUE;                                                               \
    }                                                                             \
    bool NAME ## _is_const() const { return true; }
#include "config_include.h"

  };
};

#endif

