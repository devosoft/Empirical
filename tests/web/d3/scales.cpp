#include "web/Document.hpp"
#include "web/d3/d3_init.hpp"
#include "web/d3/scales.hpp"
#include "web/_MochaTestRunner.hpp"
#include "d3_testing_utils.hpp"

// To run these tests call `cd d3 && make scales.js && .. && ../../third-party/node_modules/karma/bin/karma start karma.conf.js --filename d3/scales` from tests/web

// This file tests:
// - D3_Scales
// -- LinearScale
// -- PowScale
// -- SqrtScale
// -- LogScale
// -- SymlogScale
// -- IdentityScale
// -- TimeScale
// -- SequentialScale
// -- SequentialLogScale
// -- SequentialPowScale
// -- SequentialSqrtScale
// -- SequentialSymlogScale
// -- SequentialQuantileScale
// -- DivergingScale
// -- DivergingLogScale
// -- DivergingPowScale
// -- DivergingSqrtScale
// -- DivergingSymlogScale
// -- QuantizeScale
// -- QuantileScale
// -- ThresholdScale
// -- OrdinalScale
// -- BandScale
// -- PointScale

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
    linearInt_1 = testLinearInt.ApplyScale<int, int>(20);
    linearInt_2 = testLinearInt.ApplyScale<int, int>(50);
    linearInt_3 = testLinearInt.Invert<int>(80);
    linearInt_4 = testLinearInt.Invert<int>(320); 
    testLinearIntDomain = testLinearInt.GetDomain<int>();
    testLinearIntRange = testLinearInt.GetRange<int>();

    testLinearIntNice.SetDomain(0.5, 100.5);
    testLinearIntNice.SetRange(0, 1000);
    testLinearIntNice.Nice();
    testLinearIntNiceDomain = testLinearIntNice.GetDomain<int>();

    testLinearColor.SetDomain(10, 100);
    emp::array<std::string, 2> colorArray = {"brown", "steelblue"};
    testLinearColor.SetRange(colorArray);
    linearColor_1 = testLinearColor.ApplyScale<std::string, int>(20);
    linearColor_2 = testLinearColor.ApplyScale<std::string, int>(50);
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
    testPow1_val1 = testPow1.ApplyScale<double, double>(1.386e9);
    testPow1_val2 = testPow1.ApplyScale<double, double>(127e6);
    testPow1_val3 = testPow1.ApplyScale<double, double>(427e3);
    testPow1_Domain = testPow1.GetDomain<int>();
    testPow1_Range = testPow1.GetRange<int>();

    testPow2.SetExponent(1.5);
    testPow2.SetDomain(0, 2e9);
    testPow2.SetRange(0, 300);
    testPow2_val1 = testPow2.ApplyScale<double, double>(1.386e9);
    testPow2_val2 = testPow2.ApplyScale<double, double>(127e6);
    testPow2_val3 = testPow2.ApplyScale<double, double>(427e3);

    testPow3.SetDomain(0, 2e9);
    emp::array<std::string, 2> colorArray = {"yellow", "red"};
    testPow3.SetRange(colorArray);
    testPow3_val1 = testPow3.ApplyScale<std::string, double>(1.386e9);
    testPow3_val2 = testPow3.ApplyScale<std::string, double>(127e6);
    testPow3_val3 = testPow3.ApplyScale<std::string, double>(427e3);
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

struct TestSqrtScale : emp::web::BaseTest {
  // a regular SqrtScale with domain and range as ints/doubles
  D3::SqrtScale testSqrt1;
  double testSqrt1_val1;
  double testSqrt1_val2;
  double testSqrt1_val3;

  // a copy of the above testSqrt1 scale
  // D3::SqrtScale testSqrtCopy;
  double testSqrtCopy_val1;
  double testSqrtCopy_val2;
  double testSqrtCopy_val3;

  TestSqrtScale() { Setup(); }

  void Setup() {
    testSqrt1.SetDomain(0, 2e9);
    testSqrt1.SetRange(0, 300);
    testSqrt1_val1 = testSqrt1.ApplyScale<double, double>(1.386e9);
    testSqrt1_val2 = testSqrt1.ApplyScale<double, double>(127e6);
    testSqrt1_val3 = testSqrt1.ApplyScale<double, double>(427e3);

    D3::SqrtScale testSqrtCopy(testSqrt1);
    testSqrtCopy_val1 = testSqrtCopy.ApplyScale<double, double>(1.386e9);
    testSqrtCopy_val2 = testSqrtCopy.ApplyScale<double, double>(127e6);
    testSqrtCopy_val3 = testSqrtCopy.ApplyScale<double, double>(427e3);
  }

  void Describe() override {
    EM_ASM({
      const testSqrt1_val1 = $0;
      const testSqrt1_val2 = $1;
      const testSqrt1_val3 = $2;
      const testSqrtCopy_val1 = $3;
      const testSqrtCopy_val2 = $4;
      const testSqrtCopy_val3 = $5;

      var population = d3.scaleSqrt()
        .domain([0, 2e9])
        .range([0, 300]);

      var copyPopulation = population.copy();

      describe("creating a sqrt scale", function() {
        it("should apply the first scale correctly", function() {
          chai.assert.equal(testSqrt1_val1, population(1.386e9)); // 249.73986465920893
          chai.assert.equal(testSqrt1_val2, population(127e6));   // 75.59761901012492
          chai.assert.equal(testSqrt1_val3, population(427e3));   // 4.383491758860737
        });
        it("copy the first scale correctly", function() {
          chai.assert.equal(testSqrtCopy_val1, copyPopulation(1.386e9)); // 249.73986465920893
          chai.assert.equal(testSqrtCopy_val2, copyPopulation(127e6));   // 75.59761901012492
          chai.assert.equal(testSqrtCopy_val3, copyPopulation(427e3));   // 4.383491758860737
        });
      });
    }, testSqrt1_val1, testSqrt1_val2, testSqrt1_val3, testSqrtCopy_val1, testSqrtCopy_val2, testSqrtCopy_val3);
  }
};

struct TestLogScale : emp::web::BaseTest {
  // a log scale with no explicit base set (default is 10)
  D3::LogScale testLog1;
  double testLog1_val1;
  double testLog1_val2;
  double testLog1_val3;

  // a log scale with a base of 2
  D3::LogScale testLog2;
  double testLog2_val1;
  double testLog2_val2;
  double testLog2_val3;
  double testLog2_Unclamped;
  double testLog2_Clamped;

  TestLogScale() { Setup(); }

  void Setup() {
    testLog1.SetDomain(10, 100000).SetRange(0, 700);
    testLog1_val1 = testLog1.ApplyScale<double, double>(1000);
    testLog1_val2 = testLog1.ApplyScale<double, double>(1234);
    testLog1_val3 = testLog1.ApplyScale<double, double>(100000);

    testLog2.SetBase(2);
    testLog2.SetDomain(16, 1048576);
    testLog2.SetRange(0, 700);
    testLog2_val1 = testLog2.ApplyScale<double, double>(64);
    testLog2_val2 = testLog2.ApplyScale<double, double>(1234);
    testLog2_val3 = testLog2.ApplyScale<double, double>(1048576);
    testLog2_Unclamped = testLog2.ApplyScale<double, double>(5000000);
    
    // now clamp TestScale2 to test the SetClamp method
    testLog2.SetClamp(true);
    testLog2_Clamped = testLog2.ApplyScale<double, double>(5000000);
  }

