// This file is part of Empirical, https://github.com/mercere99/Empirical/, and is  
// Copyright (C) Michigan State University, 2015. It is licensed                
// under the MIT Software license; see doc/LICENSE

#ifndef EMP_RAW_IMAGE_H
#define EMP_RAW_IMAGE_H

#include <functional>
#include <map>
#include <string>
#include <vector>

#include "emfunctions.h"
#include "JSWrap.h"

namespace emp {

  class RawImage {
  private:
    std::string filename;
    int img_id;
    mutable bool has_loaded;
    mutable bool has_error;
    mutable std::vector<uint32_t> callbacks_on_load;
    mutable std::vector<uint32_t> callbacks_on_error;

    uint32_t loaded_callback;
    uint32_t error_callback;
  public:
    RawImage(const std::string & _filename)
      : filename(_filename), has_loaded(false), has_error(false)
    {
      loaded_callback = JSWrapOnce( std::function<void()>(std::bind(&RawImage::MarkLoaded, this)) );
      error_callback = JSWrapOnce( std::function<void()>(std::bind(&RawImage::MarkError, this)) );

      img_id = EM_ASM_INT({
        var file = Pointer_stringify($0);
        var img_id = emp_info.images.length;
        emp_info.images[img_id] = new Image();
        emp_info.images[img_id].src = file;

        emp_info.images[img_id].onload = function() {
            emp_info.image_load_count += 1;
            emp.Callback($1);
        };

        emp_info.images[img_id].onerror = function() {
            emp_info.image_error_count += 1;
            emp.Callback($2);
        };

        return img_id;
      }, filename.c_str(), loaded_callback, error_callback);

    }
    ~RawImage() { ; }

    const std::string & GetFilename() const { return filename; }
    int GetImgID() const { return img_id; }
    bool HasLoaded() const { return has_loaded; }
    bool HasError() const { return has_error; }

    void MarkLoaded() {
      has_loaded = true;
      for (uint32_t id : callbacks_on_load) empCppCallback(id);
      callbacks_on_load.resize(0);
    }

    void MarkError() {
      has_error = true;
      emp::Alert(std::string("Error loading image: ") + filename);

      for (uint32_t id : callbacks_on_error) empCppCallback(id);
      callbacks_on_error.resize(0);
    }

    void AddLoadCallback(const std::function<void()> & callback_fun) {
      callbacks_on_load.push_back( JSWrapOnce(callback_fun) );
    }
    void AddErrorCallback(const std::function<void()> & callback_fun) {
      callbacks_on_error.push_back( JSWrapOnce(callback_fun) );
    }
  };

  namespace internal {
    static std::map<std::string, RawImage *> & RawImageMap() {
      static std::map<std::string, RawImage *> raw_image_map;
    }
  }
  
  RawImage & LoadRawImage(const std::string & filename,
                          const std::function<void()> & load_callback=NULL,
                          const std::function<void()> & error_callback=NULL)
  {
    auto & raw_image_map = internal::RawImageMap();
    auto it = raw_image_map.find(filename);
    RawImage * raw_image;
    if (it == raw_image_map.end()) {        // New filename
      raw_image = new RawImage(filename);
      raw_image_map[filename] = raw_image;
    }
    else {                                  // Pre-existing filename
      raw_image = raw_image_map[filename];
    }

    if (load_callback) {
      if (raw_image->HasLoaded()) load_callback();
      else raw_image->AddLoadCallback(load_callback);
    }

    if (error_callback) {
      if (raw_image->HasError()) error_callback();
      else raw_image->AddErrorCallback(error_callback);
    }

    return *raw_image;
  }
  
};

#endif
