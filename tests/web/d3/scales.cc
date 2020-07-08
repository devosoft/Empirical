#include "web/Document.h"
#include "web/d3/d3_init.h"
#include "web/d3/scales.h"
#include "web/_MochaTestRunner.h"
#include "d3_testing_utils.h"

// This file tests:
// - D3_Scales

struct TestLinearScale : emp::web::BaseTest {
  D3::LinearScale testLinearInt;
  int linearInt_1;
  int linearInt_2;
  int linearInt_3;
  int linearInt_4;
  emp::vector<int> testLinearIntDomain;
  emp::vector<int> testLinearIntRange;

  D3::LinearScale testLinearIntNice;
  emp::vector<int> testLinearIntNiceDomain;

  D3::LinearScale testLinearColor;
  std::string linearColor_1;
  std::string linearColor_2;
  emp::vector<int> testLinearColorDomain;
  emp::vector<std::string> testLinearColorRange;

  TestLinearScale() { Setup(); }

  void Setup() { 
    testLinearInt.SetDomain(10, 130);
    testLinearInt.SetRange(0, 960);
    linearInt_1 = testLinearInt.ApplyScale<int>(20);
    linearInt_2 = testLinearInt.ApplyScale<int>(50);
    linearInt_3 = testLinearInt.Invert<int>(80);
    linearInt_4 = testLinearInt.Invert<int>(320); 
    testLinearIntDomain = testLinearInt.GetDomain<int>();
    testLinearIntRange = testLinearInt.GetRange<int>();

    testLinearIntNice.SetDomain(0.5, 100.5);
    testLinearIntNice.SetRange(0, 1000);
    testLinearIntNice.Nice();
    testLinearIntNiceDomain = testLinearIntNice.GetDomain<int>();
  
    // TODO: figure out a way for: emp::pass_array_to_javascript(testLinearIntDomain)
    // to not get overridden when testLinearColor.SetRange(colorArray) is called

    testLinearColor.SetDomain(10, 100);
    emp::array<std::string, 2> colorArray = {"brown", "steelblue"};
    testLinearColor.SetRange(colorArray);
    linearColor_1 = testLinearColor.ApplyScale<std::string>(20);
    linearColor_2 = testLinearColor.ApplyScale<std::string>(50);
    testLinearColorDomain = testLinearColor.GetDomain<int>();
    testLinearColorRange = testLinearColor.GetRange<std::string>();
  }
  
  void Describe() override {

    EM_ASM({  
      const linearInt_1 = $0;
      const linearInt_2 = $1;
      const linearInt_3 = $2;
      const linearInt_4 = $3;
      const linearColor_1 = UTF8ToString($4);
      const linearColor_2 = UTF8ToString($5);
      const linearIntDomain_0 = $6;
      const linearIntDomain_1 = $7;
      const linearIntRange_0 = $8;
      const linearIntRange_1 = $9;
      const linearIntNiceDomain_0 = $10;
      const linearIntNiceDomain_1 = $11;
      const linearColorDomain_0 = $12;
      const linearColorDomain_1 = $13;
      const linearColorRange_0 = UTF8ToString($14);
      const linearColorRange_1 = UTF8ToString($15);

      var x = d3.scaleLinear()
                  .domain([ 10, 130 ])
                  .range([ 0, 960 ]);

      var nice_x = d3.scaleLinear()
                       .domain([ 0.5, 100.5 ])
                       .range([ 0, 1000 ])
                       .nice();

      var color = d3.scaleLinear()
                      .domain([ 10, 100 ])
                      .range([ "brown", "steelblue" ]);

      describe("creating a linear scale", function() {
        it("should apply the int scale correctly", function() {
          chai.assert.equal(linearInt_1, x(20)); // 80
          chai.assert.equal(linearInt_2, x(50)); // 320
        });
        it("should invert the int scale correctly", function() {
          chai.assert.equal(linearInt_3, x.invert(80)); // 20
          chai.assert.equal(linearInt_4, x.invert(320)); // 50
        });
        it("should have the same domain for the int scale", function() {
          chai.assert.equal(linearIntDomain_0, 10); // [10, 130]
          chai.assert.equal(linearIntDomain_1, 130);
        });
        it("should have the same range for the int scale", function() {
          chai.assert.equal(linearIntRange_0, 0); // [0, 960]
          chai.assert.equal(linearIntRange_1, 960);
        });
        it("should nice the domain appropriately", function() {
          chai.assert.equal(linearIntNiceDomain_0, nice_x.domain()[0]); // [0, 110]
          chai.assert.equal(linearIntNiceDomain_1, nice_x.domain()[1]);
        });
        it("should apply the color scale correctly", function() {
          chai.assert.equal(linearColor_1, color(20)); // "rgb(154, 52, 57)"
          chai.assert.equal(linearColor_2, color(50)); // "rgb(123, 81, 103)"
        });
        it("should have the same domain for the color scale", function() {
          chai.assert.equal(linearColorDomain_0, 10); // [10, 100]
          chai.assert.equal(linearColorDomain_1, 100);
        });
        it("should have the same range for the color scale", function() {
          chai.assert.equal(linearColorRange_0, "brown"); // ["brown", "steelblue"]
          chai.assert.equal(linearColorRange_1, "steelblue");
        });
      });
    }, linearInt_1, linearInt_2, linearInt_3, linearInt_4, linearColor_1.c_str(), linearColor_2.c_str(), 
      testLinearIntDomain[0], testLinearIntDomain[1], testLinearIntRange[0], testLinearIntRange[1], testLinearIntNiceDomain[0], testLinearIntNiceDomain[1],
      testLinearColorDomain[0], testLinearColorDomain[1], testLinearColorRange[0].c_str(), testLinearColorRange[1].c_str());
  }
};

