#ifndef EMP_FUNCTIONS_H
#define EMP_FUNCTIONS_H

#include <algorithm>
#include <ctime>
#include <map>
#include <string>
#include <sstream>
#include <type_traits>
#include <initializer_list>

#define EMP_FUNCTION_TIMER(TEST_FUN) {                                       \
    std::clock_t emp_start_time = std::clock();                              \
    auto emp_result = TEST_FUN;                                              \
    std::clock_t emp_tot_time = std::clock() - emp_start_time;               \
    std::cout << "Time: "                                                    \
              << 1000.0 * ((double) emp_tot_time) / (double) CLOCKS_PER_SEC  \
              << " ms" << std::endl;                                         \
    std::cout << "Result: " << emp_result << std::endl;                      \
  }

namespace emp {

  // Toggle an input bool.
  inline bool toggle(bool & in_bool) { return (in_bool = !in_bool); }

  // % is actually remainder; this is a proper modulus command that handles negative #'s correctly.
  inline int mod(int in_val, int mod_val) {
    return (in_val < 0) ? (in_val % mod_val + mod_val) : (in_val % mod_val);
  }

  // A fast (O(log p)) integer-power command.
  static int pow(int base, int p) {
    if (p == 0) return 1;
    if (p < 0) return 0;
    
    int r = 1;
    while (true) {
      if (p & 1) r *= base;
      if (!(p >>= 1)) return r;
      base *= base;
    }
  }


  // Run both min and max on a value to put it into a desired range.
  template <typename TYPE> TYPE to_range(const TYPE & value, const TYPE & in_min, const TYPE & in_max) {
    return (value < in_min) ? in_min : ((value > in_max) ? in_max : value);
  }

  template <typename T> const T & min(const T& in1, const T& in2, const T& in3) {
    return std::min(std::min(in1,in2), in3);
  }

  // Build a min and max that allows a variable number of inputs to be compared.
  template <typename T> const T & min(std::initializer_list<const T&> lst) {
    emp_assert(lst.size > 0); // Nothing to return if nothing in the list!
    auto min_found = lst.begin();
    for (auto it = lst.begin()+1; it < lst.end(); it++) {
      if (*it < *min_found) min_found = it;
    }
    return *min_found;
  }

  template <typename T> const T & max(std::initializer_list<const T&> lst) {
    emp_assert(lst.size > 0); // Nothing to return if nothing in the list!
    auto max_found = lst.begin();
    for (auto it = lst.begin()+1; it < lst.end(); it++) {
      if (*it > *max_found) max_found = it;
    }
    return *max_found;
  }


  static std::vector<int> build_range(int min, int max, int step=1) {
    int size = (max-min) / step;
    std::vector<int> out_v(size);
    int pos = 0;
    for (int i = min; i < max; i += step) {
      out_v[pos++] = i;
    }
    return out_v;
  }

  // The following two functions are from:
  // http://stackoverflow.com/questions/5056645/sorting-stdmap-using-value
  template<typename A, typename B> std::pair<B,A> flip_pair(const std::pair<A,B> &p)
  {
    return std::pair<B,A>(p.second, p.first);
  }
  
  template<typename A, typename B> std::multimap<B,A> flip_map(const std::map<A,B> &src)
  {
    std::multimap<B,A> dst;
    std::transform(src.begin(), src.end(), std::inserter(dst, dst.begin()), flip_pair<A,B>);
    return dst;
  }

  //  ----- Variadic Template Helpers! -----

  // The following functions take in a test type and a list of types and return the index that
  // matches the test type in question.
  template <typename TEST_TYPE>
  constexpr int get_type_index() {
    // @CAO We don't have a type that matches, so ideally trigger a compile time error.
    // Given we need this to be constexpr, we can't easily put even a static assert here until C++14
    // static_assert(false && "trying to find index of non-existant type");
    return -1000000;
  }
  template <typename TEST_TYPE, typename FIRST_TYPE, typename... TYPE_LIST>
  constexpr int get_type_index() {
    return (std::is_same<TEST_TYPE, FIRST_TYPE>()) ? 0 : (get_type_index<TEST_TYPE,TYPE_LIST...>() + 1);
  }


  // These functions can be used to test if a type-set has all unique types or not.

