/*
 *  This file is part of Empirical, https://github.com/devosoft/Empirical
 *  Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  date: 2022
*/
/**
 *  @file
 */

#ifndef DEMOS_UTILS_WORDS_WORDLE_RESULT_HPP_INCLUDE
#define DEMOS_UTILS_WORDS_WORDLE_RESULT_HPP_INCLUDE

#include <string>

#include "emp/base/array.hpp"
#include "emp/base/error.hpp"
#include "emp/bits/BitVector.hpp"
#include "emp/math/math.hpp"

template <size_t WORD_SIZE=5>
class Result {
public:
  enum PositionResult { NOWHERE, ELSEWHERE, HERE };
  static constexpr size_t NUM_IDS = emp::Pow(3, WORD_SIZE);

private:
  using results_t = emp::array<PositionResult, WORD_SIZE>;

  results_t results;
  size_t id;

  /// Return a result array where each index is an associated (unique) possible result set.
  static const results_t & LookupResult(size_t result_id) {
    static emp::array<results_t, NUM_IDS> result_array;
    static bool init = false;

    // If this is our first time requsting the result array, generate it.
    if (!init) {
      init = true;
      for (size_t id = 0; id < NUM_IDS; ++id) {
        size_t tmp_id = id;
        for (size_t pos = WORD_SIZE-1; pos < WORD_SIZE; --pos) {
          const size_t magnitude = emp::Pow(3, pos);
          const size_t cur_result = tmp_id / magnitude;
          result_array[id][pos] = static_cast<PositionResult>(cur_result);
          tmp_id -= cur_result * magnitude;
        }
      }
    }

    return result_array[result_id];
  }

  /// Assume that we have results, calculate the associated ID.
  void CalcID() {
    size_t base = 1;
    id = 0;
    for (PositionResult r : results) { id += static_cast<size_t>(r) * base; base *= 3; }
  }

  /// Assume that we have an ID, lookup the correct results.
  void CalcResults() { results = LookupResult(id); }

  /// Convert a results string of 'N's, 'E's, and 'W's into a Results object.
  void FromString(const std::string & result_str) {
    emp_assert(result_str.size() == WORD_SIZE);
    for (size_t i=0; i < WORD_SIZE; ++i) {
      switch (result_str[i]) {
      case 'N': case 'n': results[i] = NOWHERE;   break;
      case 'E': case 'e': results[i] = ELSEWHERE; break;
      case 'H': case 'h': results[i] = HERE;      break;
      default:
        emp_error("Invalid character in result string", result_str[i]);
      };
    }
  }

public:
  /// Create a result by id.
  Result(size_t _id) : id(_id) { CalcResults(); }

  /// Create a result by a result array.
  Result(const results_t & _results) : results(_results) { CalcID(); }

  /// Create a result by a result string.
  Result(const std::string & result_str) { FromString(result_str); }

  /// Create a result by an guess and answer pair.
  Result(const std::string & guess, const std::string & answer) {
    emp_assert(guess.size() == WORD_SIZE);
    emp_assert(answer.size() == WORD_SIZE);
    emp::BitVector used(answer.size());
    // Test perfect matches.
    for (size_t i = 0; i < guess.size(); ++i) {
      if (guess[i] == answer[i]) { results[i] = HERE; used.Set(i); }
    }
    // Test offset matches.
    for (size_t i = 0; i < guess.size(); ++i) {
      if (guess[i] == answer[i]) continue;            // already matched.
      bool found = false;
      for (size_t j = 0; j < answer.size(); ++j) {    // seek a match elsewhere in answer!
        if (!used.Has(j) && guess[i] == answer[j]) {
          results[i] = ELSEWHERE;                     // found letter elsewhere!
          used.Set(j);                                // make sure this letter is noted as used.
          found = true;
          break;                                      // move on to next letter; we found this one.
        }
      }
      if (!found) results[i] = NOWHERE;
    }
    CalcID();                                         // Now that we know the symbols, figure out the ID.
  }

  Result(const Result & result) = default;
  Result(Result && result) = default;

  Result & operator=(const std::string & result_str) { FromString(result_str); }
  Result & operator=(const Result & result) = default;
  Result & operator=(Result && result) = default;

  bool operator==(const Result & in) const { return id == in.id; }
  bool operator!=(const Result & in) const { return id != in.id; }
  bool operator< (const Result & in) const { return id <  in.id; }
  bool operator<=(const Result & in) const { return id <= in.id; }
  bool operator> (const Result & in) const { return id >  in.id; }
  bool operator>=(const Result & in) const { return id >= in.id; }

  size_t GetID() const { return id; }
  size_t GetSize() const { return WORD_SIZE; }
  size_t size() const { return WORD_SIZE; }

  PositionResult operator[](size_t id) const { return results[id]; }

  // Test if this result is valid for the given word.
  bool IsValid(const std::string & word) const {
    // Disallow letters marked "NOWHERE" that are subsequently marked "ELSEWHERE"
    // (other order is okay).
    for (size_t pos = 0; pos < WORD_SIZE-1; ++pos) {
      if (results[pos] == NOWHERE) {
        for (size_t pos2 = pos+1; pos2 < WORD_SIZE; ++pos2) {
          if (results[pos2] == ELSEWHERE && word[pos] == word[pos2]) return false;
        }
      }
    }

    return true;
  }

  std::string ToString(
    const std::string & here="H",
    const std::string & elsewhere="E",
    const std::string & nowhere="N"
  ) const {
    std::string out; // = emp::to_string(id, "-");
    for (auto x : results) {
      if (x == HERE) out += here;
      else if (x == ELSEWHERE) out += elsewhere;
      else if (x == NOWHERE) out += nowhere;
    }
    return out;
  }
};

#endif // #ifndef DEMOS_UTILS_WORDS_WORDLE_RESULT_HPP_INCLUDE