  void Describe() override {
    EM_ASM({
      const testLog1_val1 = $0;
      const testLog1_val2 = $1;
      const testLog1_val3 = $2;
      const testLog2_val1 = $3;
      const testLog2_val2 = $4;
      const testLog2_val3 = $5;
      const testLog2_Unclamped = $6;
      const testLog2_Clamped = $7;

      var logScale = d3.scaleLog()
	      .domain([10, 100000])
	      .range([0, 700]);
      
      var logScale2 = d3.scaleLog()
        .base(2)
	      .domain([16, 1048576])
	      .range([0, 700]);
      
      var logScale2_Clamped = d3.scaleLog()
        .base(2)
	      .domain([16, 1048576])
	      .range([0, 700])
        .clamp(true);

      describe("creating a log scale", function() {
        it("should apply the first scale correctly", function() {
          chai.assert.equal(testLog1_val1, logScale(1000));   // 349.99999999999994
          chai.assert.equal(testLog1_val2, logScale(1234));   // 365.980152947014
          chai.assert.equal(testLog1_val3, logScale(100000)); // 700
        });
        it("should apply the second scale correctly", function() {
          chai.assert.equal(testLog2_val1, logScale2(64));      // 87.49999999999999
          chai.assert.equal(testLog2_val2, logScale2(1234));    // 274.2742922127871
          chai.assert.equal(testLog2_val3, logScale2(1048576)); // 700
        });
        it("should enable clamping on the second scale correctly", function() {
          chai.assert.equal(testLog2_Unclamped, logScale2(5000000));       // 798.5904790592547
          chai.assert.equal(testLog2_Clamped, logScale2_Clamped(5000000)); // 700 
        });
      });
    }, testLog1_val1, testLog1_val2, testLog1_val3, testLog2_val1, testLog2_val2, testLog2_val3, testLog2_Unclamped, testLog2_Clamped);
  }
};

struct TestSymlogScale : emp::web::BaseTest {
  // a symlog scale with no explicit constant set (default is 1)
  D3::SymlogScale testSymlog1;
  double testSymlog1_val1;
  double testSymlog1_val2;
  double testSymlog1_val3;
  double testSymlog1_val4;
  double testSymlog1_val5;

  // the same symlog scale but with a constant of 0.01
  D3::SymlogScale testSymlog2;
  double testSymlog2_val1;
  double testSymlog2_val2;
  double testSymlog2_val3;
  double testSymlog2_val4;
  double testSymlog2_val5;

  TestSymlogScale() { Setup(); }

  void Setup() {
    testSymlog1.SetDomain(-100000, 100000);
    testSymlog1.SetRange(-100, 100);
    testSymlog1_val1 = testSymlog1.ApplyScale<double, double>(-80000);
    testSymlog1_val2 = testSymlog1.ApplyScale<double, double>(-50);
    testSymlog1_val3 = testSymlog1.ApplyScale<double, double>(1.5); 
    testSymlog1_val4 = testSymlog1.ApplyScale<double, double>(50);
    testSymlog1_val5 = testSymlog1.ApplyScale<double, double>(80000);

    testSymlog2.SetDomain(-100000, 100000);
    testSymlog2.SetRange(-100, 100);
    testSymlog2.SetConstant(0.01);
    testSymlog2_val1 = testSymlog2.ApplyScale<double, double>(-80000);
    testSymlog2_val2 = testSymlog2.ApplyScale<double, double>(-50);
    testSymlog2_val3 = testSymlog2.ApplyScale<double, double>(1.5); 
    testSymlog2_val4 = testSymlog2.ApplyScale<double, double>(50);
    testSymlog2_val5 = testSymlog2.ApplyScale<double, double>(80000);
  }

  void Describe() override {
    EM_ASM({
      const testSymlog1_val1 = $0;
      const testSymlog1_val2 = $1;
      const testSymlog1_val3 = $2;
      const testSymlog1_val4 = $3;
      const testSymlog1_val5 = $4;
      const testSymlog2_val1 = $5;
      const testSymlog2_val2 = $6;
      const testSymlog2_val3 = $7;
      const testSymlog2_val4 = $8;
      const testSymlog2_val5 = $9;

      var symlogScale1 = d3.scaleSymlog()
	      .domain([-100000, 100000])
	      .range([-100, 100]);

      var symlogScale2 = d3.scaleSymlog()
	      .domain([-100000, 100000])
	      .range([-100, 100])
        .constant(0.01);

      describe("creating a symlog scale", function() {
        it("should apply the first scale correctly", function() {
          chai.assert.equal(testSymlog1_val1, symlogScale1(-80000)); // -98.06182313778929
          chai.assert.equal(testSymlog1_val2, symlogScale1(-50));    // -34.15137385860061
          chai.assert.equal(testSymlog1_val3, symlogScale1(1.5));    // 7.958793260555325
          chai.assert.equal(testSymlog1_val4, symlogScale1(50));     // 34.15137385860061
          chai.assert.equal(testSymlog1_val5, symlogScale1(80000));  // 98.06182313778928
        });
        it("should apply the second scale correctly", function() {
          chai.assert.equal(testSymlog2_val1, symlogScale2(-80000)); // -98.61557140643649
          chai.assert.equal(testSymlog2_val2, symlogScale2(-50));    // -52.843669022827925
          chai.assert.equal(testSymlog2_val3, symlogScale2(1.5));    // 31.128241911062098
          chai.assert.equal(testSymlog2_val4, symlogScale2(50));     // 52.843669022827925
          chai.assert.equal(testSymlog2_val5, symlogScale2(80000));  // 98.61557140643649
        });
      });
    }, testSymlog1_val1, testSymlog1_val2, testSymlog1_val3, testSymlog1_val4, testSymlog1_val5,
       testSymlog2_val1, testSymlog2_val2, testSymlog2_val3, testSymlog2_val4, testSymlog2_val5);
  }
};

struct TestIdentityScale : emp::web::BaseTest {
  // an identity scale 
  D3::IdentityScale testIdentity1;
  double testIdentity1_val1;
  double testIdentity1_val2;
  double testIdentity1_val3;

  TestIdentityScale() { Setup(); }

  void Setup() {
    testIdentity1.SetDomain(12, 1234.5);
    testIdentity1_val1 = testIdentity1.ApplyScale<double, double>(12);
    testIdentity1_val2 = testIdentity1.ApplyScale<double, double>(50.6789);
    testIdentity1_val3 = testIdentity1.ApplyScale<double, double>(1234);
  }

  void Describe() override {
    EM_ASM({
      const testIdentity1_val1 = $0;
      const testIdentity1_val2 = $1;
      const testIdentity1_val3 = $2;

      var identityScale1 = d3.scaleIdentity()
	      .domain([12, 1234.5]);

      describe("creating an identity scale", function() {
        it("should apply the first scale correctly", function() {
          chai.assert.equal(testIdentity1_val1, identityScale1(12));      // 12
          chai.assert.equal(testIdentity1_val2, identityScale1(50.6789)); // 50.6789
          chai.assert.equal(testIdentity1_val3, identityScale1(1234));    // 1234
        });
      });
    }, testIdentity1_val1, testIdentity1_val2, testIdentity1_val3);
  }
};

struct TestTimeScale : emp::web::BaseTest {
  // a time scale with a numeric range
  D3::TimeScale testTime1;
  double testTime1_val1;
  int testTime1_val2;
  D3::Date testTime1_val1_i;
  D3::Date testTime1_val2_i;

  // a time scale with a string range
  D3::TimeScale testTime2;
  std::string testTime2_val1;
  std::string testTime2_val2;

  TestTimeScale() { Setup(); }

