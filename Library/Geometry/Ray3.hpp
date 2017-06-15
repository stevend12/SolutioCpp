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
// Ray3.hpp                                                                   //
// 3D Ray Class Header File                                                   //
// Created June 1, 2017 (Steven Dolly)                                        //
//                                                                            //
// This header file contains a class for a double-precision three-dimensional //
// ray, with simple math operation functions.                                 //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

// Header guard
#ifndef RAY3_HPP
#define RAY3_HPP

// Standard C header files
#include <cmath>

// Custom headers
#include "Vec3.hpp"

namespace solutio
{
  class Ray3
  {
    public:
      // Default constructor (set to all zeros)
      Ray3();
      // Constructor with origin and direction setter
      Ray3(Vec3<double> o, Vec3<double> d);
      // Ray origin & direction
      Vec3<double> origin, direction;
      // Set functions
      void SetRay(Vec3<double> o, Vec3<double> d);
      // Get functions
      Vec3<double> GetPoint(double t);
      double GetLength();
  };
}

#endif
