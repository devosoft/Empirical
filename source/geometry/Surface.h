//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016-2018.
//  Released under the MIT Software license; see doc/LICENSE
//  Previously named Surface2D.h
//
//
//  This file defines a templated class to represent a 2D suface capable of maintaining data
//  about which 2D bodies are currently on that surface and rapidly identifying if they are
//  overlapping.
//
//  BODY_TYPE is the class that represents the body geometry.
//
//  Developer Notes:
//  * Should add enums to control boundary conditions (INFINITE, TOROIDAL, BOUNDED)
//  * Incorporate physics?  Can have various plug-in modules.

#ifndef EMP_SURFACE2_H
#define EMP_SURFACE2_H

#include <algorithm>
#include <functional>

#include "../base/Ptr.h"
#include "../meta/TypePack.h"
#include "../tools/TypeTracker.h"
#include "../tools/vector_utils.h"

#include "Angle2D.h"
#include "Point2D.h"

namespace emp {

  template <typename... BODY_TYPES>
  class Surface {
  public:
    using body_types = TypePack<BODY_TYPES...>;

    class BodyInfo {
    private:      
      TrackedVar body_ptr;  ///< Pointer to the bodies
      size_t id;            ///< Index in body_set to find body info
      Point center;         ///< Center position of this body on surface.
      double radius;        ///< Size of this body
      size_t color;         ///< Color of this body

    public:
      BodyInfo(TrackedVar && _ptr, size_t _id, Point _center, double _radius, size_t _color=0)
        : body_ptr(_ptr), id(_id), center(_center), radius(_radius), color(_color) { ; }
      BodyInfo(size_t _id=(size_t)-1, Point _center=Point(), double _radius=0.0)
        : BodyInfo(TrackedVar(nullptr), _id, _center, _radius) { ; }
      BodyInfo(const BodyInfo &) = default;
      BodyInfo(BodyInfo  &&) = default;

      TrackedVar & GetBodyPtr() { return body_ptr; }
      size_t GetID() const { return id; }
      Point GetCenter() const { return center; }
      double GetRadius() const { return radius; }
      size_t GetColor() const { return color; }

      void SetCenter(Point _in) { center = _in; }
      void SetRadius(double _in) { radius = _in; }
      void SetColor(size_t _in) { color = _in; }

      BodyInfo & operator=(const BodyInfo &) = default;
      BodyInfo & operator=(BodyInfo &&) = default;

      bool IsActive() const { return id != (size_t) -1; }
      void Deactivate() { radius = 0.0; id = (size_t) -1; }
    };

    using sector_t = emp::vector<size_t>;

  protected:
    /// The TypeTracker manages pointers to arbitrary surface objects.
    TypeTracker< emp::Ptr<BODY_TYPES>... > type_tracker;

    const Point max_pos;             ///< Lower-left corner of the surface.
    emp::vector<BodyInfo> body_set;  ///< Set of all bodies on surface
    emp::vector<size_t> open_ids;    ///< Set of body_set positions ready for re-use.

    // Data tracking the current bodies on this surface using SECTORS.
    bool data_active;                ///< Are we trying to keep data up-to-date?
    double max_radius;               ///< Largest radius of any body.
    size_t num_cols;                 ///< How many cols of sectors are there?
    size_t num_rows;                 ///< How many rows of sectors are there?
    size_t num_sectors;              ///< How many total sectors are there?
    double sector_width;             ///< How wide is each sector?
    double sector_height;            ///< How tall is each sector?
    emp::vector<sector_t> sectors;   ///< Which bodies are in each sector?

    // Make sure there are num_sectors sectors and remove all bodies from existing ones.
    void InitSectors() {
      size_t min_size = std::min(num_sectors, sectors.size());
      sectors.resize(num_sectors);
      for (size_t i = 0; i < min_size; i++) sectors[i].clear();
    }

    // Keep track of the largest body size found.
    // Note: Uses watermarking, so largest body will never shrink, even if removed
    //       unless the user explicitly calls RefreshBodySize()
    inline void TestBodySize(BodyInfo & body) {
      const double cur_radius = body.GetRadius();
      if (cur_radius > max_radius) {
        max_radius = cur_radius;      // Record the new radius.
        data_active = false;          // May need to rebuild sectors, so deactivate data.
      }
    }

    // Clear out the watermarked body size and update the current largest.
    inline void RefreshBodySize() {
      max_radius = 0.0;
      for (BodyInfo & body : body_set) if (body.IsActive()) TestBodySize(body);      
    }

