/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2016-2018
 *
 *  @file  scales.hpp
 *  @brief Tools for scaling graph axes in D3.
 */

#ifndef EMP_D3_SCALES_H
#define EMP_D3_SCALES_H

#include "d3_init.hpp"
#include "utils.hpp"

#include "../js_utils.hpp"

namespace D3 {

  /// Scales in D3 are functions that take input values and map them to output based on
  /// a scaling function. They are often used to map data calues to x, y coordinates in pixels
  /// describing where on the screen elements should be placed.
  /// This is a base class to inherit from - should never be made stand-alone
  class Scale : public D3_Base {
  protected:
    Scale(int id) : D3_Base(id){;};

  public:
    Scale() {;}

    /// Decoy constructor so we don't construct extra base scales
    Scale(bool derived){;};

    /// Set the output values corresponding to values in the domain. Output for values in between
    /// will be interpolated with a function determined by the type of the scale.
    /// Array should contain same number of elements as the one used to set the domain.
    template <typename T, size_t SIZE>
    Scale& SetRange(emp::array<T,SIZE> values) {
      emp::pass_array_to_javascript(values);
      MAIN_THREAD_EMP_ASM({js.objects[$0].range(emp_i.__incoming_array);}, this->id);
      return *this;
    }

    Scale& SetRange(double min, double max) {
      MAIN_THREAD_EMP_ASM({js.objects[$0].range([$1, $2]);}, this->id, min, max);
      return *this;
    }

    /// Set the input values corresponding to values in the range.
    /// Array should contain same number of elements as the one used to set the range.
    template <typename T, size_t SIZE>
    Scale& SetDomain(emp::array<T,SIZE> values) {
      emp::pass_array_to_javascript(values);
      MAIN_THREAD_EMP_ASM({js.objects[$0].domain(emp_i.__incoming_array);}, this->id);
      return *this;
    }

    Scale& SetDomain(double min, double max) {
      MAIN_THREAD_EMP_ASM({js.objects[$0].domain([$1, $2]);}, this->id, min, max);
      return *this;
    }

    /// Make a copy of this scale
    Scale Copy() {
      int new_id = EM_ASM_INT_V({return js.objects.next_id++});
      MAIN_THREAD_EMP_ASM({
	    js.objects[$1] = js.objects[$0].copy();
    }, this->id, new_id);
      return Scale(new_id);
    }

    /// Calculate the ouput for [input], based on the scale's scaling function
    double ApplyScale(double input) {
      //TODO: make this work for other types
      return EM_ASM_DOUBLE({return js.objects[$0]($1);},this->id, input);
    }

    int ApplyScale(int input) {
      return EM_ASM_INT({return js.objects[$0]($1);},this->id, input);
    }

    /// Calculate the ouput for [input], based on the scale's scaling function
    std::string ApplyScaleString(double input) {
      //TODO: make this work for other types
      char * buffer = (char *) EM_ASM_INT({
        result = js.objects[$0]($1);
        // console.log(result);
        var buffer = Module._malloc(result.length+1);
	      Module.stringToUTF8(result, buffer, result.length*4+1);
	      return buffer;

      },this->id, input);

      std::string result = std::string(buffer);
      free(buffer);
      return result;

    }


    //TODO:Getters

  };


  class QuantizeScale : public Scale {
  public:
    QuantizeScale() : Scale(true) {MAIN_THREAD_EMP_ASM({js.objects[$0]=d3.scaleQuantize()},this->id);}
    QuantizeScale(bool derived) : Scale(true) {;}

    template <typename T>
    double InvertExtent(T y) {
      return EM_ASM_DOUBLE({return js.objects[$0].invertExtent($1);},
			   this->id, y);
    }
  };

  class QuantileScale : public QuantizeScale {
  public:
    QuantileScale() : QuantizeScale(true) { MAIN_THREAD_EMP_ASM({js.objects[$0] = d3.scaleQuantile();}, this->id);}
    QuantileScale(bool derived) : QuantizeScale(true) {;}
    //TODO: Quantiles()
  };

  class ThresholdScale : public QuantizeScale {
  public:
    ThresholdScale() : QuantizeScale(true) {
      MAIN_THREAD_EMP_ASM({js.objects[$0] = d3.scaleThreshold()}, this->id);
    }
    ThresholdScale(bool derived) : QuantizeScale(true) {;}
  };

