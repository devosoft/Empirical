#ifndef EMP_RAW_IMAGE_H
#define EMP_RAW_IMAGE_H

#include <list>
#include <map>
#include <string>

#include "tools/callbacks.h"
#include "emtools/emfunctions.h"

namespace emp {

  class RawImage {
  private:
    std::string filename;
    int img_id;
    mutable bool has_loaded;
    mutable bool has_error;
    mutable std::list<Callback *> callbacks_on_load;
    mutable std::list<Callback *> callbacks_on_error;

    MethodCallback<RawImage> loaded_callback;
    MethodCallback<RawImage> error_callback;
  public:
    RawImage(const std::string & _filename)
      : filename(_filename), has_loaded(false), has_error(false)
      , loaded_callback(this, &RawImage::MarkLoaded)
      , error_callback(this, &RawImage::MarkError)
    {
      img_id = EM_ASM_INT({
        var file = Pointer_stringify($0);
        var img_id = emp_info.images.length;
        emp_info.images[img_id] = new Image();
        emp_info.images[img_id].src = file;

        emp_info.images[img_id].onload = function() {
            emp_info.image_load_count += 1;
            empJSDoCallback($1, 0);
        };

        emp_info.images[img_id].onerror = function() {
            emp_info.image_error_count += 1;
            empJSDoCallback($2, 0);
        };

        return img_id;
      }, filename.c_str(), (int) &loaded_callback, (int) &error_callback);

    }
    ~RawImage() { ; }

    const std::string & GetFilename() const { return filename; }
    int GetImgID() const { return img_id; }
    bool HasLoaded() const { return has_loaded; }
    bool HasError() const { return has_error; }

    void MarkLoaded() {
      has_loaded = true;

      while (callbacks_on_load.size()) {
        Callback * cur_callback = callbacks_on_load.front();
        callbacks_on_load.pop_front();
        cur_callback->DoCallback();
      }
    }

    void MarkError() {
      has_error = true;
      emp::Alert(std::string("Error loading image: ") + filename);

      while (callbacks_on_error.size()) {
        Callback * cur_callback = callbacks_on_error.front();
        callbacks_on_error.pop_front();
        cur_callback->DoCallback();
      }
    }

    void AddLoadCallback(Callback * load_callback) {
      callbacks_on_load.push_back(load_callback);
    }
    void AddErrorCallback(Callback * error_callback) {
      callbacks_on_error.push_back(error_callback);
    }
    template <class T> void AddLoadCallback(T * cb_target, void (T::*method_ptr)()) {
      MethodCallback<T> * cb = new MethodCallback<T>(cb_target, method_ptr);
      cb->SetDisposible(true); // Make sure callback gets deleted when triggered.
      callbacks_on_load.push_back(cb);
    }
    template <class T> void AddErrorCallback(T * cb_target, void (T::*method_ptr)()) {
      MethodCallback<T> * cb = new MethodCallback<T>(cb_target, method_ptr);
      cb->SetDisposible(true); // Make sure callback gets deleted when triggered.
      callbacks_on_error.push_back(cb);
    }
  };

  static std::map<std::string, RawImage *> raw_image_map;
  
  RawImage & LoadRawImage(const std::string & filename, Callback * load_callback=NULL, Callback * error_callback=NULL) {
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
      if (raw_image->HasLoaded()) load_callback->DoCallback();
      else raw_image->AddLoadCallback(load_callback);
    }

    if (error_callback) {
      if (raw_image->HasError()) error_callback->DoCallback();
      else raw_image->AddErrorCallback(error_callback);
    }

    return *raw_image;
  }
  
};

#endif
