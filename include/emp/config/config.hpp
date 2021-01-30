/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2016-2019.
 *
 *  @file  config.hpp
 *  @brief Maintains a set of configuration options.
 *
 *  This file defines a master configuration option Config, whose values can be loaded
 *  at runtime or else set as constant values throughout the code.
 *
 *  Assuming you have an emp::Config object called config, you can:
 *
 *  access a setting value:            config.SETTING_NAME()
 *  adjust a setting value:            config.SETTING_NAME(new_value)
 *  determine if a setting is locked:  config.SETTING_NAME_is_const()
 *  lookup a setting dynamically:      config("SETTING_NAME")
 *  adjust a setting dynamically:      config("SETTING_NAME", "new_value")
 *
 *  load settings from a stream:       config.Read(stream);
 *  load settings from a file:         config.Read(filename);
 *  save settings to a stream:         config.Write(stream);
 *  save settings to a file:           config.Write(filename);
 *
 *  write settings macros to a stream: config.WriteMacros(stream);
 *  write settings macros to a file:   config.WriteMacros(filename);
 *
 *
 *  The configuration files generated can use the following keywords in order to
 *  configure this object:
 *   include OTHER_FILENAME         -- Load in all data from another file.
 *   set SETTING_NAME VALUE         -- Set a basic configuration setting.
 *   new OBJECT_TYPE OBJECT_NAME    -- Create a new config object of a managed class.
 *   use OBJECT_TYPE OBJECT_NAME    -- Use a previouly created configuration object.
 */

#ifndef EMP_CONFIG_H
#define EMP_CONFIG_H

#include <fstream>
#include <functional>
#include <ostream>
#include <string>
#include <sstream>
#include <unordered_set>

#include "../base/errors.hpp"
#include "../base/unordered_map.hpp"
#include "../base/vector.hpp"
#include "../datastructs/map_utils.hpp"
#include "../tools/string_utils.hpp"
#include "ConfigManager.hpp"


namespace emp {
  using namespace std::placeholders;

  /// Base class for all configuration settings.
  class ConfigEntry {
  protected:
    std::string name;
    std::string type;
    std::string default_val;
    std::string desc;

    std::unordered_set<std::string> alias_set;

  public:
    ConfigEntry(const std::string _name, const std::string _type,
                 const std::string _d_val, const std::string _desc)
      : name(_name), type(_type), default_val(_d_val), desc(_desc), alias_set()
    { ; }
    virtual ~ConfigEntry() { ; }

    const std::string & GetName() const { return name; }
    const std::string & GetType() const { return type; }
    const std::string & GetDefault() const { return default_val; }
    const std::string & GetDescription() const { return desc; }

    ConfigEntry & SetName(const std::string & _in) { name = _in; return *this; }
    ConfigEntry & SetType(const std::string & _in) { type = _in; return *this; }
    ConfigEntry & SetDefault(const std::string & _in) { default_val = _in; return *this; }
    ConfigEntry & SetDescription(const std::string & _in) { desc = _in; return *this; }

    /// Alert this setting that it is aliased to alternate possible names.
    ConfigEntry & AddAlias(const std::string & _in) { alias_set.insert(_in); return *this; }

    /// Are there any alternate names for this setting?
    bool HasAlias(const std::string & _in) { return alias_set.find(_in) != alias_set.end(); }

    /// Will the provided name match this setting?
    bool IsMatch(const std::string & _in) { return name == _in || HasAlias(_in); }

    /// Retrieve the full set of aliases.
    const std::unordered_set<std::string> & GetAliases() { return alias_set; }

    /// Retrieve the value of this setting as a string.
    virtual std::string GetValue() const = 0;

    /// Conver the value of this setting into a literal that C++ would recognize as its current value.
    virtual std::string GetLiteralValue() const = 0;

    /// Use a string to set the value of this setting.
    virtual ConfigEntry & SetValue(const std::string & in_val, std::stringstream & warnings) = 0;

    /// Identify if this setting is fixed at compile time.
    virtual bool IsConst() const = 0;
  };

  /// Master configuration class that manages all of the settings.
  class Config {
  protected:

