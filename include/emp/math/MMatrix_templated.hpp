/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2018-2021.
 *
 *  @file  MVector.hpp
 *  @brief Basic matrix used for linear algebra calculations
 *  @note Status: ALPHA
 *
 *  MMatrix is short for MathMatrix to match MVector (and that is *very* open to suggestions!)
 *
 *  TODO: Matrix inverse
 */

#ifndef EMP_MATHMATRIX_H
#define EMP_MATHMATRIX_H


#include "../base/vector.hpp"
#include "./MVector.hpp"
#include <cmath>


namespace emp{
  template <size_t R, size_t C> 
  class MMatrix{
    private: 
      emp::vector<MVector<C>> data;
    public:
      size_t num_rows = R;
      size_t num_cols = C;
     
      // Constructors
      MMatrix(): data(){
        for(size_t row_idx = 0; row_idx < num_rows; ++row_idx){
          MVector<C> v;
          data.push_back(v);
        }
      } 
      MMatrix(std::initializer_list<double> val_list) : data(){
        emp_assert(val_list.size() == R * C, "Matrix declared as size ", R, "x", C,
            " but passed initializer list of size", val_list.size());
        emp::vector<double> val_vec(val_list);
        for(size_t row_idx = 0; row_idx < num_rows; ++row_idx){
          MVector<C> v;
          for(size_t col_idx = 0; col_idx < num_cols; ++col_idx){
            v[col_idx] = val_vec[row_idx * num_cols + col_idx];
          }
          data.push_back(v);
        }
      }
      MMatrix(const emp::vector<double>& in_vec) : data(){
        emp_assert(in_vec.size() == R * C, "Matrix declared as size ", R, "x", C,
            " but passed initializer list of size", in_vec.size());
        for(size_t row_idx = 0; row_idx < num_rows; ++row_idx){
          MVector<C> v;
          for(size_t col_idx = 0; col_idx < num_cols; ++col_idx){
            v[col_idx] = in_vec[row_idx * num_cols + col_idx];
          }
          data.push_back(v);
        }
      }

      // Getter
      double Get(size_t row_idx, size_t col_idx) const{
        emp_assert(row_idx < num_rows);
        emp_assert(col_idx < num_cols);
        return data[row_idx][col_idx];
      }
      // Setter
      void Set(size_t row_idx, size_t col_idx, double val){
        emp_assert(row_idx < num_rows);
        emp_assert(col_idx < num_cols);
        data[row_idx][col_idx] = val;
      }
      MVector<C> GetRow(size_t row_idx) const{
        return data[row_idx];
      }
      MVector<R> GetCol(size_t col_idx) const{
        MVector<R> res; 
        for(size_t row_idx = 0; row_idx < num_rows; ++row_idx){
          res[row_idx] = data[row_idx][col_idx];
        }
        return res;
      }

      // Addition
      MMatrix<R,C> operator+(const MMatrix<R,C>& other){
        MMatrix<R,C> res;
        for(size_t row_idx = 0; row_idx < num_rows; ++row_idx){
          for(size_t col_idx = 0; col_idx < num_cols; ++col_idx){
            res.Set(row_idx, col_idx, Get(row_idx, col_idx) + other.Get(row_idx, col_idx));
          }
        }
        return res;
      }
      MMatrix<R,C>& operator+=(const MMatrix<R,C>& other){
        MMatrix<R,C> res;
        for(size_t row_idx = 0; row_idx < num_rows; ++row_idx){
          for(size_t col_idx = 0; col_idx < num_cols; ++col_idx){
            Set(row_idx, col_idx, Get(row_idx, col_idx) + other.Get(row_idx, col_idx));
          }
        }
        return res;
      }
      // Subtraction 
      MMatrix<R,C> operator-(const MMatrix<R,C>& other){
        MMatrix<R,C> res;
        for(size_t row_idx = 0; row_idx < num_rows; ++row_idx){
          for(size_t col_idx = 0; col_idx < num_cols; ++col_idx){
            res.Set(row_idx, col_idx, Get(row_idx, col_idx) - other.Get(row_idx, col_idx));
          }
        }
        return res;
      }
      MMatrix<R,C>& operator-=(const MMatrix<R,C>& other){
        MMatrix<R,C> res;
        for(size_t row_idx = 0; row_idx < num_rows; ++row_idx){
          for(size_t col_idx = 0; col_idx < num_cols; ++col_idx){
            Set(row_idx, col_idx, Get(row_idx, col_idx) - other.Get(row_idx, col_idx));
          }
        }
        return res;
      }
      // Multiplication
      template<size_t C2>
      MMatrix<R, C2> operator*(const MMatrix<C, C2>& other){
        MMatrix<R, C2> res;
        for(size_t row_idx = 0; row_idx < num_rows; ++row_idx){
          for(size_t col_idx = 0; col_idx < C2; ++col_idx){
            res.Set(row_idx, col_idx, GetRow(row_idx).Dot(other.GetCol(col_idx)));
          }
        }
        return res;
      }
      MVector<R> operator*(const MVector<C>& other){
        MVector<R> res;
        for(size_t row_idx = 0; row_idx < num_rows; ++row_idx){
            res[row_idx] = GetRow(row_idx).Dot(other);
        }
        return res;
      }
      MMatrix<R,C> operator*(double scale_factor){
        MMatrix<R, C> res;
        for(size_t row_idx = 0; row_idx < num_rows; ++row_idx){
          for(size_t col_idx = 0; col_idx < num_cols; ++col_idx){
            res.Set(row_idx, col_idx, Get(row_idx, col_idx) * scale_factor);
          }
        }
        return res;
      }
      MMatrix<R,C> operator/(double scale_factor){
        MMatrix<R, C> res;
        for(size_t row_idx = 0; row_idx < num_rows; ++row_idx){
          for(size_t col_idx = 0; col_idx < num_cols; ++col_idx){
            res.Set(row_idx, col_idx, Get(row_idx, col_idx) / scale_factor);
          }
        }
        return res;
      }

