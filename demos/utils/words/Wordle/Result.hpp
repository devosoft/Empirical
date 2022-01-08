#include "emp/base/error.hpp"
#include "emp/base/vector.hpp"

class Result {
public:
  enum result_t { NOWHERE, ELSEWHERE, HERE };

private:
  emp::vector<result_t> results;
  size_t id;

  static emp::vector<size_t> & GetMagnitudes(const size_t max_pos) {
    static emp::vector<size_t> magnitudes;
    if (magnitudes.size() <= max_pos) {
      magnitudes.resize(max_pos+1);
      size_t base = 1;
      for (auto & x : magnitudes) { x = base; base *= 3; }
    }
    return magnitudes;
  }

  // Assume that we have results, calculate the correct ID.
  void CalcID() {
    size_t base = 1;
    id = 0;
    for (result_t r : results) { id += static_cast<size_t>(r) * base; base *= 3; }
  }

  // Assume that we have an ID, calculate the correct results.
  void CalcResults() {
    emp::vector<size_t> & magnitudes = GetMagnitudes(results.size());
    size_t tmp_id = id;
    for (size_t i = 0; i < results.size(); ++i) {
      size_t cur_result = tmp_id / magnitudes[i];
      results[i] = static_cast<result_t>(cur_result);
      tmp_id -= cur_result * magnitudes[i];
    }
  }

  void FromString(const std::string & result_str) {
    results.resize(result_str.size());
    for (size_t i=0; i < results.size(); ++i) {
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
  Result(size_t num_results, size_t _id) : results(num_results), id(_id) { CalcResults(); }

  /// Create a result by a result vector.
  Result(const emp::vector<result_t> & _results) : results(_results) { CalcID(); }

  /// Create a result by a result string.
  Result(const std::string & result_str) { FromString(result_str); }

  /// Create a result by an guess and answer pair.
  Result(const std::string & guess, const std::string & answer) : results(guess.size()) {
    emp_assert(guess.size() == answer.size());
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

  size_t GetID() const { return id; }
  size_t GetSize() const { return results.size(); }
  size_t GetIDCap() const { return GetMagnitudes(results.size())[results.size()]; }

  result_t operator[](size_t id) const { return results[id]; }

  size_t size() const { return results.size(); }
};