  void Setup() {
    D3::Date dateMin(2000, 0, 1);
    D3::Date dateMax(2000, 0, 2);
    testTime1.SetDomain(dateMin, dateMax);
    testTime1.SetRange(0, 960);
    D3::Date date1(2000, 0, 1, 5);
    D3::Date date2(2000, 0, 1, 16);

    testTime1_val1 = testTime1.ApplyScale<double, D3::Date>(date1);
    testTime1_val2 = testTime1.ApplyScale<int, D3::Date>(date2);
    testTime1_val1_i = testTime1.Invert(200);
    testTime1_val2_i = testTime1.Invert(640);

    testTime2.SetDomain(dateMin, dateMax);
    testTime2.SetRange("red", "yellow");
    testTime2_val1 = testTime2.ApplyScale<std::string, D3::Date>(date1);
    testTime2_val2 = testTime2.ApplyScale<std::string, D3::Date>(date2);
  }

  void Describe() override {

    EM_ASM({
      const testTime1_val1 = $0;
      const testTime1_val2 = $1;
      const testTime1_val1_i_month = $2;
      const testTime1_val2_i_month = $3;
      const testTime1_val1_i_year = $4;
      const testTime1_val2_i_year = $5;
      const testTime1_val1_i_hour = $6;
      const testTime1_val2_i_hour = $7;
      const testTime2_val1 = UTF8ToString($8);
      const testTime2_val2 = UTF8ToString($9);

      var timeScale = d3.scaleTime()
        .domain([new Date(2000, 0, 1), new Date(2000, 0, 2)])
        .range([0, 960]);

      var timeColor = d3.scaleTime()
        .domain([new Date(2000, 0, 1), new Date(2000, 0, 2)])
        .range(["red", "yellow"]);
        
      describe("creating a time scale", function() {
        it("should apply the first scale correctly", function() {
          chai.assert.equal(testTime1_val1, timeScale(new Date(2000, 0, 1, 5)));   // 200
          chai.assert.equal(testTime1_val2, timeScale(new Date(2000, 0, 1, 16)));  // 640
        });
        it("should invert the first scale correctly", function() {
          // check the months
          chai.assert.equal(testTime1_val1_i_month, timeScale.invert(200).getMonth());  // Sat Jan 01 2000 05:00:00 GMT-0800 (PST)
          chai.assert.equal(testTime1_val2_i_month, timeScale.invert(640).getMonth());  // Sat Jan 01 2000 16:00:00 GMT-0800 (PST)
          // check the years
          chai.assert.equal(testTime1_val1_i_year, timeScale.invert(200).getFullYear());  // Sat Jan 01 2000 05:00:00 GMT-0800 (PST)
          chai.assert.equal(testTime1_val2_i_year, timeScale.invert(640).getFullYear());  // Sat Jan 01 2000 16:00:00 GMT-0800 (PST)
          // check the hours
          chai.assert.equal(testTime1_val1_i_hour, timeScale.invert(200).getHours());  // Sat Jan 01 2000 05:00:00 GMT-0800 (PST)
          chai.assert.equal(testTime1_val2_i_hour, timeScale.invert(640).getHours());  // Sat Jan 01 2000 16:00:00 GMT-0800 (PST)
        });
        it("should apply the second scale correctly", function() {
          chai.assert.equal(testTime2_val1, timeColor(new Date(2000, 0, 1, 5)));   // rgb(255, 53, 0)
          chai.assert.equal(testTime2_val2, timeColor(new Date(2000, 0, 1, 16)));  // rgb(255, 170, 0)
        });
      });
    }, testTime1_val1, testTime1_val2, testTime1_val1_i.month, testTime1_val2_i.month, testTime1_val1_i.year, testTime1_val2_i.year, 
       testTime1_val1_i.hours, testTime1_val2_i.hours, testTime2_val1.c_str(), testTime2_val2.c_str());
  }
};

struct TestSequentialScale : emp::web::BaseTest {
  // a sequential scale 
  D3::SequentialScale testSequentialScale1;
  std::string testSequentialScale1_val1;
  std::string testSequentialScale1_val2;
  std::string testSequentialScale1_val3;

  TestSequentialScale() { Setup(); }

  void Setup() {
    testSequentialScale1.SetDomain(0, 100);
    testSequentialScale1.SetInterpolator("interpolateRainbow");
    testSequentialScale1_val1 = testSequentialScale1.ApplyScale<std::string, int>(0);
    testSequentialScale1_val2 = testSequentialScale1.ApplyScale<std::string, int>(50);
    testSequentialScale1_val3 = testSequentialScale1.ApplyScale<std::string, int>(100);
  }

  void Describe() override {
    EM_ASM({
      const testSequentialScale1_val1 = UTF8ToString($0);
      const testSequentialScale1_val2 = UTF8ToString($1);
      const testSequentialScale1_val3 = UTF8ToString($2);

      var sequentialScale = d3.scaleSequential()
        .domain([0, 100])
        .interpolator(d3.interpolateRainbow);

      describe("creating a sequential scale", function() {
        it("should apply the first scale correctly", function() {
          chai.assert.equal(testSequentialScale1_val1, sequentialScale(0));    // rgb(110, 64, 170)
          chai.assert.equal(testSequentialScale1_val2, sequentialScale(50));   // rgb(175, 240, 91)
          chai.assert.equal(testSequentialScale1_val3, sequentialScale(100));  // rgb(110, 64, 170)
        });
      });
    }, testSequentialScale1_val1.c_str(), testSequentialScale1_val2.c_str(), testSequentialScale1_val3.c_str());
  }
};

struct TestSequentialLogScale : emp::web::BaseTest {
  // a sequential log scale 
  D3::SequentialLogScale testSeqLogScale1;
  std::string testSeqLogScale1_val1;
  std::string testSeqLogScale1_val2;
  std::string testSeqLogScale1_val3;

  TestSequentialLogScale() { Setup(); }

  void Setup() {
    testSeqLogScale1.SetDomain(1e-8, 1e8);
    testSeqLogScale1.SetInterpolator("interpolatePuBuGn");
    testSeqLogScale1_val1 = testSeqLogScale1.ApplyScale<std::string, double>(1e-8);
    testSeqLogScale1_val2 = testSeqLogScale1.ApplyScale<std::string, double>(100.1234);
    testSeqLogScale1_val3 = testSeqLogScale1.ApplyScale<std::string, double>(1e8);
  }

  void Describe() override {
    EM_ASM({
      const testSeqLogScale1_val1 = UTF8ToString($0);
      const testSeqLogScale1_val2 = UTF8ToString($1);
      const testSeqLogScale1_val3 = UTF8ToString($2);

      var seqLog = d3.scaleSequentialLog()
                       .domain([ 1e-8, 1e8 ])
                       .interpolator(d3.interpolatePuBuGn);

      describe("creating a sequential log scale", function() {
        it("should apply the first scale correctly", function() {
          chai.assert.equal(testSeqLogScale1_val1, seqLog(1e-8));  // rgb(255, 247, 251)
          chai.assert.equal(testSeqLogScale1_val2, seqLog(100.1234));   // rgb(53, 146, 185)
          chai.assert.equal(testSeqLogScale1_val3, seqLog(1e8));   // rgb(1, 70, 54)
        });
      });
    }, testSeqLogScale1_val1.c_str(), testSeqLogScale1_val2.c_str(), testSeqLogScale1_val3.c_str());
  }
};

struct TestSequentialPowScale : emp::web::BaseTest {
  // a sequential pow scale 
  D3::SequentialPowScale testSeqPowScale1;
  std::string testSeqPowScale1_val1;
  std::string testSeqPowScale1_val2;
  std::string testSeqPowScale1_val3;

  TestSequentialPowScale() { Setup(); }

  void Setup() {
    testSeqPowScale1.SetExponent(0);
    testSeqPowScale1.SetDomain(1e-8, 1e8);
    testSeqPowScale1.SetInterpolator("interpolatePuBuGn");
    testSeqPowScale1_val1 = testSeqPowScale1.ApplyScale<std::string, double>(1e-8);
    testSeqPowScale1_val2 = testSeqPowScale1.ApplyScale<std::string, double>(1e5);
    testSeqPowScale1_val3 = testSeqPowScale1.ApplyScale<std::string, double>(1e8);
  }

