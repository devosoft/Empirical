#include "utils.h"

namespace D3{

  class Scale{
  protected:
    int id;
    Scale(int id);

  public:
    Scale();
    
    template <typename T, size_t SIZE>
    void SetRange(std::array<T,SIZE> values){
      PassArrayToJavascript(values);
      EM_ASM_ARGS({js.objects[$0].range(emp.__incoming_array);}, this->id);
    }

    template <typename T, size_t SIZE>
    void SetDomain(std::array<T,SIZE> values){
      PassArrayToJavascript(values);
      EM_ASM_ARGS({js.objects[$0].domain(emp.__incoming_array);}, this->id);
    }

    Scale Copy(){
      int new_id = EM_ASM_INT_V({return js.objects.length});
      EM_ASM_ARGS({
	  js.objects.push(js.objects[$0].copy());
	}, this->id);
      return Scale(new_id);
    }

    //TODO:Getters
  };

  Scale::Scale(int id){
    this->id = id;
  }

  Scale::Scale(){
    this->id = EM_ASM_INT_V({return js.objects.length});
    EM_ASM({js.objects.push(d3.scale)});
  }

  class QuantizeScale : public Scale {
  public:
    QuantizeScale();

    template <typename T>
    double InvertExtent(T y){
      return EM_ASM_DOUBLE({return js.objects[$0].invertExtent($1);}, 
			   this->id, y);
    }
  };

  QuantizeScale::QuantizeScale(){
    this->id = EM_ASM_INT_V({return js.objects.length});
    EM_ASM({js.objects.push(d3.scale.quantize())});
  }

  class QuantileScale : public QuantizeScale {
  public:
    QuantileScale();
    
    //TODO: Quantiles()
  };

  QuantileScale::QuantileScale(){
    this->id = EM_ASM_INT_V({return js.objects.length});
    EM_ASM({js.objects.push(d3.scale.quantile())});
  }

  class ThresholdScale : public QuantizeScale {
  public:
    ThresholdScale();
  };

  ThresholdScale::ThresholdScale(){
    this->id = EM_ASM_INT_V({return js.objects.length});
    EM_ASM({js.objects.push(d3.scale.threshold())});
  }

  class IdentityScale : public Scale {
  public:
    IdentityScale();

    template <typename T>
    double Invert(T y){
      return EM_ASM_DOUBLE({return js.objects[$0].invert($1);}, this->id, y);
    }

    void SetTicks(int count){
      EM_ASM_ARGS({js.objects[$0].ticks($1);}, this->id, count);
    }

    void SetTickFormat(int count, const char * format){
      //TODO: format is technically optional, but what is the point of this
      //function without it?
      EM_ASM_ARGS({js.objects[$0].tick($1, Pointer_stringify($2));}, 
		  this->id, count, format);
    }
  };

  IdentityScale::IdentityScale(){
    this->id = EM_ASM_INT_V({return js.objects.length});
    EM_ASM({js.objects.push(d3.scale.identity())});
  }

  class LinearScale : public IdentityScale {
  public:
    LinearScale();

    template <typename T, size_t SIZE>
    void SetRangeRound(std::array<T,SIZE> values){
      PassArrayToJavascript(values);
      EM_ASM_ARGS({js.objects[$0].rangeRound(emp.__incoming_array);}, this->id);
    }

    void SetInterpolate(const char * factory){
      CALL_FUNCTION_THAT_ACCEPTS_FUNCTION_1_ARG(interpolate, factory)
    }

    void Clamp(bool clamp){
      EM_ASM_ARGS({js.objects[$0].clamp($1);}, this->id, clamp);
    }

    void Nice(int count = -1){
      if (count != -1){
	EM_ASM_ARGS({js.objects[$0].nice($1);}, this->id, count);
      } else {
	EM_ASM_ARGS({js.objects[$0].nice();}, this->id);
      }
    }

  };

  LinearScale::LinearScale(){
    this->id = EM_ASM_INT_V({return js.objects.length});
    EM_ASM({js.objects.push(d3.scale.linear())});
  }

  class LogScale : public LinearScale {
  public:
    LogScale();
  };

  LogScale::LogScale(){
    this->id = EM_ASM_INT_V({return js.objects.length});
    EM_ASM({js.objects.push(d3.scale.log())});
  }

  class PowScale : public LinearScale {
  public:
    PowScale();

    //TODO: Exponent()
    //TODO Sqrt constructor
  };

  PowScale::PowScale(){
    this->id = EM_ASM_INT_V({return js.objects.length});
    EM_ASM({js.objects.push(d3.scale.pow())});
  }

  class TimeScale : public LinearScale {
  public:
    TimeScale();
  };

  TimeScale::TimeScale(){
    this->id = EM_ASM_INT_V({return js.objects.length});
    EM_ASM({js.objects.push(d3.scale.time())});
  }

  class OrdinalScale : public QuantizeScale {
  public:
    OrdinalScale();
  };

  OrdinalScale::OrdinalScale(){
    this->id = EM_ASM_INT_V({return js.objects.length});
    EM_ASM({js.objects.push(d3.scale.ordinal())});
  }

  class Category10Scale{
  protected:
    int id;
  public:
    Category10Scale();
  };

  Category10Scale::Category10Scale(){
    this->id = EM_ASM_INT_V({return js.objects.length});
    EM_ASM({js.objects.push(d3.scale.category10())});
  }

  class Category20Scale{
  protected:
    int id;
  public:
    Category20Scale();
  };

  Category20Scale::Category20Scale(){
    this->id = EM_ASM_INT_V({return js.objects.length});
    EM_ASM({js.objects.push(d3.scale.category20())});
  }


  class Category20bScale{
  protected:
    int id;
  public:
    Category20bScale();
  };

  Category20bScale::Category20bScale(){
    this->id = EM_ASM_INT_V({return js.objects.length});
    EM_ASM({js.objects.push(d3.scale.category20b())});
  }

  class Category20cScale{
  protected:
    int id;
  public:
    Category20cScale();
  };

  Category20cScale::Category20cScale(){
    this->id = EM_ASM_INT_V({return js.objects.length});
    EM_ASM({js.objects.push(d3.scale.category20c())});
  }


}
