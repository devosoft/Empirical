//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2018.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  Some example code for using ValPack

#include <iostream>
#include <string>
#include <vector>

#include "meta/ValPack.h"

#define EMP_PRINT_VAL(X) std::cout << #X << " = " << X << std::endl
#define EMP_PRINT_PACK(X) std::cout << #X << " = "; X::PrintVals(); std::cout << std::endl

int main()
{
  // using rise_t = emp::ValPack<1,2,3,4,5,6,7,8,9,10>;
  using rise_t = emp::ValPackRange<1,11>;
  std::cout << "using rise_t = emp::ValPack<1,2,3,4,5,6,7,8,9,10>" << std::endl;
  std::cout << "Printing Pack: emp::ValPack<";
  rise_t::PrintVals();
  std::cout << '>' << std::endl;
  EMP_PRINT_VAL(rise_t::GetSize());
  EMP_PRINT_VAL(rise_t::Has(0));
  EMP_PRINT_VAL(rise_t::Has(10));
  EMP_PRINT_VAL(rise_t::Has(13));
  EMP_PRINT_VAL(rise_t::Count(1));
  EMP_PRINT_VAL(rise_t::GetID(5));
  EMP_PRINT_VAL(rise_t::GetID(8));
  EMP_PRINT_VAL(rise_t::IsUnique());
  EMP_PRINT_VAL(rise_t::Sum());
  EMP_PRINT_VAL(rise_t::Product());

  using fib_t = emp::ValPack<0,1,1,2,3,5,8,13,21,34>;
  std::cout << "\nusing fib_t  = emp::ValPack<0,1,1,2,3,5,8,13,21,34>" << std::endl;
  std::cout << "Printing Pack: emp::ValPack<";
  fib_t::PrintVals();
  std::cout << '>' << std::endl;
  EMP_PRINT_VAL(fib_t::GetSize());
  EMP_PRINT_VAL(fib_t::Has(0));
  EMP_PRINT_VAL(fib_t::Has(10));
  EMP_PRINT_VAL(fib_t::Has(13));
  EMP_PRINT_VAL(fib_t::Count(1));
  EMP_PRINT_VAL(fib_t::GetID(5));
  EMP_PRINT_VAL(fib_t::GetID(8));
  EMP_PRINT_VAL(fib_t::IsUnique());
  EMP_PRINT_VAL(fib_t::Sum());
  EMP_PRINT_VAL(fib_t::Product());

  std::cout << "\nTesting sorting and removing.\n";
  using test_t = emp::ValPack<0,1,2,1,3,4,1,4,5,6,1,1,-1,-1>::push_back<0>;
  std::cout << "Starting pack: ";  test_t::PrintVals(); std::cout << std::endl;
  EMP_PRINT_VAL(test_t::Min());
  EMP_PRINT_VAL(test_t::Max());
  EMP_PRINT_PACK(test_t);
  EMP_PRINT_PACK(test_t::remove<1>);
  EMP_PRINT_PACK(test_t::pop_val<1>);
  EMP_PRINT_PACK(test_t::append<fib_t>);

  using t1 = emp::ValPack<1,2,3,4,5,6,7,8,9,10,11>;
  EMP_PRINT_PACK(t1::append<t1>);

  emp::ValPack<1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25>::PrintVals();
  std::cout << std::endl;

  EMP_PRINT_PACK(emp::pack::reverse<test_t>);
  EMP_PRINT_PACK(emp::pack::uniq<test_t>);
  EMP_PRINT_PACK(emp::pack::sort<test_t>);
  EMP_PRINT_PACK(emp::pack::Usort<test_t>);
  EMP_PRINT_PACK(emp::pack::Rsort<test_t>);
  EMP_PRINT_PACK(emp::pack::RUsort<test_t>);

  // Try out ValPAck with doubles...
  using t2 = emp::ValPack<'t','e','s','t','i','n','g','!'>;
  EMP_PRINT_PACK(t2);
  std::cout << "As string: " << t2::ToString() << std::endl;
}
