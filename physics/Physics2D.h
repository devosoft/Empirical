#ifndef EMP_PHYSICS_2D_H
#define EMP_PHYSICS_2D_H

///////////////////////////////////////////////////////////////////////////////////////////////
//
//  Physics2D - handles movement and collissions in a simple 2D world.
//
//  Sector2D - One section of a Physics world; shapes are recorded in each sector they occupy.
//
//  CircleBody2D - One individual circular object in the 2D world.
//


#include <vector>
#include <unordered_set>

#include "../tools/Circle.h"

namespace emp {

  template <typename BODY_TYPE, typename BODY_INFO> class Sector2D;

  template <typename BODY_INFO> class CircleBody2D {
  private:
    Circle perimeter;
    BODY_INFO * info;
    Sector2D<CircleBody2D<BODY_INFO>, BODY_INFO> * sector;  // What sector is this body in?
  public:
    CircleBody2D(const Circle & _p, BODY_INFO * _i) : perimeter(_p), info(_i) { ; }
    ~CircleBody2D() { ; }

    const Circle & GetPerimeter() const { return perimeter; }
    const Point & GetAnchor() const { return perimeter.GetCenter(); }
    const Point & GetCenter() const { return perimeter.GetCenter(); }
    BODY_INFO * GetInfo() { return info; }
    Sector2D<CircleBody2D<BODY_INFO>, BODY_INFO> * GetSector() { return sector; }

    CircleBody2D<BODY_INFO> & MoveTo(const Point & new_pos) { perimeter.SetCenter(new_pos); return *this; }
  };

  template <typename BODY_TYPE, typename BODY_INFO> class Sector2D {
  private:
    std::unordered_set<BODY_TYPE> body_set;

  public:
    Sector2D() { ; }
    ~Sector2D() { ; }

    bool HasBody(BODY_TYPE * test_body) const { return body_set.count(test_body); }

    Sector2D<BODY_TYPE, BODY_INFO> & AddBody(BODY_TYPE * in_body) {
      body_set.insert(in_body);
      return *this;
    }
    Sector2D<BODY_TYPE, BODY_INFO> & RemoveBody(BODY_TYPE * out_body) {
      body_set.erase(out_body);
      return *this;
    }

    const std::unordered_set<BODY_TYPE> & GetBodySet() const { return body_set; }
  };

  template <typename BODY_TYPE, typename BODY_INFO> class Physics2D {
  private:
    const int sector_cols;  // Number of sectors in the x-dimension.
    const int sector_rows;  // Number of sectors in the y-dimension.
    const int num_sectors;
    std::vector<Sector2D<BODY_TYPE, BODY_INFO> > sector_matrix; // Flattend matrix of sectors;

  public:
    Physics2D() 
      : sector_cols(100), sector_rows(100)
      , num_sectors(sector_cols * sector_rows)
      , sector_matrix(num_sectors)
    { ; }

  };

};

#endif
