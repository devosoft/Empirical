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
#include "../datastructs/Matrix.hpp"
#include "./MVector.hpp"
#include <cmath>


namespace emp{
  class MMatrix : public Matrix<double>{
    //private: 
    //  emp::vector<MVector> data;
    public:
     
      //// Constructors
      MMatrix() = delete;
      MMatrix(size_t rows, size_t cols) : Matrix(rows, cols) { ; }
      MMatrix(size_t rows, size_t cols, std::initializer_list<double> val_list)
       : Matrix(rows, cols, val_list) { ; } 
      MMatrix(size_t rows, size_t cols, const emp::vector<double>& in_vec)
       : Matrix(rows, cols, in_vec) { ; } 

      MVector GetRow(size_t row_idx) const{
        return MVector(data[row_idx]);
      }
      MVector GetCol(size_t col_idx) const{
        MVector res(n_rows); 
        for(size_t row_idx = 0; row_idx < n_rows; ++row_idx){
          res[row_idx] = data[row_idx][col_idx];
        }
        return res;
      }

      // Addition
      MMatrix operator+(const MMatrix& other){
        emp_assert(n_rows == other.num_rows() && n_cols == other.num_cols(), "Matrices must be identical sizes");
        MMatrix res(n_rows, n_cols);
        for(size_t row_idx = 0; row_idx < n_rows; ++row_idx){
          for(size_t col_idx = 0; col_idx < n_cols; ++col_idx){
            res.Set(row_idx, col_idx, Get(row_idx, col_idx) + other.Get(row_idx, col_idx));
          }
        }
        return res;
      }
      MMatrix& operator+=(const MMatrix& other){
        emp_assert(n_rows == other.num_rows() && n_cols == other.num_cols(), "Matrices must be identical sizes");
        for(size_t row_idx = 0; row_idx < n_rows; ++row_idx){
          for(size_t col_idx = 0; col_idx < n_cols; ++col_idx){
            Set(row_idx, col_idx, Get(row_idx, col_idx) + other.Get(row_idx, col_idx));
          }
        }
        return *this;
      }
      // Subtraction 
      MMatrix operator-(const MMatrix& other){
        emp_assert(n_rows == other.num_rows() && n_cols == other.num_cols(), "Matrices must be identical sizes");
        MMatrix res(n_rows, n_cols);
        for(size_t row_idx = 0; row_idx < n_rows; ++row_idx){
          for(size_t col_idx = 0; col_idx < n_cols; ++col_idx){
            res.Set(row_idx, col_idx, Get(row_idx, col_idx) - other.Get(row_idx, col_idx));
          }
        }
        return res;
      }
      MMatrix& operator-=(const MMatrix& other){
        emp_assert(n_rows == other.num_rows() && n_cols == other.num_cols(), "Matrices must be identical sizes");
        MMatrix res(n_rows, n_cols);
        for(size_t row_idx = 0; row_idx < n_rows; ++row_idx){
          for(size_t col_idx = 0; col_idx < n_cols; ++col_idx){
            Set(row_idx, col_idx, Get(row_idx, col_idx) - other.Get(row_idx, col_idx));
          }
        }
        return *this;
      }
      // Multiplication
      MMatrix operator*(const MMatrix& other){
        MMatrix res(n_rows, other.num_cols());
        emp_assert(n_cols == other.num_rows(), "Matrix dimension mismatch in multiplication");
        for(size_t row_idx = 0; row_idx < n_rows; ++row_idx){
          for(size_t col_idx = 0; col_idx < other.num_cols(); ++col_idx){
            res.Set(row_idx, col_idx, GetRow(row_idx).Dot(other.GetCol(col_idx)));
          }
        }
        return res;
      }
      MVector operator*(const MVector& other){
        emp_assert(n_cols == other.cardinality(), "Vector cardinality must match matrix columns");
        MVector res(n_rows);
        for(size_t row_idx = 0; row_idx < n_rows; ++row_idx){
            res[row_idx] = GetRow(row_idx).Dot(other);
        }
        return res;
      }
      MMatrix operator*(double scale_factor){
        MMatrix res(n_rows, n_cols);
        for(size_t row_idx = 0; row_idx < n_rows; ++row_idx){
          for(size_t col_idx = 0; col_idx < n_cols; ++col_idx){
            res.Set(row_idx, col_idx, Get(row_idx, col_idx) * scale_factor);
          }
        }
        return res;
      }
      MMatrix operator/(double scale_factor){
        MMatrix res(n_rows, n_cols);
        for(size_t row_idx = 0; row_idx < n_rows; ++row_idx){
          for(size_t col_idx = 0; col_idx < n_cols; ++col_idx){
            res.Set(row_idx, col_idx, Get(row_idx, col_idx) / scale_factor);
          }
        }
        return res;
      }

      double Determinant(){
        emp_assert(n_rows == n_cols, "Can only calculate deterimant of square matrices");
        // Base case: 1x1 matrix -> return the only value
        if(n_rows == 1 && n_cols == 1)
          return Get(1,1);
        // | a b |  Base case: 2x2 -> use the formula below
        // | c d |  Det = ad - bc
        if(n_rows == 2 && n_cols == 2)
          return Get(0,0) * Get(1,1) - Get(0,1) * Get(1,0);
        // Generic case: Recurse to find determinant
        else{
          double res = 0;
          for(size_t col_idx = 0; col_idx < n_cols; ++col_idx){
            MMatrix tmp_mat(n_rows - 1, n_cols - 1);
            size_t tmp_row_idx = 0;
            for(size_t row_idx = 1; row_idx < n_rows; ++row_idx){
              size_t tmp_col_idx = 0;
              for(size_t col_idx_2 = 0; col_idx_2 < n_cols; ++col_idx_2){
                if(col_idx_2 == col_idx)
                  continue;
                tmp_mat.Set(tmp_row_idx, tmp_col_idx, Get(row_idx, col_idx_2));
                tmp_col_idx++;
              }
              tmp_row_idx++;
            }
            if(col_idx % 2 == 0)
              res += Get(0, col_idx) * tmp_mat.Determinant(); 
            else
              res -= Get(0, col_idx) * tmp_mat.Determinant(); 
          }
          return res;
        }
      }

  };
  
  // Misc. functions
  MMatrix operator*(double scale_factor, emp::MMatrix& M){
    return M * scale_factor;
  }
  MVector operator*(MVector other, emp::MMatrix& M){
    emp_assert(other.cardinality() == M.num_rows());
    MVector res(M.num_cols());
    for(size_t col_idx = 0; col_idx < M.num_cols(); ++col_idx){
        res[col_idx] = other.Dot(M.GetCol(col_idx));
    }
    return res;
  }
}


#endif
