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
// BrachyDoseTG43.hpp                                                         //
// TG-43 Based Brachytherapy Dose Calculation Class                           //
// Created November 3, 2017 (Steven Dolly)                                    //
//                                                                            //
// This header file defines a class for dose calculation for brachytherapy    //
// sources, using the TG-43 based methodology. The BrachyDoseTG43 class reads //
// in source data and calculates the dose to a point for a given location, in //
// polar coordinates.                                                         //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

// Class header
#include "BrachyDoseTG43.hpp"

// Standard C++ header files
#include <iostream>
#include <fstream>
#include <sstream>

// Standard C header files
#include <cmath>

// Solutio C++ headers
#include "Utilities/FileIO.hpp"
#include "Utilities/DataInterpolation.hpp"

namespace solutio
{
  float GeometryFactorTG43(float r, float theta, bool is_line, float L)
  {
    float geometry_factor;
    if(is_line)
    {
      float G_0 = (atan2(-L/2.0, 1.0) - atan2(L/2.0, 1.0)) / (L*r);
      float beta = 1.0;
      if(theta == 0.0)
      {
        geometry_factor = 1.0/(r*r - (L*L)/4.0);
        geometry_factor /= G_0;
      }
      else {
        float beta = atan2(r*cos(theta*(M_PI/180.0))-L/2.0, r*sin(theta*(M_PI/180.0)))
            - atan2(r*cos(theta*(M_PI/180.0))+L/2.0, r*sin(theta*(M_PI/180.0)));
        geometry_factor = beta/(L*r*sin(theta*(M_PI/180.0)));
        geometry_factor /= G_0;
      }
    }
    else
    {
      geometry_factor = 1.0/(r*r);
    }
    return geometry_factor;
  }

  void BrachyDoseTG43::LoadData(std::string file_name)
  {
    // Initialization and open file
    std::ifstream fin;
    std::string input, str;
    size_t p1, p2;
    bool reading = true;
    float temp;

    fin.open(file_name.c_str());

    /////////////////////////////////
    // Get and display header data //
    /////////////////////////////////
    std::getline(fin, input);
    std::cout << input << '\n';
    std::getline(fin, input);
    std::cout << input << '\n';
    // Reference for data
    std::getline(fin, input);
    p1 = input.find(' '); p1++;
    reference = input.substr(p1);
    std::cout << "Reference: " << reference << '\n';
    // Source vendor name
    std::getline(fin, input);
    p1 = input.find(' '); p1++;
    vendor_name = input.substr(p1);
    std::cout << "Vendor: " << vendor_name << '\n';
    // Source model name
    std::getline(fin, input);
    p1 = input.find(' '); p1++;
    model_name = input.substr(p1);
    std::cout << "Model: " << model_name << '\n';
    // Source radionuclide name
    std::getline(fin, input);
    p1 = input.find(' '); p1++;
    nuclide_name = input.substr(p1);
    std::cout << "Nuclide: " << nuclide_name << '\n';
    // Dose rate constant and source length
    std::getline(fin, input);
    p1 = input.find(':'); p1++;
    std::stringstream(input.substr(p1)) >> dose_rate_constant;
    std::cout << "Dose rate constant: " << dose_rate_constant << '\n';
    // Source length
    std::getline(fin, input);
    p1 = input.find(':'); p1++;
    std::stringstream(input.substr(p1)) >> source_length;
    std::cout << "Source length (cm): " << source_length << '\n';
    for(int n = 0; n < 3; n++){ std::getline(fin, input); }

    // Get radial dose function data
    while(std::getline(fin, input))
    {
      if(input.find("end radial dose function data") != std::string::npos) break;
      std::vector<std::string> data = LineRead(input, ',');
      std::stringstream(data[0]) >> temp;
      r_g_r.push_back(temp);
      std::stringstream(data[1]) >> temp;
      g_r_data.push_back(temp);
    }
    for(int n = 0; n < 3; n++){ std::getline(fin, input); }

    // Get 2D anisotropy data
    std::getline(fin, input);
    std::vector<std::string> column = LineRead(input, ',');
    for(int n = 0; n < column.size(); n++)
    {
      std::stringstream(column[n]) >> temp;
      r_anisotropy_2d.push_back(temp);
    }
    while(std::getline(fin, input))
    {
      if(input.find("end 2d anisotropy factor data") != std::string::npos) break;
      std::vector<std::string> row = LineRead(input, ',');
      std::stringstream(row[0]) >> temp;
      theta_anisotropy_2d.push_back(temp);

      std::vector<float> buffer;
      for(int n = 1; n < column.size(); n++)
      {
        std::stringstream(row[n]) >> temp;
        buffer.push_back(temp);
      }
      anisotropy_2d_data.push_back(buffer);
    }

    fin.close();
  }

  float BrachyDoseTG43::GetRadialDoseFactor(float r)
  {
    return solutio::LinearInterpolation(r_g_r, g_r_data, r);
  }

  float BrachyDoseTG43::GetAnisotropyFactor(float r, float theta)
  {
    return solutio::LinearInterpolation(theta_anisotropy_2d, r_anisotropy_2d,
        anisotropy_2d_data, theta, r);
  }

  float BrachyDoseTG43::CalcDoseRate(float aks, float r, float theta)
  {
    float G = GeometryFactorTG43(r, theta, true, source_length);
    float g = GetRadialDoseFactor(r);
    float F = GetAnisotropyFactor(r, theta);
    return (aks*dose_rate_constant*G*g*F);
  }
}
