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
#include "Physics/RadioactiveDecay.hpp"
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
    SetDefaults();
  }

  void BrachyDoseTG43::SetDefaults()
  {
    data_loaded = false;
    precomputed = false;
    delta_radius = delta_theta = dose_rate_constant = source_length = 0.0;
    reference = source_type = nuclide_name = vendor_name = model_name = "NA";
  }

  void BrachyDoseTG43::ClearData()
  {
    SetDefaults();

    r_g_r.clear();
    g_r_line_data.clear();
    g_r_point_data.clear();

    theta_anisotropy_2d.clear();
    r_anisotropy.clear();
    anisotropy_2d_data.clear();
    anisotropy_1d_data.clear();
  }

  void BrachyDoseTG43::LoadData(std::string file_name)
  {
    // Clear previous data before loading new
    ClearData();

    // Initialization and open file
    std::ifstream fin;
    std::string input, str;
    size_t p1, p2;
    bool reading = true;
    double temp;
    std::vector<double>::iterator it1, it2;

    fin.open(file_name.c_str());
    if(fin.bad())
    {
      throw std::runtime_error("BrachyDoseTG43 Error: bad ifstream");
    }

    /////////////////////
    // Get header data //
    /////////////////////
    for(int n = 0; n < 9; n++)
    {
      if(!std::getline(fin, input))
      {
        throw std::runtime_error(
          "BrachyDoseTG43 Error: file header format incorrect"
        );
      }
      // Skip first two lines, import header information
      if(n > 1)
      {
        p1 = input.find(':');
        if(p1 == std::string::npos)
        {
          throw std::runtime_error(
            "BrachyDoseTG43 Error: file header format incorrect"
          );
        }
        p1+=2;
        if(p1 >= input.length())
        {
          throw std::runtime_error(
            "BrachyDoseTG43 Error: file header format incorrect"
          );
        }
        switch(n)
        {
          case 2: reference = input.substr(p1); break;
          case 3: source_type = input.substr(p1); break;
          case 4: nuclide_name = input.substr(p1); break;
          case 5: vendor_name = input.substr(p1); break;
          case 6: model_name = input.substr(p1); break;
          case 7: std::stringstream(input.substr(p1)) >> dose_rate_constant; break;
          case 8: std::stringstream(input.substr(p1)) >> source_length; break;
        }
      }
    }

    ///////////////////////////////////////////////////////////
    // Get radial dose function data (interpolate as needed) //
    ///////////////////////////////////////////////////////////
    // Skip to data
    for(int n = 0; n < 3; n++)
    {
      if(!std::getline(fin, input))
      {
        throw std::runtime_error(
          "BrachyDoseTG43 Error: radial dose function file format incorrect"
        );
      }
    }
    // Read radial dose function data
    while(std::getline(fin, input))
    {
      if(input.find("end radial dose function data") != std::string::npos) break;
      std::vector<std::string> data = LineRead(input, ',');
      if(data.size() < 3)
      {
        throw std::runtime_error(
          "BrachyDoseTG43 Error: radial dose function file format incorrect"
        );
      }
      std::stringstream(data[0]) >> temp;
      r_g_r.push_back(temp);
      std::stringstream(data[1]) >> temp;
      g_r_line_data.push_back(temp);
      std::stringstream(data[2]) >> temp;
      g_r_point_data.push_back(temp);
    }
    if(r_g_r.size() == 0)
    {
      throw std::runtime_error(
        "BrachyDoseTG43 Error: radial dose function data missing"
      );
    }
    // Nearest neighbor interpolation for r = 0
    if(r_g_r[0] != 0.0)
    {
      it1 = r_g_r.begin();
      r_g_r.insert(it1, 0.0);
      it1 = g_r_line_data.begin();
      g_r_line_data.insert(it1, g_r_line_data[0]);
      it1 = g_r_point_data.begin();
      g_r_point_data.insert(it1, g_r_point_data[0]);
    }

    ////////////////////////////////////////////////////
    // Get 2D anisotropy data (interpolate as needed) //
    ////////////////////////////////////////////////////
    bool interpolate_zero = false;
    // Skip to data
    for(int n = 0; n < 3; n++)
    {
      if(!std::getline(fin, input))
      {
        throw std::runtime_error(
          "BrachyDoseTG43 Error: anisotropy function file format incorrect"
        );
      }
    }
    // Get column headers
    if(!std::getline(fin, input))
    {
      throw std::runtime_error(
        "BrachyDoseTG43 Error: anisotropy function file format incorrect"
      );
    }
    std::vector<std::string> column = LineRead(input, ',');
    if(column.size() < 3)
    {
      throw std::runtime_error(
        "BrachyDoseTG43 Error: anisotropy function file format incorrect"
      );
    }
    for(int n = 1; n < column.size(); n++)
    {
      std::stringstream(column[n]) >> temp;
      r_anisotropy.push_back(temp);
    }
    // Nearest neighbor interpolation for r = 0
    double r_min = r_anisotropy[0];
    if(r_anisotropy[0] != 0.0)
    {
      interpolate_zero = true;
      it1 = r_anisotropy.begin();
      r_anisotropy.insert(it1, 0.0);
    }
    // Get row data
    while(std::getline(fin, input))
    {
      if(input.find("end anisotropy function data") != std::string::npos) break;
      std::vector<std::string> row = LineRead(input, ',');
      if(row.size() < 3)
      {
        throw std::runtime_error(
          "BrachyDoseTG43 Error: anisotropy function file format incorrect"
        );
      }
      if(row[0] == "point")
      {
        for(int n = 1; n < column.size(); n++)
        {
          std::stringstream(row[n]) >> temp;
          anisotropy_1d_data.push_back(temp);
        }
        // Nearest neighbor interpolation for r = 0
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
            // Linear interpolation for missing data
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
        // Nearest neighbor interpolation for r = 0
        if(interpolate_zero)
        {
          it1 = buffer.begin();
          buffer.insert(it1, buffer[0]);
        }
        anisotropy_2d_data.push_back(buffer);
      }
    }

    if(anisotropy_1d_data.size() == 0 && anisotropy_2d_data.size() == 0)
    {
      throw std::runtime_error(
        "BrachyDoseTG43 Error: anisotropy function data missing"
      );
    }

    fin.close();
    data_loaded = true;
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

  BrachyDoseTG43::CalcStats BrachyDoseTG43::CalcDoseBrachyPlan(double ref_aks,
    struct tm ref_dt, BrachyPlan plan, Vec3<double> point, bool line_source)
  {
    // Initial error checking
    if(plan.Sources.size() == 0)
    {
      throw std::runtime_error(
        "BrachyDoseTG43 Error: plan has no sources"
      );
    }
    if(plan.Applicators.size() == 0)
    {
      throw std::runtime_error(
        "BrachyDoseTG43 Error: plan has no applicators"
      );
    }
    // Set initial variables
    int counter = 0, ind = 0;
    CalcStats Results;
    Results.DoseSum = 0.0;
    Results.MinRadius = 30.0; Results.MaxRadius = -0.1; Results.AveRadius = 0.0;
    Results.MinTheta = 200.0; Results.MaxTheta = -0.1; Results.AveTheta = 0.0;
    Vec3<double> dist, direction, sp_vec;

    Radionuclide Isotope(nuclide_name);
    double df = Isotope.DecayFactor(ref_dt,
      plan.Sources[0].StrengthReferenceDateTime);
    Results.OriginalStrength = ref_aks;
    Results.NuclideName = nuclide_name;
    Results.NuclideHalfLife = Isotope.GetHalfLife();
    Results.NuclideHalfLifeUnits = Isotope.GetHalfLifeUnits();
    Results.ElapsedTime = Isotope.GetElapsedTime();
    Results.ElapsedTimeUnits = Isotope.GetElapsedTimeUnits();
    Results.DecayFactor = df;
    Results.DecayedStrength = ref_aks * Results.DecayFactor;

    for(const auto &it_a : plan.Applicators)
    {
      for(const auto &it_c : it_a.Channels)
      {
        // If there are fewer than 2 control points, throw error
        if(it_c.ControlPoints.size() <= 1)
        {
          throw std::runtime_error(
            "BrachyDoseTG43 Error: every channel must have at least two control points"
          );
        }
        // If first control point has non-zero weight; throw error
        if(it_c.ControlPoints[0].Weight > 1e-05)
        {
          throw std::runtime_error(
            "BrachyDoseTG43 Error: first control point in channel must be zero"
          );
        }
        // Initialize calculation variable
        double prev = 0.0;
        // Calculate initial direction vector (if unable, treat as point source)
        bool direction_found = true;
        ind = 0;
        sp_vec = it_c.ControlPoints[1].Position -
          it_c.ControlPoints[0].Position;
        while((sp_vec.Magnitude() < 1.0e-5) &&
          ind < it_c.ControlPoints.size()-3)
        {
          ind++;
          sp_vec = it_c.ControlPoints[(ind+1)].Position -
            it_c.ControlPoints[ind].Position;
        }
        if((sp_vec.Magnitude() < 1.0e-5) &&
          ind == it_c.ControlPoints.size()-1)
        {
          direction_found = false;
        }
        else
        {
          direction = it_c.ControlPoints[(ind+1)].Position
            - it_c.ControlPoints[ind].Position;
          direction.Normalize();
        }

        for(auto it_p = std::next(it_c.ControlPoints.begin());
          it_p != it_c.ControlPoints.end(); it_p++)
        {
          // Calculate total weight/time for current control point (CP)
          double weight = it_p->Weight - prev;
          prev = it_p->Weight;
          double dwell_time = it_c.TotalTime*
            (weight / it_c.FinalCumulativeTimeWeight);
          // Calculate number of sub-points for this CP and sub-point dwell time
          int n_sub_points;
          sp_vec = it_p->Position - (it_p-1)->Position;
          if(sp_vec.Magnitude() < 1.0e-5) n_sub_points = 2;
          else n_sub_points = std::round(sp_vec.Magnitude() / 2.0d) + 1;
          double sub_point_time = (dwell_time/3600.0) / double(n_sub_points);
          // Sum dose for each sub-point
          for(int sp = 0; sp < n_sub_points; sp++)
          {
            // Calculate sub-point position and radius to calculation point
            dist = point - ((it_p-1)->Position + (sp_vec / double(n_sub_points)));
            double r = dist.Magnitude() / 10.0;
            Results.AveRadius += r;
            if(r > Results.MaxRadius) Results.MaxRadius = r;
            if(r < Results.MinRadius) Results.MinRadius = r;
            // If using line source calculation, calculate theta first
            if(line_source && direction_found)
            {
              // If position changes, update direction
              if(sp_vec.Magnitude() > 1.0e-5)
              {
                direction = it_p->Position - (it_p-1)->Position;
                direction.Normalize();
              }
              // Calculate theta and dose
              double theta = acos(Dot(dist, direction) /
                (dist.Magnitude() * direction.Magnitude()));
              theta *= (180.0 / M_PI);
              Results.AveTheta += theta;
              if(theta > Results.MaxTheta) Results.MaxTheta = theta;
              if(theta < Results.MinTheta) Results.MinTheta = theta;
              Results.DoseSum += sub_point_time*CalcDoseRateLine(
                Results.DecayedStrength, r, theta);
            }
            // Otherwise use point calculation with radius only
            else
            {
              Results.DoseSum += sub_point_time*CalcDoseRatePoint(
                Results.DecayedStrength, r);
            }
            counter++;
          }
        }
      }
    }
    Results.AveRadius /= double(counter);
    Results.AveTheta /= double(counter);

    return Results;
  }
}
