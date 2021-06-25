/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2016-2020
 *
 *  @file  scales.hpp
 *  @brief Tools for scaling graph axes in D3.
**/

#ifndef __EMP_D3_SCALES_H__
#define __EMP_D3_SCALES_H__

#include "d3_init.hpp"

#include <iostream>
#include <string>
#include <typeinfo>
#include <map>
#include <array>
#include "../../base/vector.hpp"
#include "../../base/array.hpp"

#include "../../base/assert.hpp"
#include "../js_utils.hpp"
#include "../JSWrap.hpp"

namespace D3 {

  /// Scales in D3 are functions that take input values and map them to output based on
  /// a scaling function. They are often used to map data values to x, y coordinates in pixels
  /// describing where on the screen elements should be placed.
  /// This is a base class to inherit from - it should never be made stand-alone
  class Scale : public D3_Base {
  protected:
    Scale(int id) : D3_Base(id) { ; };
    Scale() { ; }

    /// Decoy constructor so we don't construct extra base scales
    Scale(bool derived){;};

  public:

    /// Make a copy of this scale
    /// Copy constructor
    Scale(const Scale & other) {
      int new_id = MAIN_THREAD_EM_ASM_INT({
        return emp_d3.objects.next_id++;
      });
      MAIN_THREAD_EM_ASM({
        emp_d3.objects[$1] = emp_d3.objects[$0].copy();
      }, other.id, new_id);

      this->id = new_id;
    }

    /// Set the domain of possible input values corresponding to values in the range
    /// Note that an array of strings can be passed in here
    template <typename T, size_t SIZE>
    Scale & SetDomain(const emp::array<T, SIZE> & values) {
      emp::pass_array_to_javascript(values);
      MAIN_THREAD_EM_ASM({
        emp_d3.objects[$0].domain(emp_i.__incoming_array);
      }, this->id);
      return *this;
    }

    Scale & SetDomain(double min, double max) {
      MAIN_THREAD_EM_ASM({
        emp_d3.objects[$0].domain([$1, $2]);
      }, this->id, min, max);
      return *this;
    }

    Scale & SetDomain(const std::string & lower, const std::string & upper) {
      MAIN_THREAD_EM_ASM({
        emp_d3.objects[$0].domain([UTF8ToString($1), UTF8ToString($2)]);
      }, this->id, lower.c_str(), upper.c_str());
      return *this;
    }

    /// Set the range of possible output values corresponding to values in the domain.
    /// Output for values in between will be interpolated with a function determined
    /// by the type of the scale.
    /// Note that an array of strings can be passed in here
    template <typename T, size_t SIZE>
    Scale & SetRange(const emp::array<T, SIZE> & values) {
      emp::pass_array_to_javascript(values);
      MAIN_THREAD_EM_ASM({
        emp_d3.objects[$0].range(emp_i.__incoming_array);
      }, this->id);
      return *this;
    }

    Scale & SetRange(double min, double max) {
      MAIN_THREAD_EM_ASM({
        emp_d3.objects[$0].range([$1, $2]);
      }, this->id, min, max);
      return *this;
    }

    Scale & SetRange(const std::string & lower, const std::string & upper) {
      MAIN_THREAD_EM_ASM({
        emp_d3.objects[$0].range([UTF8ToString($1), UTF8ToString($2)]);
      }, this->id, lower.c_str(), upper.c_str());
      return *this;
    }

    // ApplyScale
    // Note that when passing in an std::string as an input you must explicitly specify it 
    // in the template (but when pasing in a double or an int it will match to the proper
    // template automatically so you only need to specify the return type)
    template<typename RETURN_T, typename INPUT_T>
    RETURN_T ApplyScale(INPUT_T input) { ; }

    template<>
    std::string ApplyScale<std::string, std::string>(std::string input) {
      MAIN_THREAD_EM_ASM({
        const resultStr = emp_d3.objects[$0](UTF8ToString($1));
        emp.PassStringToCpp(resultStr);
      }, this->id, input.c_str());
      return emp::pass_str_to_cpp();
    }

    template<>
    std::string ApplyScale<std::string, double>(double input) {
      MAIN_THREAD_EM_ASM({
        const resultStr = emp_d3.objects[$0]($1);
        emp.PassStringToCpp(resultStr);
      }, this->id, input);
      return emp::pass_str_to_cpp();
    }

    template<>
    std::string ApplyScale<std::string, int>(int input) {
      MAIN_THREAD_EM_ASM({
        const resultStr = emp_d3.objects[$0]($1);
        emp.PassStringToCpp(resultStr);
      }, this->id, input);
      return emp::pass_str_to_cpp();
    }

