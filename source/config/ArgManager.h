//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//  A simple ArgManager tool for sythesizing command-line arguments,
//  URL query params, arguments and config files.


#ifndef EMP_CL_ARG_MANAGER_H
#define EMP_CL_ARG_MANAGER_H

#include <algorithm>
#include <string>
#include <vector>
#include <map>
#include <iterator>
#include <set>
#include <limits>
#include <numeric>
#include <cstdlib>
#include <optional>

#include "base/Ptr.h"
#include "base/vector.h"
#include "command_line.h"
#include "config.h"

namespace emp {

  /// A helper struct for ArgManager that specifies a single argument type.
  /// Note that the primary argument name is specified as the keys of the specs
  /// map constructor argument for ArgManager.
  /// * most_quota: at most how many words after the flag should be packed into
  ///   the argument pack? (enforced during parsing and on UseArg request)
  /// * least_quota: at least how many words after the flag should be packed
  ///   into the argument pack? (enforced on UseArg request)
  /// * aliases: what set of alternate flag names should trigger this argument? ///   (specify without - prepended)
  /// * callbaack: provide a callback function to process the argument
  /// * gobble_flags: should the argument pack accept flag words
  ///   (words that begin with -) instead of curtailing the argument pack
  ///   before the first encountered flag word.
  /// * flatten: should all argument packs collected for this key be
  ///   consolidated into a single argument pack?
  ///   (e.g., a single list of words instead of a list of lists of words).
  struct ArgSpec {

    size_t most_quota;
    size_t least_quota;
    std::string description;
    std::unordered_set<std::string> aliases;
    std::function<void(std::optional<emp::vector<std::string>>)> callback;
    bool gobble_flags;
    bool flatten;

    ArgSpec(
      const size_t quota_=0,
      const std::string description_="No description provided.",
      const std::unordered_set<std::string> aliases_=std::unordered_set<std::string>(),
      const std::function<void(std::optional<emp::vector<std::string>>)> callback_=nullptr,
      const bool gobble_flags_=false,
      const bool flatten_=false
    ) : ArgSpec(
          quota_,
          quota_,
          description_,
          aliases_,
          callback_,
          gobble_flags_,
          flatten_
        )
    { ; }

    ArgSpec(
      const size_t most_quota_,
      const size_t least_quota_,
      const std::string description_="No description provided.",
      const std::unordered_set<std::string> aliases_=std::unordered_set<std::string>(),
      const std::function<void(std::optional<emp::vector<std::string>>)> callback_=nullptr,
      const bool gobble_flags_=false,
      const bool flatten_=false
    ) : most_quota(most_quota_),
        least_quota(least_quota_),
        description(description_),
        aliases(aliases_),
        callback(callback_),
        gobble_flags(gobble_flags_),
        flatten(flatten_)
    { ; }

  };

  /// Manager for command line arguments and URL query params.
  class ArgManager {

  private:

    // the actual data collected
    std::multimap<std::string, emp::vector<std::string>> packs;

    // the specification for how to collect and dispense the data
    const std::unordered_map<std::string, ArgSpec> specs;

  public:
    // Convert input arguments to a vector of strings for easier processing.
    static emp::vector<std::string> args_to_strings(int argc, char* argv[]) {
      emp::vector<std::string> args;
      for (size_t i = 0; i < (size_t) argc; i++) {
        args.push_back(argv[i]);
      }
      return args;
    }

