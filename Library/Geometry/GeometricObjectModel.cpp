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
// GeometricObjectModel.cpp                                                   //
// Geometric Object Model Class                                               //
// Created June 10, 2017 (Steven Dolly)                                       //
//                                                                            //
// This main file contains a class for a generalized three-dimensional        //
// geometric object model. This is a collection of geometric objects, with    //
// information regarding parent-child relationships.                          //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

// Class header
#include "GeometricObjectModel.hpp"

// C++ headers
#include <iostream>

namespace solutio
{
  void GeometricObjectModel::AssignParent(std::string parent)
  {
    bool found = false;
    if(parent == "None")
    {
      object_parent.push_back(-1);
      world_id = object_name.size()-1;
    }
    else
    {
      for(int n = 0; n < object_name.size(); n++)
      {
        if(parent == object_name[n])
        {
          object_parent.push_back(n);
          found = true;
          break;
        }
      }
      if(!found) std::cout << "Error: could not find parent!\n";
    }
  }
  void GeometricObjectModel::AddGeometricObject(std::string name, GeometricObject &G,
      std::string parent_name)
  {
    object_name.push_back(name);
    object_type.push_back("NA");
    AssignParent(parent_name);
    object_pointers.push_back(&G);
  }
  // Create tree structure given all parent-child object relations
  void GeometricObjectModel::MakeTree()
  {
    int level, num_levels = 1, new_parent;
    // Determine number of levels
    for(int n = 0; n < object_name.size(); n++)
    {
      if(n == world_id) continue;
      else
      {
        level = 2;
        new_parent = object_parent[n];
        while(new_parent != world_id)
        {
          new_parent = object_parent[new_parent];
          level++;
        }
        if(level > num_levels) num_levels = level;
      }
    }
    // Start with outermost level
    std::vector<int> level1;
    level1.push_back(world_id);
    object_levels.push_back(level1);
    // Fill in remaining levels
    for(int m = 1; m < num_levels; m++)
    {
      std::vector<int> current_level;
      for(int n = 0; n < object_name.size(); n++)
      {
        if(n == world_id) continue;
        else
        {
          level = 2;
          new_parent = object_parent[n];
          while(new_parent != world_id)
          {
            new_parent = object_parent[new_parent];
            level++;
          }
          if((level-1) == m) current_level.push_back(n);
        }
      }
      object_levels.push_back(current_level);
    }
  }
}