    template<>
    double ApplyScale<double, std::string>(std::string input) {
      return MAIN_THREAD_EM_ASM_DOUBLE({
        return emp_d3.objects[$0](UTF8ToString($1));
      }, this->id, input.c_str());
    }

    template<>
    double ApplyScale<double, double>(double input) {
      return MAIN_THREAD_EM_ASM_DOUBLE({
        return emp_d3.objects[$0]($1);
      }, this->id, input);
    }

    template<>
    double ApplyScale<double, int>(int input) {
      return MAIN_THREAD_EM_ASM_DOUBLE({
        return emp_d3.objects[$0]($1);
      }, this->id, input);
    }

    template<>
    int ApplyScale<int, std::string>(std::string input) {
      return MAIN_THREAD_EM_ASM_INT({
        return emp_d3.objects[$0](UTF8ToString($1));
      }, this->id, input.c_str());
    }

    template<>
    int ApplyScale<int, double>(double input) {
      return MAIN_THREAD_EM_ASM_INT({
        return emp_d3.objects[$0]($1);
      }, this->id, input);
    }

    template<>
    int ApplyScale<int, int>(int input) {
      return MAIN_THREAD_EM_ASM_INT({
        return emp_d3.objects[$0]($1);
      }, this->id, input);
    }

    // Getter methods for a scale's domain and range
    template <typename T>
    emp::vector<T> GetDomain() {
      MAIN_THREAD_EM_ASM({
        emp_i.__outgoing_array = emp_d3.objects[$0].domain();
      }, this->id);
      // access JS array
      emp::vector<T> domain_vector;
      emp::pass_vector_to_cpp(domain_vector);
      return domain_vector;
    }

    template <typename T>
    emp::vector<T> GetRange() {
      MAIN_THREAD_EM_ASM({
        emp_i.__outgoing_array = emp_d3.objects[$0].range();
      }, this->id);
      // access JS array
      emp::vector<T> range_vector;
      emp::pass_vector_to_cpp(range_vector);
      return range_vector;
    }
  };

  //////////////////////////////////////////////////////////
  /// Scales with continuous input and continuous output ///
  //////////////////////////////////////////////////////////
  /// This is a base class to inherit from - it should never be made stand-alone
  class ContinuousScale : public Scale {
  protected:
    ContinuousScale(bool derived) : Scale(true) {;}
    ContinuousScale() : Scale(true) {;}

  public:
    // Invert is only supported if the range is numeric. If the range is not numeric, returns NaN
    template <typename T>
    double Invert(T y) {
      return MAIN_THREAD_EM_ASM_DOUBLE({
        return emp_d3.objects[$0].invert($1);
      }, this->id, y);
    }

    // .ticks()
    ContinuousScale & SetTicks(int count) {
      MAIN_THREAD_EM_ASM({
        emp_d3.objects[$0].ticks($1);
      }, this->id, count);
      return *this;
    }

    // .tickFormat()
    ContinuousScale & SetTickFormat(int count, const std::string & format="") {
      MAIN_THREAD_EM_ASM({
        const id = $0;
        const count = $1;
        const format = UTF8ToString($2);
        if (format === "") {
          emp_d3.objects[id].tickFormat(count);
        }
        else {
          emp_d3.objects[id].tickFormat(count, format);
        }
      }, this->id, count, format.c_str());

      return *this;
    }

    // .nice()
    /// Extend the domain so that it start and ends on nice values
    /// Nicing a scale only modifies the current domain
    ContinuousScale & Nice() {
      MAIN_THREAD_EM_ASM({
        emp_d3.objects[$0].nice()
      }, this->id);
      return *this;
    }

    /// Sets the scale’s range to the specified array of values
    /// while also setting the scale’s interpolator to interpolateRound
    template <typename T, size_t SIZE>
    ContinuousScale & SetRangeRound(const emp::array<T,SIZE> & values) {
      emp::pass_array_to_javascript(values);
      MAIN_THREAD_EM_ASM({
        emp_d3.objects[$0].rangeRound(emp.__incoming_array);
      }, this->id);
      return *this;
    }

    ContinuousScale & SetRangeRound(double min, double max) {
      MAIN_THREAD_EM_ASM({
        emp_d3.objects[$0].rangeRound([$1, $2]);
      }, this->id, min, max);
      return *this;
    }

    /// Enables or disables clamping accordingly
    ContinuousScale & SetClamp(bool clamp) {
      MAIN_THREAD_EM_ASM({ emp_d3.objects[$0].clamp($1); }, this->id, clamp);
      return *this;
    }

