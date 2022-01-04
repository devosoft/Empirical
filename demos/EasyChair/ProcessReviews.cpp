/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021
 *
 *  @file ProcessReviews.cpp
 */

#include <iostream>
#include <sstream>
#include <string>

#include "../../include/emp/base/vector.hpp"
#include "../../include/emp/io/File.hpp"
#include "../../include/emp/tools/string_utils.hpp"


///////////////////////
//  ReviewInfo
///////////////////////

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

  void Write(std::ostream & os=std::cout) const {
    if (is_meta) {
      os << "METAREVIEW by " << reviewer_name << ": ";
      if (overall == -1) os << "reject\n";
      if (overall == 0) os << "UNDECIDED\n";
      else os << "accept!\n";
    }
    else {
      os << "REVIEW by " << reviewer_name << ":\n";
      os << " Overall evaluation: " << overall << std::endl;
      os << " Novelty/Originality: " << novelty << std::endl;
      os << " Writing Clarity: " << writing << std::endl;
      os << " Thoroughness of Literature Review: " << lit_review << std::endl;
      os << " Thoroughness of Methods: " << methods << std::endl;
      os << " Relevance to Artificial Life Conference: " << relevance << std::endl;
      os << " Overall Quality of Work: " << quality << std::endl;
      os << " Reviewer's confidence: " << confidence << std::endl;
    }
  }

  void WriteCSV(std::ostream & os=std::cout) const {
    os << "\"" << reviewer_name << "\"," << overall;
    if (!is_meta) {
      os << "," << novelty
         << "," << writing
         << "," << lit_review
         << "," << methods
         << "," << relevance
         << "," << quality
         << "," << confidence;
    }
  }

  static void WriteCSVMetaHeaders(std::ostream & os=std::cout) {
    os << "Metareviewer,Recommendation";
  }

  static void WriteCSVHeaders(std::ostream & os=std::cout) {
    os << "Reviewer,Overall"
        << ",Novelty"
        << ",Writing"
        << ",Lit Review"
        << ",Methods"
        << ",Eelevance"
        << ",Quality"
        << ",Confidence";
  }
};


///////////////////////
//  PaperInfo
///////////////////////

struct PaperInfo
{
  int id = -1;
  std::string title = "";
  emp::vector<std::string> authors;
  ReviewInfo meta_review;
  emp::vector<ReviewInfo> reviews;
  std::string session = "";
  std::string presentation = "";   // Type of presentation requested.
  size_t length = 0;

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
       << "TITLE:    " << title << std::endl;
    meta_review.Write(os);
    for (const ReviewInfo & review : reviews) {
      review.Write(os);
    }
    os << std::endl;
  }

  void WriteCSV(std::ostream & os=std::cout) const {
    // One line for each review.
    for (const ReviewInfo & review : reviews) {
      os << id << ",\""
         << GetAuthors() << "\",\""
         << title << "\","
         << length << ",\""
         << presentation << "\",\""
         << session << "\",";
      meta_review.WriteCSV(os);
      os << ",";
      review.WriteCSV(os);
      os << "\n";
    }
  }

  void WriteCSVHeaders(std::ostream & os=std::cout) const {
    os << "Paper ID,Authors,Title,Length,Presentation,Session,";
    ReviewInfo::WriteCSVMetaHeaders(os);
    os << ",";
    ReviewInfo::WriteCSVHeaders(os);
    os << std::endl;
  }
};


///////////////////////
//  PaperSet
///////////////////////

struct PaperSet {
  emp::vector<PaperInfo> papers;
  size_t cur_line = 0;             // File line being processed.
  int cur_id = -1;                 // Current paper being setup.

  PaperSet(const std::string & review_filename, const std::string & catagory_filename)
  {
    ProcessReviewFile(review_filename);
    ProcessCatagoryFile(catagory_filename);
  }

  // For the moment, summaries are not used - just fast-forward.
  void ProcessSummary(const emp::File & file) {
    // Summary mode ends with an empty line.
    while (file[cur_line] != "") cur_line++;
  }

