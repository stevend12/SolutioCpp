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
// Gamma Analysis Functions                                                   //
// (GammaAnalysis.cpp)                                                        //
//                                                                            //
// Steven Dolly                                                               //
// July 28, 2020                                                              //
//                                                                            //
// This file contains the main code for the functions which will perform      //
// gamma analysis, both on 1D profiles as well as 2D and 3D images.           //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#include "GammaIndex.hpp"
#include "../Utilities/DataInterpolation.hpp"

#include <iostream>
#include <cmath>

namespace solutio
{
  std::vector<double> CalcGammaIndex(DoublePairVec test_dose,
    DoublePairVec ref_dose, GammaIndexSettings settings, double &pass_rate)
  {
    std::vector<double> gamma_values;
    // Check for empty vectors
    if(test_dose.empty() || ref_dose.empty())
    {
      std::cout << "Error: Test or reference dose is empty, returning empty vector\n";
      return gamma_values;
    }
    // Resample reference dose to improve calculation; get max dose to use for
    // global max and threshold settings
    double dx, xp, min_width, max_dose;
    DoublePairVec resampled_ref;
    std::pair<double,double> temp_resample;
    temp_resample.first = xp = ref_dose[0].first;
    temp_resample.second = max_dose = ref_dose[0].second;
    resampled_ref.push_back(temp_resample);
    if(ref_dose.size() > 1)
    {
      for(int n = 1; n < ref_dose.size(); n++)
      {
        double w = ref_dose[n].first - ref_dose[(n-1)].first;
        if(n == 1 || w < min_width) min_width = w;
        if(ref_dose[n].second > max_dose) max_dose = ref_dose[n].second;
      }
      if(settings.ResampleRate < 0.0) dx = 0.1*min_width;
      else dx = settings.ResampleRate*min_width;

      do
      {
        xp += dx;
        temp_resample.first = xp;
        temp_resample.second = LinearInterpolation<double>(ref_dose, xp);
        resampled_ref.push_back(temp_resample);
      } while (xp < ref_dose.back().first-dx);

      temp_resample.first = ref_dose[(ref_dose.size()-1)].first;
      temp_resample.second = ref_dose[(ref_dose.size()-1)].second;
      resampled_ref.push_back(temp_resample);
    }
    // Calculate gamma index for each test dose point
    double gamma, norm_dose;
    for(auto ite = test_dose.begin(); ite != test_dose.end(); ++ite)
    {
      for(auto itr = resampled_ref.begin(); itr != resampled_ref.end(); ++itr)
      {
        if(settings.GlobalMax) norm_dose = max_dose;
        else norm_dose = itr->second;
        double dose = ((ite->second - itr->second) / norm_dose) / settings.DoseCriteria;
        double dist = (ite->first - itr->first) / settings.DistCriteria;
        double g = sqrt(dose*dose + dist*dist);
        if(itr == resampled_ref.begin() || g < gamma) gamma = g;
      }
      gamma_values.push_back(gamma);
    }
    // Calculate gamma pass rate
    unsigned int pass_points = 0, total_points = 0;
    for(auto it = gamma_values.begin(); it != gamma_values.end(); ++it)
    {
      double d = test_dose[(std::distance(gamma_values.begin(), it))].second;
      if(d >= settings.Threshold*max_dose)
      {
        total_points++;
        if(*it <= 1.0) pass_points++;
      }
    }
    pass_rate = double(pass_points) / double(total_points);

    return gamma_values;
  }
}