    // Determine which sector a body should be in.
    size_t FindSector(Point point) {
      // Where is the body?
      const double body_x = point.GetX();
      const double body_y = point.GetY();

      // Make sure the body is in the sectors.
      emp_assert(body_x >= 0.0 && body_y >= 0.0, body_x, body_y);
      emp_assert(point <= max_pos, point, max_pos);

      // Determine which sector the body is in.
      const double sector_x = body_x / sector_width;
      const double sector_y = body_y / sector_height;

      const size_t cur_col = (size_t) sector_x;
      const size_t cur_row = (size_t) sector_y;
      const size_t cur_sector = cur_col + cur_row * num_cols;

      emp_assert(cur_sector < sectors.size());
      return cur_sector;
    }

    // Place an active body into a sector.
    void PlaceBody(BodyInfo & body) {
      if (body.IsActive()) {                          // Only place active bodies.
        size_t cur_sector = FindSector(body.GetCenter());
        emp_assert(body.GetID() < body_set.size(), body.GetID());
        emp_assert(body.GetCenter() <= max_pos, body.GetCenter(), max_pos);
        sectors[cur_sector].push_back(body.GetID());
      }
    }

    // Cleanup all of the data and mark the data as active.
    void Activate() {
      if (data_active) return; // Already active!

      // Figure out the actual number of sectors to use (currently no more than 1024).
      const double max_diameter = max_radius * 2.0;
      emp_assert(max_diameter < max_pos.GetX());  // Surface must be bigger than biggest body
      emp_assert(max_diameter < max_pos.GetY());
      num_cols = (size_t) std::min(max_pos.GetX()/max_diameter, 32.0);
      num_rows = (size_t) std::min(max_pos.GetY()/max_diameter, 32.0);

      num_sectors = num_cols * num_rows;
      sector_width = max_pos.GetX() / (double) num_cols;
      sector_height = max_pos.GetY() / (double) num_rows;

      InitSectors();   // Now that we know the sizes, we can initialize sectors.

      // Put all of the bodies into sectors
      for (BodyInfo & body : body_set) if (body.IsActive()) PlaceBody(body);

      data_active = true;
    }
  public:
    /// Create a surface providing maxumum size.
    Surface(Point _max) : max_pos(_max), body_set(), max_radius(0.0) { ; }
    ~Surface() { Clear(); }

    double GetWidth() const { return max_pos.GetX(); }
    double GetHeight() const { return max_pos.GetY(); }
    const Point & GetMaxPosition() const { return max_pos; }
    const emp::vector<BodyInfo> & GetBodySet() const { return body_set; }

    /// Determine if an id represents an active body on a surface.
    bool IsActive(size_t id) const {
      return body_set[id].IsActive();
    }
    template <typename ORIGINAL_T>
    Ptr<ORIGINAL_T> GetPtr(size_t id) const {
      emp_assert(body_set[id].IsActive());
      return type_tracker.template ToType<Ptr<ORIGINAL_T>>( body_set[id].GetBodyPtr() );
    }
    Point GetCenter(size_t id) const {
      emp_assert(body_set[id].IsActive());
      return body_set[id].GetCenter();
    }
    double GetRadius(size_t id) const {
      emp_assert(body_set[id].IsActive());
      return body_set[id].GetRadius();
    }
    size_t GetColor(size_t id) const {
      emp_assert(body_set[id].IsActive());
      return body_set[id].GetColor();
    }

    // template <typename BODY_T>
    // void SetPtr(size_t id, Ptr<BODY_T> _in) {
    //   emp_assert(body_set[id].IsActive());
    //   body_set[id].body_ptr = type_tracker.Convert(_in);
    // }
    void SetCenter(size_t id, Point _in) {
      emp_assert(body_set[id].IsActive());
      emp_assert(_in <= max_pos, _in, max_pos);
      emp_assert(_in >= emp::Point());

      // If data not active, just move the body.
      if (data_active == false) body_set[id].SetCenter(_in);
      // Otherwise need to update data.
      else {
        const size_t old_sector_id = FindSector(body_set[id].GetCenter());
        const size_t new_sector_id = FindSector(_in);
        body_set[id].SetCenter(_in);
        if (old_sector_id != new_sector_id) {
          emp::RemoveValue(sectors[old_sector_id], id); // Remove from old sector.
          sectors[new_sector_id].push_back(id);         // Insert into new sector.
        }
      }
    }
    void Translate(size_t id, Point translation) {
      emp_assert(body_set[id].IsActive());
      SetCenter(id, body_set[id].GetCenter() + translation);
    }
    void TranslateWrap(size_t id, Point translation) {
      emp_assert(body_set[id].IsActive());
      Point new_pos = body_set[id].GetCenter() + translation;
      new_pos.Mod(max_pos);
      SetCenter(id, new_pos);
    }

