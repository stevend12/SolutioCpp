/******************************************************************************/
/*                                                                            */
/* Copyright 2016-2017 Steven Dolly                                           */
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
// DataInterpolation.hpp                                                      //
// Data Interpolation Functions                                               //
// Created September 27, 2016 (Steven Dolly)                                  //
//                                                                            //
// This header file contains template functions to interpolate one- and two-  //
// dimensional data tables.                                                   //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

// Header guard
#ifndef DATAINTERPOLATION_HPP
#define DATAINTERPOLATION_HPP

// Standard C++ header files
#include <vector>
#include <utility>

// Standard C header files
#include <cmath>

namespace solutio
{
  // Utility function to find index (vector)
  template <class T>
  int FindIndex(const std::vector<T> &axis_data, T value)
  {
    int index = 0;
    if(axis_data[0] < axis_data[1])
    {
      while((index < axis_data.size()) && (value >= axis_data[index])) index++;
    }
    else
    {
      while((index < axis_data.size()) && (value <= axis_data[index])) index++;
    }
    if(index <= 0) index = 1;
    if(index >= axis_data.size()) index = axis_data.size()-1;
    return index;
  }
  // Utility function to find index (pair vector)
  template <class T>
  int FindIndex(const std::vector< std::pair<T,T> > &data, T value)
  {
    int index = 0;
    if(data[0].first < data[1].first)
    {
      while((index < data.size()) && (value >= data[index].first)) index++;
    }
    else
    {
      while((index < data.size()) && (value <= data[index].first)) index++;
    }
    if(index <= 0) index = 1;
    if(index >= data.size()) index = data.size()-1;
    return index;
  }

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
  T LinearInterpolation(const std::vector<T> &x_data, const std::vector<T> &y_data, T x_value)
  {
    int index = FindIndex(x_data, x_value);
    T f = (x_value - x_data[(index-1)]) / (x_data[index] - x_data[(index-1)]);
    T y_value = f*y_data[index] + (1-f)*y_data[(index-1)];
    return y_value;
  }

  // Normal linear interpolation for 2D vector data
  template <class T>
  T LinearInterpolation(const std::vector<T> &x_data, const std::vector<T> &y_data,
      const std::vector< std::vector<T> > &table, T x_value, T y_value)
  {
    int index = FindIndex(x_data, x_value);
    T y_1 = LinearInterpolation(y_data, table[(index-1)], y_value);
    T y_2 = LinearInterpolation(y_data, table[index], y_value);
    T f = (x_value - x_data[(index-1)]) / (x_data[index] - x_data[(index-1)]);
    T t_value = f*y_2 + (1-f)*y_1;
    return t_value;
  }

  // Normal linear interpolation for 1D vector<pair> data
  template <class T>
  T LinearInterpolation(const std::vector< std::pair<T,T> > &data, T x_value)
  {
    int index = FindIndex(data, x_value);
    T f = (x_value - data[(index-1)].first) / (data[index].first - data[(index-1)].first);
    T y_value = f*data[index].second + (1-f)*data[(index-1)].second;
    return y_value;
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
  T LinearInterpolationFast(const std::vector<T> &x_data, const std::vector<T> &y_data,
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
  T LinearInterpolationFast(const std::vector<T> &x_data, const std::vector<T> &y_data,
      const std::vector< std::vector<T> > &table, T x_value, T y_value, T delta_x,
      T delta_y)
  {
    int index = ceil((x_value-x_data[0])/delta_x);
    if(index <= 0) index = 1;
    if(index >= x_data.size()) index = x_data.size()-1;
    T y_1 = LinearInterpolationFast(y_data, table[(index-1)], y_value, delta_y);
    T y_2 = LinearInterpolationFast(y_data, table[index], y_value, delta_y);
    T f = (x_value - x_data[(index-1)]) / (x_data[index] - x_data[(index-1)]);
    T t_value = f*y_2 + (1-f)*y_1;
    return t_value;
  }

  // Fast linear interpolation for 1D vector<pair> data
  template <class T>
  T LinearInterpolationFast(const std::vector< std::pair<T,T> > &data,
    T x_value, T delta_x)
  {
    int index = ceil((x_value-data[0].first)/delta_x);
    T f = (x_value - data[(index-1)].first) / (data[index].first - data[(index-1)].first);
    T y_value = f*data[index].second + (1-f)*data[(index-1)].second;
    return y_value;
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

  // Normal log interpolation for 1D vector data
  template <class T>
  T LogInterpolation(const std::vector<T> &x_data, const std::vector<T> &y_data, T x_value)
  {
    int index = FindIndex(x_data, x_value);
    T f = (log10(x_value) - log10(x_data[(index-1)])) /
        (log10(x_data[index]) - log10(x_data[(index-1)]));
    T y_value = (pow(y_data[index], f) * pow(y_data[(index-1)],(1-f)));
    return y_value;
  }

  // Normal log interpolation for 2D vector data
  template <class T>
  T LogInterpolation(const std::vector<T> &x_data, const std::vector<T> &y_data,
      const std::vector< std::vector<T> > &table, T x_value, T y_value)
  {
    int index = FindIndex(x_data, x_value);
    T y_1 = LogInterpolation(y_data, table[(index-1)], y_value);
    T y_2 = LogInterpolation(y_data, table[index], y_value);
    T f = (log10(x_value) - log10(x_data[(index-1)])) /
        (log10(x_data[index]) - log10(x_data[(index-1)]));
    T t_value = (pow(y_2, f) * pow(y_1, (1-f)));
    return t_value;
  }

};

// End header guard
#endif
