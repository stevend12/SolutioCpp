/******************************************************************************/
/*                                                                            */
/* Copyright 2020 Steven Dolly                                                */
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
// NistEstar.hpp                                                              //
// NIST ESTAR Data (NistEstar) Manager Class                                  //
// Created May 13, 2020 (Steven Dolly)                                        //
//                                                                            //
// This file contains the header for the class which loads and stores         //
// electron data from NIST. Data from any point on the table is accessed      //
// using logarithmic interpolation.                                           //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

// Header guard
#ifndef NISTESTAR_HPP
#define NISTESTAR_HPP

// Standard C++ headers
#include <string>
#include <vector>

namespace solutio
{
  class NistEstar
  {
    public:
      // Constructor that also sets data folder path
      NistEstar(std::string folder);
      // Constructor that automatically loads element data based on atomic number
      NistEstar(std::string folder, int atomic_number);
      // Constructor that automatically loads element/compound data based on name
      NistEstar(std::string folder, std::string name);
      // Get and set functions
      std::string GetName(){ return name; }
      // File loading functions (returns true if successful)
      bool ReadFile(std::string file_name);
      bool Load(int atomic_number);
      bool Load(std::string name);
      // Material editing
      void Rename(std::string new_name);
      void ForceDensity(float new_density);
      // Get attenuation values from data using log interpolation
      double ColStoppingPower(double energy);
      double RadStoppingPower(double energy);
      double TotalStoppingPower(double energy);
      double CSDARange(double energy);
      double RadiationYield(double energy);
      double DensityEffectParameter(double energy);
      // Get material values
      double GetDensity(){ return density; }
      double GetI(){ return mean_exitation_energy; }
      std::vector< std::pair<int,double> > GetComposition()
      {
        return atomic_composition;
      }
      // Prints data to terminal screen
      void PrintTable();
      void PrintData();
    private:
      std::string data_folder;

      std::string name;
      unsigned int num_elements;
      bool is_element;
      double density;
      double mean_exitation_energy;
      std::vector< std::pair<int,double> > atomic_composition;

      std::vector<double> energies;
      std::vector<double> col_stopping_power;
      std::vector<double> rad_stopping_power;
      std::vector<double> total_stopping_power;
      std::vector<double> csda_range;
      std::vector<double> radiation_yield;
      std::vector<double> d_effect_parameter;
  };
}

// End header guard
#endif
