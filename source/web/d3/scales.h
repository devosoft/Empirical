/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2016-2020
 *
 *  @file  scales.h
 *  @brief Tools for scaling graph axes in D3.
 */


#ifndef __EMP_D3_SCALES_H__
#define __EMP_D3_SCALES_H__

#include "d3_init.h"

#include <iostream>
#include <string>
#include <typeinfo>
#include <map>
#include <array>
#include "../../base/vector.h"
#include "../../base/array.h"
#include "utils.h"

#include "../../base/assert.h"
#include "../js_utils.h"
#include "../JSWrap.h"

namespace D3 {

  /// Scales in D3 are functions that take input values and map them to output based on
  /// a scaling function. They are often used to map data values to x, y coordinates in pixels
  /// describing where on the screen elements should be placed.
  /// This is a base class to inherit from - it should never be made stand-alone
  class Scale : public D3_Base {
  protected:
    Scale(int id) : D3_Base(id) { ; };

    /// Decoy constructor so we don't construct extra base scales
    Scale(bool derived){;};
  
  public:
    Scale() { ; }

    /// Set the domain of possible input values corresponding to values in the range
    /// Note that an array of strings can be passed in here
    template <typename T, size_t SIZE>
    Scale & SetDomain(emp::array<T, SIZE> values) {
      emp::pass_array_to_javascript(values);
      EM_ASM({
        emp_d3.objects[$0].domain(emp_i.__incoming_array);
      }, this->id);
      return *this;
    }

    // template<typename T>
    // typename std::enable_if<std::is_integral<T>::value, Scale &>::type
    Scale & SetDomain(double min, double max) {
      EM_ASM({
        emp_d3.objects[$0].domain([$1, $2]);
      }, this->id, min, max);
      return *this;
    }

    /// Set the range of possible output values corresponding to values in the domain. 
    /// Output for values in between will be interpolated with a function determined 
    /// by the type of the scale.
    /// Note that an array of strings can be passed in here
    template <typename T, size_t SIZE>
    Scale & SetRange(emp::array<T, SIZE> values) {
      emp::pass_array_to_javascript(values);
      EM_ASM({
        emp_d3.objects[$0].range(emp_i.__incoming_array);
      }, this->id); 
      return *this;
    }

    // template<typename T>
    // typename std::enable_if<std::is_integral<T>::value, Scale &>::type
    Scale & SetRange(double min, double max) {
      EM_ASM({
        emp_d3.objects[$0].range([$1, $2]);
      }, this->id, min, max);
      return *this;
    }

    /// Make a copy of this scale
    Scale Copy() {
      int new_id = EM_ASM_INT({
        return emp_d3.objects.next_id++;
      });
      EM_ASM({
        emp_d3.objects[$1] = emp_d3.objects[$0].copy();
      }, this->id, new_id);
      return Scale(new_id);
    }

    std::string ApplyScaleString(double input) {
      EM_ASM({
        const resultStr = emp_d3.objects[$0]($1);
        emp.PassStringToCpp(resultStr);
      }, this->id, input);
      return emp::pass_str_to_cpp();
    }


    std::string ApplyScaleString(int input) {
      EM_ASM({
        const resultStr = emp_d3.objects[$0]($1);
        emp.PassStringToCpp(resultStr);
      }, this->id, input);
      return emp::pass_str_to_cpp();
    }

    std::string ApplyScaleString(const std::string & input) {
      EM_ASM({
        const resultStr = emp_d3.objects[$0](UTF8ToString($1));
        emp.PassStringToCpp(resultStr);
      }, this->id, input.c_str());
      return emp::pass_str_to_cpp();
    }

    double ApplyScaleDouble(double input) {
      return EM_ASM_DOUBLE({
        return emp_d3.objects[$0]($1);
      }, this->id, input);
    }

    double ApplyScaleDouble(int input) {
      return EM_ASM_DOUBLE({
        return emp_d3.objects[$0]($1);
      }, this->id, input);
    }

    double ApplyScaleDouble(const std::string & input) {
      return EM_ASM_DOUBLE({
        return emp_d3.objects[$0](UTF8ToString($1));
      }, this->id, input.c_str());
    }

    int ApplyScaleInt(double input) {
      return EM_ASM_INT({
        return emp_d3.objects[$0]($1);
      }, this->id, input);
    }

    int ApplyScaleInt(int input) {
      return EM_ASM_INT({
        return emp_d3.objects[$0]($1);
      }, this->id, input);
    }

