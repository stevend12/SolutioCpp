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

#include "../Utilities/RTPlan.hpp"

namespace solutio
{

  double GeometryFactorTG43(double r, double theta, double L);

  class BrachyDoseTG43
  {
    public:
      // Default constructor
      BrachyDoseTG43();
      // Load data from text file
      bool LoadData(std::string file_name);
      bool IsLoaded(){ return data_loaded; }
      // Pre-compute evenly-spaced tables for fast interpolation
      void PreCompute(double d_radius, double d_theta);
      // Write current data (with interpolated/pre-computed values) to text file
      void WriteData(std::string file_name);
      // Get values
      std::string GetNuclideName(){ return nuclide_name; }
      std::string GetVendorName(){ return vendor_name; }
      std::string GetModelName(){ return model_name; }
      double GetDoseRateConstant(){ return dose_rate_constant; }
      double GetSourceLength(){ return source_length; }
      double GetRadialDoseFunctionPoint(double r);
      double GetRadialDoseFunctionLine(double r);
      double GetAnisotropyFunctionPoint(double r);
      double GetAnisotropyFunctionLine(double r, double theta);
      // Calculate dose rate to point (r, theta), given air kerma strength (aks)
      double CalcDoseRatePoint(double aks, double r);
      double CalcDoseRateLine(double aks, double r, double theta);
      // Data struct that stores calculation data from a multi-source plan
      // calculation
      struct CalcStats
      {
        double DoseSum;
        double MinRadius;
        double MaxRadius;
        double AveRadius;
        double MinTheta;
        double MaxTheta;
        double AveTheta;
      };
      // Calculate the dose from a BrachyPlan object with a user-defined air
      // kerma strength and reference time
      CalcStats CalcDoseBrachyPlan(double ref_aks, struct tm ref_dt,
        BrachyPlan plan, Vec3<double> point, bool line_source = true);
    private:
      // Internal parameters
      bool data_loaded;
      bool precomputed;
      double delta_radius;
      double delta_theta;
      // Header data
      std::string reference; // Reference for data
      std::string source_type; // Type (HDR, LDR, PDR)
      std::string nuclide_name; // Source radionuclide name
      std::string vendor_name; // Source vendor name
      std::string model_name; // Source model name
      // Dose rate constant
      double dose_rate_constant;
      // Source length
      double source_length;
      // Radial dose function data
      std::vector<double> r_g_r;
      std::vector<double> g_r_line_data;
      std::vector<double> g_r_point_data;
      // 2D anisotropy factor data
      std::vector<double> theta_anisotropy_2d;
      std::vector<double> r_anisotropy;
      std::vector< std::vector<double> > anisotropy_2d_data;
      std::vector<double> anisotropy_1d_data;
  };
}

// End header guard
#endif
