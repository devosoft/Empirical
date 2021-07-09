#include "web/JSWrap.hpp"
#include "web/d3/d3_init.hpp"
#include "web/Document.hpp"
#include "web/_MochaTestRunner.hpp"
#include <iostream>
#include <functional>
#include <type_traits>
#include <utility>

#include "d3_testing_utils.hpp"

// This file tests:
// - D3_Base
//   - reference counting
//   - correct assignment of next id
// - library_d3.js
//   - find_function
//   - is_function

// This struct exists to allow us to instantiate a D3_Base object.
struct BaseTester : D3::D3_Base {
  public:
    BaseTester() : D3::D3_Base() { ; }
    BaseTester(int id) : D3::D3_Base(id) {}
};

// Test_BaseObjectIDAssignment
// - Tests:
//   - Reference counting
//   - Correct assignment of next ID
struct Test_BaseObjectIDAssignment : emp::web::BaseTest {
  // All persistent data structures necessary for test should be member variables.
  BaseTester test1{}; // This will get created 1st.
  BaseTester test2{}; // This will get created 2nd.
  BaseTester test3{}; // This will get created 3rd.
  emp::Ptr<BaseTester> test4;
  uint32_t test4_del_func_id=0;

  Test_BaseObjectIDAssignment() {
    test4 = emp::NewPtr<BaseTester>(D3::internal::NextD3ID());
    test4_del_func_id = emp::JSWrap([this]() { test4.Delete(); }, "TestDeleteBaseObject");

    this->Require(test1.GetID() == 0, "test1 id wrong");
    this->Require(test2.GetID() == 1, "test2 id wrong");
    this->Require(test3.GetID() == 2, "test3 id wrong");
    this->Require(test4->GetID() == 3, "test4 id wrong");
  }

  // Describe test (should only contain 'describe' call)
  void Describe() override {
    EM_ASM({
      describe('Base Object', function() {

        it('should create objects in Javascript', function() {
            chai.assert.equal(Object.keys(emp_d3.objects).length, 4, "emp_d3.objects");
            chai.assert.equal(Object.keys(emp_d3.counts).length, 4, "emp_d3.counts");
        });

        it('should track next_id correctly', function(){
            chai.assert.equal(emp_d3.next_id, 4, "emp_d3.next_id");
        });

        it("should do reference counting", function() {
            chai.assert.equal(emp_d3.counts[0], 1);
            chai.assert.equal(emp_d3.counts[1], 1);
            chai.assert.equal(emp_d3.counts[2], 1);
            chai.assert.equal(emp_d3.counts[3], 1);

            emp.TestDeleteBaseObject();

            chai.assert.equal(emp_d3.counts[0], 1);
            chai.assert.equal(emp_d3.counts[1], 1);
            chai.assert.equal(emp_d3.counts[2], 1);
            chai.assert(!(3 in emp_d3.counts));
            chai.assert(!(3 in emp_d3.objects));
        });

      });
    }); // -- end EM_ASM --
  }

  ~Test_BaseObjectIDAssignment() {
    emp::JSDelete(test4_del_func_id);
  }
};

// Test_LibraryD3
// - Tests the functions in library_d3.js
//  - add_searchable_namespace
//  - remove_searchable_namespace
//  - clear_emp_d3
//  - find_function
//  - is_function
struct Test_LibraryD3 : emp::web::BaseTest {