  void Describe() override {
    EM_ASM({
      const testSeqPowScale1_val1 = UTF8ToString($0);
      const testSeqPowScale1_val2 = UTF8ToString($1);
      const testSeqPowScale1_val3 = UTF8ToString($2);

      var seqPow = d3.scaleSequentialPow()
                       .exponent(0)
                       .domain([ 1e-8, 1e8 ])
                       .interpolator(d3.interpolatePuBuGn);

      describe("creating a sequential pow scale", function() {
        it("should apply the first scale correctly", function() {
          chai.assert.equal(testSeqPowScale1_val1, seqPow(1e-8));  // rgb(105, 168, 207)
          chai.assert.equal(testSeqPowScale1_val2, seqPow(1e5));   // rgb(105, 168, 207)
          chai.assert.equal(testSeqPowScale1_val3, seqPow(1e8));   // rgb(105, 168, 207)
        });
      });
    }, testSeqPowScale1_val1.c_str(), testSeqPowScale1_val2.c_str(), testSeqPowScale1_val3.c_str());
  }
};

struct TestSequentialSqrtScale : emp::web::BaseTest {
  // a sequential sqrt scale 
  D3::SequentialSqrtScale testSeqSqrtScale1;
  std::string testSeqSqrtScale1_val1;
  std::string testSeqSqrtScale1_val2;
  std::string testSeqSqrtScale1_val3;

  TestSequentialSqrtScale() { Setup(); }

  void Setup() {
    testSeqSqrtScale1.SetDomain(1e-8, 1e8);
    testSeqSqrtScale1.SetInterpolator("interpolatePuBuGn");
    testSeqSqrtScale1_val1 = testSeqSqrtScale1.ApplyScale<std::string, double>(1e-8);
    testSeqSqrtScale1_val2 = testSeqSqrtScale1.ApplyScale<std::string, double>(1e5);
    testSeqSqrtScale1_val3 = testSeqSqrtScale1.ApplyScale<std::string, double>(1e7);
  }

  void Describe() override {
    EM_ASM({
      const testSeqSqrtScale1_val1 = UTF8ToString($0);
      const testSeqSqrtScale1_val2 = UTF8ToString($1);
      const testSeqSqrtScale1_val3 = UTF8ToString($2);

      var seqSqrt = d3.scaleSequentialSqrt()
                       .domain([ 1e-8, 1e8 ])
                       .interpolator(d3.interpolatePuBuGn);

      describe("creating a sequential sqrt scale", function() {
        it("should apply the first scale correctly", function() {
          chai.assert.equal(testSeqSqrtScale1_val1, seqSqrt(1e-8));  // rgb(255, 247, 251)
          chai.assert.equal(testSeqSqrtScale1_val2, seqSqrt(1e5));   // rgb(250, 242, 248)
          chai.assert.equal(testSeqSqrtScale1_val3, seqSqrt(1e7));   // rgb(185, 198, 224)
        });
      });
    }, testSeqSqrtScale1_val1.c_str(), testSeqSqrtScale1_val2.c_str(), testSeqSqrtScale1_val3.c_str());
  }
};

struct TestSequentialSymlogScale : emp::web::BaseTest {
  // a sequential symlog scale 
  D3::SequentialSymlogScale testSeqSymlogScale1;
  std::string testSeqSymlogScale1_val1;
  std::string testSeqSymlogScale1_val2;
  std::string testSeqSymlogScale1_val3;

  TestSequentialSymlogScale() { Setup(); }

  void Setup() {
    testSeqSymlogScale1.SetConstant(5);
    testSeqSymlogScale1.SetDomain(1e-8, 1e8);
    testSeqSymlogScale1.SetInterpolator("interpolatePuBuGn");
    testSeqSymlogScale1_val1 = testSeqSymlogScale1.ApplyScale<std::string, double>(1e-7);
    testSeqSymlogScale1_val2 = testSeqSymlogScale1.ApplyScale<std::string, double>(1e5);
    testSeqSymlogScale1_val3 = testSeqSymlogScale1.ApplyScale<std::string, double>(1e7);
  }

  void Describe() override {
    EM_ASM({
      const testSeqSymlogScale1_val1 = UTF8ToString($0);
      const testSeqSymlogScale1_val2 = UTF8ToString($1);
      const testSeqSymlogScale1_val3 = UTF8ToString($2);

      var seqSymlog = d3.scaleSequentialSymlog()
                       .constant(5)
                       .domain([ 1e-8, 1e8 ])
                       .interpolator(d3.interpolatePuBuGn);

      describe("creating a sequential symlog scale", function() {
        it("should apply the first scale correctly", function() {
          chai.assert.equal(testSeqSymlogScale1_val1, seqSymlog(1e-7));  // rgb(255, 247, 251)
          chai.assert.equal(testSeqSymlogScale1_val2, seqSymlog(1e5));   // rgb(68, 152, 194)
          chai.assert.equal(testSeqSymlogScale1_val3, seqSymlog(1e7));   // rgb(1, 108, 95)
        });
      });
    }, testSeqSymlogScale1_val1.c_str(), testSeqSymlogScale1_val2.c_str(), testSeqSymlogScale1_val3.c_str());
  }
};

struct TestSequentialQuantileScale: emp::web::BaseTest {
  // a sequential quantile scale 
  D3::SequentialQuantileScale testSeqQuantScale1;
  double testSeqQuantScale1_val1;
  double testSeqQuantScale1_val2;
  double testSeqQuantScale1_val3;
  double testSeqQuantScale1_val4;

  TestSequentialQuantileScale() { Setup(); }

  void Setup() {
    emp::array<int, 3> domainArr = {100, 1, 13};
    testSeqQuantScale1.SetDomain(domainArr);
    testSeqQuantScale1_val1 = testSeqQuantScale1.ApplyScale<double, double>(1);
    testSeqQuantScale1_val2 = testSeqQuantScale1.ApplyScale<double, double>(13);
    testSeqQuantScale1_val3 = testSeqQuantScale1.ApplyScale<double, double>(99.99);
    testSeqQuantScale1_val4 = testSeqQuantScale1.ApplyScale<double, double>(100);
  }

  void Describe() override {
    EM_ASM({
      const testSeqQuantScale1_val1 = $0;
      const testSeqQuantScale1_val2 = $1;
      const testSeqQuantScale1_val3 = $2;
      const testSeqQuantScale1_val4 = $3;

      var seqQuant = d3.scaleSequentialQuantile()
                         .domain([ 100, 1, 13 ]);

      describe("creating a sequential quantile scale", function() {
        it("should apply the first scale correctly", function() {
          chai.assert.equal(testSeqQuantScale1_val1, seqQuant(1));     // 0
          chai.assert.equal(testSeqQuantScale1_val2, seqQuant(13));    // 0.5
          chai.assert.equal(testSeqQuantScale1_val3, seqQuant(99.99)); // 0.5
          chai.assert.equal(testSeqQuantScale1_val4, seqQuant(100));   // 1
        });
      });
    }, testSeqQuantScale1_val1, testSeqQuantScale1_val2, testSeqQuantScale1_val3, testSeqQuantScale1_val4);
  }
};

struct TestDivergingScale: emp::web::BaseTest {
  // a diverging scale 
  D3::DivergingScale testDivergingScale1;
  std::string testDivergingScale1_val1;
  std::string testDivergingScale1_val2;
  std::string testDivergingScale1_val3;
  emp::vector<double> testDivergingScale1_Domain;

  TestDivergingScale() { Setup(); }