    // Use argument specifications to convert command line arguments
    // to argument packs.
    static std::multimap<std::string, emp::vector<std::string>> parse(
      emp::vector<std::string> args,
      const std::unordered_map<std::string, ArgSpec> & specs = std::unordered_map<std::string, ArgSpec>()
    ) {

      auto res = std::multimap<std::string, emp::vector<std::string>>();

      const auto alias_map = std::accumulate(
        std::begin(specs),
        std::end(specs),
        std::unordered_map<std::string, std::string>(),
        [](
          std::unordered_map<std::string, std::string> l,
          const std::pair<std::string, ArgSpec> & r
        ){
          l.insert({r.first, r.first});
          for(const auto & p : r.second.aliases) l.insert({p, r.first});
          return l;
        }
      );

      // check for duplicate aliases
      emp_assert([&](){
        return alias_map.size() == std::accumulate(
          std::begin(specs),
          std::end(specs),
          specs.size(),
          [](
            const size_t l,
            const std::pair<std::string, ArgSpec> & r
          ){
            return l + r.second.aliases.size();
          }
        );
      }(), "duplicate aliases detected");

      // lookup table with leading dashes stripped
      const emp::vector<std::string> deflagged = [&args](){
        auto res = args;
        for (size_t i = 0; i < args.size(); ++i) {
          const size_t dash_stop = args[i].find_first_not_of('-');
          if (dash_stop < args[i].size()) {
            res[i].erase(0, dash_stop);
          } else if (args[i].size() == 2) {
            // in POSIX, -- means treat subsequent words as literals
            // so we remove the -- and stop deflagging subsequent words
            res.erase(std::next(std::begin(res),i));
            args.erase(std::next(std::begin(args),i));
            break;
          }
          // -, ---, ----, etc. left in place and treated as non-flags
        }
        return res;
      }();

      // if word is a valid command or alias for a command,
      // return the deflagged, dealiased command
      // otherwise, it's a positional command
      auto parse_alias = [deflagged, args, alias_map, specs](size_t i) {
        return alias_map.count(deflagged[i]) ?
          alias_map.find(deflagged[i])->second : (
            alias_map.count("_positional")
            && (
              deflagged[i] == args[i]
              || specs.find("_positional")->second.gobble_flags
            )
            ? "_positional" : "_unknown"
          );
      };

      for(size_t i = 0; i < args.size(); ++i) {

        const std::string & command = parse_alias(i);

        // if command is unknown
        // and user hasn't provided an ArgSpec for unknown commands
        if (command == "_unknown" && !specs.count("_unknown")) {
          res.insert({
              "_unknown",
              { args[i] }
          });
          continue;
        }

        const ArgSpec & spec = specs.find(parse_alias(i))->second;

        // fast forward to grab all the words for this argument pack
        size_t j;
        for (
          j = i;
          j < args.size()
          && j - i < spec.most_quota
          && (
            spec.gobble_flags
            || ! (j + 1 < args.size())
            || deflagged[j+1] == args[j+1]
          );
          ++j
        );

        // store the argument pack
        res.insert(
          {
            command,
            emp::vector<std::string>(
              std::next(
                std::begin(args),
                command == "_positional" || command == "_unknown" ? i : i+1
              ),
              j+1 < args.size() ? std::next(std::begin(args), j+1) : std::end(args)
            )
          }
        );

        i = j;

      }

      return res;

    }

    /// Make specs for builtin commands, including any config adjustment args.
    static std::unordered_map<std::string, ArgSpec> make_builtin_specs(
      const emp::Ptr<Config> config=nullptr
    ) {

      std::unordered_map<std::string, ArgSpec> res({
        {"_positional", ArgSpec(
          std::numeric_limits<size_t>::max(),
          1,
          "Positional arguments.",
          {},
          nullptr,
          false,
          true
        )},
        {"_unknown", ArgSpec(
          std::numeric_limits<size_t>::max(),
          1,
          "Unknown arguments.",
          {},
          [](std::optional<emp::vector<std::string>> res){
            if (res) {
              std::cerr << "UNKNOWN | _unknown:";
              for(const auto & v : *res) std::cerr << " " << v;
              std::cerr << std::endl;
              std::exit(EXIT_FAILURE);
            }
          }
        )},
        {"help", ArgSpec(0, "Print help information.", {"h"})},
        {"gen", ArgSpec(
          1,
          "Generate configuration file.",
          {},
          [config](std::optional<emp::vector<std::string>> res){
            if (res && config) {
              const std::string cfg_file = res->front();
              std::cout << "Generating new config file: " << cfg_file << std::endl;
              config->Write(cfg_file);
            }
          }
        )},
        {"make-const", ArgSpec(
          1,
          "Generate const version of macros file.",
          {},
          [config](std::optional<emp::vector<std::string>> res){
            if (res && config) {
              const std::string macro_file = res->front();
              std::cout << "Generating new macros file: " << macro_file << std::endl;
              config->WriteMacros(macro_file, true);
            }
          }
        )}
      });

      if (config) {
        for (const auto & e : *config) {
          const auto & entry = e.second;
          res.insert({
            entry->GetName(),
            ArgSpec(
              1,
              emp::to_string(
                entry->GetDescription(),
                " (type=", entry->GetType(),
                "; default=", entry->GetDefault(), ')'
              ),
              {},
              [config, entry](std::optional<emp::vector<std::string>> res){
                if (res && config) {
                  config->Set(entry->GetName(), res->front());
                }
              }
            )
          });
        }
      }

      return res;

    }