    /// Type-specific versions of ConfigEntry class to manage settings.
    template <class VAR_TYPE> class tConfigEntry : public ConfigEntry {
    protected:
      VAR_TYPE & entry_ref;
    public:
      tConfigEntry(const std::string _name, const std::string _type,
                   const std::string _d_val, const std::string _desc,
                   VAR_TYPE & _ref)
        : ConfigEntry(_name, _type, _d_val, _desc), entry_ref(_ref) { ; }
      ~tConfigEntry() { ; }

      std::string GetValue() const { return emp::to_string(entry_ref); }
      std::string GetLiteralValue() const { return to_literal(entry_ref); }
      ConfigEntry & SetValue(const std::string & in_val, std::stringstream & /* warnings */) {
        std::stringstream ss; ss << in_val; ss >> entry_ref; return *this;
      }
      bool IsConst() const { return false; }
    };

    /// Type-specific and CONST versions of ConfigEntry class to manage fixed settings.
    template <class VAR_TYPE> class tConfigConstEntry : public ConfigEntry {
    protected:
      const VAR_TYPE literal_val;
    public:
      tConfigConstEntry(const std::string _name, const std::string _type,
                        const std::string _d_val, const std::string _desc,
                        const VAR_TYPE & _literal_val)
        : ConfigEntry(_name, _type, _d_val, _desc), literal_val(_literal_val) { ; }
      ~tConfigConstEntry() { ; }

      std::string GetValue() const { return default_val; }
      std::string GetLiteralValue() const { return to_literal(literal_val); }
      ConfigEntry & SetValue(const std::string & in_val, std::stringstream & warnings) {
        // This is a constant setting.  If we are actually trying to change it, give a warning.
        if (in_val != GetValue()) {
          warnings << "Trying to adjust locked setting '"
                   << name << "' from '" << GetValue()
                   << "' to '" << in_val << "'. Ignoring." << std::endl;
        }
        return *this;
      }
      bool IsConst() const { return true; }
    };

    /// Special settings entry for settings created during the run (only accissibly dynamically)
    class ConfigLiveEntry : public ConfigEntry {
    public:
      ConfigLiveEntry(const std::string _name, const std::string _type,
                       const std::string _d_val, const std::string _desc)
        : ConfigEntry(_name, _type, _d_val, _desc) { ; }
      ~ConfigLiveEntry() { ; }

      std::string GetValue() const { return default_val; }
      std::string GetLiteralValue() const { return to_literal(default_val); }
      ConfigEntry & SetValue(const std::string & in_val, std::stringstream & warnings) {
        (void) warnings;
        default_val = in_val;
        return *this;
      }
      bool IsConst() const { return false; }
    };

    /// Information about a sub-group of settings.
    class ConfigGroup {
    protected:
      std::string name;
      std::string desc;
      emp::vector<ConfigEntry *> entry_set;
    public:
      ConfigGroup(const std::string & _name, const std::string & _desc)
        : name(_name), desc(_desc), entry_set()
      { ; }
      ~ConfigGroup() { ; }

      size_t GetSize() const { return entry_set.size(); }

      std::string GetName() const { return name; }
      std::string GetDesc() const { return desc; }

      ConfigEntry * GetEntry(size_t id) { return entry_set[id]; }
      ConfigEntry * GetLastEntry() { emp_assert(GetSize() > 0); return entry_set.back(); }

      void Add(ConfigEntry * new_entry) { entry_set.push_back(new_entry); }

