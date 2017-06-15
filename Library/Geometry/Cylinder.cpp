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
// Cylinder.cpp                                                               //
// Cylinder Geometric Object Class                                            //
// Created June 2, 2017 (Steven Dolly)                                        //
//                                                                            //
// This main file contains a class for a three-dimensional cylindrical        //
// geometric object, including basic geometric definitions and ray            //
// intersection calculations.                                                 //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

// Class header
#include "Cylinder.hpp"

// C Headers
#include <cmath>

namespace solutio
{
  Cylinder::Cylinder(Vec3<double> c, double r, double h)
  {
    centroid = c;
    radius = r;
    height = h;
  }
  
  double Cylinder::CalcVolume()
  {
    volume = M_PI*pow(radius,2.0)*height;
    return volume;
  }
  
  double Cylinder::RayPathlength(Ray3 ray)
  {
    double solution[2];
    double L = ray.direction.Magnitude();
    double q_a = pow(ray.direction.x,2) + pow(ray.direction.y,2);
    double q_b = 2*(ray.direction.x*ray.origin.x + ray.direction.y*ray.origin.y);
    double q_c = pow(ray.origin.x,2) + pow(ray.origin.y,2) - pow(radius,2);
    double q_check = pow(q_b,2) - 4*q_a*q_c;
    if(q_check >= 0)
    {
      solution[0] = (-q_b + sqrt(q_check)) / (2*q_a);
      solution[1] = (-q_b - sqrt(q_check)) / (2*q_a);
      return (L * fabs(solution[0]-solution[1]));
    }
    else return 0.0;
  }
}
