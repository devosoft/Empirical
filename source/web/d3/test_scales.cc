#include <iostream>
#include <vector>
#include <string>

#include "web/init.h"
#include "web/Document.h"
#include "d3_init.h"
#include "scales.h"
#include "utils.h"
#include "../js_utils.h"

#include "../../base/vector.h"

namespace UI = emp::web;
UI::Document doc("emp_d3_test");

//****************************************//
//******** Pass map to javascript ********//
//****************************************//

/// This function can be called to pass a map or two arrays (one holding keys
/// and the other hodling values) into JavaScript.
/// The resulting JavaScript object will be stored in emp.__incoming_map. 

/// @cond TEMPLATES

// For two arrays holding keys and values 
// (note that the keys vector can not hold objects or functions)
template <typename KEY_T, typename VAL_T, size_t SIZE>
void pass_map_to_javascript(emp::array<KEY_T, SIZE> & keys, emp::array<VAL_T, SIZE> & values) {
  emp::pass_array_to_javascript(keys);
  EM_ASM({
    emp_i.__incoming_map_keys = emp_i.__incoming_array;
  });

  // check to make sure each key is not an object or a function
  emp_assert(
      EM_ASM_INT({
        emp_i.__incoming_map_keys.forEach(function(key) {
          if (typeof key === "object" || typeof key === "function") { return 0; }
        });
        return 1;
      }), "Keys cannot be an object or a function");

  emp::pass_array_to_javascript(values);
  EM_ASM({
    emp_i.__incoming_map_values = emp_i.__incoming_array;

    // create dictionary
    emp_i.__incoming_map = ( {} );
    emp_i.__incoming_map_keys.forEach(function(key, val) {
      emp_i.__incoming_map[key] = emp_i.__incoming_map_values[val]
    });

    delete emp_i.__incoming_map_keys;
    delete emp_i.__incoming_map_values;
  });
}

// For passing a map directly 
template <typename KEY_T, typename VAL_T>
void pass_map_to_javascript(std::map<KEY_T, VAL_T> & dict) {
  // extract keys and values from dict
  emp::vector<KEY_T> keys;
  emp::vector<VAL_T> values;

  for (typename std::map<KEY_T, VAL_T>::iterator it = dict.begin(); it != dict.end(); ++it) {
    keys.push_back(it->first);
    values.push_back(it->second);
  }

  emp::pass_array_to_javascript(keys);
  EM_ASM({
    emp_i.__incoming_map_keys = emp_i.__incoming_array;
  });

  // check to make sure each key is not an object or a function
  emp_assert(
      EM_ASM_INT({
        emp_i.__incoming_map_keys.forEach(function(key) {
          if (typeof key === "object" || typeof key === "function") { return 0; }
        });
        return 1;
      }), "Keys cannot be an object or a function");

  emp::pass_array_to_javascript(values);
  EM_ASM({
    emp_i.__incoming_map_values = emp_i.__incoming_array;

    // create dictionary
    emp_i.__incoming_map = ( {} );
    emp_i.__incoming_map_keys.forEach(function(key, val) {
      emp_i.__incoming_map[key] = emp_i.__incoming_map_values[val]
    });

    delete emp_i.__incoming_map_keys;
    delete emp_i.__incoming_map_values;
  });
}

/// @endcond 

//***********************************//
//******** Continuous scales ********//
//***********************************//

