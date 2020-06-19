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

    
    /// Calculate the ouput for [input], based on the scale's scaling function
    // template <typename T, std::enable_if<std::is_integral<T>::value, int>>
    // std::string ApplyScaleString(T input) {
    //     EM_ASM({
    //         const resultStr = emp_d3.objects[$0]($1);
    //         emp.PassStringToCpp(resultStr);
    //     }, this->id, input);
    //     return emp::pass_str_to_cpp();
    // }

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
      return EM_ASM_INT({
        return emp_d3.objects[$0]($1);
      }, this->id, input);
    }

    double ApplyScaleDouble(const std::string & input) {
      return EM_ASM_DOUBLE({
        return emp_d3.objects[$0](UTF8ToString($1));
      }, this->id, input.c_str());
    }

    int ApplyScaleInt(double input) {
      return EM_ASM_DOUBLE({
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
    // do i need to make a separate invert that returns ints?
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

    // LinearScale& SetInterpolate(std::string factory) {
    // D3_CALLBACK_METHOD_1_ARG(interpolate, factory.c_str())
    // return *this;
    // }
    
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

    // scaleSqrt
    // A convenience function to set the exponent to 0.5
    PowScale & SqrtScale() {
      EM_ASM({ emp_d3.objects[$0].exponent(0.5);}, this->id);
      return *this;
      // return PowScale().SetExponent(0.5);
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

    LogScale & Base(double baseNum) {
      EM_ASM({ emp_d3.objects[$0].base($1);}, this->id, baseNum);
      return *this;
    }
  };

  // scaleIdentity
  // Note that Identity scales do not support rangeRound, clamp or interpolate
  class IdentityScale : public ContinuousScale {
  protected:
    IdentityScale(bool derived) : ContinuousScale(true) { ; }
  
  public:
    IdentityScale() : ContinuousScale(true) {
      EM_ASM({ emp_d3.objects[$0] = d3.scaleIdentity(); }, this->id);
    }
  };

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

  // scaleRadial
  // Note that radial scales do not support interpolate
  class RadialScale : public ContinuousScale {
  protected:
    RadialScale(bool derived) : ContinuousScale(true) { ; }
  
  public:
    RadialScale() : ContinuousScale(true) {
      EM_ASM({ emp_d3.objects[$0] = d3.scaleRadial(); }, this->id);
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

  // scaleSequential
  // no invert and interpolate
  // delete invert extent

  // scaleDiverging
  // no invert and interpolate
  // delete invert extent


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