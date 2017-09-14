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
  
  std::vector< std::pair<int, double> > GeometricObjectModel::CalcRayPathlength(Ray3 ray)
  {
    int parent_id;
    double length;
    std::vector<double> pathlengths;
    std::vector<int> ray_object_ids;
    std::vector<bool> ray_intersect(object_parent.size());
    std::vector< std::pair<int, double> > intersection_list;
    std::pair<int, double> list_entry;
    
    // Check world first
    length = object_pointers[0]->RayPathlength(ray);
    if (length < 1e-10)
    {
      list_entry.first = -1;
      list_entry.second = 0.0;
      intersection_list.push_back(list_entry);
      return intersection_list;
    }
    else
    {
      pathlengths.push_back(ray.direction.Magnitude());
      ray_object_ids.push_back(world_id);
      ray_intersect[0] = true;
    }
    // Loop for each subsequent level
    for(int m = 1; m < object_levels.size(); m++)
    {
      for(int n = 0; n < object_levels[m].size(); n++)
      {
        if(!ray_intersect[(object_parent[(object_levels[m][n])])]) continue;
        // Check if ray intersects with any children
        length = object_pointers[(object_levels[m][n])]->RayPathlength(ray);
        
        // Save material IDs and path lengths for children, subtract pathlengths
        // from parents
        if(length > 1.0e-10)
        {
          pathlengths.push_back(length);
          ray_object_ids.push_back((object_levels[m][n]));
          ray_intersect[(object_levels[m][n])] = true;
          
          parent_id = 0;
          while(object_parent[(object_levels[m][n])] !=
              ray_object_ids[parent_id]) parent_id++;
          pathlengths[parent_id] -= length;
          
        }
        else { ray_intersect[(object_levels[m][n])] = false; }
      }
    }
    for(int n = 0; n < pathlengths.size(); n++){
      list_entry.first = ray_object_ids[n];
      list_entry.second = pathlengths[n];
      intersection_list.push_back(list_entry);
    }
    
    return intersection_list;
  }
}
