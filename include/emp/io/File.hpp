/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2018-2023.
 *
 *  @file File.hpp
 *  @brief The File object maintains a simple, in-memory file.
 *  @note Status: BETA
 *
 *  @todo We need to modify this code so that File can work with Emscripten.
 *
 */

#ifndef EMP_IO_FILE_HPP_INCLUDE
#define EMP_IO_FILE_HPP_INCLUDE


#include <fstream>
#include <functional>
#include <iostream>
#include <set>
#include <string>

#include "../base/vector.hpp"
#include "../meta/FunInfo.hpp"
#include "../tools/string_utils.hpp"

namespace emp {

  /// A class to maintain files for loading, writing, storing, and easy access to components.
  class File {
  protected:
    emp::vector<std::string> lines;
    std::string file_error = "";

  public:
    File() : lines() { ; }
    File(std::istream & input) : lines() { Load(input); }
    File(const std::string & filename) : lines() { Load(filename); }
    File(const File &) = default;
    File(File &&) = default;
    ~File() { ; }

    File & operator=(const File &) = default;
    File & operator=(File &&) = default;

    /// Return const iterator to beginning of file
    auto begin() const { return std::begin(lines); }

    /// Return const iterator to end of file
    auto end() const { return std::end(lines); }

    /// Return iterator to beginning of file
    auto begin() { return std::begin(lines); }

    /// Return iterator to end of file
    auto end() { return std::end(lines); }

    /// How many lines are in this file?
    size_t GetNumLines() const { return lines.size(); }

    /// Compatibility with size()
    size_t size() const { return lines.size(); }

    /// Return entire text of the file
    emp::vector<std::string> GetAllLines() {return lines;}

    /// Index into a specific line in this file.
    std::string & operator[](size_t pos) { return lines[pos]; }

    /// Const index into a specific line in this file.
    const std::string & operator[](size_t pos) const { return lines[pos]; }

    /// Return the first line in the file.
    std::string & front() { return lines.front(); }

    /// Return a const reference to to the first line in the file.
    const std::string & front() const { return lines.front(); }

    /// Return the last line in the file.
    std::string & back() { return lines.back(); }

    /// Return a const reference to the last line in the file.
    const std::string & back() const { return lines.back(); }

    // Was there an error working with this file?
    bool HasError() const { return file_error.size(); }

    // Text of error.
    const std::string & GetError() const { return file_error; }

    // Remove any errors.
    void ClearError() { file_error.resize(0); }

    /// Append a new line to the end of the file.
    File & Append(const std::string & line) { lines.emplace_back(line); return *this; }

    /// Append a vector of lines to the end of the file.
    File & Append(const emp::vector<std::string> & in_lines) {
      size_t start_size = lines.size();
      lines.resize(start_size + in_lines.size());
      for (size_t pos = 0; pos < in_lines.size(); pos++) {
        lines[start_size+pos] = in_lines[pos];
      }
      return *this;
    }

    /// Join two files
    File & Append(const File & in_file) { return Append(in_file.lines); }

    /// Append to the end of a file.
    template <typename T>
    File & operator+=(T && in) { Append( std::forward<T>(in) ); return *this; }

    /// Insert formatted data into file
    /// This is exactly the same as operator+=
    template <typename T> auto operator<<(T &&in) {
      Append(std::forward<T>(in));
      return *this;
    }

    /// Extract first line from file
    auto operator>>(std::string & out) {
      out = size() ? front() : out;
      lines.erase(begin());
    }

    /// Test if two files are identical.
    bool operator==(const File & in) { return lines == in.lines; }

    /// Test if two files are different.
    bool operator!=(const File & in) { return lines != in.lines; }

    /// Load a line from an input stream into a file.
    File & LoadLine(std::istream & input) {
      lines.emplace_back("");
      std::getline(input, lines.back());

      // If the input file is DOS formatted, make sure to remove the \r at the end of each line.
      if (lines.back().size() && lines.back().back() == '\r') lines.back().pop_back();

      return *this;
    }

    /// Load an entire input stream into a file.
    File & Load(std::istream & input) {
      while (!input.eof()) {
        LoadLine(input);
      }
      return *this;
    }

    /// Load a file from disk using the provided name.
    /// If file does not exist, this is a nop
    File & Load(const std::string & filename) {
      std::ifstream file(filename);
      if (file.is_open()) {
        Load(file);
        file.close();
      } else {
        file_error = emp::to_string("File '", filename, "' failed to open.");
      }
      return *this;
    }