  void Setup() {
    emp::array<double, 3> domainArr = {-0.78, 0, 1.35};
    testDivergingScale1.SetDomain(domainArr);
    testDivergingScale1.SetInterpolator("interpolatePuOr");
    testDivergingScale1_val1 = testDivergingScale1.ApplyScale<std::string, double>(-0.5);
    testDivergingScale1_val2 = testDivergingScale1.ApplyScale<std::string, double>(0);
    testDivergingScale1_val3 = testDivergingScale1.ApplyScale<std::string, double>(1.01);

    testDivergingScale1_Domain = testDivergingScale1.GetDomain<double>();
  }

  void Describe() override {
    EM_ASM({
      const testDivergingScale1_val1 = UTF8ToString($0);
      const testDivergingScale1_val2 = UTF8ToString($1);
      const testDivergingScale1_val3 = UTF8ToString($2);
      const testDivergingScale1_Domain_0 = $3;
      const testDivergingScale1_Domain_1 = $4;
      const testDivergingScale1_Domain_2 = $5;

      var scaleDiverging = d3.scaleDiverging()
                            .domain( [-0.78, 0, 1.35] )
                            .interpolator(d3.interpolatePuOr);

      describe("creating a diverging quantile scale", function() {
        it("should apply the first scale correctly", function() {
          chai.assert.equal(testDivergingScale1_val1, scaleDiverging(-0.5));    // rgb(119, 98, 165)
          chai.assert.equal(testDivergingScale1_val2, scaleDiverging(0));       // rgb(243, 238, 234)
          chai.assert.equal(testDivergingScale1_val3, scaleDiverging(1.01));    // rgb(190, 100, 11)
        });
        it("should get the domain correctly", function() {
          chai.assert.equal(testDivergingScale1_Domain_0, scaleDiverging.domain()[0]); // [-0.78, 0, 1.35]
          chai.assert.equal(testDivergingScale1_Domain_1, scaleDiverging.domain()[1]);
          chai.assert.equal(testDivergingScale1_Domain_2, scaleDiverging.domain()[2]);
        });
      });
    }, testDivergingScale1_val1.c_str(), testDivergingScale1_val2.c_str(), testDivergingScale1_val3.c_str(),
       testDivergingScale1_Domain[0], testDivergingScale1_Domain[1], testDivergingScale1_Domain[2]);
  }
};

struct TestDivergingLogScale: emp::web::BaseTest {
  // a diverging log scale 
  D3::DivergingLogScale testDivergingLogScale1;
  std::string testDivergingLogScale1_val1;
  std::string testDivergingLogScale1_val2;
  std::string testDivergingLogScale1_val3;

  TestDivergingLogScale() { Setup(); }

  void Setup() {
    testDivergingLogScale1.SetDomain(1e-8, 1e8);
    testDivergingLogScale1.SetInterpolator("interpolatePuOr");
    testDivergingLogScale1_val1 = testDivergingLogScale1.ApplyScale<std::string, double>(1e-5);
    testDivergingLogScale1_val2 = testDivergingLogScale1.ApplyScale<std::string, double>(0);
    testDivergingLogScale1_val3 = testDivergingLogScale1.ApplyScale<std::string, double>(1e5);
  }

  void Describe() override {
    EM_ASM({
      const testDivergingLogScale1_val1 = UTF8ToString($0);
      const testDivergingLogScale1_val2 = UTF8ToString($1);
      const testDivergingLogScale1_val3 = UTF8ToString($2);

      var scaleDivergingLog = d3.scaleDivergingLog()
                            .domain( [1e-8, 1e8] )
                            .interpolator(d3.interpolatePuOr);

      describe("creating a diverging quantile scale", function() {
        it("should apply the first scale correctly", function() {
          chai.assert.equal(testDivergingLogScale1_val1, scaleDivergingLog(1e-5));    // rgb(82, 42, 129)
          chai.assert.equal(testDivergingLogScale1_val2, scaleDivergingLog(0));       // rgb(45, 0, 75)
          chai.assert.equal(testDivergingLogScale1_val3, scaleDivergingLog(1e5));     // rgb(217, 217, 234)
        });
      });
    }, testDivergingLogScale1_val1.c_str(), testDivergingLogScale1_val2.c_str(), testDivergingLogScale1_val3.c_str());
  }
};

struct TestDivergingPowScale: emp::web::BaseTest {
  // a diverging pow scale 
  D3::DivergingPowScale testDivergingPowScale1;
  std::string testDivergingPowScale1_val1;
  std::string testDivergingPowScale1_val2;
  std::string testDivergingPowScale1_val3;

  TestDivergingPowScale() { Setup(); }

  void Setup() {
    testDivergingPowScale1.SetDomain(1e-8, 1e8);
    testDivergingPowScale1.SetInterpolator("interpolatePuOr");
    testDivergingPowScale1_val1 = testDivergingPowScale1.ApplyScale<std::string, double>(1e-5);
    testDivergingPowScale1_val2 = testDivergingPowScale1.ApplyScale<std::string, double>(1e6);
    testDivergingPowScale1_val3 = testDivergingPowScale1.ApplyScale<std::string, double>(1.9e7);
  }

  void Describe() override {
    EM_ASM({
      const testDivergingPowScale1_val1 = UTF8ToString($0);
      const testDivergingPowScale1_val2 = UTF8ToString($1);
      const testDivergingPowScale1_val3 = UTF8ToString($2);

      var scaleDivergingPow = d3.scaleDivergingPow()
                            .domain( [1e-8, 1e8] )
                            .interpolator(d3.interpolatePuOr);

      describe("creating a diverging pow scale", function() {
        it("should apply the first scale correctly", function() {
          chai.assert.equal(testDivergingPowScale1_val1, scaleDivergingPow(1e-5));    // rgb(45, 0, 75)
          chai.assert.equal(testDivergingPowScale1_val2, scaleDivergingPow(1e6));     // rgb(47, 2, 78)
          chai.assert.equal(testDivergingPowScale1_val3, scaleDivergingPow(1.9e7));    // rgb(83, 42, 129)
        });
      });
    }, testDivergingPowScale1_val1.c_str(), testDivergingPowScale1_val2.c_str(), testDivergingPowScale1_val3.c_str());
  }
};

struct TestDivergingSqrtScale: emp::web::BaseTest {
  // a diverging sqrt scale 
  D3::DivergingSqrtScale testDivergingSqrtScale1;
  std::string testDivergingSqrtScale1_val1;
  std::string testDivergingSqrtScale1_val2;
  std::string testDivergingSqrtScale1_val3;

  TestDivergingSqrtScale() { Setup(); }

  void Setup() {
    testDivergingSqrtScale1.SetDomain(1e-8, 1e8);
    testDivergingSqrtScale1.SetInterpolator("interpolatePuOr");
    testDivergingSqrtScale1_val1 = testDivergingSqrtScale1.ApplyScale<std::string, double>(1e-5);
    testDivergingSqrtScale1_val2 = testDivergingSqrtScale1.ApplyScale<std::string, double>(1e6);
    testDivergingSqrtScale1_val3 = testDivergingSqrtScale1.ApplyScale<std::string, double>(1.9e7);
  }

  void Describe() override {
    EM_ASM({
      const testDivergingSqrtScale1_val1 = UTF8ToString($0);
      const testDivergingSqrtScale1_val2 = UTF8ToString($1);
      const testDivergingSqrtScale1_val3 = UTF8ToString($2);

      var scaleDivergingSqrt = d3.scaleDivergingSqrt()
                            .domain( [1e-8, 1e8] )
                            .interpolator(d3.interpolatePuOr);

      describe("creating a diverging sqrt scale", function() {
        it("should apply the first scale correctly", function() {
          chai.assert.equal(testDivergingSqrtScale1_val1, scaleDivergingSqrt(1e-5));    // rgb(45, 0, 75)
          chai.assert.equal(testDivergingSqrtScale1_val2, scaleDivergingSqrt(1e6));     // rgb(65, 20, 105)
          chai.assert.equal(testDivergingSqrtScale1_val3, scaleDivergingSqrt(1.9e7));   // rgb(138, 123, 179)
        });
      });
    }, testDivergingSqrtScale1_val1.c_str(), testDivergingSqrtScale1_val2.c_str(), testDivergingSqrtScale1_val3.c_str());
  }
};

