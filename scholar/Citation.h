//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  Information about a citation for a single paper / book / etc.

#ifndef EMP_CITATION_H
#define EMP_CITATION_H

#include <map>
#include <set>
#include <string>

#include "../tools/map_utils.h"
#include "../tools/string_utils.h"
#include "../tools/vector.h"
#include "Author.h"

namespace emp {

  class Citation {
  public:
    enum CITE_TYPE { ERROR=-1, UNKNOWN=0, ARTICLE, BOOK, BOOKLET, CONFERENCE, IN_BOOK, IN_COLLECTION, MANUAL,
                     MASTERS_THESIS, MISC, PHD_THESIS, PROCEEDINGS, TECH_REPORT, UNPUBLISHED };
  private:
    CITE_TYPE type;
    emp::vector<Author> author_list;
    emp::vector<std::string> notes;   // An optional note.
    std::set<std::string> keywords;   // Optional keywords.

    std::map<std::string, std::string> setting_map;

    static std::map<std::string, CITE_TYPE> & GetNameMap() {
      static std::map<std::string, CITE_TYPE> name_map;
      if (name_map.size() == 0) {
        name_map["unknown"] = UNKNOWN;
        name_map["article"] = ARTICLE;
        name_map["book"] = BOOK;
        name_map["booklet"] = BOOKLET;
        name_map["conference"] = CONFERENCE;
        name_map["inbook"] = IN_BOOK;
        name_map["incollection"] = IN_COLLECTION;
        name_map["manual"] = MANUAL;
        name_map["mastersthesis"] = MASTERS_THESIS;
        name_map["misc"] = MISC;
        name_map["phdthesis"] = PHD_THESIS;
        name_map["proceedings"] = PROCEEDINGS;
        name_map["techreport"] = TECH_REPORT;
        name_map["unpublished"] = UNPUBLISHED;
      }
      return name_map;
    }
  public:
    Citation(CITE_TYPE in_type=UNKNOWN) : type(in_type) { ; }
    Citation(const Citation &) = default;
    ~Citation() { ; }
    Citation & operator=(const Citation &) = default;

    CITE_TYPE GetType() const { return type; }
    const emp::vector<Author> & GetAuthors() const { return author_list; }
    const emp::vector<std::string> & GetNotes() const { return notes; }
    const std::set<std::string> & GetKeywords() const { return keywords; }
    const bool HasKeyword(const std::string & kw) const {
      return keywords.find(kw) != keywords.end();
    }

    const std::string & GetVal(const std::string & setting) const {
      return emp::Find(setting_map, setting, emp::empty_string());
    }

    const std::string & GetTitle() const { return GetVal("title"); }
    const std::string & GetJournal() const { return GetVal("journal"); }      // If in journal
    const std::string & GetBookTitle() const { return GetVal("book_title"); } // If in book
    const std::string & GetVolume() const { return GetVal("volume"); }
    const std::string & GetYear() const { return GetVal("year"); }
    const std::string & GetNumber() const { return GetVal("number"); }
    const std::string & GetMonth() const { return GetVal("month"); }
    const std::string & GetStartPage() const { return GetVal("start_page"); }
    const std::string & GetEndPage() const { return GetVal("end_page"); }

    void AddAuthor(const Author & author) { author_list.push_back(author); }
    void AddNote(const std::string & note) { notes.push_back(note); }
    void AddKeyword(const std::string & kw) { keywords.insert(kw); }

    template <class T> void SetVal(const std::string & setting, T && val) {
      setting_map[to_lower(setting)] = to_string(val);
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

    void SetType(CITE_TYPE in_type) { type = in_type; }
    void SetType(std::string in_type) {
      in_type = to_lower(in_type);       // <Make sure type is lowercase.
      auto & name_map = GetNameMap();
      if (name_map.find(in_type) == name_map.end()) type = ERROR;
      else type = name_map[in_type];
    }
  };

};

#endif