    // .interpolate() (need to pass in an interpolator)
    ContinuousScale & SetInterpolate(const std::string & interpolatorName) {
      // note: this doesn't allow you to specify arguments to a d3.interpolator function
      MAIN_THREAD_EM_ASM({
        const id = $0;
        const interpolator_str = UTF8ToString($1);
        var sel = emp_d3.find_function(interpolator_str);
        emp_d3.objects[id].interpolate(sel);
      }, this->id, interpolatorName.c_str());

      return *this;
    }

    // .unknown()
    ContinuousScale & SetUnknown(double value) {
      MAIN_THREAD_EM_ASM({
        emp_d3.objects[$0].unknown($1);
      }, this->id, value);
      return *this;
    }

    ContinuousScale & SetUnknown(const std::string & value) {
      MAIN_THREAD_EM_ASM({
        emp_d3.objects[$0](UTF8ToString($1));
      }, this->id, value.c_str());
      return *this;
    }
  };

  // scaleLinear
  class LinearScale : public ContinuousScale {
  protected:
    LinearScale(bool derived) : ContinuousScale(true) { ; }

  public:
    LinearScale() : ContinuousScale(true) {
      MAIN_THREAD_EM_ASM({ emp_d3.objects[$0] = d3.scaleLinear(); }, this->id);
    }
  };

  // scalePow
  class PowScale : public ContinuousScale {
  protected:
    PowScale(bool derived) : ContinuousScale(true) { ; }

  public:
    PowScale() : ContinuousScale(true) {
      MAIN_THREAD_EM_ASM({ emp_d3.objects[$0] = d3.scalePow(); }, this->id);
    }

    PowScale & SetExponent(double ex) {
      MAIN_THREAD_EM_ASM({ emp_d3.objects[$0].exponent($1);}, this->id, ex);
      return *this;
    }
  };

  // scaleSqrt
  // A convenience scale to set the exponent to 0.5 in a PowScale
  class SqrtScale : public ContinuousScale {
  protected:
    SqrtScale(bool derived) : ContinuousScale(true) { ; }

  public:
    SqrtScale() : ContinuousScale(true) {
      MAIN_THREAD_EM_ASM({ emp_d3.objects[$0] = d3.scaleSqrt(); }, this->id);
    }
  };

  // scaleLog
  class LogScale : public ContinuousScale {
  protected:
    LogScale(bool derived) : ContinuousScale(true) { ; }

  public:
    LogScale() : ContinuousScale(true) {
      MAIN_THREAD_EM_ASM({ emp_d3.objects[$0] = d3.scaleLog(); }, this->id);
    }

    LogScale & SetBase(double baseNum) {
      MAIN_THREAD_EM_ASM({ emp_d3.objects[$0].base($1);}, this->id, baseNum);
      return *this;
    }
  };

  // scaleSymlog
  class SymlogScale : public ContinuousScale {
  protected:
    SymlogScale(bool derived) : ContinuousScale(true) { ; }

  public:
    SymlogScale() : ContinuousScale(true) {
      MAIN_THREAD_EM_ASM({ emp_d3.objects[$0] = d3.scaleSymlog(); }, this->id);
    }

    SymlogScale & SetConstant(double constant) {
      MAIN_THREAD_EM_ASM({ emp_d3.objects[$0].constant($1); }, this->id, constant);
      return *this;
    }
  };

  // scaleIdentity
  class IdentityScale : public ContinuousScale {
  protected:
    IdentityScale(bool derived) : ContinuousScale(true) { ; }

  public:
    IdentityScale() : ContinuousScale(true) {
      MAIN_THREAD_EM_ASM({ emp_d3.objects[$0] = d3.scaleIdentity(); }, this->id);
    }

    // get rid of functions that shouldn't be called:
    // Identity scales do not support rangeRound, clamp or interpolate
    template <typename T, size_t SIZE>
    ContinuousScale & SetRangeRound(emp::array<T,SIZE> values) = delete;
    ContinuousScale & SetRangeRound(double min, double max) = delete;
    ContinuousScale & SetClamp(bool clamp) = delete;
    ContinuousScale & SetInterpolate(const std::string & interpolatorName) = delete;
  };

  // This functionality is included in the newest version of d3-scale, but not the newest version of base d3
  // scaleRadial
  // class RadialScale : public ContinuousScale {
  // protected:
  //   RadialScale(bool derived) : ContinuousScale(true) { ; }