struct TestDivergingSymlogScale: emp::web::BaseTest {
  // a diverging symlog scale 
  D3::DivergingSymlogScale testDivergingSymlogScale1;
  std::string testDivergingSymlogScale1_val1;
  std::string testDivergingSymlogScale1_val2;
  std::string testDivergingSymlogScale1_val3;

  TestDivergingSymlogScale() { Setup(); }

  void Setup() {
    testDivergingSymlogScale1.SetDomain(1e-8, 1e8);
    testDivergingSymlogScale1.SetInterpolator("interpolatePuOr");
    testDivergingSymlogScale1_val1 = testDivergingSymlogScale1.ApplyScale<std::string, double>(1e-5);
    testDivergingSymlogScale1_val2 = testDivergingSymlogScale1.ApplyScale<std::string, double>(1e6);
    testDivergingSymlogScale1_val3 = testDivergingSymlogScale1.ApplyScale<std::string, double>(1.9e7);
  }

  void Describe() override {
    EM_ASM({
      const testDivergingSymlogScale1_val1 = UTF8ToString($0);
      const testDivergingSymlogScale1_val2 = UTF8ToString($1);
      const testDivergingSymlogScale1_val3 = UTF8ToString($2);

      var scaleDivergingSymlog = d3.scaleDivergingSymlog()
                            .domain( [1e-8, 1e8] )
                            .interpolator(d3.interpolatePuOr);

      describe("creating a diverging symlog scale", function() {
        it("should apply the first scale correctly", function() {
          chai.assert.equal(testDivergingSymlogScale1_val1, scaleDivergingSymlog(1e-5));    // rgb(45, 0, 75)
          chai.assert.equal(testDivergingSymlogScale1_val2, scaleDivergingSymlog(1e6));     // rgb(206, 205, 228)
          chai.assert.equal(testDivergingSymlogScale1_val3, scaleDivergingSymlog(1.9e7));   // rgb(232, 232, 239)
        });
      });
    }, testDivergingSymlogScale1_val1.c_str(), testDivergingSymlogScale1_val2.c_str(), testDivergingSymlogScale1_val3.c_str());
  }
};

struct TestQuantizeScale: emp::web::BaseTest {
  // a quantize scale with a string range
  D3::QuantizeScale testQuantizeScale1;
  std::string testQuantizeScale1_val1;
  std::string testQuantizeScale1_val2;
  std::string testQuantizeScale1_val3;
  emp::array<int, 2> testQuantizeScale1_invert;
  emp::vector<double> testQuantizeScale1_thresholds;

  // a quantize scale with an integer range
  D3::QuantizeScale testQuantizeScale2;
  int testQuantizeScale2_val1;
  int testQuantizeScale2_val2;
  int testQuantizeScale2_val3;
  emp::array<int, 2> testQuantizeScale2_invert;
  emp::vector<double> testQuantizeScale2_thresholds;

  TestQuantizeScale() { Setup(); }

  void Setup() {
    testQuantizeScale1.SetDomain(0, 100);
    emp::array<std::string, 4> rangeArr = {"lightblue", "orange", "lightgreen", "pink"};
    testQuantizeScale1.SetRange(rangeArr);
    testQuantizeScale1_val1 = testQuantizeScale1.ApplyScale<std::string, int>(10);
    testQuantizeScale1_val2 = testQuantizeScale1.ApplyScale<std::string, int>(30);
    testQuantizeScale1_val3 = testQuantizeScale1.ApplyScale<std::string, int>(90);
    testQuantizeScale1_invert = testQuantizeScale1.InvertExtent("orange");
    testQuantizeScale1_thresholds = testQuantizeScale1.GetThresholds();

    testQuantizeScale2.SetDomain(10, 100);
    emp::array<int, 3> rangeArr2 = {1, 2, 4};
    testQuantizeScale2.SetRange(rangeArr2);
    testQuantizeScale2_val1 = testQuantizeScale2.ApplyScale<int, int>(20);
    testQuantizeScale2_val2 = testQuantizeScale2.ApplyScale<int, int>(50);
    testQuantizeScale2_val3 = testQuantizeScale2.ApplyScale<int, int>(80);
    testQuantizeScale2_invert = testQuantizeScale2.InvertExtent(4);
    testQuantizeScale2_thresholds = testQuantizeScale2.GetThresholds();
  }

  void Describe() override {
    EM_ASM({
      const testQuantizeScale1_val1 = UTF8ToString($0);
      const testQuantizeScale1_val2 = UTF8ToString($1);
      const testQuantizeScale1_val3 = UTF8ToString($2);
      const testQuantizeScale1_invert0 = $3;
      const testQuantizeScale1_invert1 = $4;
      const testQuantizeScale1_thresholds0 = $5;
      const testQuantizeScale1_thresholds1 = $6;
      const testQuantizeScale1_thresholds2 = $7;

      const testQuantizeScale2_val1 = $8; 
      const testQuantizeScale2_val2 = $9;
      const testQuantizeScale2_val3 = $10;
      const testQuantizeScale2_invert0 = $11;
      const testQuantizeScale2_invert1 = $12;
      const testQuantizeScale2_thresholds0 = $13;
      const testQuantizeScale2_thresholds1 = $14;

      var quantizeScale = d3.scaleQuantize()
                            .domain([0, 100])
                            .range(['lightblue', 'orange', 'lightgreen', 'pink']);

      var quantizeScale2 = d3.scaleQuantize()
                              .domain([10, 100])
                              .range([1, 2, 4]);

      describe("creating a quantize scale", function() {
        it("should apply the first scale correctly", function() {
          chai.assert.equal(testQuantizeScale1_val1, quantizeScale(10));   // "lightblue"
          chai.assert.equal(testQuantizeScale1_val2, quantizeScale(30));   // "orange"
          chai.assert.equal(testQuantizeScale1_val3, quantizeScale(90));   // "pink"
        });
        it("should InvertExtent the first scale correctly", function() {
          chai.assert.equal(testQuantizeScale1_invert0, quantizeScale.invertExtent("orange")[0]);   // [25, 50]
          chai.assert.equal(testQuantizeScale1_invert1, quantizeScale.invertExtent("orange")[1]);   
        });
        it("should get the correct thresholds for the first scale", function() {
          chai.assert.equal(testQuantizeScale1_thresholds0, quantizeScale.thresholds()[0]);   // [25, 50, 75]
          chai.assert.equal(testQuantizeScale1_thresholds1, quantizeScale.thresholds()[1]); 
          chai.assert.equal(testQuantizeScale1_thresholds2, quantizeScale.thresholds()[2]); 
        });
        it("should apply the second scale correctly", function() {
          chai.assert.equal(testQuantizeScale2_val1, quantizeScale2(20));   // 1
          chai.assert.equal(testQuantizeScale2_val2, quantizeScale2(50));   // 2
          chai.assert.equal(testQuantizeScale2_val3, quantizeScale2(80));   // 4
        });
        it("should InvertExtent the second scale correctly", function() {
          chai.assert.equal(testQuantizeScale2_invert0, quantizeScale2.invertExtent(4)[0]);   // [70, 100]
          chai.assert.equal(testQuantizeScale2_invert1, quantizeScale2.invertExtent(4)[1]);   
        });
        it("should get the correct thresholds for the second scale", function() {
          chai.assert.equal(testQuantizeScale2_thresholds0, quantizeScale2.thresholds()[0]);   // [40, 70]
          chai.assert.equal(testQuantizeScale2_thresholds1, quantizeScale2.thresholds()[1]); 
        });
      });
    }, testQuantizeScale1_val1.c_str(), testQuantizeScale1_val2.c_str(), testQuantizeScale1_val3.c_str(),
       testQuantizeScale1_invert[0], testQuantizeScale1_invert[1], testQuantizeScale1_thresholds[0], testQuantizeScale1_thresholds[1], testQuantizeScale1_thresholds[2], 
       testQuantizeScale2_val1, testQuantizeScale2_val2, testQuantizeScale2_val3, 
       testQuantizeScale2_invert[0], testQuantizeScale2_invert[1], testQuantizeScale2_thresholds[0], testQuantizeScale2_thresholds[1]);
  }
};