// scaleLinear
struct TestLinearScale {
  TestLinearScale() {
    std::cout << "------Linear Test Begin------" << std::endl;

    D3::LinearScale testLinearX;
    testLinearX.SetDomain(10, 130);
    testLinearX.SetRange(0, 960);
    int result1 = testLinearX.ApplyScale<int>(20);
    int result2 = testLinearX.ApplyScale<int>(50);
    int result1i = testLinearX.Invert<int>(80);
    int result2i = testLinearX.Invert<int>(320);

    D3::LinearScale testLinearColor;
    testLinearColor.SetDomain(10, 100);
    emp::array<std::string, 2> colorArray = {"brown", "steelblue"};
    testLinearColor.SetRange(colorArray);
    std::string result3 = testLinearColor.ApplyScale<std::string>(20);
    std::string result4 = testLinearColor.ApplyScale<std::string>(50);

    std::map<std::string, std::string> testMap = {{"applyScale1", std::to_string(result1)}, {"applyScale2", std::to_string(result2)}, \
                                                  {"invert1", std::to_string(result1i)}, {"invert2", std::to_string(result2i)}, {"color1", result3}, {"color2", result4}};
    pass_map_to_javascript(testMap);

    // add results to JS
    EM_ASM({
      var x = d3.scaleLinear()
                  .domain([ 10, 130 ])
                  .range([ 0, 960 ]);

      var color = d3.scaleLinear()
                      .domain([ 10, 100 ])
                      .range([ "brown", "steelblue" ]);

      console.log(emp_i.__incoming_map);
      console.log([ x(20).toString(), x(50), x.invert(80), x.invert(320), color(20), color(50) ]);
      console.log(emp_i.__incoming_map["applyScale1"] === x(20).toString());
      console.log(emp_i.__incoming_map["color1"] === color(20));
    });

    std::cout << "------Linear Test End------" << std::endl << std::endl;
  }
};

// scalePow
struct TestPowScale {
  TestPowScale() {
    std::cout << "------Pow Test Begin------" << std::endl;

    EM_ASM({
      var population = d3.scalePow()
        .exponent(0.5)
        .domain([0, 2e9])
        .range([0, 300]);

      console.log(population.domain()); // [0, 2000000]
      console.log(population.range()); // [0, 300]
      console.log(population(1.386e9)); // 249.73986465920893
      console.log(population(127e6)); // 75.59761901012492
      console.log(population(427e3)); // 4.383491758860737

      var population2 = d3.scalePow()
        .exponent(1.5)
        .domain([0, 2e9])
        .range([0, 300]);

      console.log(population2(1.386e9)); // 173.06972620883178
      console.log(population2(127e6)); // 4.800448807142932
      console.log(population2(427e3)); // 0.0009358754905167673

      var populationColor = d3.scalePow()
        .domain([0, 2e9])
        .range(["yellow", "red"]);

      console.log(populationColor.domain()); // [0, 2000000]
      console.log(populationColor.range()); // ["yellow", "red"]
      console.log(populationColor(1.386e9)); // 249.73986465920893
      console.log(populationColor(127e6)); // 75.59761901012492
      console.log(populationColor(427e3)); // 4.383491758860737
    });

    D3::PowScale testPowPop;
    testPowPop.SetExponent(0.5);
    testPowPop.SetDomain(0.0, 2e9);
    testPowPop.SetRange(0, 300);

    emp::vector<double> result1 = testPowPop.GetDomain<double>();
    emp::vector<double> result2 = testPowPop.GetRange<double>();
    double result3 = testPowPop.ApplyScale<double>(1.386e9);
    double result4 = testPowPop.ApplyScale<double>(127e6);
    double result5 = testPowPop.ApplyScale<double>(427e3);

    std::cout << "value 1: " << result1 << std::endl;
    std::cout << "value 2: " << result2 << std::endl;
    std::cout << "value 3: " << result3 << std::endl;
    std::cout << "value 4: " << result4 << std::endl;
    std::cout << "value 5: " << result5 << std::endl;

    D3::PowScale testPowPop2;
    testPowPop2.SetExponent(1.5);
    testPowPop2.SetDomain(0.0, 2e9);
    testPowPop2.SetRange(0, 300);
    double result6 = testPowPop2.ApplyScale<double>(1.386e9);
    double result7 = testPowPop2.ApplyScale<double>(127e6);
    double result8 = testPowPop2.ApplyScale<double>(427e3);

    std::cout << "value 6: " << result6 << std::endl;
    std::cout << "value 7: " << result7 << std::endl;
    std::cout << "value 8: " << result8 << std::endl;

    emp::array<std::string, 6> keys = {"test3", "test4", "test5", "test6", "test7", "test8"};

    emp::array<std::string, 6> values = {std::to_string(result3),
                                     std::to_string(result4), std::to_string(result5), std::to_string(result6),
                                     std::to_string(result7), std::to_string(result8)};

    pass_map_to_javascript(keys, values);
    EM_ASM({
      console.log(emp_i.__incoming_map);
    });

    D3::PowScale testPowPopColor;
    emp::array<std::string, 2> colorArray = {"yellow", "red"};
    testPowPopColor.SetDomain(0.0, 2e9);
    testPowPopColor.SetRange(colorArray);
    // emp::vector<double> result9 = testPowPopColor.GetDomainDouble();
    // emp::vector<std::string> result10 = testPowPopColor.GetRangeString();
    emp::vector<double> result9 = testPowPopColor.GetDomain<double>();
    emp::vector<std::string> result10 = testPowPopColor.GetRange<std::string>();
    std::string result11 = testPowPopColor.ApplyScale<std::string>(1.386e9);
    std::string result12 = testPowPopColor.ApplyScale<std::string>(127e6);
    std::string result13 = testPowPopColor.ApplyScale<std::string>(427e3);

    std::cout << "value 9: " << result9 << std::endl;
    std::cout << "value 10: " << result10 << std::endl;
    std::cout << "value 11: " << result11 << std::endl;
    std::cout << "value 12: " << result12 << std::endl;
    std::cout << "value 13: " << result13 << std::endl;

    std::cout << "------Pow Test End------" << std::endl << std::endl;
  }
};