  // public:
  //   RadialScale() : ContinuousScale(true) {
  //     MAIN_THREAD_EM_ASM({ emp_d3.objects[$0] = d3.scaleRadial(); }, this->id);
  //   }

  //   // get rid of functions that shouldn't be called:
  //   // Radial scales do not support interpolate
  //   ContinuousScale & SetInterpolate(const std::string & interpolatorName) = delete;
  // };

  // A struct to deal with dates that mimics the JS Date object
  struct Date {
    int year;
    int month;
    int day;
    int hours;
    int minutes;
    int seconds;
    int milliseconds;

    Date() { ; }

    // note that month should be passed in 0-indexed to keep consistent with JavaScript (0 = January)
    Date(int year, int month, int day = 1, int hours = 0, int minutes = 0,
    int seconds = 0, int milliseconds = 0) {
      this->year = year;
      this->month = month;
      this->day = day;
      this->hours = hours;
      this->minutes = minutes;
      this->seconds = seconds;
      this->milliseconds = milliseconds;
    }

    std::string ToString() {
      return std::to_string(this->year) + " " + std::to_string(this->month) + " " + std::to_string(this->day) +
      " " + std::to_string(this->hours) + ":" + std::to_string(this->minutes) + ":" + std::to_string(this->seconds) +
      ":" + std::to_string(this->milliseconds);
    }
  };

  // scaleTime
  class TimeScale : public ContinuousScale {
  protected:
    TimeScale(bool derived) : ContinuousScale(true) { ; }

  public:
    TimeScale() : ContinuousScale(true) {
      MAIN_THREAD_EM_ASM({ emp_d3.objects[$0] = d3.scaleTime(); }, this->id);
    }

    // get rid of functions that shouldn't be called
    template <typename T, size_t SIZE>
    Scale & SetDomain(const emp::array<T, SIZE> & values) = delete;
    Scale & SetDomain(double min, double max) = delete;
    Scale & SetDomain(const std::string & lower, const std::string & upper) = delete;
    template <typename T>
    double Invert(T y) = delete;
    template<typename RETURN_T, typename INPUT_T>
    RETURN_T ApplyScale(INPUT_T input) = delete;


    // special SetDomain to deal with Dates
    TimeScale & SetDomain(const Date & dateMin, const Date & dateMax) {
      MAIN_THREAD_EM_ASM({
        const id = $0;
        const yearMin = $1;
        const monthMin = $2;
        const dayMin = $3;
        const hoursMin = $4;
        const minutesMin = $5;
        const secondsMin = $6;
        const millisecondsMin = $7;

        const yearMax = $8;
        const monthMax = $9;
        const dayMax = $10;
        const hoursMax = $11;
        const minutesMax = $12;
        const secondsMax = $13;
        const millisecondsMax = $14;

        const dateMin = new Date(yearMin, monthMin, dayMin, hoursMin, minutesMin, secondsMin, millisecondsMin);
        const dateMax = new Date(yearMax, monthMax, dayMax, hoursMax, minutesMax, secondsMax, millisecondsMax);

        emp_d3.objects[$0].domain([dateMin, dateMax]);
      }, this->id, dateMin.year, dateMin.month, dateMin.day, dateMin.hours, dateMin.minutes, dateMin.seconds, dateMin.milliseconds,
                   dateMax.year, dateMax.month, dateMax.day, dateMax.hours, dateMax.minutes, dateMax.seconds, dateMax.milliseconds);
      return *this;
    }

    template<typename T>
    T ApplyScale(const Date & dateInput) { ; }

    template<>
    double ApplyScale<double>(const Date & dateInput) {
      return MAIN_THREAD_EM_ASM_DOUBLE({
        const id = $0;
        const year = $1;
        const month = $2;
        const day = $3;
        const hours = $4;
        const minutes = $5;
        const seconds = $6;
        const milliseconds = $7;

        const dateInput = new Date(year, month, day, hours, minutes, seconds, milliseconds);
        return emp_d3.objects[id](dateInput);
      }, this->id, dateInput.year, dateInput.month, dateInput.day, dateInput.hours, dateInput.minutes, dateInput.seconds, dateInput.milliseconds);
    }

    template<>
    int ApplyScale<int>(const Date & dateInput) {
      return MAIN_THREAD_EM_ASM_INT({
        const id = $0;
        const year = $1;
        const month = $2;
        const day = $3;
        const hours = $4;
        const minutes = $5;
        const seconds = $6;
        const milliseconds = $7;

        const dateInput = new Date(year, month, day, hours, minutes, seconds, milliseconds);
        return emp_d3.objects[id](dateInput);
      }, this->id, dateInput.year, dateInput.month, dateInput.day, dateInput.hours, dateInput.minutes, dateInput.seconds, dateInput.milliseconds);
    }

