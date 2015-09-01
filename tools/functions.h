#ifndef EMP_FUNCTIONS_H
#define EMP_FUNCTIONS_H

#include <ctime>
#include <functional>
#include <initializer_list>
#include <map>
#include <string>
#include <sstream>
#include <type_traits>
#include <vector>

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

  double time_fun(std::function<void()> test_fun) {
    std::clock_t start_time = std::clock();
    test_fun();
    std::clock_t tot_time = std::clock() - start_time;
    return 1000.0 * ((double) tot_time) / (double) CLOCKS_PER_SEC;
  }

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
    for (const auto & x : src) dst.insert( flip_pair(x) );
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


  // The following template takes two parameters; the real type you want it to be and a decoy
  // type that should just be evaluated for use in SFINAE.
  // To use: typename sfinae_decoy<X,Y>::type
  // This will always evaluate to X no matter what Y is.
  template <typename REAL_TYPE, typename EVAL_TYPE>
  struct sfinae_decoy { using type = REAL_TYPE; };


}

#endif
