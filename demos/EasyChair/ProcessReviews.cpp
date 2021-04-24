#include <iostream>
#include <sstream>
#include <string>

#include "../../include/emp/base/vector.hpp"
#include "../../include/emp/io/File.hpp"
#include "../../include/emp/tools/string_utils.hpp"

struct ReviewInfo
{
  bool is_meta = false;
  std::string reviewer_name = "";

  // scores (only overall used for meta-reviews)
  int overall = 0;
  int novelty = 0;
  int writing = 0;
  int lit_review = 0;
  int methods = 0;
  int relevance = 0;
  int quality = 0;
  int confidence = 0;
};

struct PaperInfo
{
  int id = -1;
  std::string title = "";
  emp::vector<std::string> authors;
  emp::vector<ReviewInfo> reviews;

  void SetAuthors(std::string in_authors) {
    authors = emp::slice(in_authors, ',');

    // Check to see if we need to split the last one.
    size_t split_pos = authors.back().find(" and ");
    if (split_pos != std::string::npos) {
      std::string last_author = authors.back().substr(split_pos+5);
      authors.back().resize(split_pos);
      authors.push_back(last_author);
    }

    // Remove all spaces at the beginning and end of author's names.
    for (std::string & author : authors) emp::justify(author);
  }

  std::string GetAuthors() const {
    std::stringstream ss;
    ss << authors[0];
    for (size_t i = 1; i < authors.size(); i++) {
      ss << ", " << authors[i];
    }
    return ss.str();
  }

  void Write(std::ostream & os=std::cout) const {
    os << "PAPER ID: " << id << std::endl
       << "AUTHORS:  " << GetAuthors() << std::endl
       << "TITLE:    " << title << std::endl
       << std::endl;
  }
};

struct PaperSet {
  std::string filename = "";
  emp::File file;
  emp::vector<PaperInfo> papers;

  enum class Mode { BASE, SUMMARY, META, REVIEW };

  PaperSet(const std::string & in_filename)
    : filename(in_filename), file(in_filename)
  {
    // Scan through the file, loading each review.
    int cur_id = -1;
    Mode mode = Mode::BASE;
    for (std::string line : file) {
      if (mode == Mode::SUMMARY) {
        // Summary mode ends with an empty line.
        if (line == "") {
          mode = Mode::BASE;
          continue;
        }
      }

      // New paper?
      if (emp::has_prefix(line, "*********************** PAPER")) {
        std::string id_string = emp::string_get_word(line, 30);
        cur_id = emp::from_string<int>(id_string);
        if (papers.size() <= (size_t) cur_id) papers.resize(cur_id+1);
        papers[cur_id].id = cur_id;
        continue;
      }

      // If we made it this far, we need to have an id.
      emp_assert(cur_id >= 0);

      if (emp::has_prefix(line, "AUTHORS:")) {
        emp::string_pop_word(line);
        papers[cur_id].SetAuthors(line);
        continue;
      }

      if (emp::has_prefix(line, "TITLE:")) {
        emp::string_pop_word(line);
        papers[cur_id].title = line;
        continue;
      }

      // Is this a summary?
      if (line == "================== SUMMARY OF REVIEWS =================") {
        mode = Mode::SUMMARY;
        continue;
      }

      // Is this a metareview?
      if (emp::has_prefix(line, "++++++++++ METAREVIEW")) {
        mode = Mode::META;
        continue;
      }

      // Is this a regular review?
      if (emp::has_prefix(line, "++++++++++ REVIEW")) {
        mode = Mode::REVIEW;
      }
    }

  }

  void Print() {
    // Print the results...
    for (const auto & paper : papers) {
      if (paper.id < 0) continue;
      paper.Write();
    }
  }

  bool CheckRating(const std::string & line, const std::string & name, int & value) {
    if (emp::has_prefix(line, name)) {
      size_t pos = name.size() + 1;
      value = emp::from_string<int>(emp::string_get_word(name, pos));
      return true;
    }
    return false;
  }
};

int main(int argc, char * argv[])
{
  if (argc != 2) {
    std::cerr << "Format: " << argv[0] << " [filename]\n";
    exit(1);
  }

  std::string filename(argv[1]);
  PaperSet ps(filename);
  ps.Print();
}