    // ApplyScale that returns a string
    template<>
    std::string ApplyScale<std::string>(const Date & dateInput) {
      MAIN_THREAD_EM_ASM({
        const id = $0;
        const year = $1;
        const month = $2;
        const day = $3;
        const hours = $4;
        const minutes = $5;
        const seconds = $6;
        const milliseconds = $7;

        const dateInput = new Date(year, month, day, hours, minutes, seconds, milliseconds);
        const resultStr = emp_d3.objects[id](dateInput);
        emp.PassStringToCpp(resultStr);
      }, this->id, dateInput.year, dateInput.month, dateInput.day, dateInput.hours, dateInput.minutes, dateInput.seconds, dateInput.milliseconds);

      return emp::pass_str_to_cpp();
    }
    
    Date Invert(double input) {
      MAIN_THREAD_EM_ASM({
        const id = $0;
        const input = $1;
        const newDate = emp_d3.objects[id].invert(input);

        emp_i.__outgoing_array = ([ newDate.getFullYear(), newDate.getMonth(), newDate.getDate(), newDate.getHours(), newDate.getMinutes(), newDate.getSeconds(), newDate.getMilliseconds() ]);
      }, this->id, input);

      // access JS array, create date struct
      emp::array<int, 7> date_array;
      emp::pass_array_to_cpp(date_array);
      Date returnDate(date_array[0], date_array[1], date_array[2], date_array[3], date_array[4], date_array[5], date_array[6]);
      return returnDate;
    }

    // we need this copy (for int inputs) since any templating has already been deleted from the base version of Invert
    // (and trying to fix this makes the code messier than just adding this second function)
    Date Invert(int input) {
      MAIN_THREAD_EM_ASM({
        const id = $0;
        const input = $1;
        const newDate = emp_d3.objects[id].invert(input);

        emp_i.__outgoing_array = ([ newDate.getFullYear(), newDate.getMonth(), newDate.getDate(), newDate.getHours(), newDate.getMinutes(), newDate.getSeconds(), newDate.getMilliseconds() ]);
      }, this->id, input);

      // access JS array, create date struct
      emp::array<int, 7> date_array;
      emp::pass_array_to_cpp(date_array);
      Date returnDate(date_array[0], date_array[1], date_array[2], date_array[3], date_array[4], date_array[5], date_array[6]);
      return returnDate;
    }
  };

  /// This is a base class for sequential or diverging scales to inherit from - it should never be called alone
  class SequentialOrDivergingScale : public ContinuousScale {
  protected:
    SequentialOrDivergingScale(bool derived) : ContinuousScale(true) { ; }
    SequentialOrDivergingScale() : ContinuousScale(true) {;}

  public:
    // get rid of functions that shouldn't be called:
    // Identity scales do not support invert or interpolate
    template <typename T>
    double Invert(T y) = delete;
    ContinuousScale & SetInterpolate(const std::string & interpolatorName) = delete;

    // .interpolator
    SequentialOrDivergingScale & SetInterpolator(const std::string & interpolatorName) {
      // note: this doesn't allow you to specify arguments to a d3.interpolator function
      MAIN_THREAD_EM_ASM({
        const id = $0;
        const interpolator_str = UTF8ToString($1);
        var sel = emp_d3.find_function(interpolator_str);
        emp_d3.objects[id].interpolator(sel);
      }, this->id, interpolatorName.c_str());

      return *this;
    }
  };

  // scaleSequential
  // is used for mapping continuous values to an output range
  // determined by a preset (or custom) interpolator
  // the input domain and output range of a sequential scale always has exactly two elements,
  // and the output range is typically specified as an interpolator rather than an array of values
  class SequentialScale : public SequentialOrDivergingScale {
  protected:
    SequentialScale(bool derived) : SequentialOrDivergingScale(true) { ; }
  public:
    SequentialScale() : SequentialOrDivergingScale(true) {
      MAIN_THREAD_EM_ASM({ emp_d3.objects[$0] = d3.scaleSequential(); }, this->id);
    }
  };

  // scaleSequentialLog
  class SequentialLogScale : public SequentialOrDivergingScale {
  protected:
    SequentialLogScale(bool derived) : SequentialOrDivergingScale(true) { ; }
  public:
    SequentialLogScale() : SequentialOrDivergingScale(true) {
      MAIN_THREAD_EM_ASM({ emp_d3.objects[$0] = d3.scaleSequentialLog(); }, this->id);
    }

