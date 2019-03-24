/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2015-2019
 *
 *  @file  mem_track.h
 *  @brief A set of macros to track how many instances of specific classes are made.
 *  @note Status: BETA
 *
 *  One way of tracking memory leaks is to simply count instances of classes.  The macros here
 *  simplify this process.
 *
 *  To setup, every constructor for a class must incude EMP_TRACK_CONSTRUCT(CLASS_NAME),
 *  and every destructor must have EMP_TRACK_DESTRUCT(CLASS_NAME).  Make sure to avoid
 *  implicit constructors/destructors or counts will be off.
 *
 *  To collect information, EMP_TRACK_COUNT(CLASS_NAME) will provide the current count
 *  for a specific class, and EMP_TRACK_STATUS will translate into a string providing
 *  information about all available classes.
 *
 *  Developer notes:
 *  @todo Currently having issues with the memory map corruption.  Perhaps it needs to use onload?
 *   Also, can try using JS maps instead?  (only when using Emscripten.)
 */

#ifndef EMP_MEM_TRACK
#define EMP_MEM_TRACK


// If EMP_NDEBUG is turned on, turn off mem checks.
#ifdef EMP_NDEBUG
#define EMP_NO_MEM_CHECK
#endif


#include <string>
#include <sstream>

#include "../base/map.h"
#include "alert.h"

#ifdef EMP_NO_MEM_CHECK

#define EMP_TRACK_CONSTRUCT(CLASS_NAME)
#define EMP_TRACK_DESTRUCT(CLASS_NAME)
#define EMP_TRACK_COUNT(CLASS_NAME) 0
#define EMP_TRACK_STATUS std::string("(not in debug mode)")

#else  // DEBUG MODE!

#define EMP_TRACK_CONSTRUCT(CLASS_NAME) emp::internal::TrackMem_Inc(#CLASS_NAME)
#define EMP_TRACK_DESTRUCT(CLASS_NAME) emp::internal::TrackMem_Dec(#CLASS_NAME)
#define EMP_TRACK_COUNT(CLASS_NAME) emp::internal::TrackMem_Count(#CLASS_NAME)
#define EMP_TRACK_STATUS emp::internal::TrackMem_Status()

namespace emp {

  namespace internal {

    static emp::map<std::string,int> & TrackMem_GetMap() {
      static emp::map<std::string,int> * track_mem_class_map = nullptr;
      if (!track_mem_class_map) track_mem_class_map = new emp::map<std::string,int>;
      return *track_mem_class_map;
    }

    static std::string TrackMem_Status() {
      auto & mem_map = TrackMem_GetMap();

      std::stringstream ss;
      for (auto stat : mem_map) {
        ss << "[" << stat.first << "] : " << stat.second << std::endl;
      }
      return ss.str();
    }

    static void TrackMem_Inc(const std::string & class_name) {
      (void) class_name;
      auto & mem_map = TrackMem_GetMap();
      if (mem_map.find(class_name) == mem_map.end()) {
        mem_map[class_name] = 0;
      }
      mem_map[class_name]++;
    }

    static void TrackMem_Dec(const std::string & class_name) {
      (void) class_name;
      auto & mem_map = TrackMem_GetMap();

      // Make sure we are not trying to delete a class that was never registered!
      if (mem_map.find(class_name) == mem_map.end()) {
        emp::CappedAlert(3, "Trying to delete unknown: [", class_name,
                         "]; map size = ", mem_map.size());
        abort();
      }
      mem_map[class_name]--;
      if (mem_map[class_name] < 0) {
        emp::CappedAlert(3, "Trying to delete too many: ", class_name);
        abort();
      }
    }

    static int TrackMem_Count(const std::string & class_name) {
      (void) class_name;
      auto & mem_map = TrackMem_GetMap();

      if (mem_map.find(class_name) == mem_map.end()) {
        return 0;
      }
      return mem_map[class_name];
    }

  }

}
#endif


#endif