    /// Constructor for raw command line arguments.
    /// This constructor is first in the constructor daisy chain.
    ArgManager(
      int argc,
      char* argv[],
      const std::unordered_map<std::string, ArgSpec> & specs_ = make_builtin_specs()
    ) : ArgManager(
      ArgManager::args_to_strings(argc, argv),
      specs_
    ) { ; }

    // Constructor for command line arguments converted to vector of string.
    /// This constructor is second in the constructor daisy chain.
    ArgManager(
      const emp::vector<std::string> args,
      const std::unordered_map<std::string, ArgSpec> & specs_ = make_builtin_specs()
    ) : ArgManager(
      ArgManager::parse(args, specs_),
      specs_
    ) { ; }

    /// Constructor that bypasses command line argument parsing where argument
    /// packs are provided directly, e.g., for use with URL query params.
    /// This constructor is last in the constructor daisy chain.
    ArgManager(
      const std::multimap<std::string, emp::vector<std::string>> & packs_,
      const std::unordered_map<std::string, ArgSpec> & specs_ = make_builtin_specs()
    ) : packs(packs_), specs(specs_) {

      // flatten any argument packs with flatten specified
      for (auto & [n, s] : specs) {
        if (s.flatten && packs.count(n)) {
          emp::vector<std::string> flat = std::accumulate(
            packs.equal_range(n).first,
            packs.equal_range(n).second,
            emp::vector<std::string>(),
            [](
              emp::vector<std::string> l,
              const std::pair<std::string, emp::vector<std::string>> & r
            ){
              l.insert(std::end(l), std::begin(r.second), std::end(r.second));
              return l;
            }
          );
          packs.erase(packs.equal_range(n).first, packs.equal_range(n).second);
          packs.insert({n, flat});
        }
      }
    }

    ~ArgManager() { ; }

    /// Trigger the callback (if specified) for an argument,
    /// consuming an argument pack if available and callback present.
    /// If no argpack is available, the callback is triggered with nullopt.
    /// Return true if an argument pack was consumed, otherwise false.
    bool CallbackArg(const std::string & name) {

      if (specs.count(name) && specs.find(name)->second.callback) {
        const auto res = UseArg(name);
        specs.find(name)->second.callback(res);
        return (bool) res;
      }

      return false;

    }

    /// Trigger all arguments with callbacks until all pertinent argument
    /// packs are consumed.
    void UseCallbacks() {

      for (const auto &[name, spec]: specs) {
        while (CallbackArg(name));
      }

    }

    /// UseArg consumes an argument pack accessed by a certain name.
    std::optional<emp::vector<std::string>> UseArg(const std::string & name) {

      const auto res = [this, name]()
        -> std::optional<emp::vector<std::string>> {

        if (!packs.count(name)) return std::nullopt;

        const auto & pack = packs.lower_bound(name)->second;

        if (specs.count(name)) {
          const auto & spec = specs.find(name)->second;
          return (
            spec.least_quota <= pack.size() && pack.size() <= spec.most_quota
          ) ? std::make_optional(pack) : std::nullopt;
        } else {
          return std::make_optional(pack);
        }

      }();

      if (res) packs.erase(packs.lower_bound(name));

      return res;

    }

