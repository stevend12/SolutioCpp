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
// NistEstar.cpp                                                              //
// NIST ESTAR Data (NistEstar) Manager Class                                  //
// Created May 13, 2020 (Steven Dolly)                                        //
//                                                                            //
// This is the main file for the class which loads and stores electron data   //
// from NIST. Data from any point on the table is accessed using logarithmic  //
// interpolation.                                                             //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////


// Class header
#include "Physics/NistEstar.hpp"

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
  NistEstar::NistEstar(std::string folder)
  {
    data_folder = folder;
  }

  // Constructor that automatically loads element data based on atomic number
  NistEstar::NistEstar(std::string folder, int atomic_number)
  {
    data_folder = folder;
    Load(atomic_number);
  }

  // Constructor that automatically loads element/compound data based on name
  NistEstar::NistEstar(std::string folder, std::string name)
  {
    data_folder = folder;
    Load(name);
  }

  // Data reading function
  bool NistEstar::ReadFile(std::string file_path)
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
    line[7] = 'e';
    std::stringstream(line) >> density;

    for(int n = 0; n < 3; n++) std::getline(fin, line);
    std::stringstream(line) >> mean_exitation_energy;

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

    // Read in electron data
    counter = 0;
    for(int n = 0; n < 10; n++) std::getline(fin, line);
    while(std::getline(fin, line))
    {
      pos = line.find('.'); pos--;
      line[(pos+5)] = line[(pos+16)] = line[(pos+27)] = line[(pos+38)] =
        line[(pos+49)] = line[(pos+60)] = line[(pos+71)] = 'e';

      str = line.substr(pos, 11);
      std::stringstream(str) >> input;
      energies.push_back(input);
      pos+=11;

      str = line.substr(pos, 11);
      std::stringstream(str) >> input;
      col_stopping_power.push_back(input);
      pos+=11;

      str = line.substr(pos, 11);
      std::stringstream(str) >> input;
      rad_stopping_power.push_back(input);
      pos+=11;

      str = line.substr(pos, 11);
      std::stringstream(str) >> input;
      total_stopping_power.push_back(input);
      pos+=11;

      str = line.substr(pos, 11);
      std::stringstream(str) >> input;
      csda_range.push_back(input);
      pos+=11;

      str = line.substr(pos, 11);
      std::stringstream(str) >> input;
      radiation_yield.push_back(input);
      pos+=11;

      str = line.substr(pos, 11);
      std::stringstream(str) >> input;
      d_effect_parameter.push_back(input);
      pos+=11;

      counter++;
    }

    fin.close();

    return true;
  }

  // Data loading function if element atomic number is given
  bool NistEstar::Load(int atomic_number)
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
  bool NistEstar::Load(std::string name)
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
  void NistEstar::Rename(std::string new_name)
  {
    std::string old_name = name;
    name = new_name;
    std::cout << "Warning: material name changed from \"" << old_name <<
        "\" to \"" << new_name << "\".\n";
  }

  // Force material to have new density if desired
  void NistEstar::ForceDensity(float new_density)
  {
    float old = density;
    density = new_density;
    std::cout << "Warning: material density for \"" << GetName() <<
        "\" changed from " << old << " to " << density << ".\n";
  }

  // Get values from data using log interpolation
  double NistEstar::ColStoppingPower(double energy)
  {
    return (LogInterpolation(energies, col_stopping_power, energy));
  }
  double NistEstar::RadStoppingPower(double energy)
  {
    return (LogInterpolation(energies, rad_stopping_power, energy));
  }
  double NistEstar::TotalStoppingPower(double energy)
  {
    return (LogInterpolation(energies, total_stopping_power, energy));
  }
  double NistEstar::CSDARange(double energy)
  {
    return (LogInterpolation(energies, csda_range, energy));
  }
  double NistEstar::RadiationYield(double energy)
  {
    return (LogInterpolation(energies, radiation_yield, energy));
  }
  double NistEstar::DensityEffectParameter(double energy)
  {
    return (LogInterpolation(energies, d_effect_parameter, energy));
  }

  // Prints data to vector of strings (each entry is a line of text, with
  // no newline characters)
  std::vector<std::string> NistEstar::Print()
  {
    std::vector<std::string> print_text;

    print_text.push_back(name);

    if(is_element) print_text.push_back("This is an element.");
    else print_text.push_back("This is not an element.");

    print_text.push_back("Density (g/cm^3) = "+std::to_string(density));
    print_text.push_back("I (eV) = "+std::to_string(mean_exitation_energy));

    print_text.push_back("Elements by Weight");
    print_text.push_back("------------------");
    for(int n = 0; n < num_elements; n++)
    {
      print_text.push_back(std::to_string(atomic_composition[n].first)+": "+
        std::to_string(atomic_composition[n].second));
    }

    print_text.push_back("Electron Data");
    print_text.push_back("-------------");
    for(int n = 0; n < energies.size(); n++)
    {
      std::stringstream ss;
      ss << energies[n]; ss << ' ';
      ss << col_stopping_power[n]; ss << ' ';
      ss << rad_stopping_power[n]; ss << ' ';
      ss << total_stopping_power[n]; ss << ' ';
      ss << csda_range[n]; ss << ' ';
      ss << radiation_yield[n]; ss << ' ';
      ss << d_effect_parameter[n];
      print_text.push_back(ss.str());
    }

    return print_text;
  }
}
