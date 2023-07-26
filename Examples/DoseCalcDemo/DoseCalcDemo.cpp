/******************************************************************************/
/*                                                                            */
/* Copyright 2021 Steven Dolly                                                */
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
// DoseCalcDemo.cpp                                                           //
// Demonstration of SolutioCpp Therapy Dose Calculation Features              //
// Created February 8, 2021 (Steven Dolly)                                    //
//                                                                            //
// This file demonstrates various uses of the SolutioCpp library for therapy  //
// dose calculation algorithms. Results can be viewed in the command line or  //
// using Octave/MATLAB and the script DoseCalcDemo.m (in same folder).        //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

// C++ headers
#include <iostream>
#include <fstream>

// Solutio library headers
#include "Therapy/BrachyDoseTG43.hpp"
#include "Utilities/DataInterpolation.hpp"

int main()
{
  std::cout << "This program tests therapy dose calculation aspects of the SolutioCpp library.\n\n";

  ///////////////////////////////
  // Test BrachyDoseTG43 class //
  ///////////////////////////////
  std::cout << "Brachytherapy Dose Calculation (TG 43)\n";
  std::cout << "--------------------------------------\n\n";
  // Set data folder
  std::string folder = "../../Data/SourceData/";
  std::string source_file = folder + "CLRP_HDR_Ir-192_Nucletron_microSelectron-v2_TG43.txt";
  solutio::BrachyDoseTG43 BrachyDoseCalc;
  // Loads data from file
  BrachyDoseCalc.LoadData(source_file);
  // Set calculation parameters
  double air_kerma_strength = 40000.0; // Units: U
  double radius = 2.3; // Units: cm
  double theta = 65.0; // Units: deg
  // Calculates dose rate at a point for the given parameters (line source)
  double dose_rate =
    BrachyDoseCalc.CalcDoseRateLine(air_kerma_strength, radius, theta);
  std::cout << "Dose Rate (Line) = " << dose_rate << " cGy/hr\n";
  // Calculates dose rate at a point for the given parameters (point source)
  dose_rate =
    BrachyDoseCalc.CalcDoseRatePoint(air_kerma_strength, radius);
  std::cout << "Dose Rate (Point) = " << dose_rate << " cGy/hr\n";

  //////////////////////////////////////////
  // Calculate secondary TG-43 parameters //
  //////////////////////////////////////////

  // Geometry factor
  std::cout << "G(" << radius << ", " << theta << ") = " <<
    solutio::GeometryFactorTG43(radius, theta, BrachyDoseCalc.GetSourceLength())
    << '\n';
  // Radial dose function
  std::cout << "g_r(" << radius << ") = " <<
    BrachyDoseCalc.GetRadialDoseFunctionLine(radius) << '\n';
  // Anisotropy function
  std::cout << "anisotropy(" << radius << ", " << theta << ") = " <<
    BrachyDoseCalc.GetAnisotropyFunctionLine(radius, theta) << '\n';

  //////////////////////////////////////////////
  // Validation Plot: TG 43 Isodose Line Plot //
  //////////////////////////////////////////////

  // Optional: pre-compute even-spaced data tables to speed up calculation time
  BrachyDoseCalc.PreCompute(0.2, 1.0);

  // Create isodose map and save to text file (plot with DoseCalcDemo.m)
  const int num_angles = 100;
  const int num_isodose = 5;

  double isodose[num_isodose] = {50.0, 100.0, 200.0, 400.0, 2000.0};
  double line_radii[num_angles][num_isodose];

  std::vector<double> radii;
  for(int r = 1; r < 51; r++) radii.push_back(0.2*r);

  for(int n = 0; n < num_angles; n++)
  {
    std::vector<double> values;
    for(int r = 0; r < radii.size(); r++)
    {
      values.push_back(BrachyDoseCalc.CalcDoseRateLine(100.0, radii[r],
        180.0 * n / num_angles));
    }
    for(int r = 0; r < num_isodose; r++)
    {
      line_radii[n][r] = solutio::LinearInterpolation(values, radii, isodose[r]);
    }
  }

  std::ofstream fout("tg43_isodose.txt");
  for(int n = 0; n < num_angles; n++)
  {
    fout << 180.0 * n / num_angles << ",";
    for(int r = 0; r < num_isodose; r++)
    {
      fout << line_radii[n][r];
      if(r != (num_isodose-1)) fout << ",";
      else fout << '\n';
    }
  }
  fout.close();
}
