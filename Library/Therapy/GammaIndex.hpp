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
// (GammaIndex.hpp)                                                           //
//                                                                            //
// Steven Dolly                                                               //
// July 28, 2020                                                              //
//                                                                            //
// This file contains the header defining the functions which will perform    //
// gamma analysis, both on 1D profiles as well as 2D and 3D images.           //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#ifndef GAMMAANALYSIS_HPP
#define GAMMAANALYSIS_HPP

#include <vector>

#include "../Utilities/SolutioItk.hpp"

namespace solutio
{
  //////////////////////////////////////////////////////////////////////////////
  // Settings for Gamma Index Calculation                                     //
  // ------------------------------------                                     //
  //                                                                          //
  // GlobalMax: Determines whether the global max dose or the dose at         //
  // reference location is used as the normalization for the dose difference  //
  // calculation.                                                             //
  //                                                                          //
  // DoseCriteria: Expressed as a fraction (e.g. 0.03 = 3%)                   //
  //                                                                          //
  // DistCriteria: Units should match units for input doses                   //
  //                                                                          //
  // PassThreshold: The gamma value threshold to determine the gamma pass     //
  // rate (almost always equal to 1)                                          //
  //                                                                          //
  // DoseThreshold: Cutoff for gamma pass rate calculation. Example: a value  //
  // of 0.1 means that all dose values less than 10% of the reference max     //
  // dose will not be included in the pass rate calculation.                  //
  //                                                                          //
  // SearchRadius: The search radius for the reference dose values used to    //
  // calculate gamma. Rather than calculating gamma for every reference dose  //
  // point at every test point, only the reference points within this radial  //
  // distance from the test point will be used for calculation. Expressed     //
  // relative to the DistCriteria. For example, if DistCriteria = 3 mm and    //
  // the SearchRadius = 2, then the search distance will be 2*3 = 6 mm.       //
  //                                                                          //
  // ResampleRate: Determines the resampling rate of the reference dose used  //
  // for calculation. Smaller values lead to more accurate gamma values but   //
  // at the cost of increased calculation times. A value < 0 will result in   //
  // the function calculating its own rate. Only used for 1D calculations.    //
  //                                                                          //
  //////////////////////////////////////////////////////////////////////////////
  struct GammaIndexSettings
  {
    bool GlobalMax = true;
    double DoseCriteria = 0.03;
    double DistCriteria = 3.0;
    double PassThreshold = 1.0;
    double DoseThreshold = 0.1;
    double SearchRadius = 2.0;
    double ResampleRate = -1.0;
  };

  // Gamma index calculation for 1D dose profiles
  using DoublePairVec = std::vector< std::pair<double,double> >;
  std::vector<double> CalcGammaIndex(DoublePairVec test_dose,
    DoublePairVec ref_dose, GammaIndexSettings settings, double &pass_rate);

  // Gamma index calculation for 2D dose images
  ItkImageF3::Pointer CalcGammaIndex2D(
    ItkImageF3::Pointer test_dose, ItkImageF3::Pointer ref_dose,
    GammaIndexSettings settings, double &pass_rate
  );
}

#endif
