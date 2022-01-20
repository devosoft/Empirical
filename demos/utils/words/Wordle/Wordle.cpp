/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2022
 *
 *  @file Wordle.cpp
 */

#include <cmath>
#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>

#include "emp/base/Ptr.hpp"
#include "emp/base/vector.hpp"
#include "emp/bits/BitSet.hpp"
#include "emp/bits/BitVector.hpp"
#include "emp/config/command_line.hpp"
#include "emp/datastructs/map_utils.hpp"
#include "emp/datastructs/vector_utils.hpp"
#include "emp/io/File.hpp"
#include "emp/tools/string_utils.hpp"

#include "Result.hpp"


template <size_t WORD_SIZE=5>
class WordSet {
private:
  static constexpr size_t MAX_LETTER_REPEAT = 4;
  using word_list_t = emp::BitVector;
  using result_t = Result<WORD_SIZE>;

  // Get the ID (0-26) associated with a letter.
  static size_t ToID(char letter) {
    emp_assert(letter >= 'a' && letter <= 'z');
    return static_cast<size_t>(letter - 'a');
  }

  static char ToLetter(size_t id) {
    emp_assert(id < 26);
    return static_cast<char>(id + 'a');
  }

  // All of the clues for a given position.
  struct PositionClues {
    size_t pos;
    std::array<word_list_t, 26> here;      // Is a given letter at this position?

    void SetNumWords(size_t num_words) {
      for (auto & x : here) x.resize(num_words);
    }
  };

  // All of the clues for zero or more instances of a given letter.
  struct LetterClues {
    size_t letter;  // [0-25]
    std::array<word_list_t, MAX_LETTER_REPEAT+1> at_least; ///< Are there at least x instances of letter? (0 is meaningless)
    std::array<word_list_t, MAX_LETTER_REPEAT+1> exactly;  ///< Are there exactly x instances of letter?

    void SetNumWords(size_t num_words) {
      for (auto & x : at_least) x.resize(num_words);
      for (auto & x : exactly) x.resize(num_words);
    }
  };

  struct WordData {
    std::string word;
    // Pre=processed data
    emp::BitSet<26> letters;        // What letters are in this word?
    emp::BitSet<26> multi_letters;  // What letters are in this word more than once?
    std::array<word_list_t, result_t::NUM_IDS> next_words;

    // Collected data
    size_t max_options = 0;         // Maximum number of word options after used as a guess.
    double ave_options = 0.0;       // Average number of options after used as a guess.
    double entropy = 0.0;           // What is the entropy (and thus information gained) for this choice?

    WordData(const std::string & in_word) : word(in_word) {
      for (char x : word) {
        size_t let_id = ToID(x);
        if (letters.Has(let_id)) multi_letters.Set(let_id);
        else letters.Set(let_id);
      }
    }
  };

  emp::vector<WordData> words;                     ///< Data about all words in this Wordle
  emp::array<PositionClues, WORD_SIZE> pos_clues;  ///< A PositionClues object for each position.
  emp::array<LetterClues,26> let_clues;            ///< Clues based off the number of letters.
  std::unordered_map<std::string, size_t> pos_map; ///< Map of words to their position ids.
  word_list_t start_options;                    ///< Current options.
  size_t start_count;                              ///< Count of start options (cached)

  std::istream & is;
  std::ostream & os;

  bool verbose = true;

public:
  WordSet(std::istream & _is, std::ostream & _os) : is(_is), os(_os) { }

  /// Include a single word into this WordSet.
  void AddWord(std::string & in_word) {
    size_t id = words.size();      // Set a unique ID for this word.
    pos_map[in_word] = id;         // Keep track of the ID for this word.
    words.emplace_back(in_word);   // Setup the word data.
  }

