//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016-2019.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  Information about a citation for a single paper / book / etc.

#ifndef EMP_CITATION_H
#define EMP_CITATION_H

#include <ostream>
#include <set>
#include <string>

#include "../base/map.hpp"
#include "../base/vector.hpp"
#include "../datastructs/map_utils.hpp"
#include "../tools/string_utils.hpp"
#include "Author.hpp"

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

    emp::map<std::string, std::string> setting_map;

    static const emp::map<std::string, CITE_TYPE> & GetNameMap() {
      static emp::map<std::string, CITE_TYPE> name_map;
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

    // Name map in opposite direction (enum -> name)
    static const emp::multimap<CITE_TYPE, std::string> & GetRNameMap() {
      static const emp::multimap<CITE_TYPE, std::string> rname_map = emp::flip_map(GetNameMap());
      return rname_map;
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
      return emp::FindRef(setting_map, setting, emp::empty_string());
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

    template <typename... ARGS>
    Citation & AddAuthor(ARGS &&... args) {
      author_list.emplace_back(std::forward<ARGS>(args)...);
      return *this;
    }
    Citation & AddNote(const std::string & note) { notes.push_back(note); return *this; }
    Citation & AddKeyword(const std::string & kw) { keywords.insert(kw); return *this; }

    template <class T> Citation & SetVal(const std::string & setting, T && val) {
      setting_map[to_lower(setting)] = to_string(val);
      return *this;
    }

    template <class T> Citation & SetTitle(T && val) { return SetVal("title", val); }
    template <class T> Citation & SetJournal(T && val) { return SetVal("journal", val); }      // If in journal
    template <class T> Citation & SetBookTitle(T && val) { return SetVal("book_title", val); } // If in book
    template <class T> Citation & SetVolume(T && val) { return SetVal("volume", val); }
    template <class T> Citation & SetYear(T && val) { return SetVal("year", val); }
    template <class T> Citation & SetNumber(T && val) { return SetVal("number", val); }
    template <class T> Citation & SetMonth(T && val) { return SetVal("month", val); }
    template <class T> Citation & SetStartPage(T && val) { return SetVal("start_page", val); }
    template <class T> Citation & SetEndPage(T && val) { return SetVal("end_page", val); }
    template <class T> Citation & SetPages(T && p1, T && p2) { SetStartPage(p1); return SetEndPage(p2); }

    void SetType(CITE_TYPE in_type) { type = in_type; }
    void SetType(std::string in_type) {
      in_type = to_lower(in_type);       // <Make sure type is lowercase.
      const auto & name_map = GetNameMap();
      auto map_it = name_map.find(in_type);
      if (map_it == name_map.end()) type = ERROR;
      else type = map_it->second;
    }

    // Printing helpers...
    void PrintAuthors(std::ostream & out=std::cout) {
      if (author_list.size() == 0) return;   // Stop if no authors.
      out << author_list[0].GetReverseName();   // Print first author.

      // Print middle authors...
      for (size_t i = 1; i < author_list.size() - 1; i++) {
        out << ", " << author_list[i].GetFullName();
      }

      // Print last author.
      if (author_list.size() > 1) out << ", and " << author_list.back().GetFullName();
    }

    // By default, print a standard citation style.
    void PrintERROR(std::ostream & out=std::cout) { out << "ERROR with printing citation."; }
    void PrintUNKNOWN(std::ostream & out=std::cout) { (void) out; } // Dynamically guess type?
    void PrintARTICLE(std::ostream & out=std::cout) { PrintAuthors(out); out << std::endl; }
    void PrintBOOK(std::ostream & out=std::cout) { (void) out; }
    void PrintBOOKLET(std::ostream & out=std::cout) { (void) out; }
    void PrintCONFERENCE(std::ostream & out=std::cout) { (void) out; }
    void PrintIN_BOOK(std::ostream & out=std::cout) { (void) out; }
    void PrintIN_COLLECTION(std::ostream & out=std::cout) { (void) out; }
    void PrintMANUAL(std::ostream & out=std::cout) { (void) out; }
    void PrintMASTERS_THESIS(std::ostream & out=std::cout) { (void) out; }
    void PrintMISC(std::ostream & out=std::cout) { (void) out; }
    void PrintPHD_THESIS(std::ostream & out=std::cout) { (void) out; }
    void PrintPROCEEDINGS(std::ostream & out=std::cout) { (void) out; }
    void PrintTECH_REPORT(std::ostream & out=std::cout) { (void) out; }
    void PrintUNPUBLISHED(std::ostream & out=std::cout) { (void) out; }

    void Print(std::ostream & out=std::cout) {
      switch (type) {
        case ERROR: PrintERROR(out); break;
        case UNKNOWN: PrintUNKNOWN(out); break;
        case ARTICLE: PrintARTICLE(out); break;
        case BOOK: PrintBOOK(out); break;
        case BOOKLET: PrintBOOKLET(out); break;
        case CONFERENCE: PrintCONFERENCE(out); break;
        case IN_BOOK: PrintIN_BOOK(out); break;
        case IN_COLLECTION: PrintIN_COLLECTION(out); break;
        case MANUAL: PrintMANUAL(out); break;
        case MASTERS_THESIS: PrintMASTERS_THESIS(out); break;
        case MISC: PrintMISC(out); break;
        case PHD_THESIS: PrintPHD_THESIS(out); break;
        case PROCEEDINGS: PrintPROCEEDINGS(out); break;
        case TECH_REPORT: PrintTECH_REPORT(out); break;
        case UNPUBLISHED: PrintUNPUBLISHED(out); break;
      }
    }
  };

}

#endif