// scaleSqrt (and copy)
struct TestSqrtScale {
  TestSqrtScale() {
    std::cout << "------Sqrt Test Begin------" << std::endl;

    EM_ASM({
      var population = d3.scaleSqrt()
        .domain([0, 2e9])
        .range([0, 300]);

      console.log(population(1.386e9)); // 249.73986465920893
      console.log(population(127e6)); // 75.59761901012492
      console.log(population(427e3)); // 4.383491758860737

      var copyPop = population.copy();
      console.log(copyPop(1.386e9)); // 249.73986465920893
      console.log(copyPop(127e6)); // 75.59761901012492
      console.log(copyPop(427e3)); // 4.383491758860737
    });

    D3::SqrtScale testSqrtPop;
    testSqrtPop.SetDomain(0.0, 2e9);
    testSqrtPop.SetRange(0, 300);
    double result1 = testSqrtPop.ApplyScale<double>(1.386e9);
    double result2 = testSqrtPop.ApplyScale<double>(127e6);
    double result3 = testSqrtPop.ApplyScale<double>(427e3);
    std::cout << "value 1: " << result1 << std::endl;
    std::cout << "value 2: " << result2 << std::endl;
    std::cout << "value 3: " << result3 << std::endl;

    // D3::SqrtScale copyPop = testSqrtPop.Copy();
    D3::SqrtScale copyPop(testSqrtPop);  // = new D3::SqrtScale(testSqrtPop);
    double result4 = copyPop.ApplyScale<double>(1.386e9);
    double result5 = copyPop.ApplyScale<double>(127e6);
    double result6 = copyPop.ApplyScale<double>(427e3);
    std::cout << "value 4: " << result4 << std::endl;
    std::cout << "value 5: " << result5 << std::endl;
    std::cout << "value 6: " << result6 << std::endl;

    std::cout << "------Sqrt Test End------" << std::endl << std::endl;
  }
};

// scaleLog
struct TestLogScale {
  TestLogScale() {
    std::cout << "------Log Test Begin------" << std::endl;

    EM_ASM({
      var logScale = d3.scaleLog()
	      .domain([10, 100000])
	      .range([0, 700]);

      console.log(logScale(1000)); // 349.99999999999994
      console.log(logScale(1234)); // 365.980152947014
      console.log(logScale(100000)); // 700

      var logScale2 = d3.scaleLog()
        .base(2)
	      .domain([16, 1048576])
	      .range([0, 700]);

      console.log(logScale2(64)); // 87.49999999999999
      console.log(logScale2(1234)); // 274.2742922127871
      console.log(logScale2(1048576)); // 700
    });

    D3::LogScale testLog;
    testLog.SetDomain(10, 100000);
    testLog.SetRange(0, 700);
    double result1 = testLog.ApplyScale<double>(1000);
    double result2 = testLog.ApplyScale<double>(1234);
    double result3 = testLog.ApplyScale<double>(100000);

    std::cout << "value 1: " << result1 << std::endl;
    std::cout << "value 2: " << result2 << std::endl;
    std::cout << "value 3: " << result3 << std::endl;

    D3::LogScale testLog2;
    testLog2.SetBase(2);
    testLog2.SetDomain(16, 1048576);
    testLog2.SetRange(0, 700);
    double result4 = testLog2.ApplyScale<double>(64);
    double result5 = testLog2.ApplyScale<double>(1234);
    double result6 = testLog2.ApplyScale<double>(1048576);

    std::cout << "value 4: " << result4 << std::endl;
    std::cout << "value 5: " << result5 << std::endl;
    std::cout << "value 6: " << result6 << std::endl;

    std::cout << "------Log Test End------" << std::endl << std::endl;
  }
};

