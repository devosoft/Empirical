/*
 *  This file is part of Empirical, https://github.com/devosoft/Empirical
 *  Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  date: 2020
*/
/**
 *  @file
 *  @brief Tools for working with ascii output.
 *  @note  Status: ALPHA
 *
 */

#ifndef EMP_IO_ASCII_UTILS_HPP_INCLUDE
#define EMP_IO_ASCII_UTILS_HPP_INCLUDE

#include <functional>
#include <iostream>
#include <ostream>
#include <stddef.h>

#include "../base/assert.hpp"
#include "../base/vector.hpp"
#include "../datastructs/vector_utils.hpp"
#include "../tools/String.hpp"

#include "ANSI.hpp"
#include "io_utils.hpp"

namespace emp {

  emp::String IOCharName(IOChar key) {
    emp::String key_name = key.GetName();
    return emp::MakeString('\'', key_name.AsANSIMagenta(), '\'');
  }

  class ANSIOptionMenu {
  private:
    class Entry {
    private:
      IOChar key;                       // E.g., 'h'
      emp::String effect_text;          // E.g., "Help"
      std::function<bool()> trigger;    // E.g., [](){ PrintHelp(); return false; }
      bool is_add_on = false;           // Should this option be an "or" on previous line?
      bool is_active = true;            // Should this option be displayed at all?
      bool is_visible = true;           // Should this be listed in a regular menu?
      
    public:
      Entry(int key, emp::String effect_text, std::function<bool()> trigger,
            bool is_add_on=false, bool is_active=true, bool is_visible=true)
        : key(key), effect_text(effect_text), trigger(trigger),
          is_add_on(is_add_on), is_active(is_active), is_visible(is_visible) { }

      // Default trigger to false (not done) if no return value is provided.
      Entry(int key, emp::String effect_text, std::function<void()> trigger,
            bool is_add_on=false, bool is_active=true, bool is_visible=true)
        : key(key), effect_text(effect_text), trigger([trigger](){ trigger(); return false; }),
          is_add_on(is_add_on), is_active(is_active), is_visible(is_visible) { }

      bool IsVisible() const { return is_active && is_visible; }

      void Print() const {
        if (is_add_on) emp::Print(" or ", IOCharName(key), " to ", effect_text);
        else emp::Print("\n", IOCharName(key), " to ",  effect_text);
      }

      void PrintActive() const { if (is_active) Print(); }
      void PrintVisible() const { if (IsVisible()) Print(); }

      bool Trigger() const { return trigger(); }

      void SetTrigger(std::function<bool()> new_trigger) { trigger = new_trigger; }
    };

    emp::String question;
    emp::vector<Entry> entry_list;
    std::unordered_map<int, size_t> key_map; // Map of keys to entires.

    // Get an entry associated with a particular key.
    Entry & GetEntryByKey(int key) {
      emp_assert(key_map.contains(key));
      return entry_list[ key_map[key] ];
    }

  public:
    ANSIOptionMenu & SetQuestion(emp::String in_q) { question = in_q; return *this; }

    ANSIOptionMenu & AddOption(int key, emp::String effect_text, std::function<bool()> trigger,
                               bool is_add_on=false, bool is_active=true, bool is_visible=true) {
      key_map[key] = entry_list.size();
      entry_list.emplace_back(key, effect_text, trigger, is_add_on, is_active, is_visible);
      return *this;
    }

    // And an option that allows for a keystroke, but does not print by default.
    ANSIOptionMenu & AddSilent(int key, emp::String effect_text, std::function<bool()> trigger,
                               bool is_add_on=false, bool is_active=true) {
      return AddOption(key, effect_text, trigger, is_add_on, is_active, false);
    }

    ANSIOptionMenu & AddAlias(int key) {
      emp_assert(entry_list.size() > 0); // No entries to alias!
      key_map[key] = entry_list.size()-1;
      return *this;
    }

    ANSIOptionMenu & SetTrigger(int key, std::function<bool()> trigger) {
      emp_assert(key_map.contains(key));
      GetEntryByKey(key).SetTrigger(trigger);
      return *this;
    }
    
    void PrintOptions() const {
      for (const Entry & entry : entry_list) {
        entry.PrintVisible();
      }
      PrintLn();
    }

    void Run() const {
      if (question.size()) Print(question);
      PrintOptions();
      bool done = false;
      while (!done) {
        int key = GetIOChar();
        auto it = key_map.find(key);
        if (it == key_map.end()) {
          PrintLn("Unknown key ", IOCharName(key), '.');
          continue;
        }
        size_t id = it->second;
        done = entry_list[id].Trigger();
      }
    }
};

  /// The following function prints an ascii bar graph on to the screen (or provided stream).
  template <typename T>
  void AsciiBarGraph( emp::vector<T> data,          ///< Data to graph
                      size_t max_width=80,          ///< What's the widest bars allowed?
                      bool show_scale=true,         ///< Should we show the scale at bottom.
                      bool max_scale_1=true,        ///< Should we limit scaling to 1:1?
                      std::ostream & os=std::cout)  ///< Where to output the bar graph?
  {
    T min_size = emp::FindMin(data);
    T max_size = emp::FindMax(data);
    double scale = ((double) max_width) / ((double) max_size);
    if (max_scale_1 && scale > 1.0) scale = 1.0;
    for (T datum : data) {
      double bar_width = datum * scale;
      while (bar_width >= 1.0) { os << '='; bar_width -= 1.0; }
      if (bar_width > 0.0) os << '~';
      os << "  (" << datum << ")\n";
    }
    if (show_scale) {
      os << "SCALE: = -> " << (1.0 / scale) << std::endl;
    }
  }


  /// Take the input data, break it into bins, and print it as a bar graph.
  template <typename T>
  void AsciiHistogram(emp::vector<T> data,           ///< Data to graph
                      size_t num_bins=40,            ///< How many bins in histogram?
                      size_t max_width=80,           ///< What's the widest bars allowed?
                      bool show_scale=true,          ///< Should we show the scale at bottom?
                      std::ostream & os=std::cout)   ///< Where to output the bar graph?
  {
    T min_val = emp::FindMin(data);
    T max_val = emp::FindMax(data);
    T val_range = max_val - min_val;
    T bin_width = val_range / (T) num_bins;

    emp::vector<size_t> bins(num_bins, 0);
    for (T d : data) {
      size_t bin_id = (size_t) ( (d - min_val) / bin_width );
      if (bin_id == num_bins) bin_id--;
      bins[bin_id]++;
    }

    AsciiBarGraph<size_t>(bins, max_width, show_scale, true, os);
  }

}

#endif // #ifndef EMP_IO_ASCII_UTILS_HPP_INCLUDE
