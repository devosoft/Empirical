/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2022
 *
 *  @file Wordle-simple.cpp
 *
 *  This version of Wordle is a bit simpler than it should be; it does not handle double letters
 *  correctly.
 */

#include <cmath>
#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>

#include "../../../include/emp/base/Ptr.hpp"
#include "../../../include/emp/base/vector.hpp"
#include "../../../include/emp/bits/BitSet.hpp"
#include "../../../include/emp/bits/BitVector.hpp"
#include "../../../include/emp/config/command_line.hpp"
#include "../../../include/emp/datastructs/map_utils.hpp"
#include "../../../include/emp/datastructs/vector_utils.hpp"
#include "../../../include/emp/io/File.hpp"
#include "../../../include/emp/tools/string_utils.hpp"

enum class Result { NOWHERE=0, ELSEWHERE, HERE };

/// A collection of results for a whole word.
struct ResultSet {
  emp::vector<Result> results;

  static const emp::vector<size_t> & PlaceValues(const size_t num_results) {
    static emp::vector<size_t> place_values;
    if (place_values.size() == 0) {
      place_values.resize(num_results);
      size_t value = 1;
      for (size_t i = 0; i < num_results; ++i) {
        place_values[i] = value;
        value *= 3;
      }
    }
    return place_values;
  }

  ResultSet(const emp::vector<Result> & in) : results(in) { }
  ResultSet(size_t size, size_t id) : results(size) {
    emp::vector<size_t> place_values = PlaceValues(results.size());
    for (size_t i = results.size()-1; i < results.size(); --i) {
      if (id > place_values[i]) {
        size_t value = id / place_values[i];
        results[i] = (Result) value;
        id -= value * place_values[i];
      }
    }
  }
  ResultSet(const ResultSet &) = default;

  size_t ToID() {
    emp::vector<size_t> place_values = PlaceValues(results.size());
    size_t id = 0;
    for (size_t i = 0; i < results.size(); ++i) {
      id += place_values[i] * (size_t) results[i];
    }
    return id;
  }
};

// A clue is a given letter, position, and result
struct Clue {
  emp::BitVector words;  // IDs of words consistent with this clue.
};

// All of the clues for a given position.
struct PositionClues {
  std::array<Clue, 26> nowhere;
  std::array<Clue, 26> elsewhere;
  std::array<Clue, 26> here;

  void SetNumWords(size_t num_words) {
    for (auto & x : nowhere) x.words.resize(num_words);
    for (auto & x : elsewhere) x.words.resize(num_words);
    for (auto & x : here) x.words.resize(num_words);
  }
};

// Trying to build a full tree of solutions...
struct SolveState {
  emp::BitVector words;
};

struct WordData {
  std::string word;
  emp::BitSet<26> letters;
  size_t max_options = 0;     // Maximum number of word options after used as a guess.
  double ave_options = 0.0;   // Average number of options after used as a guess.
  double entropy = 0.0;       // What is the entropy (and thus information gained) for this choice?
  bool is_active = false;

  WordData(const std::string & in_word) : word(in_word) {
    for (char x : word) letters.Set(x - 'a');
  }
};

class WordSet {
private:
  size_t word_length;
  emp::vector<WordData> words;
  emp::vector<PositionClues> clues;                // A PositionClues object for each position.
  std::unordered_map<std::string, size_t> pos_map; // Map of words to their position ids.
  emp::BitVector start_options;                    // Current options.
  size_t start_count;                              // Count of start options (cached)

  bool verbose = true;

  // Get the ID (0-26) associated with a letter.
  size_t ID(char letter) {
    emp_assert(letter >= 'a' && letter <= 'z');
    return static_cast<size_t>(letter - 'a');
  }

  char LET(size_t id) {
    emp_assert(id < 26);
    return (char) (id + 'a');
  }

public:
  WordSet(size_t length=5) : word_length(length) { }

  void AddWord(std::string & in_word) {
    size_t id = words.size();
    pos_map[in_word] = id;
    words.emplace_back(in_word);
  }

  void Load(std::istream & is, std::ostream & os) {
    // Load in all of the words.
    std::string in_word;
    size_t wrong_size_count = 0;
    size_t invalid_char_count = 0;
    size_t dup_count = 0;
    while (is) {
      is >> in_word;
      // Only keep words of the correct size and all lowercase.
      if (in_word.size() != word_length) { wrong_size_count++; continue; }
      if (!emp::is_lower(in_word)) { invalid_char_count++; continue; }
      if (emp::Has(pos_map, in_word)) { dup_count++; continue; }
      AddWord(in_word);
    }

    if (wrong_size_count) {
      std::cerr << "Warning: eliminated " << wrong_size_count << " words of the wrong size."
                << std::endl;
    }
    if (invalid_char_count) {
      std::cerr << "Warning: eliminated " << invalid_char_count << " words with invalid characters."
                << std::endl;
    }
    if (dup_count) {
      std::cerr << "Warning: eliminated " << dup_count << " words that were duplicates."
                << std::endl;
    }

    if (verbose) std::cerr << "Loaded " << words.size() << " valid words." << std::endl;
  }

