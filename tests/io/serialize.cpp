/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021
 *
 *  @file serialize.cpp
 */

#include <map>
#include <sstream>

#include "third-party/Catch/single_include/catch2/catch.hpp"

#include "emp/io/serialize.hpp"

TEST_CASE("Test serialize", "[io]")
{

}

// struct SerializeTest {
//   int a;
//   float b;        // unimportant data!
//   std::string c;

//   SerializeTest(int _a, float _b, std::string _c) : a(_a), b(_b), c(_c) { ; }
//   EMP_SETUP_DATAPOD(SerializeTest, a, c);
// };

// struct SerializeTest_D : public SerializeTest {
//   char d = '$';

//   SerializeTest_D(int _a, float _b, std::string _c, char _d)
//     : SerializeTest(_a, _b, _c), d(_d) { ; }
//   EMP_SETUP_DATAPOD_D(SerializeTest_D, SerializeTest, d);
// };

// struct ExtraBase {
//   double e;

//   ExtraBase(double _e) : e(_e) { ; }
//   EMP_SETUP_DATAPOD(ExtraBase, e);
// };

// struct MultiTest : public SerializeTest, public ExtraBase {
//   bool f;

//   MultiTest(int _a, float _b, std::string _c, double _e, bool _f)
//     : SerializeTest(_a, _b, _c), ExtraBase(_e), f(_f) { ; }
//   EMP_SETUP_DATAPOD_D2(MultiTest, SerializeTest, ExtraBase, f);
// };

// struct NestedTest {
//   SerializeTest st;
//   std::string name;
//   SerializeTest_D std;
//   MultiTest mt;

//   NestedTest(int a1, float b1, std::string c1,
//              int a2, float b2, std::string c2, char d2,
//              int a3, float b3, std::string c3, double e3, bool f3)
//     : st(a1, b1, c1), name("my_class"), std(a2, b2, c2, d2), mt(a3, b3, c3, e3, f3) { ; }

//   EMP_SETUP_DATAPOD(NestedTest, st, name, std, mt);
// };

// struct BuiltInTypesTest {
//   const int a;
//   emp::vector<int> int_v;

//   BuiltInTypesTest(int _a, size_t v_size) : a(_a), int_v(v_size) {
//     for (size_t i = 0; i < v_size; i++) int_v[i] = (int)(i*i);
//   }

//   EMP_SETUP_DATAPOD(BuiltInTypesTest, a, int_v);
// };

// TEST_CASE("Test serialize", "[io]")
// {
//   std::stringstream ss;
//   emp::serialize::DataPod pod(ss);


//   // Basic test...

//   SerializeTest st(7, 2.34, "my_test_string");
//   st.EMP_Store(pod);

//   SerializeTest st2(pod);

//   REQUIRE(st2.a == 7);                 // Make sure a was reloaded correctly.
//   REQUIRE(st2.c == "my_test_string");  // Make sure c was reloaded correctly.


//   // Derived class Test

//   SerializeTest_D stD(10,0.2,"three",'D');
//   stD.EMP_Store(pod);

//   SerializeTest_D stD2(pod);

//   REQUIRE(stD2.a == 10);
//   REQUIRE(stD2.c == "three");
//   REQUIRE(stD2.d == 'D');

//   // Multiply-derived class Test

//   MultiTest stM(111,2.22,"ttt",4.5,true);
//   stM.EMP_Store(pod);

//   MultiTest stM2(pod);


//   REQUIRE(stM2.a == 111);
//   REQUIRE(stM2.c == "ttt");
//   REQUIRE(stM2.e == 4.5);
//   REQUIRE(stM2.f == true);


//   // Nested objects test...

//   NestedTest nt(91, 3.14, "magic numbers",
//                 100, 0.01, "powers of 10", '1',
//                 1001, 1.001, "ones and zeros", 0.125, true);
//   nt.EMP_Store(pod);

//   NestedTest nt2(pod);

//   REQUIRE(nt2.st.a == 91);
//   REQUIRE(nt2.st.c == "magic numbers");
//   REQUIRE(nt2.name == "my_class");
//   REQUIRE(nt2.std.a == 100);
//   REQUIRE(nt2.std.c == "powers of 10");
//   REQUIRE(nt2.std.d == '1');
//   REQUIRE(nt2.mt.a == 1001);
//   REQUIRE(nt2.mt.c == "ones and zeros");
//   REQUIRE(nt2.mt.e == 0.125);
//   REQUIRE(nt2.mt.f == true);


//   // If we made it this far, everything must have worked!;

