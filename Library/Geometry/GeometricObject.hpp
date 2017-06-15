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
// GeometricObject.hpp                                                        //
// Geometric Object Parent Class                                              //
// Created June 2, 2017 (Steven Dolly)                                        //
//                                                                            //
// This header file contains a class for a generalized three-dimensional      //
// geometric object, and is used as a parent class from which to derive all   //
// specific geometric objects (e.g. cylinder, sphere).                        //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

// Header guard
#ifndef GEOMETRICOBJECT_HPP
#define GEOMETRICOBJECT_HPP

// Custom headers
#include "Ray3.hpp"

namespace solutio
{
  class GeometricObject
  {
    public:
      virtual double RayPathlength(Ray3 ray){ return 0.0; };
    protected:
      Vec3<double> centroid;
      double volume;
  };
}

#endif
