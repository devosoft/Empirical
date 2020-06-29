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

//***********************************//    
//******** Continuous Scales ********//
//***********************************// 

// for vectors
template <typename T>
void pass_map_to_javascript(emp::vector<std::string> & keys, emp::vector<T> & values) { 
  // initialize emp_js_map object
  EM_ASM({
    window["emp_cpp_map"] = ( {"cpp_keys" : [],
                              "cpp_values" : [],
                              "cpp_dict" : {} } );
  });

  emp::pass_array_to_javascript(keys);
  EM_ASM({
    window["emp_cpp_map"].cpp_keys = emp_i.__incoming_array;
  });

  emp::pass_array_to_javascript(values);
  EM_ASM({
    window["emp_cpp_map"].cpp_values = emp_i.__incoming_array;
 
    // create dictionary 
    window["emp_cpp_map"].cpp_keys.forEach(function(key, val) {
      window["emp_cpp_map"].cpp_dict[key] = window["emp_cpp_map"].cpp_values[val]
    });
    // console.log(window["emp_cpp_map"].cpp_dict);
  });
}

// for arrays
template <typename T, size_t SIZE>
void pass_map_to_javascript(emp::array<std::string, SIZE> & keys, emp::array<T, SIZE> & values) { 
  // initialize emp_js_map object
  EM_ASM({
    window["emp_cpp_map"] = ( {"cpp_keys" : [],
                              "cpp_values" : [],
                              "cpp_dict" : {} } );
  });

  emp::pass_array_to_javascript(keys);
  EM_ASM({
    window["emp_cpp_map"].cpp_keys = emp_i.__incoming_array;
  });

  emp::pass_array_to_javascript(values);
  EM_ASM({
    window["emp_cpp_map"].cpp_values = emp_i.__incoming_array;
 
    // create dictionary 
    window["emp_cpp_map"].cpp_keys.forEach(function(key, val) {
      window["emp_cpp_map"].cpp_dict[key] = window["emp_cpp_map"].cpp_values[val]
    });
    // console.log(window["emp_cpp_map"].cpp_dict);
  });
}

// for std::map
template <typename T>
void pass_map_to_javascript(std::map<std::string, T> & dict) { 
  // extract keys and values from dict
  emp::vector<std::string> keys;
  emp::vector<T> values;
  for(typename std::map<std::string, T>::iterator it = dict.begin(); it != dict.end(); ++it) {
    keys.push_back(it->first);
    values.push_back(it->second);
  }
  
  // initialize emp_js_map object
  EM_ASM({
    window["emp_cpp_map"] = ( {"cpp_keys" : [],
                              "cpp_values" : [],
                              "cpp_dict" : {} } );
  });

  emp::pass_array_to_javascript(keys);
  EM_ASM({
    window["emp_cpp_map"].cpp_keys = emp_i.__incoming_array;
  });

  emp::pass_array_to_javascript(values);
  EM_ASM({
    window["emp_cpp_map"].cpp_values = emp_i.__incoming_array;
 
    // create dictionary 
    window["emp_cpp_map"].cpp_keys.forEach(function(key, val) {
      window["emp_cpp_map"].cpp_dict[key] = window["emp_cpp_map"].cpp_values[val]
    });
    // console.log(window["emp_cpp_map"].cpp_dict);
  });
}