//   const int v_size = 43;
//   BuiltInTypesTest bitt(91, v_size);
//   bitt.EMP_Store(pod);


//   BuiltInTypesTest bitt2(pod);
// }




// DEFINE_ATTR(Foo);
// DEFINE_ATTR(Bar);
// DEFINE_ATTR(Bazz);
/*
struct ident_t {
  template <typename T>
  constexpr decltype(auto) operator()(T&& value) const {
    return std::forward<T>(value);
  }
};

constexpr ident_t ident{};

template <typename T>
struct Callable {
  T value;

  constexpr decltype(auto) operator()() & { return value; }
  constexpr decltype(auto) operator()() const & { return value; }
  constexpr decltype(auto) operator()() && { return std::move(value); }
  constexpr decltype(auto) operator()() const && { return std::move(value); }
};
template <typename T>
constexpr Callable<std::decay_t<T>> callable(T&& value) {
  return {std::forward<T>(value)};
}

struct NoCopy {
  int value;
  constexpr NoCopy(int value) : value(value) {}
  constexpr NoCopy(const NoCopy&) = delete;
  constexpr NoCopy(NoCopy&&) = default;

  constexpr NoCopy& operator=(const NoCopy&) = delete;
  constexpr NoCopy& operator=(NoCopy&&) = default;
};
constexpr bool operator==(const NoCopy& a, const NoCopy& b) {
  return a.value == b.value;
}
std::ostream& operator<<(std::ostream& out, const NoCopy& nc) {
  return out << "NoCopy{" << nc.value << "}";
}

struct {
  template <typename I, typename T>
  constexpr auto operator()(I&& init, T&& value) const {
    return std::forward<I>(init) + std::forward<T>(value).Get();
  }

  template <typename I, typename T>
  constexpr auto operator()(const char* name, I&& init, T&& value) const {
    return std::forward<I>(init) + std::forward<T>(value);
  }
} sum;

struct {
  template <typename I, typename A, typename B>
  constexpr auto operator()(I&& init, A&& a, B&& b) const {
    return std::forward<I>(init) +
           (std::forward<A>(a).Get() * std::forward<B>(b).Get());
  }
} dot;

struct {
  template <typename I, typename T>
  constexpr NoCopy operator()(I&& init, T&& value) const {
    return {std::forward<I>(init).value + std::forward<T>(value).Get().value};
  }

  template <typename I, typename T>
  constexpr NoCopy operator()(const char* name, I&& init, T&& value) const {
    return {std::forward<I>(init).value + std::forward<T>(value).value};
  }
} sum_nocopy;
*/
// TEST_CASE("Test Attribute Packs", "[io]") {
//   using namespace emp::tools;
//   // Test Construction & access
//   CONSTEXPR_REQUIRE_EQ(Foo::CallOrGetAttribute(Foo(6)).Get(), 6);
//   CONSTEXPR_REQUIRE_EQ(Foo::CallOrGetAttribute(Foo(callable(7))).Get(), 7);
//   CONSTEXPR_REQUIRE_EQ(Foo::GetOrElse(Foo(7), callable(0)), 7);
//   CONSTEXPR_REQUIRE_EQ(Foo::GetOrElse(Merge(Foo(7), Bar(6)), callable(0)), 7);
//   CONSTEXPR_REQUIRE_EQ(Foo::GetOrElse(Merge(Bazz(7), Bar(6)), callable(0)), 0);
//   CONSTEXPR_REQUIRE_EQ(Foo::GetOrGetIn(Bazz(1), Bar(2), Foo(3)), 3);
//   CONSTEXPR_REQUIRE_EQ(Foo::GetOrGetIn(Bazz(1), Foo(3), Foo(2)), 3);
//   CONSTEXPR_REQUIRE_EQ(Foo::GetOrGetIn(Foo(3), Bar(2), Bazz(1)), 3);
//   CONSTEXPR_REQUIRE_EQ(Foo::GetOrGetIn(Foo(3), Bar(2)), 3);
//   CONSTEXPR_REQUIRE_EQ(Foo::GetOrGetIn(Bar(2), Foo(3)), 3);
//   CONSTEXPR_REQUIRE_EQ(Foo::GetOrGetIn(Foo(3)), 3);

//   CONSTEXPR_REQUIRE_EQ(Foo::GetOr(Foo(7), 0), 7);
//   CONSTEXPR_REQUIRE_EQ(Foo::GetOr(Merge(Foo(7), Bar(6)), 0), 7);
//   CONSTEXPR_REQUIRE_EQ(Foo::GetOr(Merge(Bazz(7), Bar(6)), 0), 0);

