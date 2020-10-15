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
      std::string url;               ///< Full URL of file containing image.
      mutable bool has_loaded;       ///< Is this image finished loading?
      mutable bool has_error;        ///< Were there any errors in loading image?
      Signal<void()> on_load;        ///< Actions for when image is finished loading.
      Signal<void()> on_error;       ///< Actions for when image has trouble loading.

      ImageInfo(const std::string & _url)
        : img_id(-1), url(_url), has_loaded(false), has_error(false), on_load(), on_error()
      {
        size_t loaded_callback = JSWrapOnce( std::function<void()>(std::bind(&ImageInfo::MarkLoaded, this)) );
        size_t error_callback = JSWrapOnce( std::function<void()>(std::bind(&ImageInfo::MarkError, this)) );

        img_id = EM_ASM_INT({
          var url = UTF8ToString($0);
          var img_id = emp_i.images.length;
          emp_i.images[img_id] = new Image();
          emp_i.images[img_id].src = url;

          emp_i.images[img_id].onload = function() {
              emp_i.image_load_count += 1;
              emp.Callback($1);
          };

          emp_i.images[img_id].onerror = function() {
              emp_i.image_error_count += 1;
              emp.Callback($2);
          };

          return img_id;
        }, url.c_str(), loaded_callback, error_callback);
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
        emp::Alert(std::string("Error loading image: ") + url);
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
      emp::vector<Ptr<ImageInfo>> image_info;        ///< Information about each loaded image.
      std::map<std::string, size_t> image_id_map;    ///< Map of urls to loaded image ids.

    public:
      ImageManager() : image_info(0), image_id_map() { ; }
      ~ImageManager() {
        for (auto ptr : image_info) ptr.Delete();
      }

      /// Is an image with the provided name currently being managed?
      bool Has(const std::string & url) { return emp::Has(image_id_map, url); }

      /// Create a new image with the provided name.
      Ptr<ImageInfo> Add(const std::string & url) {
        emp_assert(Has(url) == false);
        size_t img_id = image_info.size();
        Ptr<ImageInfo> new_info = NewPtr<ImageInfo>(url);
        image_info.push_back(new_info);
        image_id_map[url] = img_id;
        return image_info[img_id];
      }

      /// Get the info about a specified image (loading it only if needed!)
      Ptr<ImageInfo> GetInfo(const std::string & url) {
        if (Has(url)) return image_info[ image_id_map[url] ];
        return Add(url);
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
    RawImage(const std::string & url) : info(GetManager().GetInfo(url)) { ; }
    RawImage(const RawImage &) = default;
    ~RawImage() { ; }

    RawImage & operator=(const RawImage &) = default;

    int GetID() const { return info->img_id; }
    const std::string & GetURL() const { return info->url; }
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
