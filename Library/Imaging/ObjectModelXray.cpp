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
// ObjectModelXray.cpp                                                        //
// X-ray Imaging Object Model Class                                           //
// Created June 12, 2017 (Steven Dolly)                                       //
//                                                                            //
// This header file contains a class for a three-dimensional geometric object //
// model, with specific application for x-ray imaging (e.g. radiography, CT). //
// This is a geometric object model (derived from the GeometricObjectModel    //
// class), with additional information on x-ray attenuation coefficients.     //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

// Class header
#include "ObjectModelXray.hpp"

// C++ headers
#include <iostream>

namespace solutio
{
  void ObjectModelXray::AddMaterial(std::string folder, std::string name)
  {
    NistPad NewMat(folder, name);
    MuData.push_back(NewMat);
  }

  void ObjectModelXray::AddMaterial(std::string folder, std::string name,
      std::string new_name)
  {
    NistPad NewMat(folder, name);
    NewMat.Rename(new_name);
    MuData.push_back(NewMat);
  }

  void ObjectModelXray::AddMaterial(std::string folder, std::string name,
      std::string new_name, float new_density)
  {
    NistPad NewMat(folder, name);
    NewMat.Rename(new_name);
    NewMat.ForceDensity(new_density);
    MuData.push_back(NewMat);
  }
  
  void ObjectModelXray::AssignMaterial(std::string material)
  {
    bool found = false;
    for(int n = 0; n < MuData.size(); n++)
    {
      if(material == MuData[n].get_name())
      {
        object_material_name.push_back(MuData[n].get_name());
        object_material_id.push_back(n);
        found = true;
        break;
      }
    }
    if(!found) std::cout << "Error: could not find element/material!\n";
  }
  
  void ObjectModelXray::AddObject(std::string name, GeometricObject &G,
      std::string parent_name, std::string material_name)
  {
    AddGeometricObject(name, G, parent_name);
    AssignMaterial(material_name);
  }
  
  void ObjectModelXray::TabulateAttenuationLists(std::vector<double> energies,
      std::vector<double> spectrum)
  {
    for(int e = 0; e < energies.size(); e++)
    {
      tabulated_energies.push_back(energies[e]);
    }
    for(int n = 0; n < MuData.size(); n++)
    {
      std::vector<double> current_list;
      for(int e = 0; e < energies.size(); e++)
      {
        if(spectrum[e] == 0.0)
        {
          current_list.push_back(0.0);
          continue;
        }
        else
        {
          current_list.push_back(MuData[n].LinearAttenuation(energies[e]));
        }
      }
      tabulated_mu_lists.push_back(current_list);
    }
  }
  
  bool ObjectModelXray::IsListTabulated()
  {
    return (tabulated_mu_lists.size() != 0);
  }

  double ObjectModelXray::GetRayAttenuation(Ray3 ray,
      std::vector<double> spectrum)
  {
    int parent_id;
    double length;
    std::vector<double> pathlengths;
    std::vector<int> ray_object_ids;
    std::vector<int> ray_materials;
    std::vector<bool> ray_intersect(object_parent.size());
  
    // Start at outermost level (the "world")
    pathlengths.push_back(ray.direction.Magnitude());
    ray_object_ids.push_back(world_id);
    ray_materials.push_back(object_material_id[world_id]);
    ray_intersect[0] = true;
  
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
        if(length > 1.0e-6)
        {
          pathlengths.push_back(length);
          ray_object_ids.push_back((object_levels[m][n]));
          ray_materials.push_back(object_material_id[(object_levels[m][n])]);
          ray_intersect[(object_levels[m][n])] = true;
          
          parent_id = 0;
          while(object_parent[(object_levels[m][n])] !=
              ray_object_ids[parent_id]) parent_id++;
          pathlengths[parent_id] -= length;
          
        }
        else { ray_intersect[(object_levels[m][n])] = false; }
      }
    }
    // Sum up path lengths and attenuation coefficients
    double total_sum = 0.0, energy, mu;
    for(int e = 0; e < spectrum.size(); e++){
      if(spectrum[e] == 0.0) continue;
      double energy_sum = 0.0;
      for(int n = 0; n < pathlengths.size(); n++){
        if(!IsListTabulated())
        {
          energy_sum += (MuData[(ray_materials[n])].LinearAttenuation(double(e)) * pathlengths[n]);
        }
        else
        {
          energy_sum += (tabulated_mu_lists[(ray_materials[n])][e] * pathlengths[n]);
        }
      }
      total_sum += (spectrum[e] * exp(-energy_sum));
    }
  
    return total_sum;
  }
  
  void ObjectModelXray::Print()
  {
    std::cout << "Materials\n";
    std::cout << "---------\n";
    for(int n = 0; n < MuData.size(); n++)
    {
      std::cout << (n+1) << ") " << MuData[n].get_name() << '\n';
    }
    std::cout << "\nObjects\n";
    std::cout << "-------\n";
    for(int n = 0; n < object_name.size(); n++)
    {
      std::cout << (n+1) << ") " << object_name[n] << '\n';
      std::cout << "Type: " << object_type[n] << '\n';
      std::cout << "Material: " << object_material_name[n] << '\n';
      std::cout << "Parent ID: " << object_parent[n] << '\n';
      std::cout << '\n';
    }
    for(int m = 0; m < object_levels.size(); m++)
    {
      std::cout << m << '\t';
      for(int n = 0; n < object_levels[m].size(); n++)
      {
        std::cout << object_name[(object_levels[m][n])] << '\t';
      }
      std::cout << '\n';
    }
  }
}