    /// Write this file to a provided output stream.
    File & Write(std::ostream & output) {
      for (std::string & cur_line : lines) {
        output << cur_line << '\n';
      }
      return *this;
    }

    /// Write this file to a file of the provided name.
    File & Write(const std::string & filename) {
      std::ofstream file(filename);
      Write(file);
      file.close();
      return *this;
    }

    /// Test if a substring exists on ANY line of a file.
    bool Contains(const std::string & pattern) const {
      for (const std::string & line : lines) {
        if (line.find(pattern) != std::string::npos) return true;
      }
      return false;
    }

    /// Convert this file into an std::set of lines (loses line ordering).
    std::set<std::string> AsSet() const {
      std::set<std::string> line_set;
      for (size_t i = 0; i < lines.size(); i++) {
        line_set.insert(lines[i]);
      }
      return line_set;
    }

    /// Apply a string manipulation function to all lines in the file.
    template <typename FUN_T>
    File & Apply(FUN_T fun) {
      for (std::string & cur_line : lines) {
        // If the function returns a string, assume that's what we're supposed to use.
        // Otherwise assume that the string gets modified.
        using return_t = FunInfo<FUN_T>::return_t;
        if constexpr ( std::is_same<return_t, std::string>() ) {
          cur_line = fun(cur_line);
        } else {
          fun(cur_line);          
        }
      }
      return *this;
    }

    /// Purge all lines that don't the criterion function.
    File & KeepIf(const std::function<bool(const std::string &)> & fun) {
      emp::vector<std::string> new_lines;
      for (std::string & cur_line : lines) {
        if (fun(cur_line)) new_lines.emplace_back(cur_line);
      }
      std::swap(lines, new_lines);
      return *this;
    }

    /// Keep only strings that contain a specific substring.
    File & KeepIfContains(const std::string & pattern) {
      return KeepIf(
        [&pattern](const std::string & line){ return line.find(pattern) != std::string::npos; }
      );
    }

    /// Remove all strings that contain a specific substring.
    File & RemoveIfContains(const std::string & pattern) {
      return KeepIf(
        [&pattern](const std::string & line){ return line.find(pattern) == std::string::npos; }
      );
    }

    /// Remove all lines that are empty strings.
    File & RemoveEmpty() {
      return KeepIf( [](const std::string & str){ return (bool) str.size(); } );
    }

    /// Any time multiple whitespaces are next to each other, collapse to a single WS char.
    /// Prefer '\n' if in whitespace collapsed, otherwise use ' '.
    File & CompressWhitespace() {
      Apply(compress_whitespace);
      RemoveEmpty();
      return *this;
    }

    /// Delete all whitespace; by default keep newlines.
    File & RemoveWhitespace(bool keep_newlines=true) {
      Apply(remove_whitespace);
      RemoveEmpty();
      if (!keep_newlines) {
        std::string all_lines;
        for (const std::string & cur_line : lines){
          all_lines += cur_line;
        }
        lines.resize(1);
        std::swap(lines[0], all_lines);
      }
      return *this;
    }

    /// A technique to remove all comments in a file.
    File & RemoveComments(const std::string & marker, bool skip_quotes=true) {
      Apply( [marker,skip_quotes](std::string & str) {
        size_t pos = emp::find(str, marker, 0, skip_quotes);
        if (pos !=std::string::npos) str.resize( pos );
      } );
      return *this;
    }

    /// Allow remove comments to also be specified with a single character.
    File & RemoveComments(char marker, bool skip_quotes=true) {
      return RemoveComments(emp::to_string(marker), skip_quotes);
    }

    /// Run a function on each line of a file and return the restults as a vector.
    /// Note: Function is allowed to modify string.
    template <typename T>
    emp::vector<T> Process(const std::function<T(std::string &)> & fun) {
      emp::vector<T> results(lines.size());
      for (size_t i = 0; i < lines.size(); i++) {
        results[i] = fun(lines[i]);
      }
      return results;
    }

    /// Get a series of lines.
    emp::vector<std::string> Read(size_t start, size_t end) const {
      if (end > lines.size()) end = lines.size();
      return emp::vector<std::string>(lines.begin()+start, lines.begin()+end);
    }