  void ResetOptions() {
    start_count = words.size();
    start_options.resize(start_count);
    start_options.SetAll();
  }

  // Once the words are loaded, Preprocess will collect info.
  void Preprocess() {
    // Setup all clue info to know the number of words.
    clues.resize(word_length);
    for (auto & x : clues) x.SetNumWords(words.size());

    // Loop through each word, indicating which clues it is consistent with.
    for (size_t word_id = 0; word_id < words.size(); ++word_id) {
      const std::string & word = words[word_id].word;

      // Figure out which letters are in this word.
      emp::BitSet<26> letters = words[word_id].letters;

      // Now figure out what clues it is consistent with.
      for (size_t pos=0; pos < word.size(); ++pos) {
        const char cur_letter = word[pos];
        // Incorrect letter for alternatives at this position.
        for (size_t letter_id = 0; letter_id < 26; ++letter_id) {
          if (letter_id == ID(cur_letter)) {                     // Letter is HERE.
            clues[pos].here[letter_id].words.Set(word_id);
          } else if (letters.Has(letter_id)) {                   // Letter is ELSEWHERE
            clues[pos].elsewhere[letter_id].words.Set(word_id);
          } else {                                               // Letter is NOT IN WORD
            clues[pos].nowhere[letter_id].words.Set(word_id);
          }
        }
      }
    }

    ResetOptions();
  }

  /// Limit starting options based on a specific clue.
  void AddClue(size_t pos, char letter, Result result) {
    size_t let_id = ID(letter);
    if (result == Result::NOWHERE) {
      start_options &= clues[pos].nowhere[let_id].words;
    } else if (result == Result::ELSEWHERE) {
      start_options &= clues[pos].elsewhere[let_id].words;
    } else {
      start_options &= clues[pos].here[let_id].words;
    }
    start_count = start_options.CountOnes();
  }

  void AddClue(std::string word, std::string result) {
    for (size_t i = 0; i < word.size(); ++i) {
      if (result[i] == 'N') AddClue(i, word[i], Result::NOWHERE);
      else if (result[i] == 'E') AddClue(i, word[i], Result::ELSEWHERE);
      else if (result[i] == 'H') AddClue(i, word[i], Result::HERE);
    }
  }

  emp::BitVector AnalyzeGuess(const std::string & guess, const WordData & answer) {
    // Loop through all possible answers to see how much a word cuts down choices.
    emp::BitVector options(start_options);

    for (size_t pos = 0; pos < word_length; ++pos) {
      const size_t guess_letter = ID(guess[pos]);
      if (guess[pos] == answer.word[pos]) {        // CORRECT GUESS FOR POSITION!
        options &= clues[pos].here[guess_letter].words;
      } else if (answer.letters.Has(guess_letter)) {    // WRONG POSITION
        options &= clues[pos].elsewhere[guess_letter].words;
      } else {                                          // WRONG CHARACTER
        options &= clues[pos].nowhere[guess_letter].words;
      }
    }

    return options;
  }

  // Slow way to manually call on specific words; brute-force find the entires for each.
  emp::BitVector AnalyzeGuess(const std::string & guess, const std::string & answer) {
    if (!emp::Has(pos_map, answer)) std::cerr << "UNKNOWN WORD: " << answer << std::endl;
    return AnalyzeGuess(guess, words[pos_map[answer]]);
  }

  void AnalyzeGuess(WordData & guess) {
    size_t max_options = 0;
    size_t total_options = 0;
    double entropy = 0.0;

    // Scan through all possible answers...    
    for (WordData & answer : words) {
      size_t options = AnalyzeGuess(guess.word, answer).CountOnes();
      if (options > max_options) max_options = options;
      total_options += options;
      const double p = static_cast<double>(options) / static_cast<double>(start_count);
      entropy -= p * std::log2(p);
    }
    guess.max_options = max_options;
    guess.ave_options = static_cast<double>(total_options) / static_cast<double>(words.size());
    guess.entropy = entropy;
  }

  void Analyze() {
    // for (int id = start_options.FindOne(); id >= 0; id = start_options.FindOne(id+1)) {
    for (size_t id = 0; id < words.size(); ++id) {
      AnalyzeGuess(words[id]);
    }
  }

