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
// Ray3.cpp                                                                   //
// 3D Ray Class Main File                                                     //
// Created June 2, 2017 (Steven Dolly)                                        //
//                                                                            //
// This main file contains a class for a double-precision three-dimensional   //
// ray, with simple math operation functions.                                 //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

// Class header
#include "Ray3.hpp"

namespace solutio
{
  // Default constructor (set to all zeros)
  Ray3::Ray3()
  {
    Vec3<double> o, d;
    SetRay(o, d);
  }
  // Constructor with origin and direction setter
  Ray3::Ray3(Vec3<double> o, Vec3<double> d)
  {
    SetRay(o, d);
  }
  // Set functions
  void Ray3::SetRay(Vec3<double> o, Vec3<double> d)
  {
    origin = o;
    direction = d;
  }
  // Get functions
  Vec3<double> Ray3::GetPoint(double t)
  {
    Vec3<double> point;
    point.x = origin.x + t*direction.x;
    point.y = origin.y + t*direction.y;
    point.z = origin.z + t*direction.z;
    return point;
  }
  
  double Ray3::GetLength()
  {
    return direction.Magnitude();
  }
}
