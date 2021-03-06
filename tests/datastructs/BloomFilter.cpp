#define CATCH_CONFIG_MAIN

#include "third-party/Catch/single_include/catch2/catch.hpp"

#include "emp/datastructs/BloomFilter.hpp"
#include "emp/base/vector.hpp"

TEST_CASE("Test Bloom Filter", "[datastructs]")
{
	emp::BloomParameters parameters;
	parameters.projected_element_count = 100;
	parameters.false_positive_probability = .001;
	parameters.compute_optimal_parameters();
	emp::BloomFilter b(parameters);

	CHECK(!b.contains("hello"));
	b.insert("hello");
	CHECK(b.contains("hello"));
	b.clear();
	CHECK(!b.contains("hello"));
	b.insert(5);
	CHECK(b.contains(5));

	emp::vector<int> v = {3,4,9};
	CHECK(b.contains_none(v.begin(), v.end()) == v.end());
	b.insert(v.begin(), v.end());
	CHECK(b.contains(9));
	CHECK(b.contains_all(v.begin(), v.end()) == v.end());

	CHECK(b.effective_fpp() < .001);
	CHECK(b.size() == 1440);
	CHECK(b.element_count() == 4);
	CHECK(b.hash_count() == 10);

	emp::CompressibleBloomFilter cbf(parameters);
	cbf.insert("Hi!");
	
	emp::BloomFilter b2 = b | cbf;
	CHECK(b2.contains("Hi!"));
	CHECK(b2.contains(3));

	cbf.insert(4);
	cbf.insert(10);
	emp::BloomFilter b3 = b & cbf;
	CHECK(b3.contains(4));
	CHECK(!b3.contains(10));

	emp::BloomFilter b4 = b ^ cbf;
	CHECK(!b4.contains(4));
	CHECK(b4.contains(9));

	cbf.compress(50);
	CHECK(cbf.size() == 720);
	CHECK(b4 != b3);
	CHECK(b4 == b4);
}