// scaleLinear
struct TestLinearScale {
  TestLinearScale() {
    std::cout << "------Linear Test Begin------" << std::endl;

    // initialize objects in JavaScript
    // combine two arrays to make a final lookup (dict) dictionay
    EM_ASM({
      window["test_linear_scale"] = ( {"js_keys" : [],
                                      "js_values" : [],
                                       "cpp_keys" : [],
                                       "cpp_values" : [],
                                       "js_dict" : {},
                                       "cpp_dict" : {} } );
    });

    // a vector to store results
    emp::vector<int> cpp_values_vec;

    // a vector to store names
    emp::vector<std::string> cpp_keys_vec;

    D3::LinearScale testLinearX; 
    testLinearX.SetDomain(10, 130.00);
    testLinearX.SetRange(0, 960.00);
    int result1 = testLinearX.ApplyScaleInt(20);
    int result2 = testLinearX.ApplyScaleInt(50);
    int result1i = testLinearX.Invert<int>(80);
    int result2i = testLinearX.Invert<int>(320);
    cpp_values_vec.push_back(result1); 
    cpp_values_vec.push_back(result2);
    cpp_values_vec.push_back(result1i);
    cpp_values_vec.push_back(result2i);
    cpp_keys_vec.push_back("applyScale1"); 
    cpp_keys_vec.push_back("applyScale2");
    cpp_keys_vec.push_back("invert1");
    cpp_keys_vec.push_back("invert2");

    // D3::LinearScale testLinearColor; 
    // testLinearColor.SetDomain(10, 100);
    // emp::array<std::string, 2> colorArray = {"brown", "steelblue"};
    // testLinearColor.SetRange(colorArray);
    // std::string result3 = testLinearColor.ApplyScaleString(20);
    // std::string result4 = testLinearColor.ApplyScaleString(50);
    // cpp_keys_vec.push_back("color1");
    // cpp_keys_vec.push_back("color2");
    // cpp_values_vec.push_back(result3);
    // cpp_values_vec.push_back(result4);

    pass_map_to_javascript(cpp_keys_vec, cpp_values_vec);
    
    // add results to JS
    EM_ASM({  
      var x = d3.scaleLinear()
                  .domain([ 10, 130 ])
                  .range([ 0, 960 ]);

      var color = d3.scaleLinear()
                      .domain([ 10, 100 ])
                      .range([ "brown", "steelblue" ]);

      // add js results
      window["test_linear_scale"].js_values.push(x(20), x(50), x.invert(80), x.invert(320)); //, color(20), color(50));
      window["test_linear_scale"].js_keys.push("applyScale1", "applyScale2", "invert1", "invert2"); //, "color1", "color2");

      // add to dict 
      window["test_linear_scale"].js_keys.forEach(function(key, val) {
        window["test_linear_scale"].js_dict[key] = window["test_linear_scale"].js_values[val]
      });

      // add cpp stuff
      window["test_linear_scale"].cpp_keys = window["emp_cpp_map"].cpp_keys;
      window["test_linear_scale"].cpp_values = window["emp_cpp_map"].cpp_values;
      window["test_linear_scale"].cpp_dict = window["emp_cpp_map"].cpp_dict;
      
      console.log(window["test_linear_scale"].js_dict);
      console.log(window["test_linear_scale"].cpp_dict);

      // compare both dictionaries
      console.log(JSON.stringify(window["test_linear_scale"].js_dict) === JSON.stringify(window["test_linear_scale"].cpp_dict));
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

      console.log(population2(1.386e9)); // 249.73986465920893
      console.log(population2(127e6)); // 75.59761901012492
      console.log(population2(427e3)); // 4.383491758860737

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
    testPowPop.SetDomain(0, 2e9);
    testPowPop.SetRange(0, 300);
    emp::vector<double> result1 = testPowPop.GetDomain<double>();
    emp::vector<double> result2 = testPowPop.GetRange<double>();
    double result3 = testPowPop.ApplyScaleDouble(1.386e9);
    double result4 = testPowPop.ApplyScaleDouble(127e6);
    double result5 = testPowPop.ApplyScaleDouble(427e3);

    std::cout << "value 1: " << result1 << std::endl;
    std::cout << "value 2: " << result2 << std::endl;
    std::cout << "value 3: " << result3 << std::endl;
    std::cout << "value 4: " << result4 << std::endl;
    std::cout << "value 5: " << result5 << std::endl;

    D3::PowScale testPowPop2; 
    testPowPop2.SetExponent(1.5);
    testPowPop2.SetDomain(0, 2e9);
    testPowPop2.SetRange(0, 300);
    double result6 = testPowPop2.ApplyScaleDouble(1.386e9);
    double result7 = testPowPop2.ApplyScaleDouble(127e6);
    double result8 = testPowPop2.ApplyScaleDouble(427e3);

    std::cout << "value 6: " << result6 << std::endl;
    std::cout << "value 7: " << result7 << std::endl;
    std::cout << "value 8: " << result8 << std::endl;

    D3::PowScale testPowPopColor; 
    emp::array<std::string, 2> colorArray = {"yellow", "red"};
    testPowPopColor.SetDomain(0, 2e9);
    testPowPopColor.SetRange(colorArray);
    // emp::vector<double> result9 = testPowPopColor.GetDomainDouble();
    // emp::vector<std::string> result10 = testPowPopColor.GetRangeString();
    emp::vector<double> result9 = testPowPopColor.GetDomain<double>();
    emp::vector<std::string> result10 = testPowPopColor.GetRange<std::string>();
    std::string result11 = testPowPopColor.ApplyScaleString(1.386e9);
    std::string result12 = testPowPopColor.ApplyScaleString(127e6);
    std::string result13 = testPowPopColor.ApplyScaleString(427e3);

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
    testSqrtPop.SetDomain(0, 2e9);
    testSqrtPop.SetRange(0, 300);
    double result1 = testSqrtPop.ApplyScaleDouble(1.386e9);
    double result2 = testSqrtPop.ApplyScaleDouble(127e6);
    double result3 = testSqrtPop.ApplyScaleDouble(427e3);
    std::cout << "value 1: " << result1 << std::endl;
    std::cout << "value 2: " << result2 << std::endl;
    std::cout << "value 3: " << result3 << std::endl;

    // D3::SqrtScale copyPop = testSqrtPop.Copy();
    D3::SqrtScale copyPop(testSqrtPop);  // = new D3::SqrtScale(testSqrtPop);
    double result4 = copyPop.ApplyScaleDouble(1.386e9);
    double result5 = copyPop.ApplyScaleDouble(127e6);
    double result6 = copyPop.ApplyScaleDouble(427e3);
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
    double result1 = testLog.ApplyScaleDouble(1000);
    double result2 = testLog.ApplyScaleDouble(1234);
    double result3 = testLog.ApplyScaleDouble(100000);

    std::cout << "value 1: " << result1 << std::endl;
    std::cout << "value 2: " << result2 << std::endl;
    std::cout << "value 3: " << result3 << std::endl;

    D3::LogScale testLog2; 
    testLog2.SetBase(2);
    testLog2.SetDomain(16, 1048576);
    testLog2.SetRange(0, 700);
    double result4 = testLog2.ApplyScaleDouble(64);
    double result5 = testLog2.ApplyScaleDouble(1234);
    double result6 = testLog2.ApplyScaleDouble(1048576);

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
    double result1 = testSymlog.ApplyScaleDouble(-80000);
    double result2 = testSymlog.ApplyScaleDouble(-50);
    double result3 = testSymlog.ApplyScaleDouble(1.5);
    double result4 = testSymlog.ApplyScaleDouble(50);
    double result5 = testSymlog.ApplyScaleDouble(80000);


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
    double result1 = testId.ApplyScaleDouble(12);
    double result2 = testId.ApplyScaleDouble(50.6789);
    double result3 = testId.ApplyScaleDouble(1234);

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

      console.log(timeScale(new Date(2000, 0, 1,  5))); // 200
      console.log(timeScale(new Date(2000, 0, 1, 16))); // 640
      console.log(timeScale.invert(200)); // Sat Jan 01 2000 05:00:00 GMT-0800 (PST)
      console.log(timeScale.invert(640)); // Sat Jan 01 2000 16:00:00 GMT-0800 (PST)
    });

    D3::TimeScale testTime;  
    D3::TimeScale::Date dateMin(2000, 0, 1);
    D3::TimeScale::Date dateMax(2000, 0, 2);
    testTime.SetDomain(dateMin, dateMax);
    testTime.SetRange(0, 960);
    D3::TimeScale::Date test1(2000, 0, 1, 5);
    D3::TimeScale::Date test2(2000, 0, 1, 16);
    double result1 = testTime.ApplyScaleDouble(test1);
    double result2 = testTime.ApplyScaleDouble(test2);
    D3::TimeScale::Date result1i = testTime.InvertDate(200);
    D3::TimeScale::Date result2i = testTime.InvertDate(640);

    std::cout << "value 1: " << result1 << std::endl;
    std::cout << "value 2: " << result2 << std::endl;
    std::cout << "value 1 invert: " << result1i.ToString() << std::endl;
    std::cout << "value 2 invert: " << result2i.ToString() << std::endl;

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
    std::string result1 = testSeq.ApplyScaleString(0);
    std::string result2 = testSeq.ApplyScaleString(50);
    std::string result3 = testSeq.ApplyScaleString(100);

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
    std::string result1 = testSeqQuant.ApplyScaleString(0);
    std::string result2 = testSeqQuant.ApplyScaleString(50);
    std::string result3 = testSeqQuant.ApplyScaleString(60);
    std::string result4 = testSeqQuant.ApplyScaleString(100);
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

// caleDiverging
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
    std::string result3 = testDiv.ApplyScaleString(-0.5);
    std::string result4 = testDiv.ApplyScaleString(0);
    std::string result5 = testDiv.ApplyScaleString(1.01);

    std::cout << "value 1: " << result1 << std::endl;
    std::cout << "value 3: " << result3 << std::endl;
    std::cout << "value 4: " << result4 << std::endl;
    std::cout << "value 5: " << result5 << std::endl;

    std::cout << "------Diverging Test End------" << std::endl << std::endl;
  }
};



