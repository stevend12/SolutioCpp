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
// Gamma Index Calculation Functions                                          //
// (GammaIndex.cpp)                                                           //
//                                                                            //
// Steven Dolly                                                               //
// July 28, 2020                                                              //
//                                                                            //
// This file contains the main code for the functions which will perform      //
// gamma analysis, both on 1D profiles as well as 2D and 3D images.           //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#include "GammaIndex.hpp"

#include <iostream>
#include <cmath>

#include <itkImageDuplicator.h>
#include <itkMinimumMaximumImageFilter.h>
#include <itkImageRegionIterator.h>

#include "../Utilities/DataInterpolation.hpp"

namespace solutio
{
  // Gamma index calculation for 1D dose profiles
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
    double gamma, g, norm_dose;
    for(auto ite = test_dose.begin(); ite != test_dose.end(); ++ite)
    {
      for(auto itr = resampled_ref.begin(); itr != resampled_ref.end(); ++itr)
      {
        if((ite->first - itr->first) >
          settings.SearchRadius*settings.DistCriteria) g = 10.0;
        else
        {
          if(settings.GlobalMax) norm_dose = max_dose;
          else norm_dose = itr->second;
          double dose = ((ite->second - itr->second) / norm_dose) / settings.DoseCriteria;
          double dist = (ite->first - itr->first) / settings.DistCriteria;
          g = sqrt(dose*dose + dist*dist);
        }
        if(itr == resampled_ref.begin() || g < gamma) gamma = g;
      }
      gamma_values.push_back(gamma);
    }
    // Calculate gamma pass rate
    unsigned int pass_points = 0, total_points = 0;
    for(auto it = gamma_values.begin(); it != gamma_values.end(); ++it)
    {
      double d = test_dose[(std::distance(gamma_values.begin(), it))].second;
      if(d >= settings.DoseThreshold*max_dose)
      {
        total_points++;
        if(*it <= settings.PassThreshold) pass_points++;
      }
    }
    pass_rate = double(pass_points) / double(total_points);

