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

#include "..tools/vector.h"
#include "Author.h"

namespace emp {

  class Citation {
  public:
    enum CITE_TYPE { UNKNOWN=0, ARTICLE, BOOK, BOOKLET, CONFERENCE, IN_BOOK, IN_COLLECTION, MANUAL,
                     MASTERS_THESIS, MISC, PHD_THESIS, PROCEEDINGS, TECH_REPORT, UNPUBLISHED }
  private:
    emp::vector<Author> author_list;
    std::string title;
    std::string journal;
    std::string volume;
    int year;
    int start_page;
    int end_page;
    std::set<std::string> keywords;
  public:
    Citation() { ; }
    Citation(const Citation &) = default;
    ~Citation() { ; }

    Citation & operator=(const Citation &) = default;

    void AddAuthor(const Author & author) { author_list.push_back(author); }
  };

};

#endif
