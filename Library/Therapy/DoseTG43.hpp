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
// DoseTG43.hpp                                                               //
// TG-43 Based Brachytherapy Dose Calculation Class                           //
// Created November 3, 2017 (Steven Dolly)                                    //
//                                                                            //
// This header file defines a class for dose calculation for brachytherapy    //
// sources, using the TG-43 based methodology. The DoseTG43 class reads in    //
// source data and calculates the dose to a point for a given location, in    //
// polar coordinates.                                                         //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

// Header guards
#ifndef DOSETG43_HPP
#define DOSETG43_HPP

// Standard C++ header files
#include <string>
#include <vector>

namespace solutio
{
  float GeometryFactorTG43(float r, float theta, bool is_line, float L);
  
  class DoseTG43
  {
    public:
      // Load data from text file
      void LoadData(std::string file_name, char delimiter);
      // Get values
      float Get_dose_rate_constant(){ return dose_rate_constant; }
      float Get_g_r(float r);
      float Get_anisotropy_2d(float r, float theta);
      // Calculate dose rate to point (r, theta), given air kerma strength (aks)
      float CalcDoseRate(float aks, float r, float theta);
    private:
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
