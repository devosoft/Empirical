/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2018-2021.
 *
 *  @file  MVector.hpp
 *  @brief Euclidean vector used for linear algebra calculations
 *  @note Status: ALPHA
 *
 *  MVector is just short for MathVector (and that is *very* open to suggestions!)
 *
 */

#ifndef EMP_MATHVEC_H
#define EMP_MATHVEC_H


#include "../base/vector.hpp"
#include <cmath>


namespace emp{
  class MVector{
    private:
      emp::vector<double> emp_vec;
      size_t card;
    public:
      // Constructors
      MVector(): emp_vec(), card(0) {;}
      MVector(size_t _card) : emp_vec(_card, 0), card(_card){;}
      MVector(std::initializer_list<double> val_list)
          : emp_vec(val_list)
          , card(val_list.size()){
      }
      MVector(const emp::vector<double>& in_vec) 
          : emp_vec(in_vec)
          , card(in_vec.size()){
      }

      // Getter 
      double operator[](size_t idx) const{
        emp_assert(idx < emp_vec.size());
        return emp_vec[idx];
      }
      // Setter
      double& operator[](size_t idx){
        emp_assert(idx < emp_vec.size());
        return emp_vec[idx];
      }
      size_t cardinality() const { return card; }

      //**** Basic vector arithmetic
      // Addition
      MVector operator+(const MVector& other){
        emp_assert(other.cardinality() == card);
        MVector res(card);
        for(size_t idx = 0; idx < card; ++idx){
          res[idx] = emp_vec[idx] + other[idx];
        }
        return res;
      }
      MVector& operator+=(const MVector& other){
        for(size_t idx = 0; idx < card; ++idx){
          emp_vec[idx] += other[idx];
        }
        return *this;
      }
      // Subtraction
      MVector operator-(const MVector& other){
        MVector res(card);
        for(size_t idx = 0; idx < card; ++idx){
          res[idx] = emp_vec[idx] - other[idx];
        }
        return res;
      }
      MVector& operator-=(const MVector& other){
        for(size_t idx = 0; idx < card; ++idx){
        emp_vec[idx] -= other[idx];
        }
        return *this;
      }
      // Scalar multiplication
      MVector operator*(double scale_factor){
        MVector res(card);
        for(size_t idx = 0; idx < card; ++idx){
          res[idx] = emp_vec[idx] * scale_factor;
        }
        return res;
      }
      MVector& operator*=(double scale_factor){
        for(size_t idx = 0; idx < card; ++idx){
          emp_vec[idx] *= scale_factor;
        }
        return *this;
      }
      // Scalar division
      MVector operator/(double scale_factor){
        MVector res(card);
        for(size_t idx = 0; idx < card; ++idx){
          res[idx] = emp_vec[idx] / scale_factor;
        }
        return res;
      }
      MVector& operator/=(double scale_factor){
        for(size_t idx = 0; idx < card; ++idx){
          emp_vec[idx] /= scale_factor;
        }
        return *this;
      }
      // Pairwise Multiply (dot product without the final sum)
      MVector PairwiseMult(const MVector& other){
        MVector res(card);
        for(size_t idx = 0; idx < card; ++idx){
          res[idx] = emp_vec[idx] * other[idx];
        }
        return res;
      }
      // Dot product
      double Dot(const MVector& other){
        double res = 0;
        for(size_t idx = 0; idx < card; ++idx){
          res += emp_vec[idx] * other[idx];
        }
        return res;
      }
      // Cross product
      MVector Cross(const MVector& other){
        emp_assert(card== 3 && other.cardinality() == 3, "Cross product only works with Vector 3s");
        MVector res(3);
        res[0] = emp_vec[1] * other[2] - emp_vec[2] * other[1];
        res[1] = emp_vec[2] * other[0] - emp_vec[0] * other[2];
        res[1] = emp_vec[0] * other[1] - emp_vec[1] * other[0];
        return res;
      }
      // Comparisons
      inline bool operator==(const MVector& other){
        if(card!= other.cardinality()) return false;
        bool is_match = true;
        for(size_t idx = 0; idx < card; ++idx){
          if(emp_vec[idx] != other[idx]){
            is_match = false;
            break;
          }
        }
        return is_match;
      }
      inline bool operator!=(const MVector& other){
        return !(*this == other);
      }

      // Magnitude
      double MagnitudeSquared(){
        double res = 0;
        for(size_t idx = 0; idx < card; ++idx){
          res += emp_vec[idx] * emp_vec[idx];
        }
        return res;
      }
      double Magnitude(){
        return std::sqrt(MagnitudeSquared());
      }

      // Normalize
      MVector Normalized(){
        double mag = Magnitude();
        if(mag == 0)
          return MVector(emp_vec);
        else{
          MVector res(emp_vec);
          return res / Magnitude();
        }
      }
      MVector& Normalize(){
        double mag = Magnitude();
        if(mag != 0)
          (*this) /= mag;
        return *this;
      }
  };
  // Misc. functions
  std::ostream & operator<<(std::ostream & out, const emp::MVector & v) {
    out << "<";
    for (size_t idx = 0; idx < v.cardinality(); ++idx){
      if(idx == 0)
        out << v[idx];
      else
        out << "," << v[idx];
    }
    out << ">";
    return out;
  }
  MVector operator*(double scale_factor, emp::MVector& v){
    return v * scale_factor;
  }
}

#endif