    int ApplyScaleInt(const std::string & input) {
      return EM_ASM_INT({
        return emp_d3.objects[$0](UTF8ToString($1));
      }, this->id, input.c_str());
    }
    
    //TODO:Getters
  };

  //////////////////////////////////////////////////////////
  /// Scales with continuous input and continuous output ///
  //////////////////////////////////////////////////////////
  class ContinuousScale : public Scale {
  protected: 
    ContinuousScale(bool derived) : Scale(true) {;}
  
  public: 
    ContinuousScale() : Scale(true) {;}
    
    // Invert is only supported if the range is numeric. If the range is not numeric, returns NaN
    template <typename T>
    double Invert(T y) {
      return EM_ASM_DOUBLE({
        return emp_d3.objects[$0].invert($1);
      }, this->id, y);
    }

    // .ticks()
    ContinuousScale & SetTicks(int count) {
      EM_ASM_ARGS({js.objects[$0].ticks($1);}, this->id, count);
      return *this;
    }

    // .tickFormat()
    ContinuousScale & SetTickFormat(int count, const std::string & format) {
    //TODO: format is technically optional, but what is the point of this
    //function without it?
      EM_ASM({
        emp_d3.objects[$0].tickFormat($1, UTF8ToString($2));
      }, this->id, count, format.c_str());
      return *this;
    }

    // .nice()
    /// Extend the domain so that it start and ends on nice values
    /// Nicing a scale only modifies the current domain
    Scale & MakeNice() {
      EM_ASM({ 
        emp_d3.objects[$0].nice()
      }, this->id);
      return *this;
    }

    // Sets the scale’s range to the specified array of values 
    // while also setting the scale’s interpolator to interpolateRound
    template <typename T, size_t SIZE>
    ContinuousScale & SetRangeRound(emp::array<T,SIZE> values) {
      emp::pass_array_to_javascript(values);
      EM_ASM({
        emp_d3.objects[$0].rangeRound(emp.__incoming_array);
      }, this->id);
      return *this;
    }

    ContinuousScale & SetRangeRound(double min, double max) {
      EM_ASM({
        emp_d3.objects[$0].rangeRound([$1, $2]);
      }, this->id, min, max);
      return *this;
    }

    // Enables or disables clamping accordingly
    ContinuousScale & SetClamp(bool clamp) {
      EM_ASM({ emp_d3.objects[$0].clamp($1); }, this->id, clamp);
      return *this;
    }


    // .interpolate() (need to pass in an interpolator)
    ContinuousScale & SetInterpolate(const std::string & interpolatorName) {
      // note: this doesn't allow you to specify arguments to a d3.interpolator function
      EM_ASM({
        const id = $0;
        const interpolator_str = UTF8ToString($1);
        var sel = emp_d3.find_function(interpolator_str);
        emp_d3.objects[id].interpolate(sel);
      }, this->id, interpolatorName.c_str());

      return *this;
    }

    // .unknown()
    ContinuousScale & SetUnkown(double value) {
      EM_ASM({
        emp_d3.objects[$0].unknown($1);
      }, this->id, value);
      return *this;
    }

    ContinuousScale & SetUnkown(int value) {
      EM_ASM({
        emp_d3.objects[$0].unknown($1);
      }, this->id, value);
      return *this;
    }