    /// ViewArg provides, but doesn't comsume,
    /// all argument packs under a certain name.
    emp::vector<emp::vector<std::string>> ViewArg(const std::string & name) const {

      emp::vector<emp::vector<std::string>> res;

      const auto range = packs.equal_range(name);
      for (auto it = range.first; it != range.second; ++it) {
        res.push_back(it->second);
      }

      return res;

    }

    /// Process builtin commands.
    /// Return bool for "should program proceed" (i.e., true=continue, false=exit).
    bool ProcessBuiltin(
      const emp::Ptr<Config> config=nullptr,
      std::ostream & os=std::cout
    ) {

      if (UseArg("help")) {
        PrintHelp(os);
        return false;
      }

      if (const auto res = ViewArg("_unknown"); res.size()) {
        PrintDiagnostic(os);
        PrintHelp(os);
        return false;
      }

      bool proceed = true;

      // Apply config arguments to Config object.
      if (config) {
        for (auto e : *config) {
          const auto entry = e.second;
          if (const auto res = UseArg(entry->GetName()); res) {
            config->Set(entry->GetName(), res->front());
          }
        }
      }

      if (const auto res = UseArg("gen"); res && config) {
        const std::string cfg_file = res->front();
        os << "Generating new config file: " << cfg_file << std::endl;
        config->Write(cfg_file);
        proceed = false;
      }

      if (const auto res = UseArg("make-const"); res && config)  {
        const std::string macro_file = res->front();
        os << "Generating new macros file: " << macro_file << std::endl;
        config->WriteMacros(macro_file, true);
        proceed = false;
      }

      return proceed;

    }

    /// Print the current state of the ArgManager;
    /// provide diagnostic hints about argument packs remaining.
    void PrintDiagnostic(std::ostream & os=std::cout) const {

      for (const auto & [name, pack] : packs) {
        if (name == "_unknown") {
          os << "UNKNOWN | ";
        } else if (
          specs.count(name)
          && specs.find(name)->second.least_quota <= pack.size()
          && specs.find(name)->second.most_quota >= pack.size()
        ) {
          os << "UNMET QUOTA | ";
        } else {
          os << "UNUSED | ";
        }
        os << name << ":";
        for (const auto & v : pack) {
          os << " " << v;
        }
        os << std::endl;
      }

    }

    /// Print information about all known argument types and what they're for;
    /// make pretty.
    void PrintHelp(std::ostream & os=std::cerr) const {

      os << "Usage:" << std::endl;
      // print arguments in alphabetical order
      for (
        const auto & [name, spec]
        : std::map<std::string,ArgSpec>(std::begin(specs), std::end(specs))
      ) {
        if (name != "_unknown" && name != "_positional") os << "-";
        os << name;
        for (const auto & alias : spec.aliases) os << " -" << alias;
        if (spec.least_quota == spec.most_quota) {
          os << " [ quota = " << spec.most_quota << " ]";
        } else {
          os << " [ " << spec.least_quota << " <= quota <= "
            << spec.most_quota << " ]";
        }
        os << std::endl
          << "   | "
          << spec.description
          << std::endl;

      }

    }

    /// Test if there are any unused argument packs,
    /// and if so, output an error message.
    /// Returns true if there are any unused argument packs, false otherwise.
    bool HasUnused(std::ostream & os=std::cerr) const {
      if (packs.size()) {
        PrintDiagnostic(os);
        PrintHelp(os);
        return true;
      }
      return false;
    }

  };

  namespace cl {