      void Write(std::ostream & out) const {
        // Print header information with the group name.
        out << "### " << name << " ###" << std::endl;
        // Print group description.
        auto desc_lines = slice(desc);
        for (size_t comment_line = 0; comment_line < desc_lines.size(); comment_line++) {
          out << "# " << desc_lines[comment_line] << std::endl;
        }
        out << std::endl;

        const size_t entry_count = entry_set.size();
        emp::vector<std::string> setting_info(entry_count);
        size_t max_length = 0;

        // Loop through once to figure out non-comment output
        for (size_t i = 0; i < entry_count; i++) {
          setting_info[i] = "set ";
          setting_info[i] += entry_set[i]->GetName();
          setting_info[i] += " ";
          setting_info[i] += entry_set[i]->GetValue();
          if (max_length < setting_info[i].size()) max_length = setting_info[i].size();
        }

        // Loop through a second time to actually do the printing with properly spaced comments.
        max_length += 2;
        for (size_t i = 0; i < entry_count; i++) {
          out << setting_info[i];

          // Break the description up over multiple lines.
          auto desc_lines = emp::slice(entry_set[i]->GetDescription());

          size_t start_col = setting_info[i].size();
          for (size_t comment_line = 0; comment_line < desc_lines.size(); comment_line++) {
            for (size_t ws = start_col; ws < max_length; ws++) out << ' ';
            out << "# " << desc_lines[comment_line] << std::endl;
            start_col = 0;
          }
        }

        out << std::endl; // Skip a line after each group.
      }

      void WriteMacros(std::ostream & out, bool as_const) const {
        // Print header information to register group.
        out << "  GROUP(" << name << ", \"" << desc << "\"),\n";

        // Loop through once to figure out non-comment output
        for (ConfigEntry * cur_entry : entry_set) {
          if (as_const || cur_entry->IsConst()) { out << "    CONST("; }
          else { out << "    VALUE("; }

          out << cur_entry->GetName() << ", "
              << cur_entry->GetType() << ", "
              << cur_entry->GetLiteralValue() << ", "
              << to_literal( cur_entry->GetDescription() )
              << "),\n";

          // Output aliases.
          const std::unordered_set<std::string> & alias_set = cur_entry->GetAliases();
          for (const std::string & cur_alias : alias_set) {
            out << "      ALIAS(" << cur_alias << "),\n";
          }
        }

        out << std::endl; // Skip a line after each group.
      }
    };

    // === Helper Functions ===
    ConfigGroup * GetActiveGroup() {
      if (group_set.size() == 0) {
        group_set.push_back(new ConfigGroup("DEFAULT", "Default settings group"));
      }
      return group_set.back();
    }

    ConfigEntry * GetActiveEntry() {
      ConfigGroup * group = GetActiveGroup();
      emp_assert(group->GetSize() > 0);
      return group->GetLastEntry();
    }

    // Which characters can legally be part of a variable identifier?
    bool IsVarChar(const char c) {
      if (c >= 'a' && c <= 'z') return true;
      if (c >= 'A' && c <= 'Z') return true;
      if (c >= '0' && c <= '9') return true;
      if (c == '_') return true;
      return false;
    }

    // Process a line by:
    // * Remove excess whitespace
    // * Expand all variables beginning with a $ in config line.
    // * If wrap-around, move line to extras
    void ProcessLine(std::string & cur_line, std::string & extras) {
      size_t start_pos = extras.size();          // If there were extras last time, skip them.
      if (extras.size()) cur_line.insert(0, extras);
      extras.resize(0);
      emp::left_justify(cur_line);               // Clear out leading whitespace.

      for (size_t pos = start_pos; pos < cur_line.size(); pos++) {
        const char cur_char = cur_line[pos];
        // Check for escape characters and convert them appropriately.
        if (cur_char == '\\') {
          if (pos+1 == cur_line.size()) {                    // If backslash is at end of line...
            extras = cur_line.substr(0, cur_line.size()-1);  // ...move string to extras
            cur_line.resize(0);                              // ...don't process current line
            return;                                          // ...since this is the line end, stop
          }
          // If we make it this far, we have a regular character being escaped.  Make the swap!
          const char esc_char = cur_line[pos+1];
          switch (esc_char) {
            case '$': cur_line.replace(pos, 2, "$"); break;
            case '#': cur_line.replace(pos, 2, "#"); break;
            case '\\': cur_line.replace(pos, 2, "\\"); break;
            case 'n': cur_line.replace(pos, 2, "\n"); break;
            case 'r': cur_line.replace(pos, 2, "\r"); break;
            case 't': cur_line.replace(pos, 2, "\t"); break;
          }
        }
        // A '#' indicates that a comment is starting that the rest of the line should be removed.
        else if (cur_char == '#') {
          cur_line.resize(pos);
        }
        // A '$' indicates that we should expand a variable in place.
        else if (cur_char == '$' && expand_ok) {
          size_t end_pos = pos+1;
          while (end_pos < cur_line.size() && IsVarChar(cur_line[end_pos])) end_pos++;
          const size_t var_size = end_pos - pos - 1;
          std::string var_name(cur_line, pos+1, var_size);

          if (ResolveAlias(var_name)) {
            std::string new_val = var_map[var_name]->GetValue();  // Lookup variable value.
            cur_line.replace(pos, var_size+1, new_val);           // Replace var name with value.
            pos += new_val.size();                                // Skip new text.
          } else {
            std::stringstream ss;
            ss << "Unable to process config setting '$" << var_name << "'. Ignoring." << std::endl;
            emp::NotifyError(ss.str());
          }
          // @CAO CONTINUE
        }
      }

    }