  class IdentityScale : public Scale {
  public:
    IdentityScale() : Scale(true) {
      MAIN_THREAD_EMP_ASM({js.objects[$0] = d3.scaleIdentity();}, this->id);
    }

    IdentityScale(bool derived) : Scale(true){;}

    template <typename T>
    double Invert(T y) {
      return EM_ASM_DOUBLE({return js.objects[$0].invert($1);}, this->id, y);
    }

    IdentityScale& SetTicks(int count) {
      MAIN_THREAD_EMP_ASM({js.objects[$0].ticks($1);}, this->id, count);
      return *this;
    }

    IdentityScale& SetTickFormat(int count, std::string format) {
      //TODO: format is technically optional, but what is the point of this
      //function without it?
      MAIN_THREAD_EMP_ASM({js.objects[$0].tick($1, UTF8ToString($2));},
		  this->id, count, format.c_str());
      return *this;
    }
  };

  class LinearScale : public IdentityScale {
  public:
    LinearScale() : IdentityScale(true) {
      MAIN_THREAD_EMP_ASM({js.objects[$0] = d3.scaleLinear();}, this->id);
    }

    LinearScale(bool derived) : IdentityScale(true) {;}

    template <typename T, size_t SIZE>
    LinearScale& SetRangeRound(emp::array<T,SIZE> values) {
      emp::pass_array_to_javascript(values);
      MAIN_THREAD_EMP_ASM({js.objects[$0].rangeRound(emp.__incoming_array);}, this->id);
      return *this;
    }

    LinearScale& SetRangeRound(double min, double max) {
      MAIN_THREAD_EMP_ASM({js.objects[$0].rangeRound([$1, $2]);}, this->id, min, max);
      return *this;
    }


    LinearScale& SetInterpolate(std::string factory) {
      D3_CALLBACK_METHOD_1_ARG(interpolate, factory.c_str())
      return *this;
    }

    LinearScale& Clamp(bool clamp) {
      MAIN_THREAD_EMP_ASM({js.objects[$0].clamp($1);}, this->id, clamp);
      return *this;
    }

    LinearScale& Nice(int count = -1) {
      if (count != -1){
	    MAIN_THREAD_EMP_ASM({js.objects[$0].nice($1);}, this->id, count);
      } else {
	    MAIN_THREAD_EMP_ASM({js.objects[$0].nice();}, this->id);
      }
      return *this;
    }

  };

  class LogScale : public LinearScale {
  public:
    LogScale() : LinearScale(true) {
      MAIN_THREAD_EMP_ASM({js.objects[$0] = d3.scaleLog();}, this->id);
    }

    LogScale(bool derived) : LinearScale(true){;};

  };

  class PowScale : public LinearScale {
  public:
    PowScale() : LinearScale(true) {
      MAIN_THREAD_EMP_ASM({js.objects[$0] = d3.scalePow();}, this->id);
    }

    PowScale(bool derived) : LinearScale(true){;};

    PowScale& Exponent(double ex) {
        MAIN_THREAD_EMP_ASM({js.objects[$0].exponent($1);}, this->id, ex);
        return *this;
    }
  };

  PowScale SqrtScale() {
      return PowScale().Exponent(.5);
  }

  class TimeScale : public LinearScale {
  public:
    TimeScale() : LinearScale(true) {
      MAIN_THREAD_EMP_ASM({js.objects[$0] = d3.scaleTime();}, this->id);
    }

    TimeScale(bool derived) : LinearScale(true){;};
  };

  class OrdinalScale : public QuantizeScale {
  public:
    OrdinalScale() : QuantizeScale(true) {
      EM_ASM({js.objects[$0]= d3.scaleOrdinal();}, this->id);
    }

    OrdinalScale(bool derived) : QuantizeScale(true){;}
  };


  class Category10Scale : D3_Base{
  public:
    Category10Scale() {
      EM_ASM({js.objects[$0] = d3.scaleCategory10();}, this->id);
    }
  };

  class Category20Scale : D3_Base {
  public:
    Category20Scale() {
      EM_ASM({js.objects[$0] = d3.scaleCategory20();}, this->id);
    }
  };

  class Category20bScale : D3_Base {
  public:
    Category20bScale() {
      EM_ASM({js.objects[$0] = d3.scaleCategory20b();}, this->id);
    }
  };

  class Category20cScale : D3_Base {
  protected:
    int id;
  public:
    Category20cScale() {
      EM_ASM({js.objects[$0] = d3.scaleCategory20c();}, this->id);
    }
  };
}

#endif
