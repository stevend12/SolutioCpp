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
// ObjectModelXray.hpp                                                        //
// X-ray Imaging Object Model Class                                           //
// Created June 12, 2017 (Steven Dolly)                                       //
//                                                                            //
// This header file contains a class for a three-dimensional geometric object //
// model, with specific application for x-ray imaging (e.g. radiography, CT). //
// This is a geometric object model (derived from the GeometricObjectModel    //
// class), with additional information on x-ray attenuation coefficients.     //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

// Header guard
#ifndef OBJECTMODELXRAY_HPP
#define OBJECTMODELXRAY_HPP

// Custom headers
#include "Geometry/GeometricObjectModel.hpp"
#include "Physics/NistPad.hpp"

namespace solutio
{
  class ObjectModelXray : public GeometricObjectModel
  {
    public:
      // Add a NistPad material
      void AddMaterial(std::string folder, std::string name);
      void AddMaterial(std::string folder, std::string name,
          std::string new_name);
      void AddMaterial(std::string folder, std::string name,
          std::string new_name, float new_density);
      // Assign material to geometric object
      void AssignMaterial(std::string material);
      // Add object to list
      void AddObject(std::string name, GeometricObject &G,
          std::string parent_name, std::string material_name);
      // Create preset lists of attenuation coefficients
      void TabulateAttenuationLists(std::vector<double> energies,
          std::vector<double> spectrum);
      bool IsListTabulated();
      // Get fractional photon ray attenuation through object model
      double GetRayAttenuation(Ray3 ray, std::vector<double> spectrum);
      //
      void Print();
    private:
      std::vector<std::string> object_material_name;
      std::vector<int> object_material_id;
      std::vector<NistPad> MuData;
      std::vector<double> tabulated_energies;
      std::vector< std::vector<double> > tabulated_mu_lists;
  };
}

#endif