struct TestQuantileScale: emp::web::BaseTest {
  // a quantile scale with a string range
  D3::QuantileScale testQuantileScale1;
  std::string testQuantileScale1_val1;
  std::string testQuantileScale1_val2;
  std::string testQuantileScale1_val3;
  std::string testQuantileScale1_val4;
  emp::vector<double> testQuantileScale1_quantiles;

  TestQuantileScale() { Setup(); }

  void Setup() {
    emp::array<int, 15> domainArr = {0, 5, 7, 10, 20, 30, 35, 40, 60, 62, 65, 70, 80, 90, 100};
    testQuantileScale1.SetDomain(domainArr);
    emp::array<std::string, 4> rangeArr = {"lightblue", "orange", "lightgreen", "purple"};
    testQuantileScale1.SetRange(rangeArr);
    testQuantileScale1_val1 = testQuantileScale1.ApplyScale<std::string, int>(0);
    testQuantileScale1_val2 = testQuantileScale1.ApplyScale<std::string, int>(20);
    testQuantileScale1_val3 = testQuantileScale1.ApplyScale<std::string, int>(65);
    testQuantileScale1_val4 = testQuantileScale1.ApplyScale<std::string, int>(70);
    testQuantileScale1_quantiles = testQuantileScale1.GetQuantiles();
  }

  void Describe() override {
    EM_ASM({
      const testQuantileScale1_val1 = UTF8ToString($0);
      const testQuantileScale1_val2 = UTF8ToString($1);
      const testQuantileScale1_val3 = UTF8ToString($2);
      const testQuantileScale1_val4 = UTF8ToString($3);
      const testQuantileScale1_quantiles0 = $4;
      const testQuantileScale1_quantiles1 = $5;
      const testQuantileScale1_quantiles2 = $6;

      var myData = ([0, 5, 7, 10, 20, 30, 35, 40, 60, 62, 65, 70, 80, 90, 100]);

      var quantileScale = d3.scaleQuantile()
                              .domain(myData)
                              .range([ "lightblue", "orange", "lightgreen", "purple" ]); 

      describe("creating a quantile scale", function() {
        it("should apply the first scale correctly", function() {
          chai.assert.equal(testQuantileScale1_val1, quantileScale(0));    // "lightblue"
          chai.assert.equal(testQuantileScale1_val2, quantileScale(20));   // "orange"
          chai.assert.equal(testQuantileScale1_val3, quantileScale(65));   // "lightgreen"
          chai.assert.equal(testQuantileScale1_val4, quantileScale(70));   // "purple"
        });
        it("should get the correct quantiles for the first scale", function() {
          chai.assert.equal(testQuantileScale1_quantiles0, quantileScale.quantiles()[0]);   // [15, 40, 67.5]
          chai.assert.equal(testQuantileScale1_quantiles1, quantileScale.quantiles()[1]); 
          chai.assert.equal(testQuantileScale1_quantiles2, quantileScale.quantiles()[2]); 
        });
      });
    }, testQuantileScale1_val1.c_str(),  testQuantileScale1_val2.c_str(),  testQuantileScale1_val3.c_str(),  testQuantileScale1_val4.c_str(), 
       testQuantileScale1_quantiles[0], testQuantileScale1_quantiles[1], testQuantileScale1_quantiles[2]);
  }
}; 

struct TestThresholdScale: emp::web::BaseTest {
  // a threshold scale with a string range
  D3::ThresholdScale testThresholdScale1;
  std::string testThresholdScale1_val1;
  std::string testThresholdScale1_val2;
  std::string testThresholdScale1_val3;
  std::string testThresholdScale1_val4;

  TestThresholdScale() { Setup(); }

  void Setup() {
    emp::array<int, 3> domainArr = {0, 50, 100};
    testThresholdScale1.SetDomain(domainArr);
    emp::array<std::string, 4> rangeArr = {"#ccc", "lightblue", "orange", "#ccc"};
    testThresholdScale1.SetRange(rangeArr);
    testThresholdScale1_val1 = testThresholdScale1.ApplyScale<std::string, int>(-10);
    testThresholdScale1_val2 = testThresholdScale1.ApplyScale<std::string, int>(20);
    testThresholdScale1_val3 = testThresholdScale1.ApplyScale<std::string, int>(70);
    testThresholdScale1_val4 = testThresholdScale1.ApplyScale<std::string, int>(110);
  }

  void Describe() override {
    EM_ASM({
      const testThresholdScale1_val1 = UTF8ToString($0);
      const testThresholdScale1_val2 = UTF8ToString($1);
      const testThresholdScale1_val3 = UTF8ToString($2);
      const testThresholdScale1_val4 = UTF8ToString($3);

      var thresholdScale = d3.scaleThreshold()
                              .domain([0, 50, 100])
                              .range(['#ccc', 'lightblue', 'orange', '#ccc']);

      describe("creating a threshold scale", function() {
        it("should apply the first scale correctly", function() {
          chai.assert.equal(testThresholdScale1_val1, thresholdScale(-10));   // "#ccc"
          chai.assert.equal(testThresholdScale1_val2, thresholdScale(20));    // "lightblue"
          chai.assert.equal(testThresholdScale1_val3, thresholdScale(70));    // "orange"
          chai.assert.equal(testThresholdScale1_val4, thresholdScale(110));   // "#ccc"
        });
      });
    }, testThresholdScale1_val1.c_str(), testThresholdScale1_val2.c_str(), testThresholdScale1_val3.c_str(), testThresholdScale1_val4.c_str());
  }
};  

struct TestOrdinalScale: emp::web::BaseTest {
  // an ordinal scale with a string range
  D3::OrdinalScale testOrdinalScale1;
  std::string testOrdinalScale1_val1;
  std::string testOrdinalScale1_val2;
  std::string testOrdinalScale1_val3;
  std::string testOrdinalScale1_val4;

  TestOrdinalScale() { Setup(); }

  void Setup() {
    emp::array<std::string, 12> domainArr = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
    testOrdinalScale1.SetDomain(domainArr);
    emp::array<std::string, 3> rangeArr = {"black", "#ccc", "#ccc"};
    testOrdinalScale1.SetRange(rangeArr);
    testOrdinalScale1_val1 = testOrdinalScale1.ApplyScale<std::string, int>(-10);
    testOrdinalScale1_val2 = testOrdinalScale1.ApplyScale<std::string, int>(20);
    testOrdinalScale1_val3 = testOrdinalScale1.ApplyScale<std::string, int>(70);
    testOrdinalScale1_val4 = testOrdinalScale1.ApplyScale<std::string, int>(110);
  }