    // === Protected member variables ===
    emp::vector<std::string> class_names;           // Names in class heiarchy.
    emp::unordered_map<std::string, ConfigEntry *> var_map;   // All variables across groups.
    std::string version_id;                         // Unique version ID to ensure synced config.
    emp::vector<ConfigGroup *> group_set;           // All of the config groups.
    std::stringstream warnings;                     // Aggregate warnings for combined display.
    int delay_warnings;                             // Count of delays to collect warnings for printing.
    emp::unordered_map<std::string, std::string> alias_map;   // Map all aliases to original name.

    // Map namespaces to the appropriate config object.
    emp::unordered_map<std::string, Config *> namespace_map;

    // Map new type names to the manager that handles them.
    emp::unordered_map<std::string, ConfigManager_Base *> type_manager_map;

    // Build a map of extra input commands to the function that they should call if triggered.
    emp::unordered_map<std::string, std::function<bool(std::string)> > command_map;
    emp::unordered_map<std::string, std::function<bool(std::string)> > new_map;
    emp::unordered_map<std::string, std::function<bool(std::string)> > use_map;

    // Instructions on how config should behave.
    bool expand_ok;          // Should we expand variables in the config file.

  public:
    Config(const std::string & in_version = "")
      : class_names(), var_map(), version_id(in_version), group_set(), warnings()
      , delay_warnings(0), alias_map(), namespace_map(), type_manager_map(), command_map()
      , new_map(), use_map(), expand_ok(true)
    {
      class_names.push_back("emp::Config");
    }

    ~Config() {
      // Delete all alocated memory!
      for (auto & x : var_map) delete x.second;
      for (auto & x : group_set) delete x;
      for (auto & x : type_manager_map) delete x.second;
    }

    friend class ConfigWebUI;

    ConfigEntry * operator[](const std::string & name) { return var_map[name]; }
    auto begin() -> decltype(var_map.begin()) { return var_map.begin(); }
    auto end() -> decltype(var_map.end()) { return var_map.end(); }

    const ConfigEntry * operator[](const std::string & name) const { return var_map.at(name); }
    auto cbegin() -> decltype(var_map.cbegin()) { return var_map.cbegin(); }
    auto cend() -> decltype(var_map.cend()) { return var_map.cend(); }

    auto begin() const -> const decltype(var_map.begin()) { return var_map.begin(); }
    auto end() const -> const decltype(var_map.end()) { return var_map.end(); }

    Config & SetExpandOK(bool ok=true) { expand_ok = ok; return *this; }

    bool Has(const std::string & setting_name) const {
      return (var_map.find(setting_name) != var_map.end()) ||
        (alias_map.find(setting_name) != alias_map.end());
    }

    bool ResolveAlias(std::string & setting_name) const {
      if (var_map.find(setting_name) != var_map.end()) return true;
      if (alias_map.find(setting_name) != alias_map.end()) {
        setting_name = alias_map.find(setting_name)->second;
        return true;
      }
      return false;
    }

    std::string Get(std::string setting_name) {
      if (!ResolveAlias(setting_name)) return "";  // @CAO Print warning?
      return var_map[setting_name]->GetValue();
    }