//   CONSTEXPR_REQUIRE_EQ(Merge(Foo(5), Bar(6)), Merge(Foo(5), Bar(6)));

//   // Test NoCopy
//   CONSTEXPR_REQUIRE_EQ(Foo::CallOrGetAttribute(Foo(NoCopy{7})).Get(),
//                        NoCopy{7});
//   CONSTEXPR_REQUIRE_EQ(Foo::CallOrGetAttribute(Foo(callable(NoCopy{7}))).Get(),
//                        NoCopy{7});
//   CONSTEXPR_REQUIRE_EQ(Foo::GetOrElse(Foo(NoCopy{7}), callable(NoCopy{0})),
//                        NoCopy{7});
//   CONSTEXPR_REQUIRE_EQ(
//     Foo::GetOrElse(Merge(Foo(NoCopy{7}), Bar(NoCopy{6})), callable(NoCopy{7})),
//     NoCopy{7});
//   CONSTEXPR_REQUIRE_EQ(
//     Foo::GetOrElse(Merge(Bazz(NoCopy{7}), Bar(NoCopy{6})), callable(NoCopy{0})),
//     NoCopy{0});

//   CONSTEXPR_REQUIRE_EQ(
//     Foo::GetOrGetIn(Bazz(NoCopy{1}), Bar(NoCopy{2}), Foo(NoCopy{3})),
//     NoCopy{3});
//   CONSTEXPR_REQUIRE_EQ(
//     Foo::GetOrGetIn(Bazz(NoCopy{1}), Foo(NoCopy{3}), Foo(NoCopy{2})),
//     NoCopy{3});
//   CONSTEXPR_REQUIRE_EQ(
//     Foo::GetOrGetIn(Foo(NoCopy{3}), Bar(NoCopy{2}), Bazz(NoCopy{1})),
//     NoCopy{3});
//   CONSTEXPR_REQUIRE_EQ(Foo::GetOrGetIn(Foo(NoCopy{3}), Bar(NoCopy{2})),
//                        NoCopy{3});
//   CONSTEXPR_REQUIRE_EQ(Foo::GetOrGetIn(Bar(NoCopy{2}), Foo(NoCopy{3})),
//                        NoCopy{3});
//   CONSTEXPR_REQUIRE_EQ(Foo::GetOrGetIn(Foo(NoCopy{3})), NoCopy{3});

//   CONSTEXPR_REQUIRE_EQ(Foo::GetOr(Foo(NoCopy{7}), NoCopy{0}), NoCopy{7});
//   CONSTEXPR_REQUIRE_EQ(
//     Foo::GetOr(Merge(Foo(NoCopy{7}), Bar(NoCopy{6})), NoCopy{0}), NoCopy{7});
//   CONSTEXPR_REQUIRE_EQ(
//     Foo::GetOr(Merge(Bazz(NoCopy{7}), Bar(NoCopy{6})), NoCopy{0}), NoCopy{0});

//   CONSTEXPR_REQUIRE_EQ(Merge(Foo(NoCopy{5}), Bar(NoCopy{6})),
//                        Merge(Foo(NoCopy{5}), Bar(NoCopy{6})));

//   // Test Mapping
//   CONSTEXPR_REQUIRE_EQ(Merge(Foo(ident), Bar(6))(5), Merge(Foo(5), Bar(6)));
//   CONSTEXPR_REQUIRE_EQ(Merge(Foo(ident), Bar(6))(5), Merge(Foo(5), Bar(6)));

//   CONSTEXPR_REQUIRE_EQ(Merge(Bar(6), Foo(ident))(NoCopy{5}),
//                        Merge(Foo(NoCopy{5}), Bar(6)));

//   CONSTEXPR_REQUIRE_EQ(Merge(Bar(NoCopy{6}), Foo(ident))(5),
//                        Merge(Foo(5), Bar(NoCopy{6})));

//   CONSTEXPR_REQUIRE_EQ(Merge(Bar(5), Foo(6)).Reduce(0, sum), 11);
//   CONSTEXPR_REQUIRE_EQ(
//     Merge(Bar(NoCopy{5}), Foo(NoCopy{6})).Reduce(NoCopy{0}, sum_nocopy),
//     NoCopy{11});
//   CONSTEXPR_REQUIRE_EQ(MergeReduce(0, sum, Bar(6), Foo(7)), 6 + 7);
//   // CONSTEXPR_REQUIRE_EQ(MergeReduce(0, [](auto init, auto& a, auto& b) {return
//   // init + a.Get() * b.Get();}, Bar(6) + Foo(7), Bar(11) + Foo(12)),
//   //                      6 + 7);
// }
