#ifndef EMP_SURFACE_2D_H
#define EMP_SURFACE_2D_H

///////////////////////////////////////////////////////////////////////////////////////////////
//
//  This file defines templated classes to represent a 2D suface capable of maintaining data
//  about which 2D bodies are currently on that surface and rapidly identifying if they are 
//  overlapping.
//
//  Sector2D represents a small, subset of a surface.
//  Surface2D tracks all of the sectors and represents and entire surface.
//
//  BODY_TYPE is the class that represents the body geometry.
//  BODY_INFO represents the internal infomation about the body, including the controller.
//  BASE_TYPE indicates if the physics should be calculated as integer or floating point.

#include "Body2D.h"

namespace emp {

  template <typename BODY_TYPE, typename BODY_INFO, typename BASE_TYPE> class Sector2D {
  private:
    int id;
    std::unordered_set<BODY_TYPE *> body_set;

  public:
    Sector2D() { ; }
    ~Sector2D() { ; }

    int GetID() const { return id; }
    Sector2D<BODY_TYPE, BODY_INFO, BASE_TYPE> & SetID(int _id) { id = _id; return *this; }

    bool HasBody(BODY_TYPE * test_body) const { return body_set.count(test_body); }

    Sector2D<BODY_TYPE, BODY_INFO, BASE_TYPE> & AddBody(BODY_TYPE * in_body) {
      body_set.insert(in_body);
      return *this;
    }
    Sector2D<BODY_TYPE, BODY_INFO, BASE_TYPE> & RemoveBody(BODY_TYPE * out_body) {
      body_set.erase(out_body);
      return *this;
    }

    const std::unordered_set<BODY_TYPE *> & GetBodySet() const { return body_set; }
  };

  template <typename BODY_TYPE, typename BODY_INFO, typename BASE_TYPE> class Surface2D {
  private:
    const BASE_TYPE width;       // How wide is the overall surface?
    const BASE_TYPE height;      // How tall is the overall surface?
    const int sector_cols;    // Number of sectors in the x-dimension.
    const int sector_rows;    // Number of sectors in the y-dimension.
    const int num_sectors;    // Total number of sectors on this surface.
    const BASE_TYPE col_width;   // How wide is each individual sector?
    const BASE_TYPE row_height;  // How tall is each individual sector?

    std::vector<Sector2D<BODY_TYPE, BODY_INFO, BASE_TYPE> > sector_matrix; // Flattend matrix of sectors;
    std::unordered_set<BODY_TYPE *> body_set;                              // Set of all bodies on surface

    int GetSectorID(int row, int col) const { return row * sector_cols + col; }
    int GetSectorID(const Point<BASE_TYPE> & point) const {
      return GetSectorID(point.GetX() / col_width, point.GetY() / row_height);
    }
    Sector2D<BODY_TYPE, BODY_INFO, BASE_TYPE> & GetSector(int row, int col) {
      return sector_matrix[row*sector_cols+col];
    }
    Sector2D<BODY_TYPE, BODY_INFO, BASE_TYPE> & GetSector(const Point<BASE_TYPE> & pos) {
      return sector_matrix[GetSectorID(pos)];
    }
    Sector2D<BODY_TYPE, BODY_INFO, BASE_TYPE> & GetSector(int id) {
      return sector_matrix[id];
    }

  public:
    Surface2D(BASE_TYPE _width, BASE_TYPE _height, int _cols=100, int _rows=100) 
      : width(_width), height(_height)
      , sector_cols(_cols), sector_rows(_rows)
      , num_sectors(sector_cols * sector_rows)
      , col_width(width / (double) sector_cols), row_height(height / (double) sector_rows)
      , sector_matrix(num_sectors)
    {
      // Let each sector know its ID.
      for (int i = 0; i < num_sectors; i++) sector_matrix[i].SetID(i);
    }
    ~Surface2D() { ; }

    const std::unordered_set<BODY_TYPE *> & GetBodySet() const { return body_set; }

    Surface2D<BODY_TYPE, BODY_INFO, BASE_TYPE> & AddBody(BODY_TYPE * new_body) {
      auto & sector = GetSector(new_body->GetAnchor());  // Determine which sector this body is in
      body_set.insert(new_body);                         // Add body to master list
      sector.AddBody(new_body);                          // Add body to the sector it's in
      new_body->SetSector(&sector);                      // Track sector in body
      return *this;
    }

    Surface2D<BODY_TYPE, BODY_INFO, BASE_TYPE> & MoveBody(BODY_TYPE * body, const Point<BASE_TYPE> & new_pos) {
      // Determine if the sector is changing; if so do some more work.
      const int start_id = body->GetSector()->GetID();
      const int end_id = GetSectorID(new_pos);
      if (start_id != end_id) {
        body->GetSector()->RemoveBody(body);    // Remove body from old sector
        auto & new_sector = GetSector(end_id);  // Determine which sector this body is in
        new_sector.AddBody(body);               // Add body to the sector it's in
        body->SetSector(&new_sector);           // Track sector in body
      }
      body->MoveTo(new_pos);                    // Update the position of the body
      return *this;
    }
  };


};

#endif
