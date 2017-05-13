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
// NistPad.hpp                                                                //
// NIST Photon Attenuation Data (NISTPAD) Manager Class                       //
// Created September 28, 2016 (Steven Dolly)                                  //
//                                                                            //
// This file contains the header for the class which loads and stores photon  //
// attenuation data from NIST. Data from any point on the table is accessed   //
// using logarithmic interpolation.                                           //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

// Header guard
#ifndef NISTPAD_HPP
#define NISTPAD_HPP

// Standard C++ headers
#include <string>
#include <vector>

namespace solutio
{
  class NistPad
  {
    public:
      // Default constructor and destructor
      NistPad();
      ~NistPad();
      // Constructor that also sets data folder path
      NistPad(std::string folder);
      // Constructor that automatically loads element data based on atomic number
      NistPad(std::string folder, int atomic_number);
      // Constructor that automatically loads element/compound data based on name 
      NistPad(std::string folder, std::string name);
      // Get and set functions
      std::string get_name(){ return name; }
      // File loading functions (returns true if successful)
      bool SetDataFolder(std::string folder);
      bool ReadFile(std::string file_name);
      bool Load(int atomic_number);
      bool Load(std::string name);
      // Material editing
      void Rename(std::string new_name);
      void ForceDensity(float new_density);
      // Get values from data using log interpolation
      double MassAttenuation(double energy);
      double LinearAttenuation(double energy);
      double MassAbsorption(double energy);
      double LinearAbsorption(double energy);
      // Prints all data to terminal screen
      void PrintData();
    private:
      std::string data_folder;
      
      std::string name;
      unsigned int num_elements;
      bool is_element;
      std::vector<int> atomic_number;
      std::vector<double> weight_fraction;
      double z_to_a_ratio;
      double mean_exitation_energy;
      double density;
      
      std::vector<double> energies;
      std::vector<double> mass_attenuation;
      std::vector<double> mass_energy_absorption;
      
      std::vector<int> absorption_edges;
  };
}

// End header guard
#endif