  // Base cases...
  template <typename TYPE1> constexpr bool has_unique_first_type() { return true; }
  template <typename TYPE1> constexpr bool has_unique_types() { return true; }

  template <typename TYPE1, typename TYPE2, typename... TYPE_LIST>
  constexpr bool has_unique_first_type() {
    return (!std::is_same<TYPE1, TYPE2>()) && emp::has_unique_first_type<TYPE1, TYPE_LIST...>();
  }

  template <typename TYPE1, typename TYPE2, typename... TYPE_LIST>
  constexpr bool has_unique_types() {
    return has_unique_first_type<TYPE1, TYPE2, TYPE_LIST...>()  // Check first against all others...
      && has_unique_types<TYPE2, TYPE_LIST...>();               // Recurse through other types.
  }



  // implementation details, users never invoke these directly
  // Based on Kerrek SB in http://stackoverflow.com/questions/10766112/c11-i-can-go-from-multiple-args-to-tuple-but-can-i-go-from-tuple-to-multiple
 
  namespace internal {
    template <typename FUN_TYPE, typename TUPLE_TYPE, bool is_done, int TOTAL, int... N>
    struct apply_impl {
      static void apply(FUN_TYPE fun, TUPLE_TYPE && t) {
        apply_impl<FUN_TYPE, TUPLE_TYPE, TOTAL == 1 + sizeof...(N), TOTAL, N..., sizeof...(N)>::apply(fun, std::forward<TUPLE_TYPE>(t));
      }
    };

    template <typename FUN_TYPE, typename TUPLE_TYPE, int TOTAL, int... N>
    struct apply_impl<FUN_TYPE, TUPLE_TYPE, true, TOTAL, N...> {
      static void apply(FUN_TYPE fun, TUPLE_TYPE && t) {
        fun(std::get<N>(std::forward<TUPLE_TYPE>(t))...);
      }
    };
  }
  
  // user invokes this
  template <typename FUN_TYPE, typename TUPLE_TYPE>
  void ApplyTuple(FUN_TYPE fun, TUPLE_TYPE && tuple) {
    typedef typename std::decay<TUPLE_TYPE>::type TUPLE_DECAY_TYPE;
    internal::apply_impl<FUN_TYPE, TUPLE_TYPE, 0 == std::tuple_size<TUPLE_DECAY_TYPE>::value, std::tuple_size<TUPLE_DECAY_TYPE>::value>::apply(fun, std::forward<TUPLE_TYPE>(tuple));
  }













  // --- Distance functions for any array-type objects ---

  // Hamming distance is a simple count of substitutions needed to convert on array to another.
  template <typename TYPE>
  int calc_hamming_distance(const TYPE & in1, const TYPE & in2, int offset=0) {
    if (offset < 0) return calc_hamming_distance(in2, in1, -offset);

    const auto size1 = in1.size();
    const auto size2 = in2.size();

    // Calculate by how much the strings overlap.
    int overlap = std::min( size1 - offset,  size2 );

    // Initialize the distance to that part of the strings which do not overlap.
    int num_diffs = size1 + size2 - 2 * overlap;

    // Step through the overlapped section and add on additional differences.
    for (size_t i = 0; i < overlap; i++) {
      if (in1[i + offset] != in2[i]) num_diffs++;
    }
    
    return num_diffs;
  }

  // Edit distance is the minimum number of insertions, deletions and substitutions to convert
  // one array to another.
  template <typename TYPE>
  int calc_edit_distance(const TYPE & in1, const TYPE & in2) {
    const auto size1 = in1.size();
    const auto size2 = in2.size();

    // If either size is zero, other size indicates number of insertions needed to produce it.
    if (size1 == 0) return size2;
    if (size2 == 0) return size1;

    std::vector<int> cur_row(size1);   // The row we are calculating
    std::vector<int> prev_row(size1);  // The previous row we calculated

    // Initialize the previous row to record the differece from nothing.
    for (int i = 0; i < size1; i++) prev_row[i] = i + 1;

    // Loop through all other rows
    for (int row = 0; row < (int) size2; row++) {
      // Initialize the first entry in the current row.
      if (in1[0] == in2[row]) cur_row[0] = row;
      else cur_row[0] = std::min(row, prev_row[0]) + 1;

      // Move through the cur_row and fill it in.
      for (int col = 1; col < (int) size1; col++) {
        // If the values are equal, keep the value in the upper left.
        if (in1[col] == in2[row]) cur_row[col] = prev_row[col-1];

        // Otherwise, set the current position the the minimal of the three
        // numbers to the upper right in the chart plus one.
        else {
          cur_row[col] = emp::min(prev_row[col], prev_row[col-1], cur_row[col-1]) + 1;
        }
      }

      // Swap cur_row and prev_row (keep cur vals in prev row, recycle vector cur_row)
      std::swap(cur_row, prev_row);
    }

    // Now that we are done, return the bottom-right corner of the chart.
    return prev_row[size1 - 1];
  }

