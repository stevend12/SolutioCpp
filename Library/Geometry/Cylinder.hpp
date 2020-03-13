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
// Cylinder.hpp                                                               //
// Cylinder Geometric Object Class                                            //
// Created June 2, 2017 (Steven Dolly)                                        //
//                                                                            //
// This header file contains a class for a three-dimensional cylindrical      //
// geometric object, including basic geometric definitions and ray            //
// intersection calculations.                                                 //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#ifndef CYLINDER_HPP
#define CYLINDER_HPP

// Custom headers
#include "Vec3.hpp"
#include "Ray3.hpp"
#include "GeometricObject.hpp"

#ifndef M_PI
#define M_PI (3.14159265358979323846)
#endif

namespace solutio
{
  class Cylinder : public GeometricObject
  {
    public:
      // Constructor and setter
      Cylinder(Vec3<double> c, double r, double h);
      // Get functions
      Vec3<double> GetCentroid(){ return centroid; }
      double GetRadius(){ return radius; }
      double GetHeight(){ return height; }
      // Calc functions
      double CalcVolume();
      double RayPathlength(Ray3 ray);
    private:
      double radius;
      double height;
  };
}

#endif
