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
// This is the main file for the class which loads and stores photon          //
// attenuation data from NIST. Data from any point on the table is accessed   //
// using logarithmic interpolation.                                           //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

// Class header
#include "Physics/NistPad.hpp"

// Standard C headers
#include <cstdlib>

// Standard C++ headers
#include <iostream>
#include <fstream>
#include <sstream>

// Solutio C++ headers
#include "Utilities/DataInterpolation.hpp"

namespace solutio
{
  NistPad::NistPad(std::string folder)
  {
    data_folder = folder;
  }

  // Constructor that automatically loads element data based on atomic number
  NistPad::NistPad(std::string folder, int atomic_number)
  {
    data_folder = folder;
    Load(atomic_number);
  }

  // Constructor that automatically loads element/compound data based on name
  NistPad::NistPad(std::string folder, std::string name)
  {
    data_folder = folder;
    Load(name);
  }

  // Data reading function
  bool NistPad::ReadFile(std::string file_path)
  {
    std::ifstream fin;
    std::string line, str;
    bool reading_elements = true;
    size_t pos;
    int z, counter;
    double input;
    std::pair<int,double> entry;

    fin.open(file_path.c_str());

    // Read in material information from header
    for(int n = 0; n < 5; n++) std::getline(fin, line);
    name = line;

    for(int n = 0; n < 3; n++) std::getline(fin, line);
    std::stringstream(line) >> z_to_a_ratio;

    for(int n = 0; n < 3; n++) std::getline(fin, line);
    std::stringstream(line) >> mean_exitation_energy;

    for(int n = 0; n < 3; n++) std::getline(fin, line);
    line[5] = 'e';
    std::stringstream(line) >> density;

    for(int n = 0; n < 2; n++) std::getline(fin, line);
    while(reading_elements)
    {
      std::getline(fin, line);
      if(line.empty())
      {
        reading_elements = false;
      }
      else
      {
        pos = line.find(':');
        std::stringstream(line.substr(0, pos)) >> z;
        entry.first = z;
        std::stringstream(line.substr(pos+1)) >> input;
        entry.second = input;
        atomic_composition.push_back(entry);
      }
    }
    num_elements = atomic_composition.size();
    if(num_elements == 1) is_element = true;
    else is_element = false;

    // Read in attenuation data
    counter = 0;
    for(int n = 0; n < 3; n++) std::getline(fin, line);
    while(std::getline(fin, line))
    {
      if(line[1] != '.') absorption_edges.push_back(counter);

      pos = line.find('.'); pos--;
      line[(pos+7)] = line[(pos+18)] = line[(pos+29)] = 'e';

      str = line.substr(pos, 12);
      std::stringstream(str) >> input;
      energies.push_back(input);

      str = line.substr((pos+12), 11);
      std::stringstream(str) >> input;
      mass_attenuation.push_back(input);

      str = line.substr(pos+23);
      std::stringstream(str) >> input;
      mass_energy_absorption.push_back(input);

      counter++;
    }

    fin.close();

    return true;
  }

  // Data loading function if element atomic number is given
  bool NistPad::Load(int atomic_number)
  {
    std::ifstream fin;
    std::string line;

    // Load element data file names
    std::string element_list = data_folder + "/Elements/ElementList.txt";
    std::vector<std::string> elements;

    fin.open(element_list.c_str());
    while(std::getline(fin, line)){ elements.push_back(line); }
    fin.close();

    ReadFile(data_folder + "/Elements/" + elements[(atomic_number-1)]);

    return true;
  }

