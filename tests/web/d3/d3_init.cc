//
#include "web/JSWrap.h"
#include "web/d3/d3_init.h"

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
    BaseTester() {;}
    BaseTester(int id) : D3::D3_Base(id) {}
};

void ResetContext() {
  EM_ASM({
    emp_d3.clear_emp_d3();  // Reset the emp_d3 object tracker
  });
}


void Test_BaseObjectIDAssignment() {
  ResetContext();

  BaseTester test1;
  BaseTester test2;
  BaseTester test3;
  emp::Ptr<BaseTester> test4;

  test4.New(D3::internal::NextD3ID());

  const size_t func_id = emp::JSWrap([&test4](){test4.Delete();}, "TestDeleteBaseObject");

  emp_assert(test1.GetID() == 0);
  emp_assert(test2.GetID() == 1);
  emp_assert(test3.GetID() == 2);
  emp_assert(test4->GetID() == 3);

  EM_ASM({
      describe('Base Object', function() {

        it('should create objects in Javascript', function() {
            chai.assert.equal(emp_d3.objects.length, 4);
            chai.assert.equal(emp_d3.counts.length, 4);
        });

        it('should track next_id correctly', function(){
            chai.assert.equal(emp_d3.next_id, 4);
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
            chai.assert.equal(emp_d3.counts[3], 0);
        });

      });
  });

  emp::JSDelete(func_id);
  ResetContext();
}

int main() {
  Test_BaseObjectIDAssignment();
}