    void SetRadius(size_t id, double _in) {
      emp_assert(body_set[id].IsActive());
      BodyInfo & body = body_set[id];
      body.SetRadius(_in);
      TestBodySize(body);
    }
    void ScaleRadius(size_t id, double scale) {
      emp_assert(body_set[id].IsActive());
      BodyInfo & body = body_set[id];
      body.SetRadius(body.GetRadius() * scale);
      TestBodySize(body);
    }
    void SetColor(size_t id, size_t _in) {
      emp_assert(body_set[id].IsActive());
      body_set[id].SetColor(_in);
    }
    
    void RemoveBody(size_t id) {
      emp_assert(body_set[id].IsActive());
      body_set[id].Deactivate();
      open_ids.push_back(id);
      if (data_active) {
        const size_t sector_id = FindSector(body_set[id].GetCenter());
        emp::RemoveValue(sectors[sector_id], id);
      }
    }

    /// Add a single body; return its unique ID.
    template <typename BODY_T>
    size_t AddBody(Ptr<BODY_T> _body, Point _center, double _radius, size_t _color=0) {
      static_assert(body_types::template Has<BODY_T>(),
                    "Can only add a body to surface if type was declared.");
      emp_assert(_center <= max_pos, _center, max_pos);

      const size_t id = (open_ids.size()) ? open_ids.back() : body_set.size();
      BodyInfo info = { type_tracker.Convert(_body), id, _center, _radius, _color };

      // Recycle an id if we can (otherwise extend the array)
      if (open_ids.size()) {
        open_ids.pop_back();
        body_set[id] = info; // Add body to master list
      } else {
        body_set.push_back(info);                         // Add body to master list
      }
      TestBodySize(info);                                 // Keep track of largest body seen.
      if (data_active) PlaceBody(info);                   // Add new body to a sector (if tracking).
      return id;
    }

    /// Allow bodies to be provided as raw pointers as well.
    template <typename BODY_T>
    size_t AddBody(BODY_T * _body, Point _center, double _radius, size_t _color=0) {
      return AddBody(emp::ToPtr<BODY_T>(_body), _center, _radius, _color);
    }

    /// Remove all bodies from the surface.
    Surface & Clear() {
      data_active = false;
      body_set.clear();
      max_radius = 0.0;
      num_sectors = 0;
      return *this;
    }

    /// Add a function to call in the case of overlaps (using an std::function object)
    template <typename BODY1_T, typename BODY2_T>
    void AddOverlapFun(const std::function< void(BODY1_T &, BODY2_T &) > & overlap_fun) {
      type_tracker.template AddFunction(
        [overlap_fun](Ptr<BODY1_T> ptr1, Ptr<BODY2_T> ptr2){ overlap_fun(*ptr1, *ptr2); }
      );
    }

    /// Add a function to call in the case of overlaps (using a function pointer)
    template <typename BODY1_T, typename BODY2_T>
    void AddOverlapFun( void (*overlap_fun)(BODY1_T &, BODY2_T &) ) {
      AddOverlapFun( std::function<void(BODY1_T&, BODY2_T&)>(overlap_fun) );
    }
 
    /// Add a to call a lambda function in the case of overlaps (using a function pointer)
    template <typename LAMBDA_T>
    void AddOverlapFun( const LAMBDA_T & fun ) {
      AddOverlapFun( to_function(fun) );
    }


    /// Determine if two bodies overlap.
    static inline bool TestOverlap(const BodyInfo & body1, const BodyInfo & body2) {
      const Point xy_dist = body1.GetCenter() - body2.GetCenter();
      const double sqr_dist = xy_dist.SquareMagnitude();
      const double total_radius = body1.GetRadius() + body2.GetRadius();
      const double sqr_radius = total_radius * total_radius;
      return (sqr_dist < sqr_radius);
    }