// scaleSymlog
struct TestSymlogScale {
  TestSymlogScale() {
    std::cout << "------Symlog Test Begin------" << std::endl;

    EM_ASM({
      var logScale = d3.scaleSymlog()
	      .domain([-100000, 100000])
        .constant(0.01)
	      .range([-100, 100]);

      console.log(logScale(-80000)); // -98.61557140643649
      console.log(logScale(-50)); // -52.843669022827925
      console.log(logScale(1.5)); // 31.128241911062098
      console.log(logScale(50)); // 52.843669022827925
      console.log(logScale(80000)); // 98.61557140643649
    });

    D3::SymlogScale testSymlog;
    testSymlog.SetDomain(-100000, 100000);
    testSymlog.SetConstant(0.01);
    testSymlog.SetRange(-100, 100);
    double result1 = testSymlog.ApplyScale<double>(-80000);
    double result2 = testSymlog.ApplyScale<double>(-50);
    double result3 = testSymlog.ApplyScale<double>(1.5);
    double result4 = testSymlog.ApplyScale<double>(50);
    double result5 = testSymlog.ApplyScale<double>(80000);


    std::cout << "value 1: " << result1 << std::endl;
    std::cout << "value 2: " << result2 << std::endl;
    std::cout << "value 3: " << result3 << std::endl;
    std::cout << "value 4: " << result4 << std::endl;
    std::cout << "value 5: " << result5 << std::endl;


    std::cout << "------Symlog Test End------" << std::endl << std::endl;
  }
};

// scaleIdentity
struct TestIdentityScale {
  TestIdentityScale() {
    std::cout << "------Identity Test Begin------" << std::endl;

    EM_ASM({
      var identityScale = d3.scaleIdentity()
        .domain([ 12, 1234 ]);

      console.log(identityScale(12)); // 12
      console.log(identityScale(50.6789)); // 50.6789
      console.log(identityScale(1234)); // 1234
    });

    D3::IdentityScale testId;
    testId.SetDomain(12, 1234);
    double result1 = testId.ApplyScale<double>(12);
    double result2 = testId.ApplyScale<double>(50.6789);
    double result3 = testId.ApplyScale<double>(1234);

    std::cout << "value 1: " << result1 << std::endl;
    std::cout << "value 2: " << result2 << std::endl;
    std::cout << "value 3: " << result3 << std::endl;

    std::cout << "------Identity Test End------" << std::endl << std::endl;
  }
};