  /// Also analyze non-word guesses.
  void AnalyzeAll() {
    std::string guess(word_length, 'a');
    size_t best_max_options = 10000;
    double best_ave_options = 10000.0;
    double best_entropy = 0.0;
    std::string best_max_options_word = "";
    std::string best_ave_options_word = "";
    std::string best_entropy_word = "";

    size_t silent_count = 0;  // Keep a count of how many loops since out last output.
    while (true) {
      size_t max_options = 0;
      size_t total_options = 0;
      double entropy = 0.0;

      // Scan through all possible answers...    
      for (WordData & answer : words) {
        size_t options = AnalyzeGuess(guess, answer).CountOnes();
        if (options > max_options) max_options = options;
        total_options += options;
        const double p = static_cast<double>(options) / static_cast<double>(start_count);
        entropy -= p * std::log2(p);
      }
      double ave_options = static_cast<double>(total_options) / static_cast<double>(words.size());

      ++silent_count;
      if (max_options < best_max_options) {
        best_max_options = max_options;
        best_max_options_word = guess;
        std::cout << "New best MAX options: " << guess << " : " << max_options << std::endl;
        silent_count = 0;
      }
      if (ave_options < best_ave_options) {
        best_ave_options = ave_options;
        best_ave_options_word = guess;
        std::cout << "New best AVE options: " << guess << " : " << ave_options << std::endl;
        silent_count = 0;
      }
      if (entropy > best_entropy) {
        best_entropy = entropy;
        best_entropy_word = guess;
        std::cout << "New best ENTROPY: " << guess << " : " << entropy << std::endl;
        silent_count = 0;
      }
      if (silent_count >= 10000) {
        std::cout << "...processing... ('" << guess << "')" << std::endl;
        silent_count = 0;
      }

      // Now move on to the next word...
      size_t inc_pos = word_length - 1;  // find the first non-z letter.
      while (inc_pos < word_length && guess[inc_pos] == 'z') {
        guess[inc_pos] = 'a';
        --inc_pos;
      }
      if (inc_pos == word_length) break;
      ++guess[inc_pos];
    }
  }

  /// Print all of the words with a given set of IDs.
  void PrintWords(const emp::BitVector & word_ids) {
    size_t count = 0;
    for (int id = word_ids.FindOne(); id >= 0; id = word_ids.FindOne(id+1)) {
      if (count) std::cout << ",";
      std::cout << words[id].word;
      ++count;
    }
    std::cout << " (" << count << " words found)" << std::endl;
  }

  /// Print all of the results, sorted by max number of options.
  void PrintResults() {
    for (size_t i = 0; i < words.size(); ++i) {
      words[i].is_active = start_options.Has(i);
    }
    emp::Sort(words, [](const WordData & w1, const WordData & w2){
      if (w1.is_active == w2.is_active) {
	return w1.max_options < w2.max_options;
      }
      return w2.is_active;
    });
    for (auto & word : words) {
      std::cout << word.word
                << ", " << word.max_options
                << ", " << word.ave_options
                << ", " << word.is_active
                << std::endl;
    }
  }
};

int main(int argc, char* argv[])
{
  emp::vector<std::string> args = emp::cl::args_to_strings(argc, argv);

  if (args.size() > 3) {
    std::cerr << "May provide am input filename (with the word list to use) and output filename (for results)"
        << std::endl;
    exit(1);
  }

  WordSet word_set(5);

  if (args.size() == 1) word_set.Load(std::cin, std::cout);
  else {
    std::ifstream in_file{args[1]};
    if (args.size() == 2) word_set.Load(in_file, std::cout);
    else {
      std::ofstream out_file{args[2]};
      word_set.Load(in_file, out_file);
    }
  }

  word_set.Preprocess();

  //word_set.AddClue("aloes", "NNNEN");
  word_set.AddClue("rates", "NENEN");
  // word_set.AddClue("login", "ENNEN");
  // word_set.AddClue("dimly", "NHNHH");
  //  word_set.AddClue("finch", "NNNNN");

  /*
  word_set.AddClue(0,'a',Result::NOWHERE);
  word_set.AddClue(1,'l',Result::NOWHERE);
  word_set.AddClue(2,'o',Result::NOWHERE);
  word_set.AddClue(3,'e',Result::ELSEWHERE);
  word_set.AddClue(4,'s',Result::NOWHERE);

  word_set.AddClue(0,'d',Result::NOWHERE);
  word_set.AddClue(1,'i',Result::ELSEWHERE);
  word_set.AddClue(2,'r',Result::NOWHERE);
  word_set.AddClue(3,'t',Result::NOWHERE);
  word_set.AddClue(4,'y',Result::NOWHERE);

  word_set.AddClue(0,'h',Result::NOWHERE);
  word_set.AddClue(1,'e',Result::NOWHERE);
  word_set.AddClue(2,'n',Result::NOWHERE);
  word_set.AddClue(3,'g',Result::NOWHERE);
  word_set.AddClue(4,'e',Result::HERE);
  */
  
  word_set.Analyze();
  word_set.PrintResults();
//  word_set.AnalyzeAll();
}