  void Describe() override {
    EM_ASM({
      const testOrdinalScale1_val1 = UTF8ToString($0);
      const testOrdinalScale1_val2 = UTF8ToString($1);
      const testOrdinalScale1_val3 = UTF8ToString($2);
      const testOrdinalScale1_val4 = UTF8ToString($3);
      var myData = (['Jan', 'Feb', 'Mar', 'Apr', 'May', 'Jun', 'Jul', 'Aug', 'Sep', 'Oct', 'Nov', 'Dec']);

      var ordinalScale = d3.scaleOrdinal()
                            .domain(myData)
                            .range(['black', '#ccc', '#ccc']);

      describe("creating an ordinal scale", function() {
        it("should apply the first scale correctly", function() {
          chai.assert.equal(testOrdinalScale1_val1, ordinalScale("Jan"));    // "black"
          chai.assert.equal(testOrdinalScale1_val2, ordinalScale("Feb"));    // "#ccc"
          chai.assert.equal(testOrdinalScale1_val3, ordinalScale("Mar"));    // "#ccc"
          chai.assert.equal(testOrdinalScale1_val4, ordinalScale("Apr"));    // "black"
        });
      });
    }, testOrdinalScale1_val1.c_str(), testOrdinalScale1_val2.c_str(), testOrdinalScale1_val3.c_str(), testOrdinalScale1_val4.c_str());
  }
};  

struct TestBandScale: emp::web::BaseTest {
  // a band scale
  D3::BandScale testBandScale1;
  int testBandScale1_val1;
  int testBandScale1_val2;
  int testBandScale1_val3;
  double testBandScale1_bandwidth;
  double testBandScale1_step;

  TestBandScale() { Setup(); }

  void Setup() {
    emp::array<std::string, 5> domainArr = {"Mon", "Tue", "Wed", "Thu", "Fri"};
    testBandScale1.SetDomain(domainArr);
    testBandScale1.SetRange(0, 200);
    testBandScale1_val1 = testBandScale1.ApplyScale<int, std::string>("Mon");
    testBandScale1_val2 = testBandScale1.ApplyScale<int, std::string>("Tue");
    testBandScale1_val3 = testBandScale1.ApplyScale<int, std::string>("Fri");
    testBandScale1_bandwidth = testBandScale1.GetBandwidth();
    testBandScale1_step = testBandScale1.GetStep();
  } 
 
  void Describe() override {
    EM_ASM({
      const testBandScale1_val1 = $0;
      const testBandScale1_val2 = $1;
      const testBandScale1_val3 = $2;
      const testBandScale1_bandwidth = $3;
      const testBandScale1_step = $4;

      var bandScale = d3.scaleBand()
                        .domain(['Mon', 'Tue', 'Wed', 'Thu', 'Fri'])
                        .range([0, 200]);

      describe("creating a band scale", function() {
        it("should apply the first scale correctly", function() {
          chai.assert.equal(testBandScale1_val1, bandScale("Mon"));    // 0
          chai.assert.equal(testBandScale1_val2, bandScale("Tue"));    // 40
          chai.assert.equal(testBandScale1_val3, bandScale("Fri"));    // 160
        });
        it("should get the bandwidth correctly", function() {
          chai.assert.equal(testBandScale1_bandwidth, bandScale.bandwidth());  // 40
        });
        it("should get the step correctly", function() {
          chai.assert.equal(testBandScale1_step, bandScale.step());    // 40
        });
      });
    }, testBandScale1_val1, testBandScale1_val2, testBandScale1_val3, 
       testBandScale1_bandwidth, testBandScale1_step);
  }
};  

struct TestPointScale: emp::web::BaseTest {
  // a point scale
  D3::PointScale testPointScale1;
  int testPointScale1_val1;
  int testPointScale1_val2;
  int testPointScale1_val3;
  double testPointScale1_bandwidth;
  double testPointScale1_step;

  TestPointScale() { Setup(); }

  void Setup() {
    emp::array<std::string, 5> domainArr = {"Mon", "Tue", "Wed", "Thu", "Fri"};
    testPointScale1.SetDomain(domainArr);
    testPointScale1.SetRange(0, 500);
    testPointScale1_val1 = testPointScale1.ApplyScale<int, std::string>("Mon");
    testPointScale1_val2 = testPointScale1.ApplyScale<int, std::string>("Tue");
    testPointScale1_val3 = testPointScale1.ApplyScale<int, std::string>("Fri");
    testPointScale1_bandwidth = testPointScale1.GetBandwidth();
    testPointScale1_step = testPointScale1.GetStep();
  } 
 
  void Describe() override {
    EM_ASM({
      const testPointScale1_val1 = $0;
      const testPointScale1_val2 = $1;
      const testPointScale1_val3 = $2;
      const testPointScale1_bandwidth = $3;
      const testPointScale1_step = $4;

      var pointScale = d3.scalePoint()
                          .domain(['Mon', 'Tue', 'Wed', 'Thu', 'Fri'])
                          .range([0, 500]);  

      describe("creating a point scale", function() {
        it("should apply the first scale correctly", function() {
          chai.assert.equal(testPointScale1_val1, pointScale("Mon"));    // 0
          chai.assert.equal(testPointScale1_val2, pointScale("Tue"));    // 125
          chai.assert.equal(testPointScale1_val3, pointScale("Fri"));    // 500
        });
        it("should get the bandwidth correctly", function() {
          chai.assert.equal(testPointScale1_bandwidth, pointScale.bandwidth());  // 0
        });
        it("should get the step correctly", function() {
          chai.assert.equal(testPointScale1_step, pointScale.step());    // 125
        });
      });
    }, testPointScale1_val1, testPointScale1_val2, testPointScale1_val3, 
       testPointScale1_bandwidth, testPointScale1_step);
  }
};  

emp::web::MochaTestRunner test_runner;

int main() { 
  // emp::Initialize();
  // D3::internal::get_emp_d3();

  test_runner.AddTest<TestLinearScale>("LinearScale");
  test_runner.AddTest<TestPowScale>("PowScale");
  test_runner.AddTest<TestSqrtScale>("SqrtScale");
  test_runner.AddTest<TestLogScale>("LogScale");
  test_runner.AddTest<TestSymlogScale>("SymlogScale");
  test_runner.AddTest<TestIdentityScale>("IdentityScale");
  test_runner.AddTest<TestTimeScale>("TimeScale");
  test_runner.AddTest<TestSequentialScale>("SequentialScale");
  test_runner.AddTest<TestSequentialLogScale>("SequentialLogScale");
  test_runner.AddTest<TestSequentialPowScale>("SequentialPowScale");
  test_runner.AddTest<TestSequentialSqrtScale>("SequentialSqrtScale");
  test_runner.AddTest<TestSequentialSymlogScale>("SequentialSymlogScale");
  test_runner.AddTest<TestSequentialQuantileScale>("SequentialQuantileScale");
  test_runner.AddTest<TestDivergingScale>("DivergingScale");
  test_runner.AddTest<TestDivergingLogScale>("DivergingLogScale");
  test_runner.AddTest<TestDivergingPowScale>("DivergingPowScale");
  test_runner.AddTest<TestDivergingSqrtScale>("DivergingSqrtScale");
  test_runner.AddTest<TestDivergingSymlogScale>("DivergingSymlogScale");
  test_runner.AddTest<TestQuantizeScale>("QuantizeScale");
  test_runner.AddTest<TestQuantileScale>("QuantileScale");
  test_runner.AddTest<TestThresholdScale>("ThresholdScale");
  test_runner.AddTest<TestOrdinalScale>("OrdinalScale");
  test_runner.AddTest<TestBandScale>("BandScale");
  test_runner.AddTest<TestPointScale>("PointScale");

  test_runner.OnBeforeEachTest([]() {
    ResetD3Context();
  });
  
  test_runner.Run();
}