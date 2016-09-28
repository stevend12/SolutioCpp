/******************************************************************************/
/*                                                                            */
/* Copyright 2016 Steven Dolly                                                */
/*                                                                            */
/* Licensed under the Apache License, Version 2.0 (the "License");            */
/* you may not use this file except in compliance with the License.           */
/* You may obtain a copy of the License at:                                   */
/*                                                                            */
/*     http://www.apache.org/licenses/LICENSE-2.0                             */
/*                                                                            */
/* Unless required by applicable law or agreed to in writing, software        */
/* distributed under the License is distributed on an "AS IS" BASIS,          */
/* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.   */
/* See the License for the specific language governing permissions and        */
/* limitations under the License.                                             */
/*                                                                            */
/******************************************************************************/

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// Data Interpolation Header File (DataInterpolation.hpp)                     //
// Created September 27, 2016 (Steven Dolly)                                  //
//                                                                            //
// This header file contains template functions to interpolate one- and two-  //
// dimensional data tables.                                                   //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

// Header guard
#ifndef DATAINTERPOLATION_HPP
#define DATAINTERPOLATION_HPP

// C++ header files
#include <vector>

// C header files
#include <cmath>

namespace solutio
{

  //////////////////////////////////////////////////////////////////////////////
  //                                                                          //
  // Normal linear interpolation: unspecified sample size                     //
  //                                                                          //
  // Normal interpolation for data with unknown and/or irregularly-spaced     //
  // samples. The row and column indices are found by search comparison,      //
  // followed by linear interpolation. Various dimensions (e.g. 1D, 2D) and   //
  // data containers (e.g. array, vector, pair) are included in the template  //
  // functions).                                                              //
  //                                                                          //
  //////////////////////////////////////////////////////////////////////////////

  // Normal linear interpolation for 1D vector data
  template <class T>
  T LinearInterpolation(std::vector<T> x_data, std::vector<T> y_data, T x_value)
  {
    int index = 0;
    while((index < x_data.size()) && (x_value >= x_data[index])) index++;
    if(index <= 0) index = 1;
    if(index >= x_data.size()) index = x_data.size()-1;
    T f = (x_value - x_data[(index-1)]) / (x_data[index] - x_data[(index-1)]);
    T y_value = f*y_data[index] + (1-f)*y_data[(index-1)];
    return y_value;
  }
  
  // Normal linear interpolation for 2D vector data
  template <class T>
  T LinearInterpolation(std::vector<T> x_data, std::vector<T> y_data,
      std::vector< std::vector<T> > table, T x_value, T y_value)
  {
    int index = 0;
    while((index < x_data.size()) && (x_value >= x_data[index])) index++;
    if(index <= 0) index = 1;
    if(index >= x_data.size()) index = x_data.size()-1;
    float y_1 = LinearInterpolation(y_data, table[(index-1)], y_value);
    float y_2 = LinearInterpolation(y_data, table[index], y_value);
    T f = (x_value - x_data[(index-1)]) / (x_data[index] - x_data[(index-1)]);
    T t_value = f*y_2 + (1-f)*y_1;
    return t_value;
  }
  
  //////////////////////////////////////////////////////////////////////////////
  //                                                                          //
  // Fast linear interpolation: specified sample size                         //
  //                                                                          //
  // Fast interpolation for data known and regularly-spaced sample size. The  //
  // row and column indices calculated from the given row and column spacing, //
  // followed by linear interpolation. Various dimensions (e.g. 1D, 2D) and   //
  // data containers (e.g. array, vector, pair) are included in the template  //
  // functions).                                                              //
  //                                                                          //
  //////////////////////////////////////////////////////////////////////////////

  // Fast linear interpolation for 1D vector data
  template <class T>
  T LinearInterpolationFast(std::vector<T> x_data, std::vector<T> y_data,
      T x_value, T delta_x)
  {
    int index = ceil((x_value-x_data[0])/delta_x);
    if(index <= 0) index = 1;
    if(index >= x_data.size()) index = x_data.size()-1;
    T f = (x_value - x_data[(index-1)]) / (x_data[index] - x_data[(index-1)]);
    T y_value = f*y_data[index] + (1-f)*y_data[(index-1)];
    return y_value;
  }
  
  // Fast linear interpolation for 2D vector data
  template <class T>
  T LinearInterpolationFast(std::vector<T> x_data, std::vector<T> y_data,
      std::vector< std::vector<T> > table, T x_value, T y_value, T delta_x,
      T delta_y)
  {
    int index = ceil((x_value-x_data[0])/delta_x);
    if(index <= 0) index = 1;
    if(index >= x_data.size()) index = x_data.size()-1;
    float y_1 = LinearInterpolationQuick(y_data, table[(index-1)],
        y_value, delta_y);
    float y_2 = LinearInterpolationQuick(y_data, table[index],
        y_value, delta_y);
    T f = (x_value - x_data[(index-1)]) / (x_data[index] - x_data[(index-1)]);
    T t_value = f*y_2 + (1-f)*y_1;
    return t_value;
  }
  
  //////////////////////////////////////////////////////////////////////////////
  //                                                                          //
  // Normal logarithmic interpolation: unspecified sample size                //
  //                                                                          //
  // Normal interpolation for data with unknown and/or irregularly-spaced     //
  // samples. The row and column indices are found by search comparison,      //
  // followed by logarithmic interpolation. Various dimensions (e.g. 1D, 2D)  //
  // and data containers (e.g. array, vector, pair) are included in the       //
  // template functions).                                                     //
  //                                                                          //
  //////////////////////////////////////////////////////////////////////////////
  
  template <class T>
  T LogInterpolation(std::vector<T> x_data, std::vector<T> y_data, T x_value)
  {
    int index = 0;
    while((index < x_data.size()) && (x_value >= x_data[index])) index++;
    if(index <= 0) index = 1;
    if(index >= x_data.size()) index = x_data.size()-1;
    T f = (log10(x_value) - log10(x_data[(index-1)])) /
        (log10(x_data[index]) - log10(x_data[(index-1)]));
    T value_y = (pow(y_data[index], f) * pow(y_data[(index-1)],(1-f)));
    return value_y;
  }
  
};
  
// End header guard
#endif