struct TestPowScale : emp::web::BaseTest {
  D3::PowScale testPow1;
  double testPow1_val1;
  double testPow1_val2;
  double testPow1_val3;
  emp::vector<int> testPow1_Domain;
  emp::vector<int> testPow1_Range;

  D3::PowScale testPow2;
  double testPow2_val1;
  double testPow2_val2;
  double testPow2_val3;

  D3::PowScale testPow3;
  std::string testPow3_val1;
  std::string testPow3_val2;
  std::string testPow3_val3;

  TestPowScale() { Setup(); }

  void Setup() { 
    testPow1.SetExponent(0.5);
    testPow1.SetDomain(0, 2e9);
    testPow1.SetRange(0, 300);
    testPow1_val1 = testPow1.ApplyScale<double>(1.386e9);
    testPow1_val2 = testPow1.ApplyScale<double>(127e6);
    testPow1_val3 = testPow1.ApplyScale<double>(427e3);
    testPow1_Domain = testPow1.GetDomain<int>();
    testPow1_Range = testPow1.GetRange<int>();

    testPow2.SetExponent(1.5);
    testPow2.SetDomain(0, 2e9);
    testPow2.SetRange(0, 300);
    testPow2_val1 = testPow2.ApplyScale<double>(1.386e9);
    testPow2_val2 = testPow2.ApplyScale<double>(127e6);
    testPow2_val3 = testPow2.ApplyScale<double>(427e3);

    testPow3.SetDomain(0, 2e9);
    emp::array<std::string, 2> colorArray = {"yellow", "red"};
    testPow3.SetRange(colorArray);
    testPow3_val1 = testPow3.ApplyScale<std::string>(1.386e9);
    testPow3_val2 = testPow3.ApplyScale<std::string>(127e6);
    testPow3_val3 = testPow3.ApplyScale<std::string>(427e3);
  }

  void Describe() override {
  
   EM_ASM({  
     const testPow1_val1 = $0;
     const testPow1_val2 = $1;
     const testPow1_val3 = $2;
     const testPow1_Domain_0 = $3;
     const testPow1_Domain_1 = $4;
     const testPow1_Range_0 = $5;
     const testPow1_Range_1 = $6;
     const testPow2_val1 = $7;
     const testPow2_val2 = $8;
     const testPow2_val3 = $9;
     const testPow3_val1 = UTF8ToString($10);
     const testPow3_val2 = UTF8ToString($11);
     const testPow3_val3 = UTF8ToString($12);

     var population = d3.scalePow()
                          .exponent(0.5)
                          .domain([0, 2e9])
                          .range([0, 300]);

     var population2 = d3.scalePow()
                          .exponent(1.5)
                          .domain([0, 2e9])
                          .range([0, 300]);

     var population3 = d3.scalePow()
                          .domain([0, 2e9])
                          .range(["yellow", "red"]);

     describe("creating a pow scale", function() {
        it("should apply the first scale correctly", function() {
          chai.assert.equal(testPow1_val1, population(1.386e9)); // 249.73986465920893
          chai.assert.equal(testPow1_val2, population(127e6));    // 75.59761901012492
          chai.assert.equal(testPow1_val3, population(427e3));    // 4.383491758860737
        });
        it("should have the same domain for the first scale", function() {
          chai.assert.equal(testPow1_Domain_0, population.domain()[0]); // [0, 2000000000]
          chai.assert.equal(testPow1_Domain_1, population.domain()[1]);
        });
        it("should have the same range for the first scale", function() {
          chai.assert.equal(testPow1_Range_0, population.range()[0]); // [0, 300]
          chai.assert.equal(testPow1_Range_1, population.range()[1]);
        });
        it("should apply the second scale correctly", function() {
          chai.assert.equal(testPow2_val1, population2(1.386e9)); // 173.06972620883178
          chai.assert.equal(testPow2_val2, population2(127e6));    // 4.800448807142932
          chai.assert.equal(testPow2_val3, population2(427e3));    // 0.0009358754905167673
        });
        it("should apply the third scale correctly", function() {
          chai.assert.equal(testPow3_val1, population3(1.386e9)); // "rgb(255, 78, 0)"
          chai.assert.equal(testPow3_val2, population3(127e6));    // "rgb(255, 239, 0)"
          chai.assert.equal(testPow3_val3, population3(427e3));    // "rgb(255, 255, 0)"
        });
      });
    }, testPow1_val1, testPow1_val2, testPow1_val3, testPow1_Domain[0], testPow1_Domain[1], testPow1_Range[0], testPow1_Range[1],
       testPow2_val1, testPow2_val2, testPow2_val3, testPow3_val1.c_str(), testPow3_val2.c_str(), testPow3_val3.c_str());
  }
};

emp::web::MochaTestRunner test_runner;

int main() {
  emp::Initialize();
  D3::internal::get_emp_d3();

  test_runner.AddTest<TestLinearScale>("LinearScale");
  test_runner.AddTest<TestPowScale>("PowScale");
  
  test_runner.OnBeforeEachTest([]() {
    ResetD3Context();
  });
  
  test_runner.Run();
}