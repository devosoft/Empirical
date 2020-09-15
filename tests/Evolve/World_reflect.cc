#define CATCH_CONFIG_MAIN
#ifndef NDEBUG
	#define TDEBUG
#endif

#include "third-party/Catch/single_include/catch2/catch.hpp"

#include "Evolve/World.h"
#include "Evolve/World_reflect.h"

#include <sstream>
#include <iostream>

class MyOrg
{
	private:
		double value;
	public:
		explicit MyOrg(double val) : value(val) {}

		double GetFitness(){ return value; }
		double DoMutations(emp::Random& r){ value = value+1.0; return value; }
		double GetValue() { return value; }
		void Print(std::ostream &os) { os << value; }
};

TEST_CASE("Test World reflect", "[Evolve]")
{
	emp::World<double> world;
	world.InjectAt(2.2, 0);
	world.InjectAt(8.5, 1);
	world.InjectAt(9.3, 2);

	std::stringstream ss;
	emp::SetDefaultPrintFun_impl<emp::World<double>, double>(world, true);
	world.Print(ss, "", " ");
	REQUIRE(ss.str() == "2.2 8.5 9.3 ");
	ss.str(std::string());

	#ifndef NDEBUG
	emp::SetDefaultFitFun_impl<emp::World<double>, double>(world);
	double org = 2.2;
	world.CalcFitnessOrg(org);
	REQUIRE(emp::assert_last_fail);
	emp::assert_clear();

	emp::SetDefaultMutFun_impl<emp::World<double>, double>(world);
	world.DoMutationsOrg(org);
	REQUIRE(emp::assert_last_fail);
	emp::assert_clear();

	emp::SetDefaultPrintFun_impl<emp::World<double>, double>(world);
	world.Print();
	REQUIRE(emp::assert_last_fail);
	emp::assert_clear();
	#endif

	emp::World<MyOrg> world1;
	world1.InjectAt(MyOrg(7.1), 0);
	world1.InjectAt(MyOrg(2.4), 1);
	world1.InjectAt(MyOrg(11.8), 2);

	MyOrg org3(3.7);
	REQUIRE(world1.CalcFitnessOrg(org3) == 3.7);

	world1.DoMutationsOrg(org3);
	REQUIRE( (org3.GetValue() == 4.7) );

	world1.Print(ss, "", " ");
	REQUIRE(ss.str() == "7.1 2.4 11.8 ");
}