    Config & Set(std::string setting_name, const std::string & new_value,
                  const std::string & in_desc="") {
      if (!ResolveAlias(setting_name)) {
        // This setting is not currently in the map!  We should put it in, but let user know.
        warnings << "Unknown setting '" << setting_name << "'.  Creating." << std::endl;
        var_map[setting_name] = new ConfigLiveEntry(setting_name, "std::string", new_value, in_desc);
        GetActiveGroup()->Add(var_map[setting_name]);
      }
      var_map[setting_name]->SetValue(new_value, warnings);
      if (!delay_warnings && warnings.rdbuf()->in_avail()) {
        emp::NotifyWarning(warnings.str());
        warnings.str(std::string()); // Clear the warnings.
      }
      return *this;
    }

    std::string operator()(const std::string & setting_name) { return Get(setting_name); }

    Config & operator()(const std::string & setting_name, const std::string & new_value) {
      return Set(setting_name, new_value);
    }

    void AddAlias(const std::string & base_name, const std::string & alias_name) {
      emp_assert( var_map.find(base_name) != var_map.end() );  // Make sure base exists.
      emp_assert( !Has(alias_name) ); // Make sure alias does not!
      alias_map[alias_name] = base_name;
      var_map[base_name]->AddAlias(alias_name);
    }

    // Generate a text representation (typically a file) for the state of Config
    void Write(std::ostream & out) const {
      // @CAO Start by printing some file header information?

      // Next print each group and its information.
      for (auto it = group_set.begin(); it != group_set.end(); it++) {
        (*it)->Write(out);
      }

      // Next, loop through all internal namespaces and print them out.
      for (auto & x : namespace_map) {
        out << "\n############################################################\n";
        out << "namespace " << x.first << "\n\n";
        x.second->Write(out);
        out << "end_namespace " << x.first << "\n";
      }
    }

    // If a string is passed into Write, treat it as a filename.
    void Write(std::string filename) const {
      std::ofstream out(filename);
      Write(out);
      out.close();
    }

    // Generate a text representation (typically a file) for the state of Config
    void WriteMacros(std::ostream & out, bool as_const=false) const {
      out << "/////////////////////////////////////////////////////////////////////////////////\n"
          << "//  This is an auto-generated file that defines a set of configuration options.\n"
          << "//\n"
          << "//  To create a new config from scratch, the format is:\n"
          << "//    EMP_BUILD_CONFIG( CLASS_NAME, OPTIONS... )\n"
          << "//\n"
          << "//  To extend an existing config, simply use:\n"
          << "//    EMP_EXTEND_CONFIG( NEW_NAME, BASE_CLASS, OPTIONS... )\n"
          << "//\n"
          << "//  The available OPTIONS are:\n"
          << "//\n"
          << "//  GROUP(group name, group description string)\n"
          << "//   Start a new group of configuration options.  Group structure is preserved\n"
          << "//   when user-accessible configuration options are generated.\n"
          << "//\n"
          << "//  VALUE(variable name, type, default value, description string)\n"
          << "//   Create a new setting in the emp::Config object that can be easily accessed.\n"
          << "//\n"
          << "//  CONST(variable name, type, fixed value, description string)\n"
          << "//   Create a new configuration constant that cannot be changed.  In practice,\n"
          << "//   allows broader optimizations in the code.\n"
          << "//\n"
          << "//  ALIAS(alias name)\n"
          << "//   Include an alias for the previous setting.  This command is useful to\n"
          << "//   maintain backward compatibility if names change in newer software versions.\n"
          << "\n"
          << "EMP_BUILD_CONFIG(" << class_names.back() << ","
          << std::endl;

      // Next print each group and it's information.
      for (auto it = group_set.begin(); it != group_set.end(); it++) {
        (*it)->WriteMacros(out, as_const);
      }

      out << ")" << std::endl;
    }

    // If a string is passed into Write, treat it as a filename.
    void WriteMacros(std::string filename, bool as_const=false) const {
      std::ofstream out(filename);
      WriteMacros(out, as_const);
      out.close();
    }


