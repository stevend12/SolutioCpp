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

  // Gamma index calculation for two 2D profiles
  solutio::GenericImage<double> CalcGammaIndex2D(
    solutio::GenericImage<double> test_dose,
    solutio::GenericImage<double> ref_dose,
    GammaIndexSettings settings, double &pass_rate)
  {
    solutio::GenericImage<double> gamma_image;

    // Check for 2D, monochrome images
    bool checks_out = true;
    unsigned int * test_im_size = test_dose.GetImageSize();
    unsigned int * ref_im_size = ref_dose.GetImageSize();
    checks_out = (test_im_size[2] == 1) && (test_im_size[3] == 1) &&
      (ref_im_size[2] == 1) && (ref_im_size[3] == 1);
    if(!checks_out)
    {
      std::cout << "Test or reference dose image not monochrome 2D\n";
      return gamma_image;
    }

    // Preliminary calculations
    double max_dose = ref_dose.GetMaxValue();
    double * ref_im_dim = ref_dose.GetPixelDimensions();
    double * test_im_dim = test_dose.GetPixelDimensions();
    double * ref_im_o = ref_dose.GetPixelOrigin();
    double * test_im_o = test_dose.GetPixelOrigin();

    // Calculate gamma index image
    std::vector<double> gamma_vector;
    std::vector<double> test_vector = test_dose.GetImage();
    std::vector<double> ref_vector = ref_dose.GetImage();
    double norm_dose, gamma, g, dose, tx, ty, rx, ry, dist;
    for(auto it = test_vector.begin(); it != test_vector.end(); ++it)
    {
      if(*it < settings.Threshold*max_dose) gamma_vector.push_back(0.0);
      else
      {
        tx = 0.0;
        ty = 0.0;
        for(auto itr = ref_vector.begin(); itr != ref_vector.end(); ++itr)
        {
          if(settings.GlobalMax) norm_dose = max_dose;
          else norm_dose = *itr;
          dose = ((*it - *itr) / norm_dose) / settings.DoseCriteria;
          rx = 0.0;
          ry = 0.0;
          dist = sqrt(((tx-rx)*(tx-rx)) + ((ty-ry)*(ty-ry))) / settings.DistCriteria;
          g = sqrt(dose*dose + dist*dist);
          if(it == ref_vector.begin() || g < gamma) gamma = g;
        }
      }
    }

    return gamma_image;
  }
}
