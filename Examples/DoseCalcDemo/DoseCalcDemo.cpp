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

// Solutio library headers
#include "Therapy/BrachyDoseTG43.hpp"

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
  BrachyDoseCalc.LoadData(source_file);
}