    ContinuousScale & SetUnkown(const std::string & value) {
      EM_ASM({
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
      EM_ASM({ emp_d3.objects[$0] = d3.scaleLinear(); }, this->id);
    }
  };

  // scalePow
  class PowScale : public ContinuousScale {
  protected:
    PowScale(bool derived) : ContinuousScale(true) { ; }
  
  public:
    PowScale() : ContinuousScale(true) {
      EM_ASM({ emp_d3.objects[$0] = d3.scalePow(); }, this->id);
    }

    PowScale & SetExponent(double ex) {
      EM_ASM({ emp_d3.objects[$0].exponent($1);}, this->id, ex);
      return *this;
    }
    // PowScale & SqrtScale() {
    //   EM_ASM({ emp_d3.objects[$0].exponent(0.5);}, this->id);
    //   return *this;
    //   // return PowScale().SetExponent(0.5);
    // }
  };

  // scaleSqrt
  // A convenience scale to set the exponent to 0.5 in a PowScale
  class SqrtScale : public ContinuousScale {
  protected:
    SqrtScale(bool derived) : ContinuousScale(true) { ; }
  
  public:
    SqrtScale() : ContinuousScale(true) {
      EM_ASM({ emp_d3.objects[$0] = d3.scaleSqrt(); }, this->id);
    } 
  };
    

  // scaleLog
  class LogScale : public ContinuousScale {
  protected:
    LogScale(bool derived) : ContinuousScale(true) { ; }
  
  public:
    LogScale() : ContinuousScale(true) {
      EM_ASM({ emp_d3.objects[$0] = d3.scaleLog(); }, this->id);
    }

    LogScale & SetBase(double baseNum) {
      EM_ASM({ emp_d3.objects[$0].base($1);}, this->id, baseNum);
      return *this;
    }
  };

  // scaleSymlog
  class SymlogScale : public ContinuousScale {
  protected:
    SymlogScale(bool derived) : ContinuousScale(true) { ; }
  
  public:
    SymlogScale() : ContinuousScale(true) {
      EM_ASM({ emp_d3.objects[$0] = d3.scaleSymlog(); }, this->id);
    }

    SymlogScale & SetConstant(double constant) {
      EM_ASM({ emp_d3.objects[$0].constant($1);}, this->id, constant);
      return *this;
    }
  };

  // scaleIdentity
  class IdentityScale : public ContinuousScale {
  protected:
    IdentityScale(bool derived) : ContinuousScale(true) { ; }
  
  public:
    IdentityScale() : ContinuousScale(true) {
      EM_ASM({ emp_d3.objects[$0] = d3.scaleIdentity(); }, this->id);
    }

    // get rid of functions that shouldn't be called:
    // Identity scales do not support rangeRound, clamp or interpolate
    template <typename T, size_t SIZE>
    ContinuousScale & SetRangeRound(emp::array<T,SIZE> values) = delete;
    ContinuousScale &SetRangeRound(double min, double max) = delete;
    ContinuousScale & SetClamp(bool clamp) = delete;
    ContinuousScale & SetInterpolate(const std::string & interpolatorName) = delete;
  };  

  // scaleRadial
  // class RadialScale : public ContinuousScale {
  // protected:
  //   RadialScale(bool derived) : ContinuousScale(true) { ; }
  
  // public:
  //   RadialScale() : ContinuousScale(true) {
  //     EM_ASM({ emp_d3.objects[$0] = d3.scaleRadial(); }, this->id);
  //   }

  //   // get rid of functions that shouldn't be called:
  //   // Radial scales do not support interpolate
  //   ContinuousScale & SetInterpolate(const std::string & interpolatorName) = delete;
  // };  

  // scaleTime
  class TimeScale : public ContinuousScale {
  protected:
    TimeScale(bool derived) : ContinuousScale(true) { ; }
  
  public:
    TimeScale() : ContinuousScale(true) {
      EM_ASM({ emp_d3.objects[$0] = d3.scaleTime(); }, this->id);
    }

    // get rid of functions that shouldn't be called 
    template <typename T, size_t SIZE>
    Scale & SetDomain(emp::array<T, SIZE> values) = delete;
    Scale & SetDomain(double min, double max) = delete;
    double ApplyScaleDouble(double input) = delete;
    double ApplyScaleDouble(int input) = delete;
    double ApplyScaleDouble(const std::string & input) = delete;
    int ApplyScaleInt(double input) = delete;
    int ApplyScaleInt(int input) = delete;
    int ApplyScaleInt(const std::string & input) = delete;
    template <typename T>
    double Invert(T y) = delete;
    
    // A struct to deal with dates that mimics the JS Date object
    struct Date {
      int year;
      int month;
      int day;
      int hours;
      int minutes;
      int seconds;
      int milliseconds; 

      // note that month should be passed in 0 indexed to keep consistent with JavaScript (0 = January)
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

    // special SetDomain to deal with Dates
    TimeScale & SetDomain(const Date & dateMin, const Date & dateMax) {
      EM_ASM({
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

    double ApplyScaleDouble(const Date & dateInput) {
      return EM_ASM_DOUBLE({
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

    // special Invert for dates
    Date InvertDate(double input) {
      EM_ASM({
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
  
  // scaleSequential
  // is used for mapping continuous values to an output range 
  // determined by a preset (or custom) interpolator
  // the input domain and output range of a sequential scale always has exactly two elements,
  // and the output range is typically specified as an interpolator rather than an array of values
  class SequentialScale : public ContinuousScale {
  protected:
    SequentialScale(bool derived) : ContinuousScale(true) { ; }
  
  public:
    SequentialScale() : ContinuousScale(true) {
      EM_ASM({ emp_d3.objects[$0] = d3.scaleSequential(); }, this->id);
    }

    // get rid of functions that shouldn't be called:
    // Identity scales do not support invert or interpolate
    template <typename T>
    double Invert(T y) = delete;
    ContinuousScale & SetInterpolate(const std::string & interpolatorName) = delete;
  
    
    // .interpolator
    SequentialScale & SetInterpolator(const std::string & interpolatorName) {
      // note: this doesn't allow you to specify arguments to a d3.interpolator function
      EM_ASM({
        const id = $0;
        const interpolator_str = UTF8ToString($1);
        var sel = emp_d3.find_function(interpolator_str);
        emp_d3.objects[id].interpolator(sel);
      }, this->id, interpolatorName.c_str());

      return *this;
    }
  };

  // scaleSequentialLog
  class SequentialLogScale : public SequentialScale {
  protected:
    SequentialLogScale(bool derived) : SequentialScale(true) { ; }
  public:
    SequentialLogScale() : SequentialScale(true) {
      EM_ASM({ emp_d3.objects[$0] = d3.scaleSequentialLog(); }, this->id);
    }
  };

  // scaleSequentialPow
  class SequentialPowScale : public SequentialScale {
  protected:
    SequentialPowScale(bool derived) : SequentialScale(true) { ; }
  public:
    SequentialPowScale() : SequentialScale(true) {
      EM_ASM({ emp_d3.objects[$0] = d3.scaleSequentialPow(); }, this->id);
    }
  };

  // scaleSequentialSqrt
  class SequentialSqrtScale : public SequentialScale {
  protected:
    SequentialSqrtScale(bool derived) : SequentialScale(true) { ; }
  public:
    SequentialSqrtScale() : SequentialScale(true) {
      EM_ASM({ emp_d3.objects[$0] = d3.scaleSequentialSqrt(); }, this->id);
    }
  };

  // scaleSequentialSymlog
  class SequentialSymlogScale : public SequentialScale {
  protected:
    SequentialSymlogScale(bool derived) : SequentialScale(true) { ; }
  public:
    SequentialSymlogScale() : SequentialScale(true) {
      EM_ASM({ emp_d3.objects[$0] = d3.scaleSequentialSymlog(); }, this->id);
    }
  };

  // scaleSequentialQuantile
  class SequentialQuantileScale : public SequentialScale {
  protected:
    SequentialQuantileScale(bool derived) : SequentialScale(true) { ; }
  public:
    SequentialQuantileScale() : SequentialScale(true) {
      EM_ASM({ emp_d3.objects[$0] = d3.scaleSequentialQuantile(); }, this->id);
    }

    // .quantiles  
    // Returns an array of n + 1 quantiles. For example, if n = 4, returns an array of five numbers: 
    // the minimum value, the first quartile, the median, the third quartile, and the maximum.
    emp::vector<double> GetQuantiles(const int n) {
      EM_ASM({
        emp_i.__outgoing_array = emp_d3.objects[$0].quantiles($1);
      }, this->id, n);
      // access JS array
      emp::vector<double> quantile_vector;
      emp::pass_vector_to_cpp(quantile_vector);
      return quantile_vector;
    }
  };


  // scaleDiverging
  // map a continuous, numeric input domain to a continuous output range. 
  // However, unlike continuous scales, the input domain and output range of a diverging scale always has exactly three elements,
  // and the output range is typically specified as an interpolator rather than an array of values
  class DivergingScale : public ContinuousScale {
  protected:
    DivergingScale(bool derived) : ContinuousScale(true) { ; }
  
  public:
    DivergingScale() : ContinuousScale(true) {
      EM_ASM({ emp_d3.objects[$0] = d3.scaleSequential(); }, this->id);
    }

    // get rid of functions that shouldn't be called:
    // Identity scales do not support invert or interpolate
    template <typename T>
    double Invert(T y) = delete;
    ContinuousScale & SetInterpolate(const std::string & interpolatorName) = delete;
  
    
    // .interpolator
    DivergingScale & SetInterpolator(const std::string & interpolatorName) {
      // note: this doesn't allow you to specify arguments to a d3.interpolator function
      EM_ASM({
        const id = $0;
        const interpolator_str = UTF8ToString($1);
        var sel = emp_d3.find_function(interpolator_str);
        emp_d3.objects[id].interpolator(sel);
      }, this->id, interpolatorName.c_str());

      return *this;
    }
  };

  // scaleDivergingLog
  class DivergingLogScale : public DivergingScale {
  protected:
    DivergingLogScale(bool derived) : DivergingScale(true) { ; }
  public:
    DivergingLogScale() : DivergingScale(true) {
      EM_ASM({ emp_d3.objects[$0] = d3.scaleDivergingLog(); }, this->id);
    }
  };

  // scaleDivergingPow
  class DivergingPowScale : public DivergingScale {
  protected:
    DivergingPowScale(bool derived) : DivergingScale(true) { ; }
  public:
    DivergingPowScale() : DivergingScale(true) {
      EM_ASM({ emp_d3.objects[$0] = d3.scaleDivergingPow(); }, this->id);
    }
  };

  // scaleDivergingSqrt
  class DivergingSqrtScale : public DivergingScale {
  protected:
    DivergingSqrtScale(bool derived) : DivergingScale(true) { ; }
  public:
    DivergingSqrtScale() : DivergingScale(true) {
      EM_ASM({ emp_d3.objects[$0] = d3.scaleDivergingSqrt(); }, this->id);
    }
  };

  // scaleDivergingSymlog
  class DivergingSymlogScale : public DivergingScale {
  protected:
    DivergingSymlogScale(bool derived) : DivergingScale(true) { ; }
  public:
    DivergingSymlogScale() : DivergingScale(true) {
      EM_ASM({ emp_d3.objects[$0] = d3.scaleDivergingSymlog(); }, this->id);
    }
  };

  
  ////////////////////////////////////////////////////////
  /// Scales with continuous input and discrete output ///
  ////////////////////////////////////////////////////////
  class ContinuousInputDiscreteOutputScale : public Scale {
  protected: 
    ContinuousInputDiscreteOutputScale(bool derived) : Scale(true) {;}
      
  public: 
    ContinuousInputDiscreteOutputScale() : Scale(true) {;}

    // needs to be fixed to return array and take in string
    template <typename T>
    double InvertExtent(T y) {
      return EM_ASM_DOUBLE({
        return emp_d3.objects[$0].invertExtent($1);
      }, this->id, y);
    }
  };

  // scaleQuantize
  // ticks tickformat nice thresholds
  class QuantizeScale : public ContinuousInputDiscreteOutputScale { 
  protected: 
    QuantizeScale(bool derived) : ContinuousInputDiscreteOutputScale(true) {;}
      
  public:
    QuantizeScale() : ContinuousInputDiscreteOutputScale(true) {
      EM_ASM({
        emp_d3.objects[$0] = d3.scaleQuantize();
      }, this->id);
    }
  };

  // scaleQuantile
  class QuantileScale : public ContinuousInputDiscreteOutputScale {
  protected:
    // is there another design pattern besides decoy constructors?
    QuantileScale(bool derived) : ContinuousInputDiscreteOutputScale(true) {;}
      
  public:
    QuantileScale() : ContinuousInputDiscreteOutputScale(true) { 
      EM_ASM({
        emp_d3.objects[$0] = d3.scaleQuantile();
      }, this->id);
    }
    //TODO: .quantiles() -- will return an array of numbers 
  };

  // scaleThreshold
  // no quantiles
  class ThresholdScale : public ContinuousInputDiscreteOutputScale {
  protected:
    ThresholdScale(bool derived) : ContinuousInputDiscreteOutputScale(true) {;}

  public:
    ThresholdScale() : ContinuousInputDiscreteOutputScale(true) {
      EM_ASM({
        emp_d3.objects[$0] = d3.scaleThreshold()
      }, this->id);
    }
  };

  //////////////////////////////////////////////////////////
  ///   Scales with discrete input and discrete output   ///
  //////////////////////////////////////////////////////////
  class DiscreteScale : public Scale {
  protected: 
    DiscreteScale(bool derived) : Scale(true) {;}
  
  public: 
    DiscreteScale() : Scale(true) {;}

    // get rid of functions that shouldn't be called 
    Scale & SetDomain(double min, double max) = delete;

    DiscreteScale & SetDomain(int min, int max) {
      EM_ASM({
        emp_d3.objects[$0].domain([$1, $2]);
      }, this->id, min, max);
      return *this;
    }
  };

  // scaleOrdinal
  class OrdinalScale : public DiscreteScale {
  protected:
    OrdinalScale(bool derived) : DiscreteScale(true) {;}

  public:
    OrdinalScale() : DiscreteScale(true) {
      EM_ASM({
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
      EM_ASM({
        emp_d3.objects[$0] = d3.scaleBand()
      }, this->id);
    }
  };

  // scalePoint
  class PointScale : public DiscreteScale {
  protected:
    PointScale(bool derived) : DiscreteScale(true) {;}

  public:
    PointScale() : DiscreteScale(true) {
      EM_ASM({
        emp_d3.objects[$0] = d3.scalePoint()
      }, this->id);
    }
  };
}

#endif