    /// A simple class to manage command-line arguments that were passed in.
    /// Derived from emp::vector<std::string>, but with added functionality for argument handling.
    class ArgManager : public emp::vector<std::string> {
    private:
      using parent_t = emp::vector<std::string>;
      emp::vector<std::string> arg_names;
      emp::vector<std::string> arg_descs;

    public:
      ArgManager() : parent_t(), arg_names(), arg_descs() { ; }
      ArgManager(int argc, char* argv[])
       : parent_t(args_to_strings(argc, argv)), arg_names(), arg_descs() { ; }
      ~ArgManager() { ; }

      /// UseArg takes a name, a variable and an optional description.  If the name exists,
      /// it uses the next argument to change the value of the variable.
      /// Return 1 if found, 0 if not found, and -1 if error (no value provided)
      template <typename T>
      int UseArg(const std::string & name, T & var, const std::string & desc="") {
        arg_names.push_back(name);
        arg_descs.push_back(desc);
        return use_arg_value(*this, name, var);
      }

      /// UseArg can also take a config object and a name, and use the argument to set the
      /// config object.
      int UseArg(const std::string & name, Config & config, const std::string & cfg_name,
                 const std::string & desc="") {
        arg_names.push_back(name);
        arg_descs.push_back(desc);
        std::string var;
        bool rv = use_arg_value(*this, name, var);
        if (rv==1) config.Set(cfg_name, var);
        return rv;
      }

      /// UseFlag takes a name and an optional description.  If the name exists, return true,
      /// otherwise return false.
      bool UseFlag(const std::string & name, const std::string & desc="") {
        arg_names.push_back(name);
        arg_descs.push_back(desc);
        return use_arg(*this, name);
      }

      /// Print information about all known argument types and what they're for; make pretty.
      void PrintHelp(std::ostream & os) const {
        size_t max_name_size = 0;
        for (const auto & name : arg_names) {
          if (max_name_size < name.size()) max_name_size = name.size();
        }
        for (size_t i = 0; i < arg_names.size(); i++) {
          os << arg_names[i]
             << std::string(max_name_size + 1 - arg_names[i].size(), ' ')
             << arg_descs[i]
             << std::endl;
        }
      }

      /// Test if there are any unprocessed arguments, and if so, output an error.
      bool HasUnknown(std::ostream & os=std::cerr) const {
        if (size() > 1) {
          os << "Unknown args:";
          for (size_t i = 1; i < size(); i++) os << " " << (*this)[i];
          os << std::endl;
          PrintHelp(os);
          return true;
        }
        return false;
      }

      /// Leaving TestUnknown for backward compatability; returns opposite of HasUnknown().
      bool TestUnknown(std::ostream & os=std::cerr) const { return !HasUnknown(os); }

      /// Convert settings from a configure object to command-line arguments.
      /// Return bool for "should program proceed" (i.e., true=continue, false=exit).
      bool ProcessConfigOptions(Config & config, std::ostream & os,
                                const std::string & cfg_file="",
                                const std::string & macro_file="")
      {
        // Scan through the config object to generate command line flags for each setting.
        for (auto e : config) {
          auto entry = e.second;
          std::string desc = emp::to_string( entry->GetDescription(),
                                             " (type=", entry->GetType(),
                                             "; default=", entry->GetDefault(), ')' );
          UseArg(to_string('-', entry->GetName()), config, entry->GetName(), desc);
        }

        // Determine if we're using any special options for comman line flags.
        bool print_help    = UseFlag("--help", "Print help information.");
        bool create_config = cfg_file.size() && UseFlag("--gen", "Generate configuration file.");
        bool const_macros  = macro_file.size() && UseFlag("--make-const", "Generate const version of macros file.");

        if (print_help)    { PrintHelp(os); return false; }
        if (create_config) {
          os << "Generating new config file: " << cfg_file << std::endl;
          config.Write(cfg_file);
          return false;
        }
        if (const_macros)  {
          os << "Generating new macros file: " << macro_file << std::endl;
          config.WriteMacros(macro_file, true);
          return false;
        }

        return true;
      }

    };


  }
}

#endif
