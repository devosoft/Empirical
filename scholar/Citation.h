// This file is part of Empirical, https://github.com/mercere99/Empirical/, and is  
// Copyright (C) Michigan State University, 2015. It is licensed                
// under the MIT Software license; see doc/LICENSE

#ifndef EMP_CITATION_H
#define EMP_CITATION_H

//////////////////////////////////////////////////////////////////////////////////////////
//
//  Information about a single citation for a paper.
//

#include <set>
#include <string>
#include <vector>

#include "Author.h"

namespace emp {

  class Citation {
  private:
    std::vector<Author> author_list;
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
