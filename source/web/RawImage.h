/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2015-2018
 *
 *  @file  RawImage.h
 *  @brief Handle the fundamental loading of an image (without Widget tracking)
 */

#ifndef EMP_RAW_IMAGE_H
#define EMP_RAW_IMAGE_H

#include <functional>
#include <map>
#include <string>
#include <vector>

#include "../base/Ptr.h"
#include "../base/vector.h"
#include "../control/Signal.h"
#include "../tools/map_utils.h"

#include "emfunctions.h"
#include "JSWrap.h"

namespace emp {

  namespace internal {
    /// Detailed information about an image
    struct ImageInfo {
      int img_id;                    ///< Unique ID for this image.
      std::string filename;          ///< Full URL of file containing image.
      mutable bool has_loaded;       ///< Is this image finished loading?
      mutable bool has_error;        ///< Were there any errors in loading image?
      Signal<void()> on_load;        ///< Actions for when image is finished loading.
      Signal<void()> on_error;       ///< Actions for when image has trouble loading.

      ImageInfo(const std::string & _filename)
        : img_id(-1), filename(_filename), has_loaded(false), has_error(false), on_load(), on_error()
      {
        size_t loaded_callback = JSWrapOnce( std::function<void()>(std::bind(&ImageInfo::MarkLoaded, this)) );
        size_t error_callback = JSWrapOnce( std::function<void()>(std::bind(&ImageInfo::MarkError, this)) );

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

      /// Trigger this image as loaded.
      void MarkLoaded() {
        has_loaded = true;  // Mark that load is finished for future use.
        on_load.Trigger();  // Trigger any other code that needs to be run now.
        on_load.Clear();    // Now that the load is finished, we don't need to run these again.
      }

      /// Trigger this image as having an error.
      void MarkError() {
        has_error = true;
        emp::Alert(std::string("Error loading image: ") + filename);
        on_error.Trigger();  // Trigger any other code that needs to be run now.
        on_error.Clear();    // Now that the load is finished, we don't need to run these again.
      }

      /// Add a new function to be called when the image finishes loading.
      void OnLoad(const std::function<void()> & callback_fun) {
        on_load.AddAction(callback_fun);
      }

      /// Add a new function to be called if an image load has an error.
      void OnError(const std::function<void()> & callback_fun) {
        on_error.AddAction(callback_fun);
      }
    };

    class ImageManager {
    private:
      emp::vector<Ptr<ImageInfo>> image_info;            ///< Information about each loaded image.
      std::map<std::string, size_t> image_id_map;  ///< Map of filenames to loaded image ids.

    public:
      ImageManager() : image_info(0), image_id_map() { ; }
      ~ImageManager() {
        for (auto ptr : image_info) ptr.Delete();
      }

      /// Is an image with the provided name currently being managed?
      bool Has(const std::string & filename) { return emp::Has(image_id_map, filename); }

      /// Create a new image with the provided name.
      Ptr<ImageInfo> Add(const std::string & filename) {
        emp_assert(Has(image_id_map, filename) == false);
        size_t img_id = image_info.size();
        Ptr<ImageInfo> new_info = NewPtr<ImageInfo>(filename);
        image_info.push_back(new_info);
        image_id_map[filename] = img_id;
        return image_info[img_id];
      }

      /// Get the info about a specified image (loading it only if needed!)
      Ptr<ImageInfo> GetInfo(const std::string & filename) {
        if (Has(filename)) return image_info[ image_id_map[filename] ];
        return Add(filename);
      }
    };

  } // End internal namespace

  /// Fundamental information about a single image.
  class RawImage {
  private:
    Ptr<internal::ImageInfo> info;

    static internal::ImageManager & GetManager() {
      static internal::ImageManager manager;
      return manager;
    }
  public:
    RawImage(const std::string & filename) : info(GetManager().GetInfo(filename)) { ; }
    ~RawImage() { ; }

    int GetImgID() const { return info->img_id; }
    const std::string & GetFilename() const { return info->filename; }
    bool HasLoaded() const { return info->has_loaded; }
    bool HasError() const { return info->has_error; }

    /// Add a new function to be called when the image finishes loading.
    void OnLoad(const std::function<void()> & callback_fun) {
      if (HasLoaded()) callback_fun();
      else info->on_load.AddAction(callback_fun);
    }

    /// Add a new function to be called if an image load has an error.
    void OnError(const std::function<void()> & callback_fun) {
      if (HasError()) callback_fun();
      else info->on_error.AddAction(callback_fun);
    }
  };

}

#endif