      MMatrix<R,C> Transposed(){
        MMatrix<R, C> res;
        for(size_t row_idx = 0; row_idx < num_rows; ++row_idx){
          for(size_t col_idx = 0; col_idx < num_cols; ++col_idx){
            res.Set(col_idx, row_idx, Get(row_idx, col_idx));
          }
        }
        return res;
      }
  };
  
  // Misc. functions
  template <size_t R, size_t C>
  std::ostream & operator<<(std::ostream & out, const emp::MMatrix<R, C> & M) {
    for(size_t row_idx = 0; row_idx < M.num_rows; ++row_idx){
      out << "| ";
      for(size_t col_idx = 0; col_idx < M.num_cols; ++col_idx){
        if(col_idx == 0)
          out << M.Get(row_idx, col_idx);
        else
          out << " " << M.Get(row_idx,col_idx);
        
      }
      out << " |" << std::endl;
    }
    return out;
  }
  template <size_t R, size_t C>
  MMatrix<R, C> operator*(double scale_factor, emp::MMatrix<R,C>& M){
    return M * scale_factor;
  }
  template <size_t R, size_t C>
  MVector<C> operator*(MVector<R> other, emp::MMatrix<R,C>& M){
    MVector<C> res;
    for(size_t col_idx = 0; col_idx < M.num_cols; ++col_idx){
        res[col_idx] = other.Dot(M.GetCol(col_idx));
    }
    return res;
  }
  
  template<size_t R, size_t C>
  double GetDeterminant(const MMatrix<R,C> & mat){
    // Generic case: Recurse to find determinant
    double res = 0;
    for(size_t col_idx = 0; col_idx < mat.num_cols; ++col_idx){
      MMatrix<R - 1, C - 1> tmp_mat;
      size_t tmp_row_idx = 0;
      for(size_t row_idx = 1; row_idx < mat.num_rows; ++row_idx){
        size_t tmp_col_idx = 0;
        for(size_t col_idx_2 = 0; col_idx_2 < mat.num_cols; ++col_idx_2){
          if(col_idx_2 == col_idx)
            continue;
          tmp_mat.Set(tmp_row_idx, tmp_col_idx, mat.Get(row_idx, col_idx_2));
          tmp_col_idx++;
        }
        tmp_row_idx++;
      }
      if(col_idx % 2 == 0)
        res += mat.Get(0, col_idx) * GetDeterminant(tmp_mat); 
      else
        res -= mat.Get(0, col_idx) * GetDeterminant(tmp_mat); 
    }
    return res;
  }

  template<>
  double GetDeterminant(const MMatrix<1,1> & mat){
    // Base case: 1x1 matrix -> return the only value
    return mat.Get(1,1);
  }
  template<>
  double GetDeterminant(const MMatrix<2,2> &  mat){
    // | a b |  Base case: 2x2 -> use the formula below
    // | c d |  Det = ad - bc
    return mat.Get(0,0) * mat.Get(1,1) - mat.Get(0,1) * mat.Get(1,0);
  }
}


#endif
