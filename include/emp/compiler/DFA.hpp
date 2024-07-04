/*
 *  This file is part of Empirical, https://github.com/devosoft/Empirical
 *  Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  date: 2016-2024.
*/
/**
 *  @file
 *  @brief A Deterministic Finite Automata simulator.
 *  @note Status: BETA
 */

#ifndef EMP_COMPILER_DFA_HPP_INCLUDE
#define EMP_COMPILER_DFA_HPP_INCLUDE

#include <cstdint>
#include <limits>
#include <stddef.h>
#include <string>

#include "../base/array.hpp"
#include "../base/vector.hpp"
#include "../io/CPPFile.hpp"
#include "../tools/string_utils.hpp"

namespace emp {

  template <int NUM_SYMBOLS=128, typename STOP_TYPE=uint8_t>
  class tDFA {
  private:
    emp::vector< emp::array<int, NUM_SYMBOLS> > transitions;
    emp::vector< STOP_TYPE > stop_id;  // 0=not stop; other values for STOP return value.

    using this_t = tDFA<NUM_SYMBOLS, STOP_TYPE>;
  public:
    using stop_t = STOP_TYPE;

    /// How many states is this DFA using?
    size_t GetSize() const { return transitions.size(); }

    /// Add Additional empty states.
    void Resize(size_t new_size) {
      auto old_size = transitions.size();
      transitions.resize(new_size);
      stop_id.resize(new_size, 0);
      for (auto i = old_size; i < transitions.size(); i++) transitions[i].fill(-1);
    }

    /// Add a single new, empty state and return its position.
    size_t AddState() {
      size_t out_id = GetSize();
      Resize(out_id+1);
      return out_id;
    }

    /// Return an array of all transitions associated with a specified state.
    const emp::array<int, NUM_SYMBOLS> & GetTransitions(size_t from) const {
      return transitions[from];
    }

    /// Add a specific transition associated with an input symbol.
    void SetTransition(size_t from, size_t to, size_t sym) {
      emp_assert(from < transitions.size());
      emp_assert(to < transitions.size());
      emp_assert(sym < NUM_SYMBOLS);
      transitions[from][sym] = (int) to;
    }

    /// Set the stop value (no matter what it currently is)
    void SetStop(size_t state, stop_t stop_val=1) {
      emp_assert(state < transitions.size());
      stop_id[state] = stop_val;
    }

    /// Set the stop value only if it's higher than the current stop value.
    void AddStop(size_t state, stop_t stop_val=1) {
      emp_assert(state < transitions.size());
      // If we are given a stop value and its higher than our previous stop, use it!
      if (stop_val > stop_id[state]) stop_id[state] = stop_val;
    }

    /// Get the stop value associated with a state.
    stop_t GetStop(int state) const { return (state == -1) ? 0 : stop_id[(size_t)state]; }

    /// Test if a state is still valid.
    bool IsActive(int state) const { return state != -1; }

    /// Test if a state has a stop.
    bool IsStop(int state) const { return (state == -1) ? false : stop_id[(size_t)state]; }

    // If a size_t is passed in, it can't be -1...
    stop_t GetStop(size_t state) const { return stop_id[state]; }
    bool IsActive(size_t /* state */) const { return true; }
    bool IsStop(size_t state) const { return stop_id[state]; }

    /// Return the new state after a symbol occurs.
    int Next(int state, size_t sym) const {
      emp_assert(state >= -1 && state < (int) transitions.size());
      // emp_assert(sym >= 0 && sym < NUM_SYMBOLS, sym, (char) sym);
      return (state < 0 || sym >= NUM_SYMBOLS) ? -1 : transitions[(size_t)state][sym];
    }

    /// Return the new state after a series of symbols.
    int Next(int state, std::string sym_set) const {
      for (char x : sym_set) state = Next(state, (size_t) x);
      return state;
    }

    /// Determine if an entire series of symbols is valid.
    stop_t Test(const std::string & str) const {
      int out = Next(0, str);
      return GetStop(out);
    }

    /// Print details about this DFA.
    void Print(std::ostream & os=std::cout) const {
      os << "Num states = " << GetSize() << std::endl << "Stop IDs:";
      for (size_t i = 0; i < GetSize(); i++) if(IsStop(i)) os << " " << i;
      os << std::endl;

      for (size_t i = 0; i < transitions.size(); i++) {
        os << " " << i << " ->";
        for (size_t s = 0; s < NUM_SYMBOLS; s++) {
          if (transitions[i][s] == -1) continue;
          os << " " << to_literal((char) s) << ":" << transitions[i][s];
        }
        if (IsStop(i)) os << " [STOP=" << ((int) GetStop(i)) << "]";
        os << std::endl;
      }

    }

    // Print out this DFA as compilable C++ code.
    void WriteCPP(emp::CPPFile & file,
                  std::string object_name="AutoDFA"
                 ) const {
      file.Include("<array>");
      file.Include("<string>");

      file.AddCode("class ", object_name, " {")
          .AddCode("private:")
          .AddCode("  static constexpr size_t NUM_SYMBOLS=", NUM_SYMBOLS, ";")
          .AddCode("  static constexpr size_t NUM_STATES=", GetSize(), ";")
          .AddCode("  using row_t = std::array<int, NUM_SYMBOLS>;")
          .AddCode("  static constexpr std::array<row_t, NUM_STATES> table = {{");
      for (size_t state=0; state < GetSize(); ++state) {
        if (state) file.AddCode("    ,{");
        else       file.AddCode("    {");
        for (size_t symbol=0; symbol < NUM_SYMBOLS; ++symbol) {
          if (symbol) file.AppendCode(",");
          file.AppendCode(transitions[state][symbol]);
        }
        file.AppendCode("}");
      }
      file.AddCode("  }};")
          .AddCode("  static constexpr std::array<size_t, NUM_STATES> stop_id = {");
      for (size_t state=0; state < GetSize(); ++state) {
        if (state) file.AppendCode(",");
        file.AppendCode(static_cast<size_t>(stop_id[state]));
      }
      file.AddCode("  };");

      file.AddCode("public:")
          .AddCode("  static constexpr size_t size() { return ", GetSize(), "; }")
          .AddCode("  static constexpr size_t GetStop(int state) {")
          .AddCode("    return (state >= 0) ? stop_id[state] : 0;")
          .AddCode("  }")
          .AddCode("  static constexpr int GetNext(int state, size_t sym) {")
          .AddCode("    return state >= 0 ? table[(size_t)state][sym] : -1;")
          .AddCode("  }")
          .AddCode("  static constexpr int GetNext(int state, std::string syms) {")
          .AddCode("    for (char x : syms) state = GetNext(state, (size_t) x);")
          .AddCode("    return state;")
          .AddCode("  }")
          .AddCode("  static constexpr size_t Test(const std::string & str) {")
          .AddCode("    int out = GetNext(0, str);")
          .AddCode("    return GetStop(out);")
          .AddCode("  }")

          .AddCode("  ")
          .AddCode("};");
    }
  };

  /// Setup DFA to be a simple tDFA with the basic character set for symbols.
  using DFA = tDFA<128, uint8_t>;

}

#endif // #ifndef EMP_COMPILER_DFA_HPP_INCLUDE
