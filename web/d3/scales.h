#ifndef __SCALES_H__
#define __SCALES_H__

#include "d3_init.h"
#include "utils.h"

#include "../../emtools/js_utils.h"

namespace D3 {

  //Base class to inherit from - should never be made stand-alone
  class Scale : public D3_Base {
  protected:
    Scale(int id) : D3_Base(id){;};

  public:
    Scale() {;}

    //Decoy constructor so we don't construct extra base scales
    Scale(bool derived){;};

    template <typename T, size_t SIZE>
    void SetRange(std::array<T,SIZE> values) {
      emp::pass_array_to_javascript(values);
      EM_ASM_ARGS({js.objects[$0].range(emp_i.__incoming_array);}, this->id);
    }

    template <typename T, size_t SIZE>
    void SetDomain(std::array<T,SIZE> values) {
      emp::pass_array_to_javascript(values);
      EM_ASM_ARGS({js.objects[$0].domain(emp_i.__incoming_array);}, this->id);
    }

    Scale Copy() {
      int new_id = EM_ASM_INT_V({return js.objects.length});
      EM_ASM_ARGS({
	  js.objects.push(js.objects[$0].copy());
	}, this->id);
      return Scale(new_id);
    }

    double ApplyScale(double input) {
      //TODO: make this work for other types
      return EM_ASM_DOUBLE({return js.objects[$0]($1);},this->id, input);
    }
    //TODO:Getters

  };


  class QuantizeScale : public Scale {
  public:
    QuantizeScale() : Scale(true) {EM_ASM_ARGS({js.objects[$0]=d3.scale.quantize()},this->id);}
    QuantizeScale(bool derived) : Scale(true) {;}

    template <typename T>
    double InvertExtent(T y) {
      return EM_ASM_DOUBLE({return js.objects[$0].invertExtent($1);},
			   this->id, y);
    }
  };

  class QuantileScale : public QuantizeScale {
  public:
    QuantileScale() : QuantizeScale(true) { EM_ASM_ARGS({js.objects[$0] = d3.scale.quantile();}, this->id);}
    QuantileScale(bool derived) : QuantizeScale(true) {;}
    //TODO: Quantiles()
  };

  class ThresholdScale : public QuantizeScale {
  public:
    ThresholdScale() : QuantizeScale(true) {
      EM_ASM_ARGS({js.objects[$0] = d3.scale.threshold()}, this->id);
    }
    ThresholdScale(bool derived) : QuantizeScale(true) {;}
  };

  class IdentityScale : public Scale {
  public:
    IdentityScale() : Scale(true) {
      EM_ASM_ARGS({js.objects[$0] = d3.scale.identity();}, this->id);
    }

    IdentityScale(bool derived) : Scale(true){;}

    template <typename T>
    double Invert(T y) {
      return EM_ASM_DOUBLE({return js.objects[$0].invert($1);}, this->id, y);
    }

    void SetTicks(int count) {
      EM_ASM_ARGS({js.objects[$0].ticks($1);}, this->id, count);
    }

    void SetTickFormat(int count, std::string format) {
      //TODO: format is technically optional, but what is the point of this
      //function without it?
      EM_ASM_ARGS({js.objects[$0].tick($1, Pointer_stringify($2));},
		  this->id, count, format.c_str());
    }
  };

  class LinearScale : public IdentityScale {
  public:
    LinearScale() : IdentityScale(true) {
      EM_ASM_ARGS({js.objects[$0] = d3.scale.linear();}, this->id);
    }

    LinearScale(bool derived) : IdentityScale(true) {;}

    template <typename T, size_t SIZE>
    void SetRangeRound(std::array<T,SIZE> values) {
      emp::pass_array_to_javascript(values);
      EM_ASM_ARGS({js.objects[$0].rangeRound(emp.__incoming_array);}, this->id);
    }

    void SetInterpolate(std::string factory) {
      D3_CALLBACK_METHOD_1_ARG(interpolate, factory.c_str())
    }

    void Clamp(bool clamp) {
      EM_ASM_ARGS({js.objects[$0].clamp($1);}, this->id, clamp);
    }

    void Nice(int count = -1) {
      if (count != -1){
	    EM_ASM_ARGS({js.objects[$0].nice($1);}, this->id, count);
      } else {
	    EM_ASM_ARGS({js.objects[$0].nice();}, this->id);
      }
    }

  };

  class LogScale : public LinearScale {
  public:
    LogScale() : LinearScale(true) {
      EM_ASM_ARGS({js.objects[$0] = d3.scale.log();}, this->id);
    }

    LogScale(bool derived) : LinearScale(true){;};

  };

  class PowScale : public LinearScale {
  public:
    PowScale() : LinearScale(true) {
      EM_ASM_ARGS({js.objects[$0] = d3.scale.pow();}, this->id);
    }

    PowScale(bool derived) : LinearScale(true){;};

    //TODO: Exponent()
    //TODO Sqrt constructor
  };

  class TimeScale : public LinearScale {
  public:
    TimeScale() : LinearScale(true) {
      EM_ASM_ARGS({js.objects[$0] = d3.scale.time();}, this->id);
    }

    TimeScale(bool derived) : LinearScale(true){;};
  };

  class OrdinalScale : public QuantizeScale {
  public:
    OrdinalScale() : QuantizeScale(true) {
      EM_ASM({js.objects[$0]= d3.scale.ordinal();}, this->id);
    }

    OrdinalScale(bool derived) : QuantizeScale(true){;}
  };


  class Category10Scale : D3_Base{
  public:
    Category10Scale() {
      EM_ASM({js.objects[$0] = d3.scale.category10();}, this->id);
    }
  };

  class Category20Scale : D3_Base {
  public:
    Category20Scale() {
      EM_ASM({js.objects[$0] = d3.scale.category20();}, this->id);
    }
  };

  class Category20bScale : D3_Base {
  public:
    Category20bScale() {
      EM_ASM({js.objects[$0] = d3.scale.category20b();}, this->id);
    }
  };

  class Category20cScale : D3_Base {
  protected:
    int id;
  public:
    Category20cScale() {
      EM_ASM({js.objects[$0] = d3.scale.category20c();}, this->id);
    }
  };
}

#endif