    /// Get a series of lines until a line meets a certain condition.
    emp::vector<std::string> ReadUntil(size_t start, auto test_fun) const {
      size_t end = start;
      while (end < lines.size() && !test_fun(lines[end])) ++end;
      return Read(start, end);
    }

    /// Get a series of lines while lines continue to meet a certain condition.
    emp::vector<std::string> ReadWhile(size_t start, auto test_fun) const {
      size_t end = start;
      while (end < lines.size() && test_fun(lines)) ++end;
      return Read(start, end);
    }

    /// Remove the first column from the file, returning it as a vector of strings.
    emp::vector<std::string> ExtractCol(char delim=',') {
      return Process<std::string>( [delim](std::string & line){
        return string_pop(line, delim);
      });
    }

    /// Remove the first column from the file, returning it as a vector of a specified type.
    template <typename T>
    emp::vector<T> ExtractColAs(char delim=',') {
      return Process<T>( [delim](std::string & line){
        return emp::from_string<T>(string_pop(line, delim));
      });
    }

    /// Convert a row of a file to a vector of string views.
    emp::vector<std::string_view> ViewRowSlices(size_t row_id, char delim=',') {
      return view_slices(lines[row_id], delim);
    }

    /// Remove the first row from the file, returning it as a vector of strings.
    emp::vector<std::string> ExtractRow(char delim=',') {
      // Identify the data as string_views
      emp::vector<std::string_view> sv_row = ViewRowSlices(0, delim);

      // Build the array to return and copy strings into it.
      emp::vector<std::string> out_row(sv_row.size());
      for (size_t i=0; i < sv_row.size(); i++) out_row[i] = sv_row[i];

      // Remove the row to be extrated and return the result.
      lines.erase(begin());
      return out_row;
    }

    /// Remove the first row from the file, returning it as a vector of a specified type.
    template <typename T>
    emp::vector<T> ExtractRowAs(char delim=',') {
      // Identify the data as string_views
      emp::vector<std::string_view> sv_row = ViewRowSlices(0, delim);

      // Build the array to return and copy strings into it.
      emp::vector<T> out_row(sv_row.size());
      for (size_t i=0; i < sv_row.size(); i++) out_row[i] = from_string<T>(sv_row[i]);

      // Remove the row to be extrated and return the result.
      lines.erase(begin());
      return out_row;
    }

    template <typename T>
    emp::vector< emp::vector<T> > ToData(char delim=',') {
      emp::vector< emp::vector<T> > out_data(lines.size());

      emp::vector<std::string_view> sv_row;
      for (size_t row_id = 0; row_id < lines.size(); row_id++) {
        view_slices(lines[row_id], sv_row, delim);
        out_data[row_id].resize(sv_row.size());
        for (size_t i=0; i < sv_row.size(); i++) {
          out_data[row_id][i] = from_string<T>(sv_row[i]);
        }
      }

      return out_data;
    }

    // A File::Scan object allows a user to easily step through a File.
    class Scan {
    private:
      const File & file;
      size_t line = 0;

    public:
      Scan(const File & in, size_t start=0) : file(in), line(start) { }
      Scan(const Scan & in) = default;

      const File & GetFile() const { return file; }
      size_t GetLine() const { return line; }

      bool AtStart() const { return line == 0; }
      bool AtEnd() const { return line >= file.size(); }
      operator bool() const { return !AtEnd(); }

      void Set(size_t in_line) { line = in_line; }
      void Reset() { line = 0; }
      void SetEnd() { line = file.size(); }

      // Get the very next line.
      const std::string & Read() {
        if (line > file.size()) return emp::empty_string();
        return file[line++];
      }

      // Get a block of lines.
      emp::vector<std::string> ReadTo(size_t end) {
        emp_assert(end >= line);
        if (end > file.size()) end = file.size();
        size_t start = line;
        line = end;
        return file.Read(start, end);
      }

      // Get a block of lines, ending when a condition is met.
      emp::vector<std::string> ReadUntil(auto test_fun) {
        auto out = file.ReadUntil(line, test_fun);
        line += out.size();
        return out;
      }

      // Get a block of lines for as lone as a condition is met.
      emp::vector<std::string> ReadWhile(auto test_fun) {
        auto out = file.ReadWhile(line, test_fun);
        line += out.size();
        return out;
      }
    };

    Scan StartScan(size_t start=0) const { return Scan(*this, start); }
  };

}

#endif // #ifndef EMP_IO_FILE_HPP_INCLUDE