    // Determine if a body overlaps with any others in a specified sector.
    inline void FindSectorOverlaps(BodyInfo & body1, size_t sector_id,
                                   size_t start_id=0) {
      emp_assert(body1.IsActive());
      auto & sector = sectors[sector_id];
      for (size_t body2_id = start_id; body2_id < sector.size(); body2_id++){
        if (body_set[body2_id].IsActive() == false) continue;
        BodyInfo & body2 = body_set[sector[body2_id]];
        if (TestOverlap(body1, body2)) type_tracker(body1.GetBodyPtr(), body2.GetBodyPtr());
      }
    }

    // The following function will test all relevant pairs of bodies and run the passed-in
    // function on those objects that overlap.

    void FindOverlaps() {
      Activate();  // Make sure data structures are setup.

      // Loop through all of the sectors to identify collisions.
      for (size_t sector_id = 0; sector_id < num_sectors; sector_id++) {
       sector_t & cur_sector = sectors[sector_id];

        const size_t sector_col = sector_id % num_cols;
        const size_t sector_row = sector_id / num_cols;
        const bool left_ok = (sector_col > 0);
        const bool up_ok = (sector_row > 0);
        const bool ul_ok = left_ok && up_ok;
        const bool ur_ok = up_ok && (sector_col < num_cols - 1);
        const size_t ul_sector_id = sector_id - num_cols - 1;

        // Loop through all bodies in this sector
        for (size_t body_id = 0; body_id < cur_sector.size(); body_id++) {
          if (body_set[body_id].IsActive() == false) continue;
          BodyInfo & body = body_set[cur_sector[body_id]];

          // Compare against the bodies after this one in the current sector.
          FindSectorOverlaps(body, sector_id, body_id+1);      // Test remaining bodies here.
          if (ul_ok) FindSectorOverlaps(body, ul_sector_id);   // Test bodies to upper-left.
          if (up_ok) FindSectorOverlaps(body, ul_sector_id+1); // Test bodies above
          if (ur_ok) FindSectorOverlaps(body, ul_sector_id+2); // Test bodies to upper-right.
          if (left_ok) FindSectorOverlaps(body, sector_id-1);  // Test bodies to left.
        }
      }

      // Make sure all bodies are in a legal position on the surface.
      // @CAO: Need to move to physics!
      // for (Ptr<BODY_TYPE> cur_body : body_set) {
      //   if (body_set[id].IsActive() == false) continue;
      //   cur_body->FinalizePosition(max_pos);
      // }
    }

    /// Determine if there are any overlaps with a single provided body (that may or may not be on surface).
    void FindOverlap(BodyInfo & body) {
      Activate();  // Make sure data structures are setup.

      emp_assert(body.IsActive());
      const size_t sector_id = FindSector(body.GetCenter());
      const size_t sector_col = sector_id % num_cols;
      const size_t sector_row = sector_id / num_cols;
      auto & cur_sector = sectors[sector_id];

      // Compare against bodies in its own sector.
      for (size_t body2_id : cur_sector) {
        emp_assert(body_set[body2_id].IsActive());
        if (body.GetID() == body2_id) continue;             // Don't match with self!
        BodyInfo & body2 = body_set[body2_id];
        if (TestOverlap(body, body2)) type_tracker(body.GetBodyPtr(), body2.GetBodyPtr());
      }

      const bool left_ok  = (sector_col > 0);
      const bool right_ok = (sector_col < num_cols - 1);
      const bool up_ok    = (sector_row > 0);
      const bool down_ok  = (sector_row < num_rows - 1);

      const size_t up_sector_id = sector_id - num_cols;
      const size_t down_sector_id = sector_id + num_cols;

      // Compare against bodies in other sectors....
      if (up_ok)               FindSectorOverlaps(body, up_sector_id);
      if (up_ok && left_ok)    FindSectorOverlaps(body, up_sector_id-1);
      if (up_ok && right_ok)   FindSectorOverlaps(body, up_sector_id+1);
      if (left_ok)             FindSectorOverlaps(body, sector_id-1);
      if (right_ok)            FindSectorOverlaps(body, sector_id+1);
      if (down_ok)             FindSectorOverlaps(body, down_sector_id); 
      if (down_ok && left_ok)  FindSectorOverlaps(body, down_sector_id-1);
      if (down_ok && right_ok) FindSectorOverlaps(body, down_sector_id+1);
    }
    
    void FindOverlap(size_t body_id) {
      emp_assert(body_set[body_id].IsActive());
      FindOverlap(body_set[body_id]);
    }

    // Find overlaps using a distance from a point.
    void FindOverlap(Point center, double radius) {
      BodyInfo tmp_body(center, radius);
      FindOverlap(tmp_body);
    }

  };


}

#endif
