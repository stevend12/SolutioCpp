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
// GeometricObjectModel.hpp                                                   //
// Geometric Object Model Class                                               //
// Created June 10, 2017 (Steven Dolly)                                       //
//                                                                            //
// This header file contains a class for a generalized three-dimensional      //
// geometric object model. This is a collection of geometric objects, with    //
// information regarding parent-child relationships.                          //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

// Header guard
#ifndef GEOMETRICOBJECTMODEL_HPP
#define GEOMETRICOBJECTMODEL_HPP

// C++ headers
#include <string>
#include <vector>

// Custom headers
#include "GeometricObject.hpp"

namespace solutio
{
  class GeometricObjectModel
  {
    public:
      virtual void AddGeometricObject(std::string name, GeometricObject &G,
          std::string parent_name);
      void MakeTree();
    protected:
      void AssignParent(std::string parent);
      std::vector<std::string> object_name;
      std::vector<std::string> object_type;
      std::vector<int> object_parent;
      int world_id;
      std::vector<GeometricObject *> object_pointers;
      std::vector< std::vector<int> > object_levels;
  };
}

#endif