//***********************************//    
//******** __________ Scales ********//
//***********************************// 

// struct TestQuantizeScale {
//   TestQuantizeScale() {
//     EM_ASM({
//       var color = d3.scaleQuantize()
//         .domain([0, 1])
//         .range(["brown", "steelblue"]);

//       console.log(color(0.49)); // "brown"
//       console.log(color(0.51)); // "steelblue"
//     )}; 

//     D3::QuantizeScale testQuantize = D3::QuantizeScale();
//     emp::array<std::string, 2> colors = {"brown", "steelblue"};
//     testQuantize.SetDomain(0, 1);
//     // quantScale.SetRange({"brown", "steelblue"});
//     testQuantize.SetRange(colors);

//     std::string value1 = testQuantize.ApplyScaleString(0.49);
//     std::string value2 = testQuantize.ApplyScaleString(0.51);

//     std::cout << "value 1: " << value1 << std::endl;
//     std::cout << "value 2: " << value2 << std::endl;
//   }
// };


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

  TestSequentialQuantileScale test10{};
  TestDivergingScale test11{};

  // emp::array<std::string, 5> keysCpp = {"test1", "test2", "test3", "test4", "test5"};
  // // emp::array<int, 5> keysCpp = {11, 12, 13, 14, 15};
  // emp::array<std::string, 5> vals = {"red", "blue", "blue", "yellow", "purple"};
  // pass_map_to_javascript(keysCpp, vals); 

  // std::map<std::string, std::string> testMap = {{"test1", "a"}, {"test2", "b"}, {"test3", "c"}, {"test4", "d"}, {"test5", "e"}}; 
  // pass_map_to_javascript(testMap);  
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
//     // double result1 = testRad.ApplyScaleDouble(10);
//     // double result2 = testRad.ApplyScaleDouble(777);
//     // double result3 = testRad.ApplyScaleDouble(1000);

//     // std::cout << "value 1: " << result1 << std::endl;
//     // std::cout << "value 2: " << result2 << std::endl;
//     // std::cout << "value 3: " << result3 << std::endl;

//     std::cout << "------Radial Test End------" << std::endl << std::endl;
//   }
// };
