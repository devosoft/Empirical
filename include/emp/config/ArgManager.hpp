/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019
 *
 *  @file ArgManager.hpp
 *  @brief A tool for sythesizing command-line arguments, URL query params, and config files.
 *  @note Status: BETA
 */

#ifndef EMP_CONFIG_ARGMANAGER_HPP_INCLUDE
#define EMP_CONFIG_ARGMANAGER_HPP_INCLUDE

#include <algorithm>
#include <cstdlib>
#include <iterator>
#include <limits>
#include <map>
#include <numeric>
#include <set>
#include <string>
#include <vector>

#include "../base/optional.hpp"
#include "../base/Ptr.hpp"
#include "../base/vector.hpp"

#include "command_line.hpp"
#include "config.hpp"

namespace emp {

  /// A helper struct for ArgManager that specifies a single argument type.
  /// Note that the primary argument name is specified as the keys of the specs
  /// map constructor argument for ArgManager.

  struct ArgSpec {

    /// Max words after flag to be used (enforced during parsing and on UseArg request)
    size_t most_quota;

    /// Minimum words after flag required (enforced during parsing and on UseArg request)
    size_t least_quota;

    /// User-level description of this flag.
    std::string description;

    /// Set of alternate flag names that will trigger this flag
    std::unordered_set<std::string> aliases;

    /// Function that can process this flag and its arguments.
    std::function<void(emp::optional<emp::vector<std::string>>)> callback;

    /// Should this flag collect subsequent flags (that begin with -) as arguments?
    bool gobble_flags;

    /// If this flag is used multiple times, should we combine all argument packs?
    ///   (e.g., a single list of words instead of a list of lists of words).
    bool flatten;

