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
  double GeometryFactorTG43(double r, double theta, double L)
  {
    double geometry_factor = 0.0;
    double G_0 = (atan2(-L/2.0, 1.0) - atan2(L/2.0, 1.0)) / L;
    if(theta == 0.0)
    {
      geometry_factor = 1.0/(r*r - (L*L)/4.0);
      geometry_factor /= G_0;
    }
    else {
      double beta = atan2(r*cos(theta*(M_PI/180.0))-L/2.0, r*sin(theta*(M_PI/180.0)))
          - atan2(r*cos(theta*(M_PI/180.0))+L/2.0, r*sin(theta*(M_PI/180.0)));
      geometry_factor = beta/(L*r*sin(theta*(M_PI/180.0)));
      geometry_factor /= G_0;
    }
    return geometry_factor;
  }

  // Default constructor
  BrachyDoseTG43::BrachyDoseTG43()
  {
    data_loaded = false;
    precomputed = false;
  }

  bool BrachyDoseTG43::LoadData(std::string file_name)
  {
    // Initialization and open file
    std::ifstream fin;
    std::string input, str;
    size_t p1, p2;
    bool reading = true;
    double temp;
    std::vector<double>::iterator it1, it2;

    fin.open(file_name.c_str());
    if(fin.bad()) return false;

    /////////////////////////////////
    // Get and display header data //
    /////////////////////////////////
    // Skip first two lines
    if(!std::getline(fin, input)) return false;
    if(!std::getline(fin, input)) return false;
    // Reference for data
    if(!std::getline(fin, input)) return false;
    p1 = input.find(' '); p1++;
    reference = input.substr(p1);
    // Source type
    if(!std::getline(fin, input)) return false;
    p1 = input.find(' '); p1++;
    source_type = input.substr(p1);
    // Source radionuclide name
    if(!std::getline(fin, input)) return false;
    p1 = input.find(' '); p1++;
    nuclide_name = input.substr(p1);
    // Source vendor name
    if(!std::getline(fin, input)) return false;
    p1 = input.find(' '); p1++;
    vendor_name = input.substr(p1);
    // Source model name
    if(!std::getline(fin, input)) return false;
    p1 = input.find(' '); p1++;
    model_name = input.substr(p1);
    // Dose rate constant and source length
    if(!std::getline(fin, input)) return false;
    p1 = input.find(':'); p1++;
    std::stringstream(input.substr(p1)) >> dose_rate_constant;
    // Source length
    if(!std::getline(fin, input)) return false;
    p1 = input.find(':'); p1++;
    std::stringstream(input.substr(p1)) >> source_length;

    for(int n = 0; n < 3; n++)
    {
      if(!std::getline(fin, input)) return false;
    }

    // Get radial dose function data; interpolate as needed
    while(std::getline(fin, input))
    {
      if(input.find("end radial dose function data") != std::string::npos) break;
      std::vector<std::string> data = LineRead(input, ',');
      if(data.size() < 3) return false;
      std::stringstream(data[0]) >> temp;
      r_g_r.push_back(temp);
      std::stringstream(data[1]) >> temp;
      g_r_line_data.push_back(temp);
      std::stringstream(data[2]) >> temp;
      g_r_point_data.push_back(temp);
    }
    // Nearest neighbor interpolation for r = 0
    if(r_g_r.size() == 0) return false;
    if(r_g_r[0] != 0.0)
    {
      it1 = r_g_r.begin();
      r_g_r.insert(it1, 0.0);
      it1 = g_r_line_data.begin();
      g_r_line_data.insert(it1, g_r_line_data[0]);
      it1 = g_r_point_data.begin();
      g_r_point_data.insert(it1, g_r_point_data[0]);
    }

    for(int n = 0; n < 3; n++)
    {
      if(!std::getline(fin, input)) return false;
    }

    // Get 2D anisotropy data; interpolate as needed
    bool interpolate_zero = false;
    if(!std::getline(fin, input)) return false;
    std::vector<std::string> column = LineRead(input, ',');
    if(column.size() < 3) return false;
    for(int n = 1; n < column.size(); n++)
    {
      std::stringstream(column[n]) >> temp;
      r_anisotropy.push_back(temp);
    }
    double r_min = r_anisotropy[0];
    if(r_anisotropy[0] != 0.0)
    {
      interpolate_zero = true;
      it1 = r_anisotropy.begin();
      r_anisotropy.insert(it1, 0.0);
    }
    while(std::getline(fin, input))
    {
      if(input.find("end anisotropy function data") != std::string::npos) break;
      std::vector<std::string> row = LineRead(input, ',');
      if(row.size() < 3) return false;
      if(row[0] == "point")
      {
        for(int n = 1; n < column.size(); n++)
        {
          std::stringstream(row[n]) >> temp;
          anisotropy_1d_data.push_back(temp);
        }
        if(interpolate_zero)
        {
          it1 = anisotropy_1d_data.begin();
          anisotropy_1d_data.insert(it1, anisotropy_1d_data[0]);
        }
      }
      else
      {
        std::stringstream(row[0]) >> temp;
        theta_anisotropy_2d.push_back(temp);

        std::vector<double> buffer;
        for(int n = column.size()-1; n > 0; n--)
        {
          if(row[n] == "-")
          {
            it1 = r_anisotropy.begin() + n + 1;
            it2 = r_anisotropy.end();
            std::vector<double> r_buf(it1, it2);
            it1 = buffer.begin();
            buffer.insert(it1,
              LinearInterpolation<double>(r_buf, buffer, r_anisotropy[n]));
          }
          else
          {
            std::stringstream(row[n]) >> temp;
            it1 = buffer.begin();
            buffer.insert(it1, temp);
          }
        }
        if(interpolate_zero)
        {
          it1 = buffer.begin();
          buffer.insert(it1, buffer[0]);
        }
        anisotropy_2d_data.push_back(buffer);
      }
    }

    fin.close();
    
    data_loaded = true;
    return true;
  }

  void BrachyDoseTG43::WriteData(std::string file_name)
  {
    std::ofstream fout(file_name.c_str());
    // Write header data
    fout << "TG-43 Brachytherapy Source Data\n";
    fout << "-------------------------------\n";
    fout << "Reference: " << reference << '\n';
    fout << "Source Type: " << source_type << '\n';
    fout << "Nuclide: " << nuclide_name << '\n';
    fout << "Vendor: " << vendor_name << '\n';
    fout << "Model: " << model_name << '\n';
    fout << "Dose Rate Constant: " << dose_rate_constant << '\n';
    fout << "Source Length (cm): " << source_length << "\n\n";
    // Radial dose function data
    fout << "Radial Dose Function\n";
    fout << "--------------------\n";
    for(int n = 0; n < r_g_r.size(); n++)
    {
      fout << r_g_r[n] << ',' << g_r_line_data[n] << ',' << g_r_point_data[n]
        << '\n';
    }
    fout << '\n';
    // 2D anisotropy factor data
    fout << "Anisotropy Function\n";
    fout << "-------------------\n";
    for(int n = 0; n < r_anisotropy.size(); n++)
    {
      fout << r_anisotropy[n];
      if(n != r_anisotropy.size()-1) fout << ',';
    }
    fout << '\n';
    for(int n = 0; n < theta_anisotropy_2d.size(); n++)
    {
      fout << theta_anisotropy_2d[n] << ',';
      for(int a = 0; a < anisotropy_2d_data[n].size(); a++)
      {
        fout << anisotropy_2d_data[n][a];
        if(n != anisotropy_2d_data[n].size()-1) fout << ',';
      }
      fout << '\n';
    }
    fout << "point,";
    for(int a = 0; a < anisotropy_1d_data.size(); a++)
    {
      fout << anisotropy_1d_data[a];
      if(a != anisotropy_1d_data.size()-1) fout << ',';
    }
    fout << '\n';

    fout.close();
  }

  void BrachyDoseTG43::PreCompute(double d_radius, double d_theta)
  {
    if(data_loaded)
    {
      delta_radius = d_radius;
      delta_theta = d_theta;
      double x;
      std::vector<double> temp_r, temp_t, temp_grp, temp_grl, temp_a1d;
      std::vector< std::vector<double> > temp_2d;
      // Make radius and theta axes
      x = r_g_r[0];
      while(x <= r_g_r[(r_g_r.size()-1)])
      {
        temp_r.push_back(x);
        x += delta_radius;
      }
      x = theta_anisotropy_2d[0];
      while(x <= theta_anisotropy_2d[(theta_anisotropy_2d.size()-1)])
      {
        temp_t.push_back(x);
        x += delta_theta;
      }
      // Make 1d data
      for(int n = 0; n < temp_r.size(); n++)
      {
        temp_grp.push_back(GetRadialDoseFunctionPoint(temp_r[n]));
        temp_grl.push_back(GetRadialDoseFunctionLine(temp_r[n]));
        temp_a1d.push_back(GetAnisotropyFunctionPoint(temp_r[n]));
      }
      // Make 2D data
      for(int a = 0; a < temp_t.size(); a++)
      {
        std::vector<double> buffer;
        for(int n = 0; n < temp_r.size(); n++)
        {
          buffer.push_back(GetAnisotropyFunctionLine(temp_r[n], temp_t[a]));
        }
        temp_2d.push_back(buffer);
      }
      // Set new data and toggle precompiled status
      r_g_r = temp_r;
      g_r_line_data = temp_grl;
      g_r_point_data = temp_grp;
      r_anisotropy = temp_r;
      theta_anisotropy_2d = temp_t;
      anisotropy_2d_data = temp_2d;
      anisotropy_1d_data = temp_a1d;
      precomputed = true;
    }
  }

  double BrachyDoseTG43::GetRadialDoseFunctionPoint(double r)
  {
    double ans;
    if(precomputed)
    {
      ans = solutio::LinearInterpolationFast(r_g_r, g_r_point_data, r, delta_radius);
    }
    else ans = solutio::LinearInterpolation(r_g_r, g_r_point_data, r);
    return ans;
  }

  double BrachyDoseTG43::GetRadialDoseFunctionLine(double r)
  {
    double ans;
    if(precomputed)
    {
      ans = solutio::LinearInterpolationFast(r_g_r, g_r_line_data, r, delta_radius);
    }
    else ans = solutio::LinearInterpolation(r_g_r, g_r_line_data, r);
    return ans;
  }

  double BrachyDoseTG43::GetAnisotropyFunctionPoint(double r)
  {
    double ans;
    if(precomputed)
    {
      ans = solutio::LinearInterpolationFast(r_anisotropy, anisotropy_1d_data, r, delta_radius);
    }
    else ans = solutio::LinearInterpolation(r_anisotropy, anisotropy_1d_data, r);
    return ans;
  }

  double BrachyDoseTG43::GetAnisotropyFunctionLine(double r, double theta)
  {
    double ans;
    if(precomputed)
    {
      //ans = solutio::LinearInterpolationFast(theta_anisotropy_2d, r_anisotropy,
      //  anisotropy_2d_data, theta, r, delta_theta, delta_radius);
      ans = solutio::LinearInterpolation(theta_anisotropy_2d, r_anisotropy,
        anisotropy_2d_data, theta, r);
    }
    else ans = solutio::LinearInterpolation(theta_anisotropy_2d, r_anisotropy,
      anisotropy_2d_data, theta, r);
    return ans;
  }

  double BrachyDoseTG43::CalcDoseRatePoint(double aks, double r)
  {
    double G = 1.0 / (r*r);
    double g = GetRadialDoseFunctionPoint(r);
    double F = GetAnisotropyFunctionPoint(r);
    return (aks*dose_rate_constant*G*g*F);
  }

  double BrachyDoseTG43::CalcDoseRateLine(double aks, double r, double theta)
  {
    double G = GeometryFactorTG43(r, theta, source_length);
    double g = GetRadialDoseFunctionLine(r);
    double F = GetAnisotropyFunctionLine(r, theta);
    return (aks*dose_rate_constant*G*g*F);
  }
}