// scaleTime
struct TestTimeScale {
  TestTimeScale() {
    std::cout << "------Time Test Begin------" << std::endl;

    EM_ASM({
      var timeScale = d3.scaleTime()
        .domain([new Date(2000, 0, 1), new Date(2000, 0, 2)])
        .range([0, 960]);

      console.log(timeScale(new Date(2000, 0, 1, 5))); // 200
      console.log(timeScale(new Date(2000, 0, 1, 16))); // 640
      console.log(timeScale.invert(200)); // Sat Jan 01 2000 05:00:00 GMT-0800 (PST)
      console.log(timeScale.invert(640)); // Sat Jan 01 2000 16:00:00 GMT-0800 (PST)

      var timeColor = d3.scaleTime()
                    .domain([new Date(2000, 0, 1), new Date(2000, 0, 2)])
                    .range(["red", "yellow"]);

      console.log(timeColor(new Date(2000, 0, 1, 5))); // rgb(255, 53, 0)
      console.log(timeColor(new Date(2000, 0, 1, 16))); // rgb(255, 170, 0)
    });

    D3::TimeScale testTime;
    D3::Date dateMin(2000, 0, 1);
    D3::Date dateMax(2000, 0, 2);
    testTime.SetDomain(dateMin, dateMax);
    testTime.SetRange(0, 960);
    D3::Date date1(2000, 0, 1, 5);
    D3::Date date2(2000, 0, 1, 16);
    double result1 = testTime.ApplyScale<double>(date1);
    int result2 = testTime.ApplyScale<int>(date2);
    D3::Date result1i = testTime.Invert(200);
    D3::Date result2i = testTime.Invert(640);

    D3::TimeScale testColorTime;
    testColorTime.SetDomain(dateMin, dateMax);
    emp::array<std::string, 2> colorArray = {"red", "yellow"};
    testColorTime.SetRange(colorArray);
    // std::string result3 = testColorTime.ApplyScaleString(date1);
    // std::string result4 = testColorTime.ApplyScaleString(date2);  
    std::string result3 = testColorTime.ApplyScale<std::string>(date1);
    std::string result4 = testColorTime.ApplyScale<std::string>(date2);  

    std::cout << "value 1: " << result1 << std::endl;
    std::cout << "value 2: " << result2 << std::endl;
    std::cout << "value 1 invert: " << result1i.ToString() << std::endl;
    std::cout << "value 2 invert: " << result2i.ToString() << std::endl;
    std::cout << "value 3: " << result3 << std::endl;
    std::cout << "value 4: " << result4 << std::endl;

    std::cout << "------Time Test End------" << std::endl << std::endl;
  }
};

// scaleSequential
struct TestSequentialScale {
  TestSequentialScale() {
    std::cout << "------Sequential Test Begin------" << std::endl;

    EM_ASM({
      var sequentialScale = d3.scaleSequential()
        .domain([0, 100])
        .interpolator(d3.interpolateRainbow);

      console.log(sequentialScale(0));   // returns 'rgb(110, 64, 170)'
      console.log(sequentialScale(50));  // returns 'rgb(175, 240, 91)'
      console.log(sequentialScale(100)); // returns 'rgb(110, 64, 170)'
    });

    D3::SequentialScale testSeq;
    testSeq.SetDomain(0, 100);
    testSeq.SetInterpolator("interpolateRainbow");
    std::string result1 = testSeq.ApplyScale<std::string>(0);
    std::string result2 = testSeq.ApplyScale<std::string>(50);
    std::string result3 = testSeq.ApplyScale<std::string>(100);

    std::cout << "value 1: " << result1 << std::endl;
    std::cout << "value 2: " << result2 << std::endl;
    std::cout << "value 3: " << result3 << std::endl;

    std::cout << "------Sequential Test End------" << std::endl << std::endl;
  }
};

// scaleSequentialQuantile
struct TestSequentialQuantileScale {
  TestSequentialQuantileScale() {
    std::cout << "------SequentialQuantile Test Begin------" << std::endl;

    EM_ASM({
      var myData = ( [0, 5, 7, 10, 20, 30, 35, 40, 60, 62, 65, 70, 80, 90, 100] );

      var seq = d3.scaleSequentialQuantile()
                  .domain(myData)
                  .interpolator(d3.interpolateRdYlBu);

      console.log(seq(0));  // returns rgb(165, 0, 38)
      console.log(seq(50));  // returns rgb(250, 248, 193)
      console.log(seq(60));  // returns rgb(231, 245, 227)
      console.log(seq(100)); // returns rgb(49, 54, 149)
      // This functionality is included in the newest version of d3-scale, but not base d3
      // console.log(seq.quantiles(4)); // should return [0, 15, 40, 67.5, 100]
    });

    emp::array<int, 15> myData = {0, 5, 7, 10, 20, 30, 35, 40, 60, 62, 65, 70, 80, 90, 100};
    D3::SequentialQuantileScale testSeqQuant;
    testSeqQuant.SetDomain(myData);
    testSeqQuant.SetInterpolator("interpolateRdYlBu");
    std::string result1 = testSeqQuant.ApplyScale<std::string>(0);
    std::string result2 = testSeqQuant.ApplyScale<std::string>(50);
    std::string result3 = testSeqQuant.ApplyScale<std::string>(60);
    std::string result4 = testSeqQuant.ApplyScale<std::string>(100);
    // This functionality is included in the newest version of d3-scale, but not base d3
    // emp::vector<double> result5 = testSeq.GetQuantiles(4);

    std::cout << "value 1: " << result1 << std::endl;
    std::cout << "value 2: " << result2 << std::endl;
    std::cout << "value 3: " << result3 << std::endl;
    std::cout << "value 4: " << result4 << std::endl;
    // std::cout << "value 5: " << result5 << std::endl;

    std::cout << "------SequentialQuantile Test End------" << std::endl << std::endl;
  }
};

