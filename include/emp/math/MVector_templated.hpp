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
  template <size_t N> 
  class MVector{
    private:
      emp::vector<double> emp_vec;
    public:
      const size_t cardinality = N;
      // Constructors
      MVector() : emp_vec(N, 0){;}
      MVector(std::initializer_list<double> val_list) : emp_vec(val_list){
        emp_assert(val_list.size() == N, "Vector declared as size ", N, 
            " but passed initializer list of size", val_list.size());
      }
      MVector(const emp::vector<double>& in_vec) : emp_vec(in_vec){
        emp_assert(in_vec.size() == N, "Vector declared as size ", N, 
            " but passed Empirical vector of size", in_vec.size());
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

      //**** Basic vector arithmetic
      // Addition
      MVector<N> operator+(const MVector<N>& other){
        MVector<N> res;
        for(size_t idx = 0; idx < cardinality; ++idx){
          res[idx] = emp_vec[idx] + other[idx];
        }
        return res;
      }
      MVector<N>& operator+=(const MVector<N>& other){
        for(size_t idx = 0; idx < cardinality; ++idx){
          emp_vec[idx] += other[idx];
        }
        return *this;
      }
      // Subtraction
      MVector<N> operator-(const MVector<N>& other){
        MVector<N> res;
        for(size_t idx = 0; idx < cardinality; ++idx){
          res[idx] = emp_vec[idx] - other[idx];
        }
        return res;
      }
      MVector<N>& operator-=(const MVector<N>& other){
        for(size_t idx = 0; idx < cardinality; ++idx){
         emp_vec[idx] -= other[idx];
        }
        return *this;
      }
      // Scalar multiplication
      MVector<N> operator*(double scale_factor){
        MVector<N> res;
        for(size_t idx = 0; idx < cardinality; ++idx){
          res[idx] = emp_vec[idx] * scale_factor;
        }
        return res;
      }
      MVector<N>& operator*=(double scale_factor){
        for(size_t idx = 0; idx < cardinality; ++idx){
          emp_vec[idx] *= scale_factor;
        }
        return *this;
      }
      // Scalar division
      MVector<N> operator/(double scale_factor){
        MVector<N> res;
        for(size_t idx = 0; idx < cardinality; ++idx){
          res[idx] = emp_vec[idx] / scale_factor;
        }
        return res;
      }
      MVector<N>& operator/=(double scale_factor){
        for(size_t idx = 0; idx < cardinality; ++idx){
          emp_vec[idx] /= scale_factor;
        }
        return *this;
      }
      // Pairwise Multiply (dot product without the final sum)
      MVector<N> PairwiseMult(const MVector<N>& other){
        MVector<N> res;
        for(size_t idx = 0; idx < cardinality; ++idx){
          res[idx] = emp_vec[idx] * other[idx];
        }
        return res;
      }
      // Dot product
      double Dot(const MVector<N>& other){
        double res = 0;
        for(size_t idx = 0; idx < cardinality; ++idx){
          res += emp_vec[idx] * other[idx];
        }
        return res;
      }
      // Cross product
      MVector<3> Cross(const MVector<3>& other){
        emp_assert(cardinality == 3, "Cross product only works with Vector 3s");
        MVector<3> res;
        res[0] = emp_vec[1] * other[2] - emp_vec[2] * other[1];
        res[1] = emp_vec[2] * other[0] - emp_vec[0] * other[2];
        res[1] = emp_vec[0] * other[1] - emp_vec[1] * other[0];
        return res;
      }
      // Comparisons
      template <size_t M>
      inline bool operator==(const MVector<M>& other){
        if(N != M) return false;
        bool is_match = true;
        for(size_t idx = 0; idx < cardinality; ++idx){
          if(emp_vec[idx] != other[idx]){
            is_match = false;
            break;
          }
        }
        return is_match;
      }
      template <size_t M>
      inline bool operator!=(const MVector<M>& other){
        return !(*this == other);
      }

      // Magnitude
      double MagnitudeSquared(){
        double res = 0;
        for(size_t idx = 0; idx < cardinality; ++idx){
          res += emp_vec[idx] * emp_vec[idx];
        }
        return res;
      }
      double Magnitude(){
        return std::sqrt(MagnitudeSquared());
      }

      // Normalize
      MVector<N> Normalized(){
        double mag = Magnitude();
        if(mag == 0)
          return MVector<N>(emp_vec);
        else{
          MVector<N> res(emp_vec);
          return res / Magnitude();
        }
      }
      MVector<N>& Normalize(){
        double mag = Magnitude();
        if(mag != 0)
          (*this) /= mag;
        return *this;
      }

      
  };
  // Misc. functions
  template <size_t N>
  std::ostream & operator<<(std::ostream & out, const emp::MVector<N> & v) {
    out << "<";
    for (size_t idx = 0; idx < v.cardinality; ++idx){
      if(idx == 0)
        out << v[idx];
      else
        out << "," << v[idx];
    }
    out << ">";
    return out;
  }
  template <size_t N>
  MVector<N> operator*(double scale_factor, emp::MVector<N>& v){
    return v * scale_factor;
  }
}

#endif
