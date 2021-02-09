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

// Header guards
#ifndef BRACHYDOSETG43_HPP
#define BRACHYDOSETG43_HPP

// Standard C++ header files
#include <string>
#include <vector>

namespace solutio
{
  float GeometryFactorTG43(float r, float theta, bool is_line = false, float L = 0.0);

  class BrachyDoseTG43
  {
    public:
      // Load data from text file
      void LoadData(std::string file_name);
      // Get values
      float GetDoseRateConstant(){ return dose_rate_constant; }
      float GetRadialDoseFactor(float r);
      float GetAnisotropyFactor(float r, float theta);
      // Calculate dose rate to point (r, theta), given air kerma strength (aks)
      float CalcDoseRate(float aks, float r, float theta);
    private:
      // Header data
      std::string reference; // Reference for data
      std::string vendor_name; // Source vendor name
      std::string model_name; // Source model name
      std::string nuclide_name; // Source radionuclide name
      // Dose rate constant
      float dose_rate_constant;
      // Source length
      float source_length;
      // Radial dose function data
      std::vector<float> r_g_r;
      std::vector<float> g_r_data;
      // 2D anisotropy factor data
      std::vector<float> theta_anisotropy_2d;
      std::vector<float> r_anisotropy_2d;
      std::vector< std::vector<float> > anisotropy_2d_data;
  };
}

// End header guard
#endif
