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
//#include "utils.h"

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
    
    public:
        Scale() { ; }

        /// Decoy constructor so we don't construct extra base scales
        ///// make this protected?
        Scale(bool derived){;};

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

        /// Calculate the output for [input], based on the scale's scaling function
        /// Returns an output item with the same type (T2) as the range of the scale
        /// The input value (T) should have the same type as the domain
        // template <typename T, typename T2>
        // T2 ApplyScale(T input) {
        //     return EM_ASM({
        //         return emp_d3.objects[$0]($1);
        //     }, this->id, input);
        // }
        // std::string ApplyScaleString(double input) {
        //     char * buffer = (char *) EM_ASM_INT({
        //         let result = emp_d3.objects[$0]($1);
        //         // console.log(result);
        //         var buffer = Module._malloc(result.length+1);
        //         Module.stringToUTF8(result, buffer, result.length*4+1);
        //         return buffer;
        //         }, this->id, input);
        //     std::string result = std::string(buffer);
        //     free(buffer);
        //     return result;
        // }

         /// Calculate the ouput for [input], based on the scale's scaling function
        std::string ApplyScaleString(double input) {
            EM_ASM({
                const resultStr = emp_d3.objects[$0]($1));
                emp.PassStringToCpp(resultStr);
            }, this->id, input);
            return emp::pass_str_to_cpp();
        }

        std::string ApplyScaleString(int input) {
            EM_ASM({
                const resultStr = emp_d3.objects[$0]($1));
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

        double ApplyScale(double input) {
            return EM_ASM_DOUBLE({
                return emp_d3.objects[$0]($1);
            }, this->id, input);
        }

        double ApplyScale(const std::string & input) {
            return EM_ASM_DOUBLE({
                return emp_d3.objects[$0](UTF8ToString($1));
            }, this->id, input.c_str());
        }

        int ApplyScale(int input) {
            return EM_ASM_INT({
                return emp_d3.objects[$0]($1);
            }, this->id, input);
        }

        int ApplyScale(const std::string & input) {
            return EM_ASM_INT({
                return emp_d3.objects[$0](UTF8ToString($1));
            }, this->id, input.c_str());
        }
        
        //TODO:Getters
    };

    ////////////////////////////////////////////////////////
    /// Scales with continuous input and discrete output ///
    ////////////////////////////////////////////////////////
    class ContinuousInputDiscreteOutputScale : public Scale {
    public: 
        ContinuousInputDiscreteOutputScale() : Scale(true) {;}

        template <typename T>
        double InvertExtent(T y) {
            return EM_ASM_DOUBLE({
                return emp_d3.objects[$0].invertExtent($1);
            }, this->id, y);
        }

    protected: 
        ContinuousInputDiscreteOutputScale(bool derived) : Scale(true) {;}
    };

    class QuantizeScale : public ContinuousInputDiscreteOutputScale { 
    public:
        QuantizeScale() : ContinuousInputDiscreteOutputScale(true) {
            EM_ASM({
                emp_d3.objects[$0] = d3.scaleQuantize();
            }, this->id);
        }
        // template <typename T>
        // double InvertExtent(T y) {
        //     return EM_ASM_DOUBLE({
        //         return emp_d3.objects[$0].invertExtent($1);
        //     }, this->id, y);
        // }

    protected: 
        QuantizeScale(bool derived) : ContinuousInputDiscreteOutputScale(true) {;}

    };

    class QuantileScale : public ContinuousInputDiscreteOutputScale {
    public:
        QuantileScale() : ContinuousInputDiscreteOutputScale(true) { 
            EM_ASM({
                emp_d3.objects[$0] = d3.scaleQuantile();
            }, this->id);
        }

    protected:
        // is there another design pattern besides decoy constructors?
        QuantileScale(bool derived) : ContinuousInputDiscreteOutputScale(true) {;}
        //TODO: .quantiles() -- will return an array of numbers 
    };

    class ThresholdScale : public ContinuousInputDiscreteOutputScale {
    public:
        ThresholdScale() : ContinuousInputDiscreteOutputScale(true) {
            EM_ASM({
                emp_d3.objects[$0] = d3.scaleThreshold()}
            , this->id);
        }

    protected:
        ThresholdScale(bool derived) : ContinuousInputDiscreteOutputScale(true) {;}
    };
    
    //////////////////////////////////////////////////////////
    /// Scales with continuous input and continuous output ///
    //////////////////////////////////////////////////////////
    class ContinuousScale : public Scale {
    public: 
        ContinuousScale() : Scale(true) {;}

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
        ///// How can we make sure this only gets called on the domain?
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
        ContinuousScale & Clamp(bool clamp) {
            EM_ASM({ emp_d3.objects[$0].clamp($1); }, this->id, clamp);
            return *this;
        }


        // .interpolate() (need to pass in an interpolator)

    protected: 
        ContinuousScale(bool derived) : Scale(true) {;}
    };

    class LinearScale : public ContinuousScale {
    public:
        LinearScale() : ContinuousScale(true) {
            EM_ASM({ emp_d3.objects[$0] = d3.scaleLinear(); }, this->id);
        }

    protected:
        LinearScale(bool derived) : ContinuousScale(true) { ; }
    }
    // scaleLinear
    // scaleIdentity
    // scalePow
    // scaleSqrt
    // scaleLog
    // scaleTime
    // scaleSequential
    // .clamping() .nice() .invert()

    /// Scales with discrete input and discrete output
    // scaleOrdinal
    // scaleBand
    // scalePoint
}

#endif