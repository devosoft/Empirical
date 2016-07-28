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

    template <class T> void SetVal(const std::string & setting, T && val) {
      setting_map[setting] = to_string(val);
    }
    
    template <class T> void SetTitle(T && val) { setting_map["title"] = to_string(val); }
    template <class T> void SetJournal(T && val) { setting_map["journal"] = to_string(val); }
    template <class T> void SetBookTitle(T && val) { setting_map["book_title"] = to_string(val); }
    template <class T> void SetVolume(T && val) { setting_map["volume"] = to_string(val); }
    template <class T> void SetYear(T && val) { setting_map["year"] = to_string(val); }
    template <class T> void SetNumber(T && val) { setting_map["number"] = to_string(val); }
    template <class T> void SetMonth(T && val) { setting_map["month"] = to_string(val); }
    template <class T> void SetStartPage(T && val) { setting_map["start_page"] = to_string(val); }
    template <class T> void SetEndPage(T && val) { setting_map["end_page"] = to_string(val); }
  };

};

#endif
