#ifndef EMP_MEM_TRACK
#define EMP_MEM_TRACK

//////////////////////////////////////////////////////////////////////////////////////////
//
//  This class creates macros to track how many instances of specific classes are made.
//


#include <map>
#include <string>

#include "assert.h"

#ifdef NDEBUG

#define EMP_TRACK_CONSTRUCT(CLASS_NAME)
#define EMP_TRACK_DESTRUCT(CLASS_NAME)
#define EMP_TRACK_COUNT(CLASS_NAME) 0

#else  // DEBUG MODE!

#define EMP_TRACK_CONSTRUCT(CLASS_NAME) emp::internal::TrackMem_Inc(#CLASS_NAME)
#define EMP_TRACK_DESTRUCT(CLASS_NAME) emp::internal::TrackMem_Dec(#CLASS_NAME)
#define EMP_TRACK_COUNT(CLASS_NAME) emp::internal::TrackMem_Count(#CLASS_NAME)

#endif


namespace emp {

  namespace internal {
    std::map<std::string,int> track_mem_class_map;

    void TrackMem_Inc(const std::string & class_name) {
      if (track_mem_class_map.find(class_name) == track_mem_class_map.end()) {
        track_mem_class_map[class_name] = 0;
      }
      track_mem_class_map[class_name]++;
    }

    void TrackMem_Dec(const std::string & class_name) {
      // Make sure we are not trying to delete a class that was never registered!
      emp_assert(track_mem_class_map.find(class_name) != track_mem_class_map.end());
      track_mem_class_map[class_name]--;
    }

    int TrackMem_Count(const std::string & class_name) {
      if (track_mem_class_map.find(class_name) == track_mem_class_map.end()) {
        track_mem_class_map[class_name] = 0;
      }
      return track_mem_class_map[class_name];
    }
      
  }

}

#endif
