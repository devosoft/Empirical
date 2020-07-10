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
    testSqrt1_val1 = testSqrt1.ApplyScale<double>(1.386e9);
    testSqrt1_val2 = testSqrt1.ApplyScale<double>(127e6);
    testSqrt1_val3 = testSqrt1.ApplyScale<double>(427e3);

    D3::SqrtScale testSqrtCopy(testSqrt1);
    testSqrtCopy_val1 = testSqrtCopy.ApplyScale<double>(1.386e9);
    testSqrtCopy_val2 = testSqrtCopy.ApplyScale<double>(127e6);
    testSqrtCopy_val3 = testSqrtCopy.ApplyScale<double>(427e3);
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
    testLog1_val1 = testLog1.ApplyScale<double>(1000);
    testLog1_val2 = testLog1.ApplyScale<double>(1234);
    testLog1_val3 = testLog1.ApplyScale<double>(100000);

    testLog2.SetBase(2);
    testLog2.SetDomain(16, 1048576);
    testLog2.SetRange(0, 700);
    testLog2_val1 = testLog2.ApplyScale<double>(64);
    testLog2_val2 = testLog2.ApplyScale<double>(1234);
    testLog2_val3 = testLog2.ApplyScale<double>(1048576);
    testLog2_Unclamped = testLog2.ApplyScale<double>(5000000);
    
    // now clamp TestScale2 to test the SetClamp method
    testLog2.SetClamp(true);
    testLog2_Clamped = testLog2.ApplyScale<double>(5000000);
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
    testSymlog1_val1 = testSymlog1.ApplyScale<double>(-80000);
    testSymlog1_val2 = testSymlog1.ApplyScale<double>(-50);
    testSymlog1_val3 = testSymlog1.ApplyScale<double>(1.5); 
    testSymlog1_val4 = testSymlog1.ApplyScale<double>(50);
    testSymlog1_val5 = testSymlog1.ApplyScale<double>(80000);

    testSymlog2.SetDomain(-100000, 100000);
    testSymlog2.SetRange(-100, 100);
    testSymlog2.SetConstant(0.01);
    testSymlog2_val1 = testSymlog2.ApplyScale<double>(-80000);
    testSymlog2_val2 = testSymlog2.ApplyScale<double>(-50);
    testSymlog2_val3 = testSymlog2.ApplyScale<double>(1.5); 
    testSymlog2_val4 = testSymlog2.ApplyScale<double>(50);
    testSymlog2_val5 = testSymlog2.ApplyScale<double>(80000);
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
    testIdentity1_val1 = testIdentity1.ApplyScale<double>(12);
    testIdentity1_val2 = testIdentity1.ApplyScale<double>(50.6789);
    testIdentity1_val3 = testIdentity1.ApplyScale<double>(1234);
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

    testTime1_val1 = testTime1.ApplyScale<double>(date1);
    testTime1_val2 = testTime1.ApplyScale<int>(date2);
    testTime1_val1_i = testTime1.Invert(200);
    testTime1_val2_i = testTime1.Invert(640);

    testTime2.SetDomain(dateMin, dateMax);
    testTime2.SetRange("red", "yellow");
    testTime2_val1 = testTime2.ApplyScale<std::string>(date1);
    testTime2_val2 = testTime2.ApplyScale<std::string>(date2);
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

emp::web::MochaTestRunner test_runner;

int main() {
  emp::Initialize();
  D3::internal::get_emp_d3();

  test_runner.AddTest<TestLinearScale>("LinearScale");
  test_runner.AddTest<TestPowScale>("PowScale");
  test_runner.AddTest<TestSqrtScale>("SqrtScale");
  test_runner.AddTest<TestLogScale>("LogScale");
  test_runner.AddTest<TestSymlogScale>("SymlogScale");
  test_runner.AddTest<TestIdentityScale>("IdentityScale");
  test_runner.AddTest<TestTimeScale>("TimeScale");
  
  test_runner.OnBeforeEachTest([]() {
    ResetD3Context();
  });
  
  test_runner.Run();
}