    return gamma_values;
  }

  // Gamma index calculation for 2D dose images
  ItkImageF3::Pointer CalcGammaIndex2D(
    ItkImageF3::Pointer test_dose, ItkImageF3::Pointer ref_dose,
    GammaIndexSettings settings, double &pass_rate)
  {
    using ImageType = ItkImageF3;

    // Get test and reference image sizes
    int test_size[3], ref_size[3];
    ImageType::RegionType image_region;
    ImageType::SizeType image_size;
    ImageType::IndexType image_start;
    image_region = test_dose->GetLargestPossibleRegion();
    image_size = image_region.GetSize();
    for(int n = 0; n < 3; n++) test_size[n] = image_size[n];
    image_region = ref_dose->GetLargestPossibleRegion();
    image_size = image_region.GetSize();
    for(int n = 0; n < 3; n++) ref_size[n] = image_size[n];

    // Create gamma image by copying test dose image
    using DuplicatorType = itk::ImageDuplicator<ImageType>;
    DuplicatorType::Pointer duplicator = DuplicatorType::New();
    duplicator->SetInputImage(test_dose);
    duplicator->Update();
    ImageType::Pointer gamma_image = duplicator->GetOutput();
    gamma_image->FillBuffer(itk::NumericTraits<float>::Zero);

    // Get image origin & spacing
    ImageType::PointType to, ro;
    to = test_dose->GetOrigin();
    ro = ref_dose->GetOrigin();
    const ImageType::SpacingType & ts = test_dose->GetSpacing();
    const ImageType::SpacingType & rs = ref_dose->GetSpacing();

    // Check for 2D, monochrome images
    bool checks_out = (test_size[2] == 1) && (ref_size[2] == 1);
    if(!checks_out)
    {
      std::cout << "Test or reference dose image not 2D\n";
      return gamma_image;
    }

    // Checks for coplanar images; parallel and same plane (within rounding error)
    // Implement later...

    //////////////////////////////
    // Preliminary calculations //
    //////////////////////////////

    // Get reference image max
    typedef itk::MinimumMaximumImageFilter<ImageType> MinMaxFilterType;
    MinMaxFilterType::Pointer filter = MinMaxFilterType::New();
    filter->SetInput(ref_dose);
    filter->Update();
    const float max_dose = filter->GetMaximum();

    // Calculate search distance
    const float search_dist = settings.SearchRadius*settings.DistCriteria;

    // Test/reference/output image iterators and points
    using ConstIteratorType = itk::ImageRegionConstIterator<ImageType>;
    using IteratorType = itk::ImageRegionIterator<ImageType>;
    ConstIteratorType it_test(test_dose, test_dose->GetLargestPossibleRegion());
    ConstIteratorType it_ref(ref_dose, ref_dose->GetLargestPossibleRegion());
    IteratorType it_out(gamma_image, gamma_image->GetLargestPossibleRegion());
    ImageType::PointType tp, rp;
    ImageType::IndexType ri;

    // Create position vector image for reference dose
    using VecImageType = itk::Image<itk::Vector<float, 3>, 3>;
    VecImageType::IndexType mask_start = { { 0, 0, 0 } };
    VecImageType::SizeType mask_size = {
      { ref_size[0], ref_size[1], ref_size[2] }
    };

    VecImageType::RegionType mask_region;
    mask_region.SetIndex(mask_start);
    mask_region.SetSize(mask_size);

    VecImageType::Pointer mask_image = VecImageType::New();
    mask_image->SetRegions(mask_region);
    mask_image->Allocate();

    using VecIteratorType = itk::ImageRegionIterator<VecImageType>;
    VecIteratorType it_mask(mask_image, mask_image->GetLargestPossibleRegion());

    VecImageType::PixelType pixel_position;

    it_ref.GoToBegin();
    it_mask.GoToBegin();
    while(!it_ref.IsAtEnd())
    {
      ref_dose->TransformIndexToPhysicalPoint(it_ref.GetIndex(), rp);
      pixel_position[0] = rp[0];
      pixel_position[1] = rp[1];
      pixel_position[2] = rp[2];
      mask_image->SetPixel(it_mask.GetIndex(), pixel_position);
      ++it_ref;
      ++it_mask;
    }

    ///////////////////////////
    // Calculate gamma index //
    ///////////////////////////

    float norm_dose, gamma, g, n_test = 0.0;
    pass_rate = 0.0;

    // Iterate through all test dose pixels
    it_test.GoToBegin();
    it_out.GoToBegin();
    while(!it_test.IsAtEnd())
    {
      // Get test pixel dose and position
      const ImageType::PixelType td = it_test.Value();
      test_dose->TransformIndexToPhysicalPoint(it_test.GetIndex(), tp);
      itk::Vector<float, 3> tv = tp.GetVectorFromOrigin();
      // Get nearest reference pixel and calculate search ROI according to the
      // search distance parameter
      ref_dose->TransformPhysicalPointToIndex(tp, ri);
      int m_start, m_end, m_size;
      for(int n = 0; n < 3; n++)
      {
        m_start = ri[n] - round(search_dist / rs[n]);
        if(m_start < 0) m_start = 0;
        if(m_start > ref_size[n]-1) m_start = ref_size[n]-1;
        m_end = ri[n] + round(search_dist / rs[n]);
        if(m_end < 0) m_end = 0;
        if(m_end > ref_size[n]-1) m_end = ref_size[n]-1;
        m_size = (m_end - m_start) + 1;

        image_start[n] = mask_start[n] = m_start;
        image_size[n] = mask_size[n] = m_size;
      }
      image_region.SetIndex(image_start);
      image_region.SetSize(image_size);
      it_ref.SetRegion(image_region);
      mask_region.SetIndex(mask_start);
      mask_region.SetSize(mask_size);
      it_mask.SetRegion(mask_region);
      // Iterate through ROI to find minimum gamma
      it_ref.GoToBegin();
      it_mask.GoToBegin();
      while(!it_ref.IsAtEnd())
      {
        // Get reference dose pixel position and value
        const ImageType::PixelType rd = it_ref.Value();
        const VecImageType::PixelType rv = it_mask.Value();
        // Calculate dose and distance differences
        if(settings.GlobalMax) norm_dose = max_dose;
        else norm_dose = rd;
        float dose = ((td - rd) / norm_dose) / settings.DoseCriteria;
        float dist = (tv-rv).GetNorm() / settings.DistCriteria;
        // Calculate gamma index and compare for minimum; increment
        g = sqrt(dose*dose + dist*dist);
        if(it_ref.IsAtBegin() || g < gamma) gamma = g;
        ++it_ref;
        ++it_mask;
      }
      // Set pixel as minimum gamma value
      it_out.Set(gamma);
      // If dose is above threshold, include in gamma pass rate calculation
      if(td >= settings.DoseThreshold*max_dose)
      {
        n_test += 1.0;
        if(gamma <= settings.PassThreshold) pass_rate += 1.0;
      }
      // Move to next test pixel
      ++it_test;
      ++it_out;
    }

    // Calculate pass rate
    pass_rate /= n_test;

    return gamma_image;
  }
}