  // Use edit distance to find the minimum number of insertions, deletions and substitutions
  // to convert one array to another, and then insert gaps into the arrays appropriately.
  template <typename TYPE, typename GAP_TYPE>
  int align(TYPE & in1, TYPE & in2, GAP_TYPE gap) {
    const auto size1 = in1.size();
    const auto size2 = in2.size();

    // If either size is zero, other size indicates number of insertions needed to produce it.
    if (size1 == 0) return size2;
    if (size2 == 0) return size1;

    std::vector<int> cur_row(size1);   // The row we are calculating
    std::vector<int> prev_row(size1);  // The previous row we calculated
    std::vector<std::vector<char> > edit_info(size2, std::vector<char>(size1));

    // Initialize the previous row to record the differece from nothing.
    for (int i = 0; i < size1; i++) {
      prev_row[i] = i + 1;
      edit_info[0][i] = 'i';
    }

    // Loop through all other rows
    for (int row = 0; row < (int) size2; row++) {
      // Initialize the first entry in the current row.
      if (in1[0] == in2[row]) { cur_row[0] = row; edit_info[row][0] = 's'; }
      else { cur_row[0] = prev_row[0] + 1; edit_info[row][0] = 'd'; }
      
      // Move through the cur_row and fill it in.
      for (int col = 1; col < (int) size1; col++) {
        // If the values are equal, keep the value in the upper left.
        if (in1[col] == in2[row]) { cur_row[col] = prev_row[col-1]; edit_info[row][col] = 's'; }

        // Otherwise, set the current position to the minimum of the three
        // numbers to the left, upper, or upper left in the chart plus one.
        else {
          cur_row[col] = emp::min(prev_row[col], prev_row[col-1], cur_row[col-1]) + 1;
          if (cur_row[col] == prev_row[col]+1)   { edit_info[row][col] = 'd'; }
          if (cur_row[col] == prev_row[col-1]+1) { edit_info[row][col] = 's'; }
          if (cur_row[col] == cur_row[col-1]+1)  { edit_info[row][col] = 'i'; }
        }
      }

      // Swap cur_row and prev_row (keep cur vals in prev row, recycle vector cur_row)
      std::swap(cur_row, prev_row);
    }

    // Fill in gaps in the sequences to make them align!
    
    int c = size1-1;
    int r = size2-1;
    int length = 0;

    while (c >= 0 || r >= 0) {
      if (c < 0) { ++length; --r; continue; }
      else if (r < 0) { ++length; --c; continue; }

      char cur_move = edit_info[r][c];
      switch(cur_move) {
      case 's': --c; --r; ++length; break;
      case 'd': --r; ++length; break;
      case 'i': --c; ++length; break;
      };
    }
      
    c = size1-1;
    r = size2-1;

    TYPE out1(length, gap);
    TYPE out2(length, gap);

    int pos = length - 1;

    while (c >= 0 && r >= 0) {
      switch(edit_info[r][c]) {
      case 's': out1[pos] = in1[c]; out2[pos] = in2[r]; --c; --r; break;
      case 'd': out1[pos] = gap;    out2[pos] = in2[r];      --r; break;
      case 'i': out1[pos] = in1[c]; out2[pos] = gap;    --c;      break;
      };
      --pos;
    }
    while (c >= 0) { out1[pos] = in1[c]; --c; --pos; }
    while (r >= 0) { out2[pos] = in2[r]; --r; --pos; }

    in1 = out1;
    in2 = out2;

    // Now that we are done, return the bottom-right corner of the chart.
    return prev_row[size1 - 1];
  }



};

#endif
