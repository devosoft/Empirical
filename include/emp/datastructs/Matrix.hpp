/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2018-2021.
 *
 *  @file  Matrix.hpp
 *  @brief Basic matrix implementation to hold two-dimensional data. 
 *  @note Status: ALPHA
 *
 *  The goal of this class is to make accessing easier than a vector of vectors.
 *  If you need a matrix implementation capable of linear algebra, see include/emp/math/Matrix.hpp
 */

#ifndef EMP_MATRIX_H
#define EMP_MATRIX_H

#include "../base/vector.hpp"

namespace emp{
  template <typename T>
  class Matrix{
    protected: 
      emp::vector<emp::vector<T>> data;
      size_t n_cols;
      size_t n_rows;
    public:
      // Constructors
      Matrix() = delete;
      Matrix(size_t rows, size_t cols)
          : data(), n_cols(cols), n_rows(rows){
        T default_val = { };
        for(size_t row_idx = 0; row_idx < n_rows; ++row_idx){
          emp::vector<T> v(cols, default_val);
          data.push_back(v);
        }
      } 
      Matrix(size_t rows, size_t cols, T default_val)
          : data(), n_cols(cols), n_rows(rows){
        for(size_t row_idx = 0; row_idx < n_rows; ++row_idx){
          emp::vector<T> v(cols, default_val);
          data.push_back(v);
        }
      } 
      Matrix(size_t rows, size_t cols, std::initializer_list<T> val_list) 
          : data(), n_cols(cols), n_rows(rows){
        emp_assert(val_list.size() == rows * cols, "Matrix declared as size ", rows, "x", cols,
            " but passed initializer list of size", val_list.size());
        emp::vector<T> val_vec(val_list);
        for(size_t row_idx = 0; row_idx < n_rows; ++row_idx){
          emp::vector<T> v(cols);
          for(size_t col_idx = 0; col_idx < n_cols; ++col_idx){
            v[col_idx] = val_vec[row_idx * n_cols + col_idx];
          }
          data.push_back(v);
        }
      }
      Matrix(size_t rows, size_t cols, const emp::vector<T>& in_vec) 
          : data(), n_cols(cols), n_rows(rows){
            std::cout << n_rows << "x" << n_cols << std::endl;
        emp_assert(in_vec.size() == rows * cols, "Matrix declared as size ", rows, "x", cols,
            " but passed initializing vector of size", in_vec.size());
        for(size_t row_idx = 0; row_idx < n_rows; ++row_idx){
          emp::vector<T> v(cols);
          for(size_t col_idx = 0; col_idx < n_cols; ++col_idx){
            v[col_idx] = in_vec[row_idx * n_cols + col_idx];
          }
          data.push_back(v);
        }
      }

      // Getter
      T Get(size_t row_idx, size_t col_idx) const{
        emp_assert(row_idx < n_rows);
        emp_assert(col_idx < n_cols);
        return data[row_idx][col_idx];
      }
      // Setter
      void Set(size_t row_idx, size_t col_idx, T val){
        emp_assert(row_idx < n_rows);
        emp_assert(col_idx < n_cols);
        data[row_idx][col_idx] = val;
      }
      size_t num_rows() const { return n_rows; }
      size_t num_cols() const { return n_cols; }
      const emp::vector<T>& GetRow(size_t row_idx) const{
        return data[row_idx];
      }
      emp::vector<T> GetCol(size_t col_idx) const{
        emp::vector<T> res(n_rows); 
        for(size_t row_idx = 0; row_idx < n_rows; ++row_idx){
          res[row_idx] = data[row_idx][col_idx];
        }
        return res;
      }
      void SetRow(size_t row_idx, const emp::vector<T>& v){
        emp_assert(v.size() == n_cols);
        data[row_idx] = v;
      }
      void SetCol(size_t col_idx, const emp::vector<T>& v){
        emp_assert(v.size() == n_rows);
        for(size_t row_idx = 0; row_idx < n_rows; ++row_idx){
          data[row_idx][col_idx] = v[row_idx];
        }
      }
      Matrix Transposed(){
        Matrix res(n_rows, n_cols);
        for(size_t row_idx = 0; row_idx < n_rows; ++row_idx){
          for(size_t col_idx = 0; col_idx < n_cols; ++col_idx){
            res.Set(col_idx, row_idx, Get(row_idx, col_idx));
          }
        }
        return res;
      }
      emp::vector<T>& operator[](size_t idx){
        emp_assert(idx < data.size(), "Index out of bounds:", idx, "Number of rows:", data.size());
        return data[idx];
      }
      const emp::vector<T>& operator[](size_t idx) const {
        emp_assert(idx < data.size(), "Index out of bounds:", idx, "Number of rows:", data.size());
        return data[idx];
      }
      void ExpandTo(size_t rows, size_t cols, T val){
        n_rows = rows;
        n_cols = cols;
        for(size_t row_idx = 0; row_idx < data.size(); ++row_idx){
          data[row_idx].resize(cols, val);
        }
        for(size_t row_idx = data.size(); row_idx < n_rows; ++row_idx){
          emp::vector<T> v(cols, val);
          data.push_back(v);
        }
      }
      void ExpandTo(size_t rows, size_t cols){
        T val = { };
        ExpandTo(rows, cols, val);
      }
  };
  
  // Misc. functions
  template <typename T>
  std::ostream & operator<<(std::ostream & out, const emp::Matrix<T> & M) {
    for(size_t row_idx = 0; row_idx < M.num_rows(); ++row_idx){
      out << "| ";
      for(size_t col_idx = 0; col_idx < M.num_cols(); ++col_idx){
        if(col_idx == 0)
          out << M.Get(row_idx, col_idx);
        else
          out << " " << M.Get(row_idx,col_idx);
        
      }
      out << " |" << std::endl;
    }
    return out;
  }
}

#endif