    SequentialLogScale & SetBase(double baseNum) {
      MAIN_THREAD_EM_ASM({ emp_d3.objects[$0].base($1);}, this->id, baseNum);
      return *this;
    }
  };

  // scaleSequentialPow
  class SequentialPowScale : public SequentialOrDivergingScale {
  protected:
    SequentialPowScale(bool derived) : SequentialOrDivergingScale(true) { ; }
  public:
    SequentialPowScale() : SequentialOrDivergingScale(true) {
      MAIN_THREAD_EM_ASM({ emp_d3.objects[$0] = d3.scaleSequentialPow(); }, this->id);
    }

    SequentialPowScale & SetExponent(double ex) {
      MAIN_THREAD_EM_ASM({ emp_d3.objects[$0].exponent($1);}, this->id, ex);
      return *this;
    }
  };

  // scaleSequentialSqrt
  class SequentialSqrtScale : public SequentialOrDivergingScale {
  protected:
    SequentialSqrtScale(bool derived) : SequentialOrDivergingScale(true) { ; }
  public:
    SequentialSqrtScale() : SequentialOrDivergingScale(true) {
      MAIN_THREAD_EM_ASM({ emp_d3.objects[$0] = d3.scaleSequentialSqrt(); }, this->id);
    }
  };

  // scaleSequentialSymlog
  class SequentialSymlogScale : public SequentialOrDivergingScale {
  protected:
    SequentialSymlogScale(bool derived) : SequentialOrDivergingScale(true) { ; }
  public:
    SequentialSymlogScale() : SequentialOrDivergingScale(true) {
      MAIN_THREAD_EM_ASM({ emp_d3.objects[$0] = d3.scaleSequentialSymlog(); }, this->id);
    }

    SequentialSymlogScale & SetConstant(double constant) {
      MAIN_THREAD_EM_ASM({ emp_d3.objects[$0].constant($1); }, this->id, constant);
      return *this;
    }
  };

  // scaleSequentialQuantile
  class SequentialQuantileScale : public SequentialOrDivergingScale {
  protected:
    SequentialQuantileScale(bool derived) : SequentialOrDivergingScale(true) { ; }
  public:
    SequentialQuantileScale() : SequentialOrDivergingScale(true) {
      MAIN_THREAD_EM_ASM({ emp_d3.objects[$0] = d3.scaleSequentialQuantile(); }, this->id);
    }

    // This functionality is included in the newest version of d3-scale, but not base d3
    // .quantiles
    // Returns an array of n + 1 quantiles. For example, if n = 4, returns an array of five numbers:
    // the minimum value, the first quartile, the median, the third quartile, and the maximum.
    // emp::vector<double> GetQuantiles(int n) {
    //   MAIN_THREAD_EM_ASM({
    //     emp_i.__outgoing_array = emp_d3.objects[$0].quantiles($1);
    //   }, this->id, n);
    //   // access JS array
    //   emp::vector<double> quantile_vector;
    //   emp::pass_vector_to_cpp(quantile_vector);
    //   return quantile_vector;
    // }
  };


  // scaleDiverging
  // map a continuous, numeric input domain to a continuous output range.
  // However, unlike continuous scales, the input domain and output range of a diverging scale always has exactly three elements,
  // and the output range is typically specified as an interpolator rather than an array of values
  class DivergingScale : public SequentialOrDivergingScale {
  protected:
    DivergingScale(bool derived) : SequentialOrDivergingScale(true) { ; }
  public:
    DivergingScale() : SequentialOrDivergingScale(true) {
      MAIN_THREAD_EM_ASM({ emp_d3.objects[$0] = d3.scaleDiverging(); }, this->id);
    }
  };

  // scaleDivergingLog
  class DivergingLogScale : public SequentialOrDivergingScale {
  protected:
    DivergingLogScale(bool derived) : SequentialOrDivergingScale(true) { ; }
  public:
    DivergingLogScale() : SequentialOrDivergingScale(true) {
      MAIN_THREAD_EM_ASM({ emp_d3.objects[$0] = d3.scaleDivergingLog(); }, this->id);
    }

    DivergingLogScale & SetBase(double baseNum) {
      MAIN_THREAD_EM_ASM({ emp_d3.objects[$0].base($1);}, this->id, baseNum);
      return *this;
    }
  };

  // scaleDivergingPow
  class DivergingPowScale : public SequentialOrDivergingScale {
  protected:
    DivergingPowScale(bool derived) : SequentialOrDivergingScale(true) { ; }
  public:
    DivergingPowScale() : SequentialOrDivergingScale(true) {
      MAIN_THREAD_EM_ASM({ emp_d3.objects[$0] = d3.scaleDivergingPow(); }, this->id);
    }