// scaleDiverging
struct TestDivergingScale {
  TestDivergingScale() {
    std::cout << "------Diverging Test Begin------" << std::endl;

    EM_ASM({
      scaleAnomalyPuOr = d3.scaleDiverging()
                            .domain( [-0.78, 0, 1.35] )
                            .interpolator(d3.interpolatePuOr);

      console.log(scaleAnomalyPuOr.domain()); // [-0.78, 0, 1.35]
      console.log(scaleAnomalyPuOr(-0.5)); // "rgb(119, 98, 165)"
      console.log(scaleAnomalyPuOr(0)); // "rgb(243, 238, 234)"
      console.log(scaleAnomalyPuOr(1.01)); //"rgb(190, 100, 11)"
    });

    D3::DivergingScale testDiv;
    emp::array<double, 3> domainArr = {-0.78, 0, 1.35};
    testDiv.SetDomain(domainArr);
    testDiv.SetInterpolator("interpolatePuOr");
    emp::vector<double> result1 = testDiv.GetDomain<double>();
    std::string result3 = testDiv.ApplyScale<std::string>(-0.5);
    std::string result4 = testDiv.ApplyScale<std::string>(0);
    std::string result5 = testDiv.ApplyScale<std::string>(1.01);

    std::cout << "value 1: " << result1 << std::endl;
    std::cout << "value 3: " << result3 << std::endl;
    std::cout << "value 4: " << result4 << std::endl;
    std::cout << "value 5: " << result5 << std::endl;

    std::cout << "------Diverging Test End------" << std::endl << std::endl;
  }
};

//***********************************//
//********     Run tests     ********//
//***********************************//
int main() {

  D3::internal::get_emp_d3();

  // ContinuousScale tests
  TestLinearScale test1{};
  TestPowScale test2{};
  TestSqrtScale test3{};
  TestLogScale test4{};
  TestSymlogScale test5{};
  TestIdentityScale test6{};
  TestTimeScale test7{};
  TestSequentialScale test8{};
  TestSequentialQuantileScale test9{};
  TestDivergingScale test10{};
}

// This functionality is included in the newest version of d3-scale, but not base d3
// scaleRadial
// struct TestRadialScale {
//   TestRadialScale() {
//     std::cout << "------Radial Test Begin------" << std::endl;

//     EM_ASM({
//       var radialScale = d3.scaleRadial();

//       console.log(radialScale(0.10)); // 12
//       console.log(radialScale(0.777)); // 50.6789
//       console.log(radialScale(1.000)); // 1234
//     });

//     // D3::RadialScale testRad = D3::RadialScale();
//     // // testRad.SetDomain(10, 1000);
//     // testRad.SetRange(30, 300);
//     // double result1 = testRad.ApplyScale<double>(10);
//     // double result2 = testRad.ApplyScale<double>(777);
//     // double result3 = testRad.ApplyScale<double>(1000);

//     // std::cout << "value 1: " << result1 << std::endl;
//     // std::cout << "value 2: " << result2 << std::endl;
//     // std::cout << "value 3: " << result3 << std::endl;

//     std::cout << "------Radial Test End------" << std::endl << std::endl;
//   }
// };