  // Data loading function if element/compound name is given
  bool NistPad::Load(std::string name)
  {
    std::ifstream fin;
    std::string line;
    bool found = false, element = false;
    size_t p1, p2;
    int id;

    // Load element data file names; search elements first
    std::string element_list = data_folder + "/Elements/ElementList.txt";
    std::vector<std::string> element_names, element_files;
    fin.open(element_list.c_str());
    while(std::getline(fin, line))
    {
      element_files.push_back(line);
      p1 = line.find('-')+1;
      p2 = line.find('.');
      element_names.push_back(line.substr(p1, p2-p1));
    }
    fin.close();
    for(int n = 0; n < element_names.size(); n++)
    {
      if(element_names[n] == name)
      {
        id = n;
        found = true;
        element = true;
        break;
      }
    }

    // If not an element, search compounds
    std::string compound_list = data_folder + "/Compounds/CompoundList.txt";
    std::vector<std::string> compound_names, compound_files;
    fin.open(compound_list.c_str());
    while(std::getline(fin, line))
    {
      p1 = line.find('\t');
      compound_names.push_back(line.substr(0,p1));
      compound_files.push_back(line.substr(p1+1));
    }
    fin.close();
    for(int n = 0; n < compound_names.size(); n++)
    {
      if(compound_names[n] == name ||
          compound_files[n].substr(0,compound_files[n].find('.')) == name)
      {
        id = n;
        found = true;
        break;
      }
    }

    if(found)
    {
      if(element)
      {
        ReadFile(data_folder + "/Elements/" + element_files[id]);
      }
      else
      {
        ReadFile(data_folder + "/Compounds/" + compound_files[id]);
      }
    }
    else
    {
      std::cout << "Error: could not find specified element/material!\n";
      std::cout << "Attempted search: " << data_folder << '\n';
    }

    return found;
  }

  // Change material name if desired
  void NistPad::Rename(std::string new_name)
  {
    std::string old_name = name;
    name = new_name;
    std::cout << "Warning: material name changed from \"" << old_name <<
        "\" to \"" << new_name << "\".\n";
  }

  // Force material to have new density if desired
  void NistPad::ForceDensity(float new_density)
  {
    float old = density;
    density = new_density;
    std::cout << "Warning: material density for \"" << GetName() <<
        "\" changed from " << old << " to " << density << ".\n";
  }

  // Get values from data using log interpolation
  double NistPad::MassAttenuation(double energy)
  {
    return (LogInterpolation(energies, mass_attenuation, energy));
  }
  double NistPad::LinearAttenuation(double energy)
  {
    return (density*LogInterpolation(energies, mass_attenuation, energy));
  }
  double NistPad::MassAbsorption(double energy)
  {
    return (LogInterpolation(energies, mass_energy_absorption, energy));
  }
  double NistPad::LinearAbsorption(double energy)
  {
    return (density*LogInterpolation(energies, mass_energy_absorption, energy));
  }

  // Calculate effective atomic number
  double NistPad::PowerLawEffectiveZ(double m)
  {
    double z_eff = 0.0, sum_f = 0.0, f;
    std::vector<double> f_list;
    for(int n = 0; n < atomic_composition.size(); n++)
    {
      f = atomic_composition[n].second *
        ElementZARatio[(atomic_composition[n].first)];
      f_list.push_back(f);
      sum_f += f;
    }
    for(int n = 0; n < atomic_composition.size(); n++)
    {
      z_eff += (f_list[n]/sum_f)*pow(double(atomic_composition[n].first), m);
    }
    z_eff = pow(z_eff, 1.0/m);
    return z_eff;
  }

  // Prints data to vector of strings (each entry is a line of text, with
  // no newline characters)
  std::vector<std::string> NistPad::Print()
  {
    std::vector<std::string> print_text;

    print_text.push_back(name);

    if(is_element) print_text.push_back("This is an element.");
    else print_text.push_back("This is not an element.");

    print_text.push_back("Z/A = "+std::to_string(z_to_a_ratio));
    print_text.push_back("I (eV) = "+std::to_string(mean_exitation_energy));
    print_text.push_back("Density (g/cm^3) = "+std::to_string(density));

    print_text.push_back("Elements by Weight");
    print_text.push_back("------------------");
    for(int n = 0; n < num_elements; n++)
    {
      print_text.push_back(std::to_string(atomic_composition[n].first)+": "+
        std::to_string(atomic_composition[n].second));
    }

    print_text.push_back("Attenuation Data");
    print_text.push_back("----------------");
    for(int n = 0; n < energies.size(); n++)
    {
      std::stringstream ss;
      ss << energies[n]; ss << ' ';
      ss << mass_attenuation[n]; ss << ' ';
      ss << mass_energy_absorption[n];
      print_text.push_back(ss.str());
    }

    if(absorption_edges.size() > 0)
    {
      print_text.push_back("Absorption Edges");
      print_text.push_back("----------------");
      for(int n = 0; n < absorption_edges.size(); n++)
      {
        std::stringstream ss;
        ss << energies[(absorption_edges[n])]; ss << ' ';
        ss << mass_attenuation[(absorption_edges[n])]; ss << ' ';
        ss << mass_energy_absorption[(absorption_edges[n])];
        print_text.push_back(ss.str());
      }
    }

    return print_text;
  }
}