    DivergingPowScale & SetExponent(double ex) {
      MAIN_THREAD_EM_ASM({ emp_d3.objects[$0].exponent($1);}, this->id, ex);
      return *this;
    }
  };

  // scaleDivergingSqrt
  class DivergingSqrtScale : public SequentialOrDivergingScale {
  protected:
    DivergingSqrtScale(bool derived) : SequentialOrDivergingScale(true) { ; }
  public:
    DivergingSqrtScale() : SequentialOrDivergingScale(true) {
      MAIN_THREAD_EM_ASM({ emp_d3.objects[$0] = d3.scaleDivergingSqrt(); }, this->id);
    }
  };

  // scaleDivergingSymlog
  class DivergingSymlogScale : public SequentialOrDivergingScale {
  protected:
    DivergingSymlogScale(bool derived) : SequentialOrDivergingScale(true) { ; }
  public:
    DivergingSymlogScale() : SequentialOrDivergingScale(true) {
      MAIN_THREAD_EM_ASM({ emp_d3.objects[$0] = d3.scaleDivergingSymlog(); }, this->id);
    }

    DivergingSymlogScale & SetConstant(double constant) {
      MAIN_THREAD_EM_ASM({ emp_d3.objects[$0].constant($1); }, this->id, constant);
      return *this;
    }
  };


  ////////////////////////////////////////////////////////
  /// Scales with continuous input and discrete output ///
  ////////////////////////////////////////////////////////
  /// This is a base class to inherit from - it should never be called alone
  class ContinuousInputDiscreteOutputScale : public Scale {
  protected:
    ContinuousInputDiscreteOutputScale(bool derived) : Scale(true) {;}
    ContinuousInputDiscreteOutputScale() : Scale(true) {;}

  public:
    
    emp::array<int, 2> InvertExtent(double input) {
      MAIN_THREAD_EM_ASM({
        emp_i.__outgoing_array = emp_d3.objects[$0].invertExtent($1);
      }, this->id, input);
      // access JS array
      emp::array<int, 2> invertExtentArr;
      emp::pass_array_to_cpp(invertExtentArr);
      return invertExtentArr;
    }

    emp::array<int, 2> InvertExtent(const std::string & input) {
      MAIN_THREAD_EM_ASM({
        emp_i.__outgoing_array = emp_d3.objects[$0].invertExtent(UTF8ToString($1));
      }, this->id, input.c_str());
      // access JS array
      emp::array<int, 2> invertExtentArr;
      emp::pass_array_to_cpp(invertExtentArr);
      return invertExtentArr;
    }
  };

  // scaleQuantize
  class QuantizeScale : public ContinuousInputDiscreteOutputScale {
  protected:
    QuantizeScale(bool derived) : ContinuousInputDiscreteOutputScale(true) {;}

  public:
    QuantizeScale() : ContinuousInputDiscreteOutputScale(true) {
      MAIN_THREAD_EM_ASM({
        emp_d3.objects[$0] = d3.scaleQuantize();
      }, this->id);
    }

    // .ticks()
    QuantizeScale & SetTicks(int count) {
      MAIN_THREAD_EM_ASM({
        emp_d3.objects[$0].ticks($1);
      }, this->id, count);
      return *this;
    }

    // .tickFormat()
    QuantizeScale & SetTickFormat(int count, const std::string & format="") {
      MAIN_THREAD_EM_ASM({
        const id = $0;
        const count = $1;
        const format = UTF8ToString($2);
        if (format === "") {
          emp_d3.objects[id].tickFormat(count);
        }
        else {
          emp_d3.objects[id].tickFormat(count, format);
        }
      }, this->id, count, format.c_str());

      return *this;
    }

    // .nice()
    QuantizeScale & Nice() {
      MAIN_THREAD_EM_ASM({
        emp_d3.objects[$0].nice()
      }, this->id);
      return *this;
    }

    // .thresholds()
    emp::vector<double> GetThresholds() {
      MAIN_THREAD_EM_ASM({
        emp_i.__outgoing_array = emp_d3.objects[$0].thresholds();
      }, this->id);
      // access JS array
      emp::vector<double> thresholds_vec;
      emp::pass_vector_to_cpp(thresholds_vec);
      return thresholds_vec;
    }
  };

  // scaleQuantile
  class QuantileScale : public ContinuousInputDiscreteOutputScale {
  protected:
    QuantileScale(bool derived) : ContinuousInputDiscreteOutputScale(true) {;}