    /// Read in from a text representation (typically a file) to set the state of Config.
    /// Return success state.
    bool Read(std::istream & input, const std::string & cur_namespace="") {
      // Load in the file one line at a time and process each line.
      std::string cur_line, extras;
      delay_warnings++;

      // Loop through the file until eof is hit (does this work for other streams?)
      while (!input.eof()) {
        std::getline(input, cur_line);         // Get the current input line.
        ProcessLine(cur_line, extras);         // Clean up line; act on aliases.

        if (cur_line == "") continue;          // Skip empty lines.

        std::string command = emp::string_pop_word(cur_line);

        if (command == "set") {
          // Set a specific value.
          std::string setting_name = emp::string_pop_word(cur_line);
          Set(setting_name, cur_line);
        }
        else if (command == "include") {
          // Determine the filename to include.
          std::string filename = emp::string_pop_word(cur_line);

          // Process the new file (before automatically coming back to this one)
          Read(filename);
        }
        else if (command == "namespace") {
          std::string namespace_name = emp::string_pop_word(cur_line);
          if (cur_line.size() > 0) {
            warnings << "namespace " << namespace_name
                     << " cannot have additional arguments.  Ignoring.\n";
          }
          if (emp::Has(namespace_map, namespace_name) == false) {
            emp::NotifyError(emp::to_string("Unknown namespace '", namespace_name, "'.  Aborting."));
            return false;
          }
          Config * ns_config = namespace_map[namespace_name];
          ns_config->Read(input, namespace_name);
        }
        else if (command == "end_namespace") {
          std::string namespace_name = emp::string_pop_word(cur_line);
          if (cur_line.size() > 0) {
            warnings << "end_namespace " << namespace_name
                     << " cannot have additional arguments.  Ignoring.\n";
          }
          if (namespace_name != cur_namespace) {
            emp::NotifyError(emp::to_string("Cannot end namespace '", namespace_name,
                                            "' while in namespace '", cur_namespace, "'.  Aborting."));
            return false;
          }
          return true;
        }
        else if (command == "new") {
          std::string type_name = emp::string_pop_word(cur_line);
          if (emp::Has(new_map, type_name) == false) {
            emp::NotifyError(emp::to_string("Command 'new' failede: Unknown type '",
                                            type_name, "'.  Aborting."));
            return false;
          }
          // @CAO Make sure remainder of line is a single identifier?
          new_map[type_name](cur_line);
        }
        else if (command == "use") {
          std::string type_name = emp::string_pop_word(cur_line);
          // @CAO Make sure type exists!
          use_map[type_name](cur_line);
        }
        else if (command_map.find(command) != command_map.end()) {
          // Run this custom command.
          command_map[command](cur_line);
        }
        else {
          // We don't know this command... give an error and move on.
          std::stringstream ss;
          ss << "Unknown configuration command '" << command << "'. Ignoring." << std::endl;
          emp::NotifyError(ss.str());
        }
      }

      // Print out all accumulated warnings (if any).
      if (warnings.rdbuf()->in_avail()) {
        emp::NotifyWarning(warnings.str());
        warnings.str(std::string()); // Clear the warnings.
      }
      delay_warnings--;

      return true;
    }

    bool Read(std::string filename, bool error_on_missing_file=true) {
      std::ifstream in_file(filename);
      if (in_file.fail()) {
        std::stringstream ss;
        ss << "Unable to open config file '" << filename << "'. Ignoring." << std::endl;
        if (error_on_missing_file) emp::NotifyError(ss.str());
        return false;
      }
      bool success = Read(in_file);
      in_file.close();
      return success;
    }

    void AddNameSpace(Config & config, const std::string & namespace_name) {
      namespace_map[namespace_name] = &config;
    }

    void AddCommand(const std::string & command_name, std::function<bool(std::string)> command_fun) {
      // Give a warning if we are re-defining an existing command.
      if (command_map.find(command_name) != command_map.end()) {
        warnings << "Re-defining command '" << command_name << "'. Allowing." << std::endl;
        if (!delay_warnings) {
          emp::NotifyWarning(warnings.str());
          warnings.str(std::string()); // Clear the warnings.
        }
      }
      command_map[command_name] = command_fun;
    }

    void AddNewCallback(const std::string & type_name, std::function<bool(std::string)> new_fun) {
      // Give a warning if we are re-defining an existing command.
      if (new_map.find(type_name) != new_map.end()) {
        warnings << "Re-defining config type '" << type_name << "'. Allowing." << std::endl;
        if (!delay_warnings) {
          emp::NotifyWarning(warnings.str());
          warnings.str(std::string()); // Clear the warnings.
        }
      }
      new_map[type_name] = new_fun;
    }