    ArgSpec(
      const size_t quota_=0,
      const std::string description_="No description provided.",
      const std::unordered_set<std::string> aliases_=std::unordered_set<std::string>(),
      const std::function<void(emp::optional<emp::vector<std::string>>)> callback_=nullptr,
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
      const std::function<void(emp::optional<emp::vector<std::string>>)> callback_=nullptr,
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

  // compares member variables of lhs and rhs, ignoring callback functions
  // useful for tests
  bool operator==(const ArgSpec& lhs, const ArgSpec& rhs) {
    return std::tuple{
      lhs.most_quota,
      lhs.least_quota,
      lhs.description,
      lhs.aliases,
      lhs.gobble_flags,
      lhs.flatten
    } == std::tuple{
      rhs.most_quota,
      rhs.least_quota,
      rhs.description,
      rhs.aliases,
      rhs.gobble_flags,
      rhs.flatten
    };
  }

  /// Manager for command line arguments and URL query params.
  class ArgManager {

  public:
    using pack_t = emp::vector<std::string>;
    using pack_map_t = std::multimap<std::string, pack_t>;
    using spec_map_t = std::unordered_map<std::string, ArgSpec>;

  private:
    // the actual data collected
    pack_map_t packs;

    // the specification for how to collect and dispense the data
    const spec_map_t specs;

  public:
    /// Convert input arguments to a vector of strings for easier processing.
    static pack_t args_to_strings(int argc, char* argv[]) {
      pack_t args;
      for (size_t i = 0; i < (size_t) argc; i++) {
        args.push_back(argv[i]);
      }
      return args;
    }

    /// Use argument specifications to convert command line arguments
    /// to argument packs.
    static pack_map_t parse(
      pack_t args,
      const spec_map_t & specs = spec_map_t()
    ) {

      auto res = pack_map_t();

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
      emp_assert(alias_map.size() == std::accumulate(
        std::begin(specs),
        std::end(specs),
        specs.size(),
        [](const size_t l, const std::pair<std::string, ArgSpec> & r){
          return l + r.second.aliases.size();
        }
      ), "duplicate aliases detected");

      // lookup table with leading dashes stripped
      // this is an immediately-invoked lambda
      const pack_t deflagged = [&args](){
        auto res = args;
        for (size_t i = 0; i < args.size(); ++i) {

          const size_t dash_stop = args[i].find_first_not_of('-');
          if (dash_stop == 0) {
            // nop
          } else if (dash_stop < args[i].size()) {
            res[i].erase(0, dash_stop); // remove initial dash
          } else if (args[i].size() == 2) {
            // in POSIX, -- means treat subsequent words as literals
            // so we remove the -- and stop deflagging subsequent words
            res.erase(std::next(std::begin(res),i));
            args.erase(std::next(std::begin(args),i));
            break;
          }
          // " ", -, ---, ----, etc. left in place and treated as non-flags

        }
        return res;
      }();

      // If word is a valid command or alias for a command,
      // return the deflagged, dealiased command...
      // otherwise, it's a positional command.
      // In this context, positional commands are options that take
      // option-arguments
      auto parse_alias = [deflagged, args, alias_map, specs](const size_t i) {
        const std::string deflag = deflagged[i];

        // the whole deflagged version is an alias
        // and there were leading hyphens that WERE removed then, return that
        if ( alias_map.count(deflag) && args[i] != deflag ) {
          return pack_t{alias_map.find(deflag)->second};
        }

        pack_t commands;
        // since it might be a concatenation of single-letter commands,
        // we must go through it letter by letter
        // try looking at each of the characters
        for (const char ch : deflag) {
          if (alias_map.count( std::string{ch} )) {

            // check that the command does not take arguments
            if (
              specs.find(
                alias_map.find(std::string{ch})->second
              )->second.most_quota != 0
            ) {
              // put strung-together commands that take arguments
              // into _invalid
              return pack_t{"_invalid"};
            };

            commands.push_back(alias_map.find(
              std::string{ch}
            )->second);

          } else {
            // found a bad letter! abort and return positional or unknown
            if (
              alias_map.count("_positional") && (
                deflag == args[i]
                || specs.find("_positional")->second.gobble_flags
              )
            ){
              return pack_t{"_positional"};
            } else {
              return pack_t{"_unknown"};
            }
          }
        }

        // found all good letters
        return commands;
      };

      if (!args.size()) return res;
      res.insert({"_command", {args[0]}});

      for(size_t i = 1; i < args.size(); ++i) {

        // there *could* be multiple commands contained
        // if the user passed something a la tar -czvf
        // so we loop through them one-by-one
        const pack_t & commands = parse_alias(i);

        for (const auto & command : commands) {

          // if command is unknown
          // and user hasn't provided an ArgSpec for unknown commands
          if (command == "_unknown" && !specs.count("_unknown")) {
            res.insert({
                "_unknown",
                { args[i] }
            });
            continue;
          }
          // if command is unknown
          // and user hasn't provided an ArgSpec for invalid commands
          if (command == "_invalid" && !specs.count("_invalid")) {
            res.insert({
                "_invalid",
                { args[i] }
            });
            continue;
          }

          const ArgSpec & spec = specs.find(command)->second;

          // fast forward to grab all the words for this argument pack
          size_t j;
          for (
            j = i;
            j < args.size()
              && j - i < spec.most_quota
              && (
                spec.gobble_flags
                || !( j+1 < args.size() )
                || deflagged[j+1] == args[j+1]
            );
            ++j
          );

          // store the argument pack
          res.insert({
              command,
              pack_t(
                std::next(
                  std::begin(args),
                  command == "_positional"
                    || command == "_unknown"
                    || command == "_invalid"
                  ? i : i+1
                ),
                j+1 < args.size() ? std::next(std::begin(args), j+1) : std::end(args)
              )
          });
          i = j;

        }

      }

      return res;

    }

    /// Make specs for builtin commands, including any config adjustment args.
    static spec_map_t make_builtin_specs(
      const emp::Ptr<Config> config=nullptr
    ) {

      spec_map_t res({
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
          [](emp::optional<pack_t> res){
            if (res) {
              std::cerr << "UNKNOWN | _unknown:";
              for(const auto & v : *res) std::cerr << " " << v;
              std::cerr << std::endl;
              std::exit(EXIT_FAILURE);
            }
          }
        )},
        {"_command", ArgSpec(
          1,
          1,
          "Command name.",
          {},
          [](emp::optional<emp::vector<std::string>> /* res */ ){ /*no-op*/ }
        )},
        {"help", ArgSpec(0, "Print help information.", {"h"})},
        {"gen", ArgSpec(
          1,
          "Generate configuration file.",
          {},
          [config](emp::optional<pack_t> res){
            if (res && config) {
              const std::string cfg_file = res->front();
              std::cout << "Generating new config file: " << cfg_file << std::endl;
              config->Write(cfg_file);
            }
          }
        )}
        // @CAO: Removing "make-const" as a default until we have it working properly.
        //,
        // {"make-const", ArgSpec(
        //   1,
        //   "Generate const version of macros file.",
        //   {},
        //   [config](emp::optional<pack_t> res){
        //     if (res && config) {
        //       const std::string macro_file = res->front();
        //       std::cout << "Generating new macros file: " << macro_file << std::endl;
        //       config->WriteMacros(macro_file, true);
        //     }
        //   }
        // )}
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
              [config, entry](emp::optional<pack_t> res){
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
      const spec_map_t & specs_ = make_builtin_specs()
    ) : ArgManager(
      ArgManager::args_to_strings(argc, argv),
      specs_
    ) { ; }

    // Constructor for command line arguments converted to vector of string.
    /// This constructor is second in the constructor daisy chain.
    ArgManager(
      const pack_t args,
      const spec_map_t & specs_ = make_builtin_specs()
    ) : ArgManager(
      ArgManager::parse(args, DealiasSpecs(specs_)),
      specs_
    ) { ; }

    /// Constructor that bypasses command line argument parsing where argument
    /// packs are provided directly, e.g., for use with URL query params.
    /// This constructor is last in the constructor daisy chain.
    ArgManager(
      const pack_map_t & packs_,
      const spec_map_t & specs_ = make_builtin_specs()
    ) : packs(packs_), specs(DealiasSpecs(specs_)) {

      // Flatten any argument packs with `flatten` specified; move into packs.
      for (auto & [n, s] : specs) {
        if (s.flatten && packs.count(n)) {
          pack_t flat = std::accumulate(
            packs.equal_range(n).first,
            packs.equal_range(n).second,
            pack_t(),
            [](
              pack_t l,
              const std::pair<std::string, pack_t> & r
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
    emp::optional<pack_t> UseArg(const std::string & name) {

      const auto res = [this, name]()
        -> emp::optional<pack_t> {

        if (!packs.count(name)) return std::nullopt;

        const auto & pack = packs.lower_bound(name)->second;

        if (specs.count(name)) {
          const auto & spec = specs.find(name)->second;
          return (
            spec.least_quota <= pack.size() && pack.size() <= spec.most_quota
          ) ? emp::make_optional(pack) : std::nullopt;
        } else {
          return emp::make_optional(pack);
        }

      }();

      if (res) packs.erase(packs.lower_bound(name));

      return res;

    }

    /// ViewArg provides, but doesn't comsume,
    /// all argument packs under a certain name.
    emp::vector<pack_t> ViewArg(const std::string & name) const {

      emp::vector<pack_t> res;

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

      UseArg("_command");

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

    // Argspecs with a string including flags separated by '|' are turned
    // into one argspec with a list of aliases
    // example input: specs[“—help|-h”] = ArgSpec a with a.aliases = {}
    // example output: specs[“—help”] = ArgSpecs b with b.aliases = {“-h”}
    static spec_map_t DealiasSpecs(spec_map_t inSpecsMap) {
      spec_map_t outSpecs;

      //convert each Argspec
      for (auto iSpec : inSpecsMap) {
        std::string flags = iSpec.first;
        std::vector <std::string> aliases = slice(flags, '|');
        std::string mainFlag = aliases[0];
        aliases.erase(aliases.begin());

        //set aliases
        ArgSpec oSpec = iSpec.second;
        for(auto iAlias : aliases) {
          oSpec.aliases.insert(iAlias);
        }

        outSpecs.emplace(mainFlag, oSpec);
      }

      return outSpecs;
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

#endif // #ifndef EMP_CONFIG_ARGMANAGER_HPP_INCLUDE
