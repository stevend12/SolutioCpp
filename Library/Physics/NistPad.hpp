/******************************************************************************/
/*                                                                            */
/* Copyright 2016 Steven Dolly                                                */
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
      // Constructor that also sets data folder path
      NistPad(std::string folder);
      // Constructor that automatically loads element data based on atomic number
      NistPad(std::string folder, int atomic_number);
      // Constructor that automatically loads element/compound data based on name
      NistPad(std::string folder, std::string name);
      // Get and set functions
      std::string GetName(){ return name; }
      // File loading functions (returns true if successful)
      bool ReadFile(std::string file_name);
      bool Load(int atomic_number);
      bool Load(std::string name);
      // Material editing
      void Rename(std::string new_name);
      void ForceDensity(float new_density);
      // Get table size and energies for a row entry
      int GetNumRows(){ return energies.size(); }
      double GetEnergy(int r){ return energies[r]; }
      // Get absorption edge rows
      std::vector<int> GetAbsorptionEdges(){ return absorption_edges; }
      // Get attenuation values from data using log interpolation
      double MassAttenuation(double energy);
      double LinearAttenuation(double energy);
      double MassAbsorption(double energy);
      double LinearAbsorption(double energy);
      // Get material values
      double GetZtoA(){ return z_to_a_ratio; }
      double GetI(){ return mean_exitation_energy; }
      double GetDensity(){ return density; }
      std::vector< std::pair<int,double> > GetComposition()
      {
        return atomic_composition;
      }
      // Calculate effective atomic number, using the power law
      double PowerLawEffectiveZ(double m);
      // Prints data to vector of strings (each entry is a line of text, with
      // no newline characters)
      std::vector<std::string> Print();
    private:
      std::string data_folder;

      std::string name;
      unsigned int num_elements;
      bool is_element;
      std::vector< std::pair<int,double> > atomic_composition;
      double z_to_a_ratio;
      double mean_exitation_energy;
      double density;

      std::vector<double> energies;
      std::vector<double> mass_attenuation;
      std::vector<double> mass_energy_absorption;

      std::vector<int> absorption_edges;
  };

  // List of element Z/A ratios, use to calculate effective atomic number
  static const double ElementZARatio[92] = {
    0.99212,
    0.49968,
    0.43221,
    0.44384,
    0.46245,
    0.49954,
    0.49976,
    0.50002,
    0.47372,
    0.49555,
    0.47847,
    0.49373,
    0.48181,
    0.49848,
    0.48428,
    0.49897,
    0.47951,
    0.45059,
    0.48595,
    0.49903,
    0.46712,
    0.45948,
    0.4515,
    0.46157,
    0.45506,
    0.46556,
    0.45815,
    0.47708,
    0.45636,
    0.45879,
    0.44462,
    0.44071,
    0.44046,
    0.4306,
    0.43803,
    0.42959,
    0.43291,
    0.43369,
    0.43867,
    0.43848,
    0.4413,
    0.43777,
    0.43919,
    0.43534,
    0.43729,
    0.43225,
    0.43572,
    0.427,
    0.42676,
    0.4212,
    0.41889,
    0.40752,
    0.41764,
    0.4113,
    0.41383,
    0.40779,
    0.41035,
    0.41395,
    0.41871,
    0.41597,
    0.42094,
    0.41234,
    0.41457,
    0.40699,
    0.409,
    0.40615,
    0.40623,
    0.40655,
    0.40844,
    0.40453,
    0.40579,
    0.40338,
    0.40343,
    0.4025,
    0.40278,
    0.39958,
    0.40058,
    0.39984,
    0.40108,
    0.39882,
    0.39631,
    0.39575,
    0.39717,
    0.40195,
    0.40479,
    0.38736,
    0.3901,
    0.38934,
    0.39202,
    0.38787,
    0.39388,
    0.38651
  };
}

// End header guard
#endif