    void AddUseCallback(const std::string & type_name, std::function<bool(std::string)> use_fun) {
      // Give a warning if we are re-defining an existing command.
      if (emp::Has(use_map, type_name)) {
        warnings << "Re-defining config type '" << type_name << "'. Allowing." << std::endl;
        if (!delay_warnings) {
          emp::NotifyWarning(warnings.str());
          warnings.str(std::string()); // Clear the warnings.
        }
      }
      use_map[type_name] = use_fun;
    }


    template <class MANAGED_TYPE>
    void AddManagedType(const std::string & type_keyword, const std::string & command_keyword,
                        std::function<bool(MANAGED_TYPE &, std::string)> fun_callback)
    {
      ConfigManager<MANAGED_TYPE> * new_manager = new ConfigManager<MANAGED_TYPE>(type_keyword, command_keyword, fun_callback);
      type_manager_map[type_keyword] = new_manager;

      AddCommand(command_keyword,
                 std::bind(&ConfigManager<MANAGED_TYPE>::CommandCallback, new_manager, _1) );
      AddNewCallback(type_keyword,
                     std::bind(&ConfigManager<MANAGED_TYPE>::NewObject, new_manager, _1) );
      AddUseCallback(type_keyword,
                     std::bind(&ConfigManager<MANAGED_TYPE>::UseObject, new_manager, _1) );
    }

    /// Access group_set using this method since it is protected
    emp::vector<ConfigGroup *> GetGroupSet(){
      return group_set;
    }

  };

}

// Below are macros that help build the config classes.