  public:
    QuantileScale() : ContinuousInputDiscreteOutputScale(true) {
      MAIN_THREAD_EM_ASM({
        emp_d3.objects[$0] = d3.scaleQuantile();
      }, this->id);
    }

    // .quantiles()
    emp::vector<double> GetQuantiles() {
      MAIN_THREAD_EM_ASM({
        emp_i.__outgoing_array = emp_d3.objects[$0].quantiles();
      }, this->id);
      // access JS array
      emp::vector<double> quantiles_vec;
      emp::pass_vector_to_cpp(quantiles_vec);
      return quantiles_vec;
    }
  };

  // scaleThreshold
  class ThresholdScale : public ContinuousInputDiscreteOutputScale {
  protected:
    ThresholdScale(bool derived) : ContinuousInputDiscreteOutputScale(true) {;}

  public:
    ThresholdScale() : ContinuousInputDiscreteOutputScale(true) {
      MAIN_THREAD_EM_ASM({
        emp_d3.objects[$0] = d3.scaleThreshold()
      }, this->id);
    }
  };

  //////////////////////////////////////////////////////////
  ///   Scales with discrete input and discrete output   ///
  //////////////////////////////////////////////////////////
  /// This is a base class to inherit from - it should never be called alone
  class DiscreteScale : public Scale {
  protected:
    DiscreteScale(bool derived) : Scale(true) {;}
    DiscreteScale() : Scale(true) {;}
  };

  // scaleOrdinal
  class OrdinalScale : public DiscreteScale {
  protected:
    OrdinalScale(bool derived) : DiscreteScale(true) {;}

  public:
    OrdinalScale() : DiscreteScale(true) {
      MAIN_THREAD_EM_ASM({
        emp_d3.objects[$0] = d3.scaleOrdinal()
      }, this->id);
    }
  };

  // scaleBand
  class BandScale : public DiscreteScale {
  protected:
    BandScale(bool derived) : DiscreteScale(true) {;}

  public:
    BandScale() : DiscreteScale(true) {
      MAIN_THREAD_EM_ASM({
        emp_d3.objects[$0] = d3.scaleBand()
      }, this->id);
    }

    BandScale & Round(bool val) {
      MAIN_THREAD_EM_ASM({
        emp_d3.objects[$0].round($1);
      }, this->id, val);
      return *this;
    }

    BandScale & SetInnerPadding(double pad) {
      MAIN_THREAD_EM_ASM({
        emp_d3.objects[$0].paddingInner($1);
      }, this->id, pad);
      return *this;
    }

    BandScale & SetOuterPadding(double pad) {
      MAIN_THREAD_EM_ASM({
        emp_d3.objects[$0].paddingOuter($1);
      }, this->id, pad);
      return *this;
    }

    /// A convenience method for setting the inner and outer padding to the same padding value. 
    /// If padding is not specified, returns the inner padding.
    BandScale & SetPadding(double pad) {
      MAIN_THREAD_EM_ASM({
        emp_d3.objects[$0].padding($1);
      }, this->id, pad);
      return *this;
    }

    double GetBandwidth() {
      return MAIN_THREAD_EM_ASM_DOUBLE({
        return emp_d3.objects[$0].bandwidth();
      }, this->id);
    }

    double GetStep() {
      return MAIN_THREAD_EM_ASM_DOUBLE({
        return emp_d3.objects[$0].step();
      }, this->id);
    }
  };

  // scalePoint
  // Point scales are a variant of band scales with the bandwidth fixed to zero
  class PointScale : public DiscreteScale {
  protected:
    PointScale(bool derived) : DiscreteScale(true) {;}

  public:
    PointScale() : DiscreteScale(true) {
      MAIN_THREAD_EM_ASM({
        emp_d3.objects[$0] = d3.scalePoint()
      }, this->id);
    }

    PointScale & Round(bool val) {
      MAIN_THREAD_EM_ASM({
        emp_d3.objects[$0].round($1);
      }, this->id, val);
      return *this;
    } 
    
    PointScale & SetPadding(double pad) {
      MAIN_THREAD_EM_ASM({
        emp_d3.objects[$0].padding($1);
      }, this->id, pad);
      return *this;
    }

    double GetBandwidth() {
      return MAIN_THREAD_EM_ASM_DOUBLE({
        return emp_d3.objects[$0].bandwidth();
      }, this->id);
    }

    double GetStep() {
      return MAIN_THREAD_EM_ASM_DOUBLE({
        return emp_d3.objects[$0].step();
      }, this->id);
    }
  };
}

#endif