  void Describe() override {
    EM_ASM({
      describe("library_d3.js", function() {
        // describe("objects")
        describe("searchable namespaces", function() {
          it("should have expected default values", function() {
            chai.assert.deepEqual(emp_d3.searchable_namespaces, ["d3", "emp"]);
          });
        });

        describe("add_searchable_namespace", function() {
          it("should fail to add a non-existing namespace", function() {
            const success = emp_d3.add_searchable_namespace("THIS_JS_NAMESPACE_SHOULD_NOT_EXIST_HELLO_FROM_MY_COUCH");
            chai.assert(!success);
            chai.assert.deepEqual(emp_d3.searchable_namespaces, ["d3", "emp"]);
          });

          it("should add an existing namespace", function() {
            window["library_d3_test_ns"] = {};
            const success = emp_d3.add_searchable_namespace("library_d3_test_ns");
            chai.assert(success);
            chai.assert.deepEqual(emp_d3.searchable_namespaces, ["d3", "emp", "library_d3_test_ns"]);
          });
        });

        describe("remove_searchable_namespace", function() {
          it("should fail to remove namespace that is not in searchable_namespaces", function() {
            var current = [];
            for (ns of emp_d3.searchable_namespaces) { current.push(ns); }
            const success = emp_d3.remove_searchable_namespace("THIS_JS_NAMESPACE_SHOULD_NOT_EXIST_HELLO_FROM_MY_COUCH");
            chai.assert(!success);
            chai.assert.deepEqual(current, emp_d3.searchable_namespaces);
          });

          it("should remove namespace in searchable_namespace", function() {
            const success = emp_d3.remove_searchable_namespace("library_d3_test_ns");
            chai.assert(success);
            chai.assert.deepEqual(emp_d3.searchable_namespaces, ["d3", "emp"]);
          });
        });

        describe("clear_emp_d3", function() {
          it("should reset emp_d3 internal state", function() {
            // Throw some trash into the emp_d3 internal state objects.
            emp_d3.objects["garbage"] = "monster";
            emp_d3.counts["garbage"] = 128;
            emp_d3.objects["dumpster"] = "fire";
            emp_d3.counts["dumpster"] = 256;
            emp_d3.next_id = 512;
            emp_d3.searchable_namespaces = ["this", "is", "not", "what", "this", "should", "be"];
            // Take out the trash.
            emp_d3.clear_emp_d3();
            // Assert that we took out the trash.
            chai.assert.deepEqual(emp_d3.objects, {});
            chai.assert.deepEqual(emp_d3.counts, {});
            chai.assert.deepEqual(emp_d3.next_id, 0);
            chai.assert.deepEqual(emp_d3.searchable_namespaces, ["d3", "emp"]);
          });
        });

        describe("find_function", function() {
          before(function() {
            window["library_d3_test_ns"]["real_function"] = function() { return 42; };
            window["library_d3_test_ns"]["fake_function"] = "not a function";
            window["very_exposed_very_real_function"] = function() { return 43; };
            emp_d3.add_searchable_namespace("library_d3_test_ns");
          });

          it("should fail to return a function that doesn't exist", function() {
            const func0 = emp_d3.find_function("this_function_does_not_exist");
            const func1 = emp_d3.find_function("fake_function");
            chai.assert.equal(func0, "this_function_does_not_exist");
            chai.assert.equal(func1, "fake_function");
          });

          it("should return a function that exists", function() {
            // find function in root namespace
            var root_func = emp_d3.find_function("very_exposed_very_real_function");
            chai.assert.typeOf(root_func, "function");
            chai.assert.equal(root_func(), 43);

            // find function in custom namespace
            const custom_func = emp_d3.find_function("real_function");
            chai.assert.typeOf(custom_func, "function");
            chai.assert.equal(custom_func(), 42);

            // find function in emp namespace
            const emp_func = emp_d3.find_function("PassStringToCpp");
            chai.assert.typeOf(emp_func, "function");

            // find function in d3 namespace
            const d3_func = emp_d3.find_function("interpolatePurples");
            chai.assert.typeOf(d3_func, "function");
          });
        });

        describe("is_function", function() {
          it("should fail to find a function that doesn't exist", function() {
            chai.assert(!emp_d3.is_function("this_function_does_not_exist"));
            chai.assert(!emp_d3.is_function("fake_function"));
          });
          it("should find a function that exists", function() {
            // find function in root namespace
            chai.assert(emp_d3.is_function("very_exposed_very_real_function"));
            // find function in custom namespace
            chai.assert(emp_d3.is_function("real_function"));
            // find function in emp namespace
            chai.assert(emp_d3.is_function("PassStringToCpp"));
            // find function in d3 namespace
            chai.assert(emp_d3.is_function("interpolatePurples"));
          });
        });
      });
    });
  }

};

emp::web::MochaTestRunner test_runner;
int main() {
  D3::internal::get_emp_d3(); // Initialize emp_d3

  test_runner.Initialize({"emp_test_container"});

  std::cout << "Creating test manager." << std::endl;
  test_runner.AddTest<Test_BaseObjectIDAssignment>("D3 Base Object ID Assignment");
  test_runner.AddTest<Test_LibraryD3>("LibraryD3");

  test_runner.OnBeforeEachTest([]() {
    ResetD3Context();
  });

  test_runner.Run();
}