// Check that all of the commands are legal so that sensible errors can be produced.
// (legal commands convert to two arguments; illeagal ones stay as one, so second arg is error!)
#define EMP_CONFIG__ERROR_CHECK(CMD) EMP_GET_ARG(2, EMP_CONFIG__ARG_OKAY_ ## CMD, \
                                     static_assert(false, "Unknown Config option: " #CMD);, ~)
#define EMP_CONFIG__ARG_OKAY_VALUE(...) ~,
#define EMP_CONFIG__ARG_OKAY_CONST(...) ~,
#define EMP_CONFIG__ARG_OKAY_const(...) ~,
#define EMP_CONFIG__ARG_OKAY_GROUP(...) ~,
#define EMP_CONFIG__ARG_OKAY_ALIAS(...) ~,
#define EMP_CONFIG__ARG_OKAY_ ~,


// Macros to handle declaration of protected member variables.
// Note, unneeded macros defined to nothing, as is extra ending in '_' to allow trailing comma.
#define EMP_CONFIG__DECLARE(CMD) EMP_CONFIG__DECLARE_ ## CMD
#define EMP_CONFIG__DECLARE_VALUE(NAME, TYPE, DEFAULT, DESC) TYPE m_ ## NAME;
#define EMP_CONFIG__DECLARE_CONST(NAME, TYPE, DEFAULT, DESC)
#define EMP_CONFIG__DECLARE_const(NAME, TYPE, DEFAULT, DESC)
#define EMP_CONFIG__DECLARE_GROUP(NAME, DESC)
#define EMP_CONFIG__DECLARE_ALIAS(NAME)
#define EMP_CONFIG__DECLARE_

// Macros to handle construction of vars.
#define EMP_CONFIG__CONSTRUCT(CMD) EMP_CONFIG__CONSTRUCT_ ## CMD
#define EMP_CONFIG__CONSTRUCT_VALUE(NAME, TYPE, DEFAULT, DESC) , m_ ## NAME(DEFAULT)
#define EMP_CONFIG__CONSTRUCT_CONST(NAME, TYPE, DEFAULT, DESC)
#define EMP_CONFIG__CONSTRUCT_const(NAME, TYPE, DEFAULT, DESC)
#define EMP_CONFIG__CONSTRUCT_GROUP(NAME, DESC)
#define EMP_CONFIG__CONSTRUCT_ALIAS(NAME)
#define EMP_CONFIG__CONSTRUCT_

// Macros to initialize internal representation of variables.
#define EMP_CONFIG__INIT(CMD) EMP_CONFIG__INIT_ ## CMD
#define EMP_CONFIG__INIT_VALUE(NAME, TYPE, DEFAULT, DESC)                               \
  var_map[#NAME] = new tConfigEntry<TYPE>(#NAME, #TYPE, #DEFAULT, DESC, m_ ## NAME);    \
  GetActiveGroup()->Add(var_map[#NAME]);
#define EMP_CONFIG__INIT_CONST(NAME, TYPE, VALUE, DESC)                                 \
  var_map[#NAME] = new tConfigConstEntry<TYPE>(#NAME, #TYPE, #VALUE, DESC, VALUE);      \
  GetActiveGroup()->Add(var_map[#NAME]);
#define EMP_CONFIG__INIT_const(NAME, TYPE, VALUE, DESC)                                 \
  var_map[#NAME] = new tConfigConstEntry<TYPE>(#NAME, #TYPE, #VALUE, DESC, VALUE);      \
  GetActiveGroup()->Add(var_map[#NAME]);
#define EMP_CONFIG__INIT_GROUP(NAME, DESC)                                              \
  group_set.push_back(new ConfigGroup(#NAME, DESC));
#define EMP_CONFIG__INIT_ALIAS(NAME)                                                    \
  AddAlias(GetActiveEntry()->GetName(), #NAME);
#define EMP_CONFIG__INIT_

// Build Get and Set Accessors, as well as const check
#define EMP_CONFIG__ACCESS(CMD) EMP_CONFIG__ACCESS_ ## CMD
#define EMP_CONFIG__ACCESS_VALUE(NAME, TYPE, DEFAULT, DESC)                     \
  inline const TYPE & NAME() const { return m_ ## NAME; }                       \
  const TYPE & NAME(const TYPE & _in) { m_ ## NAME = _in; return m_ ## NAME; }  \
  bool NAME ## _is_const() const { return false; }
#define EMP_CONFIG__ACCESS_CONST(NAME, TYPE, VALUE, DESC)                       \
  constexpr static TYPE NAME() { return VALUE; }                                \
  TYPE NAME(const TYPE & _in) {                                                 \
    std::stringstream ss;                                                       \
    ss << "Trying to set const '" << #NAME << "'. Ignoring." << std::endl;      \
    emp::NotifyWarning(ss.str());                                               \
    return VALUE;                                                               \
  }                                                                             \
  bool NAME ## _is_const() const { return true; }
#define EMP_CONFIG__ACCESS_const(NAME, TYPE, VALUE, DESC) EMP_CONFIG__ACCESS_CONST(NAME, TYPE, VALUE, DESC)
#define EMP_CONFIG__ACCESS_GROUP(NAME, DESC)
#define EMP_CONFIG__ACCESS_ALIAS(NAME)
#define EMP_CONFIG__ACCESS_

#define EMP_BUILD_CONFIG(CLASS_NAME, ...) EMP_EXTEND_CONFIG(CLASS_NAME, emp::Config, __VA_ARGS__)

#define EMP_EXTEND_CONFIG(CLASS_NAME, BASE_NAME, ...)     \
  EMP_WRAP_EACH(EMP_CONFIG__ERROR_CHECK, __VA_ARGS__)     \
  class CLASS_NAME : public BASE_NAME {                   \
  protected:                                              \
    bool is_ ## CLASS_NAME;                               \
    EMP_WRAP_EACH(EMP_CONFIG__DECLARE, __VA_ARGS__)       \
  public:                                                 \
    CLASS_NAME() : is_ ## CLASS_NAME(true)                \
    EMP_WRAP_EACH(EMP_CONFIG__CONSTRUCT, __VA_ARGS__)     \
    {                                                     \
      class_names.push_back(#CLASS_NAME);                 \
      EMP_WRAP_EACH(EMP_CONFIG__INIT, __VA_ARGS__)        \
    }                                                     \
    EMP_WRAP_EACH(EMP_CONFIG__ACCESS, __VA_ARGS__)        \
  };

#endif
