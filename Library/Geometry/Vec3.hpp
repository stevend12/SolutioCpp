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
// Vec3.hpp                                                                   //
// 3D Vector Class                                                            //
// Created May 31, 2017 (Steven Dolly)                                        //
//                                                                            //
// This header file contains a template class for a generalized               //
// three-dimensional vector, with simple math operation functions.            //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

// Header guard
#ifndef VEC3_HPP
#define VEC3_HPP

// Standard C header files
#include <cmath>

namespace solutio
{
  template <class T>
  class Vec3
  {
    public:
      // Default constructor (set to all zeros)
      Vec3();
      // Constructor with component setter
      Vec3(T ix, T iy, T iz);
      // Vector components
      T x, y, z;
      // Set function
      void Set(T ix, T iy, T iz){ x = ix; y = iy; z = iz; };
      // Overloaded operators
      void operator=(const Vec3 &v)
      { 
        x = v.x;
        y = v.y;
        z = v.z;
      }
      // Simple math functions
	  void Scale(T factor){ x *= factor; y *= factor; z *= factor; };
	  T Magnitude();
	  void Normalize();
  };

  template <class T>
  Vec3<T>::Vec3()
  {
    x = y = z = 0;
  }

  template <class T>
  Vec3<T>::Vec3(T ix, T iy, T iz)
  {
    Set(ix, iy, iz);
  }

  template <class T>
  T Vec3<T>::Magnitude()
  {
    return sqrt(pow(x,2) + pow(y,2) + pow(z,2));
  }
  
  template <class T>
  void Vec3<T>::Normalize()
  {
    T factor = Magnitude();
    x /= factor;
    y /= factor;
    z /= factor;
  };

  // Overloaded operators for Vec3 class
  template <class T>
  Vec3<T> operator+(const Vec3<T> &a, const Vec3<T> &b)
  {
    Vec3<T> sum;
    sum.x = a.x + b.x;
    sum.y = a.y + b.y;
    sum.z = a.z + b.z;
    return sum;
  }
  
  template <class T>
  Vec3<T> operator-(const Vec3<T> &a, const Vec3<T> &b)
  {
    Vec3<T> diff;
    diff.x = a.x - b.x;
    diff.y = a.y - b.y;
    diff.z = a.z - b.z;
    return diff;
  }
  
  template <class T>
  Vec3<T> operator*(const Vec3<T> &a, const T &b)
  {
    Vec3<T> prod;
    prod.x = a.x*b;
    prod.y = a.y*b;
    prod.z = a.z*b;
    return prod;
  }

  // Special vector functions for vector class
  template <class T>
  T Dot(const Vec3<T> &a, const Vec3<T> &b)
  {
    T d = a.x*b.x + a.y*b.y + a.z*b.z;
    return d;
  }

  template <class T>
  Vec3<T> Cross(const Vec3<T> &a, const Vec3<T> &b)
  {
    Vec3<T> c;
    c.x = a.y*b.z - a.z*b.y;
    c.y = a.z*b.x - a.x*b.z;
    c.z = a.x*b.y - a.y*b.x;
    return c;
  }
}

#endif
