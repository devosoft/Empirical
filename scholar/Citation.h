//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  Information about a citation for a single paper / book / etc.

#ifndef EMP_CITATION_H
#define EMP_CITATION_H

#include <set>
#include <string>

#include "../tools/string_utils.h"
#include "../tools/vector.h"
#include "Author.h"

namespace emp {

  class Citation {
  public:
    enum CITE_TYPE { UNKNOWN=0, ARTICLE, BOOK, BOOKLET, CONFERENCE, IN_BOOK, IN_COLLECTION, MANUAL,
                     MASTERS_THESIS, MISC, PHD_THESIS, PROCEEDINGS, TECH_REPORT, UNPUBLISHED }
  private:
    emp::vector<Author> author_list;
    std::vector<std::string> notes;   // An optional note.
    std::set<std::string> keywords;   // Optional keywords.

    std::map<std::string, std::string> setting_map;
  public:
    Citation() { ; }
    Citation(const Citation &) = default;
    ~Citation() { ; }

    Citation & operator=(const Citation &) = default;

    void AddAuthor(const Author & author) { author_list.push_back(author); }
    void AddNote(const std::string & note) { notes.push_back(note); }
    void AddKeyword(const std::string & kw) { keywords.insert(kw); }

    template <class T> void SetVal(const std::string & setting, T && val) {
      setting_map[setting] = to_string(val);
    }

    template <class T> void SetTitle(T && val) { SetVal("title", val); }
    template <class T> void SetJournal(T && val) { SetVal("journal", val); }      // If in journal
    template <class T> void SetBookTitle(T && val) { SetVal("book_title", val); } // If in book
    template <class T> void SetVolume(T && val) { SetVal("volume", val); }
    template <class T> void SetYear(T && val) { SetVal("year", val); }
    template <class T> void SetNumber(T && val) { SetVal("number", val); }
    template <class T> void SetMonth(T && val) { SetVal("month", val); }
    template <class T> void SetStartPage(T && val) { SetVal("start_page", val); }
    template <class T> void SetEndPage(T && val) { SetVal("end_page", val); }
  };

};

#endif
