#ifndef EMP_D3_HISTOGRAM_H
#define EMP_D3_HISTOGRAM_H

#include "d3_init.hpp"
#include "dataset.hpp"
#include "../../base/vector.hpp"
#include "../../datastructs/tuple_struct.hpp"
#include "../js_utils.hpp"

namespace D3 {

    struct HistogramBin {
        EMP_BUILD_INTROSPECTIVE_TUPLE( int, x0,
                                       int, x1,
                                       int, length

        )
    };

    class Histogram : public D3::D3_Base {
    protected:
        uint32_t value_fun_id = -1;
    public:
        Histogram(){
            MAIN_THREAD_EM_ASM({js.objects[$0] = d3.histogram();}, this->id);
        };

        ~Histogram() {
            emp::JSDelete(value_fun_id);
        }

        Histogram& SetDomain(double x, double y) {
            MAIN_THREAD_EM_ASM({js.objects[$0].domain([$1, $2]);}, this->id, x, y);
            return (*this);
        }

        Histogram& Domain(double x, double y) {
            return SetDomain(x, y);
        }

        emp::array<double, 2> GetDomain() {
            MAIN_THREAD_EM_ASM({emp_i.__outgoing_array = js.objects[$0].domain();}
                        , this->id);
            emp::array<double, 2> domain;
            emp::pass_array_to_cpp(domain);
            return domain;
        }

        emp::array<double, 2> Domain() {
            return GetDomain();
        }

        Histogram& SetThresholds(int count) {
            MAIN_THREAD_EM_ASM({js.objects[$0].thresholds($1);}, this->id, count);
            return (*this);
        }

        Histogram& SetThresholds(std::string threshold_generator) {
            MAIN_THREAD_EM_ASM({
                js.objects[$0].thresholds(UTF8ToString($1));
            }, this->id, threshold_generator.c_str());
            return (*this);
        }

        template <typename T>
        Histogram& Thresholds(T thresh) {
            return SetThresholds(thresh);
        }


        template <typename DATA_TYPE>
        Dataset Call(emp::vector<DATA_TYPE> data) {
            emp::pass_array_to_javascript(data);
            Dataset bins = Dataset();
            MAIN_THREAD_EM_ASM({
                console.log(emp_i.__incoming_array);
                js.objects[$1] = js.objects[$0](emp_i.__incoming_array);
                console.log(js.objects[$1]);
            }, this->id, bins.GetID());

            return bins;
        }

        template <typename DATA_TYPE>
        Dataset operator()(emp::vector<DATA_TYPE> data) {
            return Call(data);
        }

        /// @cond TEMPLATES
        template <typename T>
        emp::sfinae_decoy<Histogram, decltype(&T::operator())>
        SetValueAccessor(T func) {
          value_fun_id = JSWrap(func, emp::to_string(id)+"_return_value");
          MAIN_THREAD_EM_ASM({
              js.objects[$0].value(window["emp"][$0+"_return_value"]);
          }, this->id);
          return (*this);
        }
        /// @endcond

        Histogram& SetValueAccessor(std::string func) {
            D3_CALLBACK_METHOD_1_ARG(value, func.c_str());
            return (*this);
        }

        template <typename T>
        Histogram& Value(T func) {
            return SetValueAccessor(func);
        }
    };
}

#endif