  // Process details about a meta-review
  void ProcessMeta(const emp::File & file) {
    const std::string & line = file[cur_line++];
    ReviewInfo & info = papers[cur_id].meta_review;
    info.is_meta = true;
    info.reviewer_name = line.substr(23,line.size()-34);

    // Collect recommendation result (-1 = decline, 0 = undecided, 1 = accept)
    const std::string & result = file[cur_line++].substr(16);
    if (result == "accept") info.overall = 1;
    else if (result == "reject") info.overall = -1;
    else info.overall = 0;

    // Meta mode ends with an empty line.
    while (file[cur_line] != "") cur_line++;
  }

  bool CheckRating(const std::string & line, const std::string & name, int & value) {
    if (emp::has_prefix(line, name)) {
      size_t pos = name.size() + 1;
      value = emp::from_string<int>(emp::string_get_word(line, pos));
      return true;
    }
    return false;
  }

  // Process details about a meta-review
  void ProcessReview(const emp::File & file) {
    const std::string & line = file[cur_line++];
    papers[cur_id].reviews.push_back(ReviewInfo{});
    ReviewInfo & info = papers[cur_id].reviews.back();
    info.is_meta = false;
    info.reviewer_name = line.substr(21,line.size()-32);

    // Reviews end with an empty line.
    while (file[cur_line] != "") {
      CheckRating(file[cur_line], "Overall evaluation:", info.overall);
      CheckRating(file[cur_line], "Novelty/Originality:", info.novelty);
      CheckRating(file[cur_line], "Writing Clarity:", info.writing);
      CheckRating(file[cur_line], "Thoroughness of Literature Review:", info.lit_review);
      CheckRating(file[cur_line], "Thoroughness of Methods:", info.methods);
      CheckRating(file[cur_line], "Relevance to Artificial Life Conference:", info.relevance);
      CheckRating(file[cur_line], "Overall Quality of Work:", info.quality);
      CheckRating(file[cur_line], "Reviewer's confidence:", info.confidence);

      cur_line++;
    }
  }

  void ProcessReviewFile(const std::string & filename) {
    emp::File file(filename);

    // Scan through the file, loading each review.
    for (cur_line = 0; cur_line < file.size(); ++cur_line) {
      std::string line = file[cur_line];

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
        emp::justify(papers[cur_id].title);
        continue;
      }

      // Is this a summary?
      if (line == "================== SUMMARY OF REVIEWS =================") {
        ProcessSummary(file);
        continue;
      }

      // Is this a metareview?
      if (emp::has_prefix(line, "++++++++++ METAREVIEW")) {
        ProcessMeta(file);
        continue;
      }

      // Is this a regular review?
      if (emp::has_prefix(line, "++++++++++ REVIEW")) {
        ProcessReview(file);
        continue;
      }
    }
  }

  void ProcessCatagoryFile(const std::string & filename) {
    emp::File file(filename);

    // Skip the first line; process the rest.
    for (size_t i=1; i < file.size(); i++) {
      auto row = file.ViewRowSlices(i);
      size_t id = emp::from_string<size_t>(row[0]);
      if (row.size() > 4) papers[id].length = emp::from_string<size_t>(row[3]);
      if (row.size() > 4) papers[id].presentation = row[4];
      if (row.size() > 5) papers[id].session = row[5];
    }
  }

  void Print() {
    // Print the results...
    for (const auto & paper : papers) {
      if (paper.id < 0) continue;
      paper.Write();
    }
  }

  void PrintCVS() {
    // Find the first review and use it to print the headers.
    size_t first = 0;
    while (papers[first].id < 0) first++;
    papers[first].WriteCSVHeaders();

    // Now print all of the actual reviews.
    for (const auto & paper : papers) {
      if (paper.id < 0) continue;
      paper.WriteCSV();
    }
  }

};

int main(int argc, char * argv[])
{
  if (argc != 3) {
    std::cerr << "Format: " << argv[0] << " [review filename] [catagory filename]\n";
    exit(1);
  }

  std::string review_filename(argv[1]);
  std::string catagory_filename(argv[2]);
  PaperSet ps(review_filename, catagory_filename);
  ps.PrintCVS();
}