  /// Load a whole series for words (from a file) into this WordSet
  void Load() {
    // Load in all of the words.
    std::string in_word;
    size_t wrong_size_count = 0;
    size_t invalid_char_count = 0;
    size_t dup_count = 0;
    while (is) {
      is >> in_word;
      // Only keep words of the correct size and all lowercase.
      if (in_word.size() != WORD_SIZE) { wrong_size_count++; continue; }
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

  /// Clear out all prior guess information.
  void ResetOptions() {
    start_count = words.size();
    start_options.resize(start_count);
    start_options.SetAll();
  }

  // Limit the current options based on a single guess and its result.

  word_list_t EvalGuess(const std::string & guess, const result_t & result) {
    emp_assert(guess.size() == WORD_SIZE);
    emp_assert(result.size() == WORD_SIZE);

    emp::array<uint8_t, 26> letter_counts;
    std::fill(letter_counts.begin(), letter_counts.end(), 0);
    emp::BitSet<26> letter_fail;
    word_list_t word_options = start_options;

    // First add letter clues and collect letter information.
    for (size_t i = 0; i < WORD_SIZE; ++i) {
      const size_t cur_letter = ToID(guess[i]);
      if (result[i] == result_t::HERE) {
        word_options &= pos_clues[i].here[cur_letter];
        ++letter_counts[cur_letter];
      } else if (result[i] == result_t::ELSEWHERE) {
        word_options &= ~pos_clues[i].here[cur_letter];
        ++letter_counts[cur_letter];
      } else {  // Must be 'N'
        word_options &= ~pos_clues[i].here[cur_letter];
        letter_fail.Set(cur_letter);
      }
    }

    // Next add letter clues.
    for (size_t letter_id = 0; letter_id < 26; ++letter_id) {
      const size_t let_count = letter_counts[letter_id];
      if (let_count) {
        word_options &= let_clues[letter_id].at_least[let_count];
      }
      if (letter_fail.Has(letter_id)) {
        word_options &= let_clues[letter_id].exactly[let_count];
      }
    }

    return word_options;
  }


  void AnalyzeGuess(WordData & guess, const word_list_t & cur_words) {
    size_t max_options = 0;
    size_t total_options = 0;
    size_t option_count = 0;
    double entropy = 0.0;
    const double word_count = static_cast<double>(words.size());

    // Scan through all of the possible result IDs.
    for (size_t result_id = 0; result_id < result_t::NUM_IDS; ++result_id) {
      word_list_t next_options = guess.next_words[result_id] & cur_words;
      size_t num_options = next_options.CountOnes();
      if (num_options > max_options) max_options = num_options;
      total_options += num_options * num_options;
      option_count++;
      double p = static_cast<double>(num_options) / word_count;
      if (p > 0.0) entropy -= p * std::log2(p);
    }

    guess.max_options = max_options;
    guess.ave_options = static_cast<double>(total_options) / static_cast<double>(words.size());
    guess.entropy = entropy;
  }


  /// Once the words are loaded, Preprocess will collect info.
  void Preprocess() {
    std::cout << "Beginning pre-process phase..." << std::endl;

    // Setup all position clue info to know the number of words.
    for (size_t i=0; i < WORD_SIZE; ++i) {
      pos_clues[i].pos = i;
      pos_clues[i].SetNumWords(words.size());
    }

    // Setup all letter clue information
    for (size_t let=0; let < 26; let++) {
      let_clues[let].letter = let;
      let_clues[let].SetNumWords(words.size());
    }

    // Counters for number of letters.
    emp::array<uint8_t, 26> letter_counts;

    // Loop through each word, indicating which clues it is consistent with.
    for (size_t word_id = 0; word_id < words.size(); ++word_id) {
      const std::string & word = words[word_id].word;

      // Figure out which letters are in this word.
      std::fill(letter_counts.begin(), letter_counts.end(), 0);      // Reset counters to zero.
      for (const char letter : word) ++letter_counts[ToID(letter)];  // Count letters.

      // Setup the LETTER clues that word is consistent with.
      for (size_t letter_id = 0; letter_id < 26; ++letter_id) {
        const size_t cur_count = letter_counts[letter_id];
        let_clues[letter_id].exactly[cur_count].Set(word_id);
        for (uint8_t count = 0; count <= cur_count; ++count) {
          let_clues[letter_id].at_least[count].Set(word_id);
        }
      }

      // Now figure out what POSITION clues it is consistent with.
      for (size_t pos=0; pos < word.size(); ++pos) {
        const size_t cur_letter = ToID(word[pos]);
        pos_clues[pos].here[cur_letter].Set(word_id);
      }
    }

    std::cout << "...clues are initialized..." << std::endl;

    ResetOptions();

    // Loop through words one more time, filling out result lists and collecting data.
    size_t word_count = 0;
    const size_t step = words.size() / 100;
    for (auto & word_info : words) {
      if (++word_count % step == 0) {
        std::cout << ".";
        std::cout.flush();
      }
      for (size_t result_id = 0; result_id < result_t::NUM_IDS; ++result_id) {
        Result result(result_id);
        if (!result.IsValid(word_info.word)) continue;
        word_info.next_words[result_id] = EvalGuess(word_info.word, result_id);
      }
      AnalyzeGuess(word_info, start_options);
    }

    std::cout << "...words are analyzed..." << std::endl;
  }

  // /// Also analyze non-word guesses.
  // void AnalyzeAll() {
  //   std::string guess(WORD_SIZE, 'a');
  //   size_t best_max_options = 10000;
  //   double best_ave_options = 10000.0;
  //   double best_entropy = 0.0;
  //   std::string best_max_options_word = "";
  //   std::string best_ave_options_word = "";
  //   std::string best_entropy_word = "";

  //   size_t silent_count = 0;  // Keep a count of how many loops since out last output.
  //   while (true) {
  //     size_t max_options = 0;
  //     size_t total_options = 0;
  //     double entropy = 0.0;

  //     // Scan through all possible answers...
  //     for (WordData & answer : words) {
  //       size_t options = AnalyzeGuess(guess, answer).CountOnes();
  //       if (options > max_options) max_options = options;
  //       total_options += options;
  //       const double p = static_cast<double>(options) / static_cast<double>(start_count);
  //       entropy -= p * std::log2(p);
  //     }
  //     double ave_options = static_cast<double>(total_options) / static_cast<double>(words.size());

  //     ++silent_count;
  //     if (max_options < best_max_options) {
  //       best_max_options = max_options;
  //       best_max_options_word = guess;
  //       std::cout << "New best MAX options: " << guess << " : " << max_options << std::endl;
  //       silent_count = 0;
  //     }
  //     if (ave_options < best_ave_options) {
  //       best_ave_options = ave_options;
  //       best_ave_options_word = guess;
  //       std::cout << "New best AVE options: " << guess << " : " << ave_options << std::endl;
  //       silent_count = 0;
  //     }
  //     if (entropy > best_entropy) {
  //       best_entropy = entropy;
  //       best_entropy_word = guess;
  //       std::cout << "New best ENTROPY: " << guess << " : " << entropy << std::endl;
  //       silent_count = 0;
  //     }
  //     if (silent_count >= 10000) {
  //       std::cout << "...processing... ('" << guess << "')" << std::endl;
  //       silent_count = 0;
  //     }

  //     // Now move on to the next word...
  //     size_t inc_pos = WORD_SIZE - 1;  // find the first non-z letter.
  //     while (inc_pos < WORD_SIZE && guess[inc_pos] == 'z') {
  //       guess[inc_pos] = 'a';
  //       --inc_pos;
  //     }
  //     if (inc_pos == WORD_SIZE) break;
  //     ++guess[inc_pos];
  //   }
  // }

  /// Print all of the words with a given set of IDs.
  void PrintWords(const word_list_t & word_ids, size_t max_count=(size_t)-1) const {
    std::cout << "(" << word_ids.CountOnes() << " words) ";
    size_t count = 0;
    for (int id = word_ids.FindOne(); id >= 0; id = word_ids.FindOne(id+1)) {
      if (count) std::cout << ",";
      std::cout << words[id].word;
      if (++count == max_count) {
        if (id > 0) std::cout << " ...";
        break;
      }
    }
    // std::cout << " (" << word_is.CountOnes() << " words)" << std::endl;
  }

  void PrintPosClues(size_t pos) const {
    const PositionClues & clue = pos_clues[pos];
    std::cout << "Position " << pos << ":\n";
    for (uint8_t i = 0; i < 26; ++i) {
      std::cout << " '" << clue.let << "' : ";
      PrintWords(clue.here[i], 10);
      std::cout << std::endl;
    }
  }

  void PrintLetterClues(char letter) const {
    const LetterClues & clue = let_clues[ToID(letter)];
    std::cout << "Letter '" << clue.letter << "':\n";
    for (size_t i = 0; i <= MAX_LETTER_REPEAT; ++i) {
      std::cout << "EXACTLY " << i << ":  ";
      PrintWords(clue.exactly[i], 20);
      std::cout << std::endl;
    }
    for (size_t i = 0; i <= MAX_LETTER_REPEAT; ++i) {
      std::cout << "AT LEAST " << i << ": ";
      PrintWords(clue.at_least[i], 20);
      std::cout << std::endl;
    }
  }

  void PrintWordData(const WordData & word) const {
    std::cout << "WORD:     " << word.word << std::endl;
    std::cout << "Letters:  " << word.letters << std::endl;
    std::cout << "Multi:    " << word.multi_letters << std::endl;
    std::cout << "MAX Opts: " << word.max_options << std::endl;
    std::cout << "AVE Opts: " << word.ave_options << std::endl;
    std::cout << "Entropy:  " << word.entropy << std::endl;
    std::cout << std::endl;

    size_t total_count = 0;
    for (size_t result_id = 0; result_id < result_t::NUM_IDS; ++result_id) {
      result_t result(result_id);
      word_list_t result_words = word.next_words[result_id];
      std::cout << result_id << " - " << result.ToString() << " ";
      PrintWords(result_words, 10);
      total_count += result_words.CountOnes();
      std::cout << std::endl;
    }
    std::cout << "Total Count: " << total_count << std::endl;
  }

  void PrintWordData(size_t id) const { PrintWordData(words[id]); }
  void PrintWordData(const std::string & word) {
    PrintWordData(words[pos_map[word]]);
  }

  // Reorder words.  NOTE: This is destructive to all word_list data!
  void SortWords(const std::string & sort_type="max") {
    using wd_t = const WordData &;
    if (sort_type == "max") {
      emp::Sort(words, [](wd_t w1, wd_t w2){
        if (w1.max_options == w2.max_options) return w1.ave_options < w2.ave_options; // tiebreak
        return w1.max_options < w2.max_options;
      } );
    } else if (sort_type == "ave") {
      emp::Sort(words, [](wd_t w1, wd_t w2){
        if (w1.ave_options == w2.ave_options) return w1.max_options < w2.max_options; // tiebreak
        return w1.ave_options < w2.ave_options;
      } );
    } else if (sort_type == "entropy") {
      emp::Sort(words, [](wd_t w1, wd_t w2){ return w1.entropy > w2.entropy; } );
    } else if (sort_type == "word") {
      emp::Sort(words, [](wd_t w1, wd_t w2){ return w1.word < w2.word; } );
    }
    for (size_t i = 0; i < words.size(); i++) { pos_map[words[i].word] = i; } // Update ID tracking.
  }

  /// Print all of the results, sorted by max number of options.
  void PrintResults() {
    SortWords();
    for (auto & word : words) {
      std::cout << word.word
                << ", " << word.max_options
                << ", " << word.ave_options
                << ", " << word.entropy
                << std::endl;
    }
  }

  /// Print out all words as HTML.
  void PrintHTMLWord(const WordData & word) const {
    std::string filename = emp::to_string("web/words/", word.word, ".html");
    std::ofstream of(filename);

    // const std::string black("&#11035;");
    static const std::string white("&#11036;");
    static const std::string green("&#129001;");
    static const std::string yellow("&#129000;");

    of << "<!doctype html>\n<html lang=\"en\">\n<head>\n <title>Wordle Analysis: '"
       << word.word << "'</title>\n</head>\n<body>\n";

    of << "<h3>Wordle Analysis: " << word.word << "</h3>\n\n";
    of << "Worst case words remaining: " << word.max_options << "<br>\n";
    of << "Expected words remaining: " << word.ave_options << "<br>\n";
    of << "Information provided: " << word.entropy << "<br>\n<p>\n";

    // Loop through all possible results.
    // for (size_t result_id = 0; result_id < result_t::NUM_IDS; ++result_id) {
    for (size_t result_id = result_t::NUM_IDS-1; result_id < result_t::NUM_IDS; --result_id) {
      result_t result(result_id);
      word_list_t result_words = word.next_words[result_id];

      of << result.ToString(green, yellow, white) << " (" << result_words.CountOnes() << " words) : ";

      for (int id = result_words.FindOne(); id >= 0; id = result_words.FindOne(id+1)) {
        of << "<a href=\""  << words[id].word << ".html\">" << words[id].word << "</a> ";
      }

      of << "<br>\n";
    }


    of << "</body>\n</html>\n";

    os << "Printed file '" << filename << "'." << std::endl;
  }

  void PrintHTMLWordID(int id) const { PrintHTMLWord(words[(size_t) id]); }
  void PrintHTMLWord(const std::string & word) {
    PrintHTMLWord(words[pos_map[word]]);
  }

  void PrintHTMLIndex(const std::string & order) {
    SortWords(order);
    std::string filename = emp::to_string("web/index-", order, ".html");
    std::ofstream of(filename);

    of << "<!doctype html>\n<html lang=\"en\">\n<head>\n <title>Wordle Analysis: INDEX"
       "</title>\n</head>\n<body>\n"
       "<h2>Analysis of Wordle Guesses</h2>\n"
       "<p>\nWhen a guess is made in a game of Wordle, the results limit the set of words for the answer."
       " A more useful guess will limit the remaining possibilities to be as small as possible."
       " But the question remains: Which word should we choose first?"
       " Here are some analyses to help make that decision.\n"
       "<p>\nBelow are a list of 5-letter words "
       "(from <a href=\"https://www-cs-faculty.stanford.edu/~knuth/sgb-words.txt\">here</a>)"
       " with data on each.  The columns are:<br>\n"
       "<table><tr><td><b>ExpectedWords</b>:"
       "           <td>The average number of possible words if this were your first guess. (smaller is better!)</tr>\n"
       "       <tr><td><b>MaximumWords</b>:"
       "           <td>The largest possible number of words remaining after this guess. (smaller is better!)</tr>\n"
       "       <tr><td><b>Information</b>:"
       "           <td>The number of bits of information this guess provides about the final answer. (larger is better!)</tr>\n"
       "</table><p>\n"
       "Click on any column to sort by it. "
       "Click on any word to see the exact breakdown of how possible first guesses limit future options.\n"
       "<p>\n";

    of << "<table><tr><th><a href=\"index-word.html\">Word</a>"
       << "<th><a href=\"index-ave.html\">ExpectedWords</a>"
       << "<th><a href=\"index-max.html\">MaximumWords</a>"
       << "<th><a href=\"index-entropy.html\">Information</a></tr>\n";
    for (const auto & word : words) {
      of << "<tr><td><a href=\"words/" << word.word << ".html\">" << word.word << "</a>"
         << "<td>" << word.ave_options
         << "<td>" << word.max_options
         << "<td>" << word.entropy
         << "</tr>\n";
    }
  }

  void PrintHTML() {
    size_t count = 0;
    std::cout << "Printing HTML files..." << std::endl;
    size_t step = words.size() / 100;
    for (auto & word : words) {
      if (count % step == 0) { std::cout << "."; std::cout.flush(); }
      PrintHTMLWord(word);
    }
    PrintHTMLIndex("ave");
    PrintHTMLIndex("entropy");
    PrintHTMLIndex("max");
    PrintHTMLIndex("word");
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

  emp::Ptr<std::istream> is_ptr = &std::cin;
  if (args.size() > 1) is_ptr = emp::NewPtr<std::ifstream>(args[1]);

  emp::Ptr<std::ostream> os_ptr = &std::cout;
  if (args.size() > 2) os_ptr = emp::NewPtr<std::ofstream>(args[2]);

  WordSet<5> word_set(*is_ptr, *os_ptr);
  word_set.Load();
  word_set.SortWords("word");

  word_set.Preprocess();
  // word_set.AddClue(0,'a',result_t::ELSEWHERE);
  // word_set.AddClue(1,'l',result_t::ELSEWHERE);
  // word_set.AddClue(2,'o',result_t::NOWHERE);
  // word_set.AddClue(3,'e',result_t::NOWHERE);
  // word_set.AddClue(4,'s',result_t::NOWHERE);

  // word_set.PrintLetterClues('x');
  // word_set.PrintPosClues(0);
  // word_set.PrintWordData(0);
  // word_set.PrintWordData("aloes");
  // word_set.PrintResults();
  // word_set.AnalyzeAll();
  // word_set.PrintHTMLWordID(0);
  // word_set.PrintHTMLWord("aloes");
  word_set.PrintHTML();

  if (args.size() > 1) is_ptr.Delete();
  if (args.size() > 2) os_ptr.Delete();
}
