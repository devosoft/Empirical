#define CATCH_CONFIG_MAIN

#include "third-party/Catch/single_include/catch.hpp"

#include "base/vector.h"

#include <sstream>
#include <iostream>

TEST_CASE("Test vector", "[base]")
{
	emp::vector<std::string> vec = {"a", "b", "c"};
	std::string sum;
	for(auto it=vec.cbegin(); it!=vec.cend(); it++){
		sum += *it;
	}
	REQUIRE(sum == "abc");
	
	/// This throws an error...
	// because it's a reverse iterator?
	//sum += *vec.rend();
	//REQUIRE(sum == "abca");

	auto itr = vec.emplace(vec.begin()+1, "1");
	REQUIRE(*itr == "1");
	REQUIRE(*(itr-1) == "a");
	
	emp::vector<bool> bvec;
	bvec.resize(1);
	REQUIRE(bvec.size() == 1);
	bvec[0] = true;
	REQUIRE(bvec[0] == true);
	bvec.resize(5,false);
	REQUIRE(bvec[1] == false);
	
	/// Can't seem to get front() and back() to work for bool vector
	/// throws error on this line in vector.h "size_t pos = (size_t) (*this - v_ptr->begin());"
	//auto b = bvec.front();
	
	emp::vector<bool> bvec2 = { true, false, true, false };
	REQUIRE(bvec2.size() == 4);
	bvec2.pop_back();
	REQUIRE(bvec2.size() == 3);
	
	std::stringstream ss;
	ss << bvec2;
	REQUIRE(ss.str() == "1 0 1 ");
	ss.str(std::string());
	
	emp::vector<bool> bvec3(3);
	/// Can't do this 
	/// "error: invalid initialization of non-const reference of type 'bool&' from an rvalue of type 'bool'"
	//ss >> bvec3;
}