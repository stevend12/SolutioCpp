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
// PhotonDemo.cpp                                                             //
// Demonstration of Photon Interaction Functions                              //
// Created September 30, 2016 (Steven Dolly)                                  //
//                                                                            //
// This file demonstrates various uses of the Solutio C++ library for photon  //
// interaction simulations. Results can be viewed using Octave/MATLAB and the //
// script PhysicsDemo.m (in same folder)                                      //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

// C++ headers
#include <iostream>
#include <fstream>

// Solutio library headers
#include "Physics/NistEstar.hpp"
#include "Physics/NistPad.hpp"
#include "Imaging/Tasmip.hpp"

int main()
{
  std::cout << "This program tests physics aspects of the SolutioCpp library.\n";
  ////////////////////////
  // Test NistPad class //
  ////////////////////////
  std::cout << "Photon mass attenution coefficients @ 2.5 MeV:\n";
  // Set data folder
  std::string folder = "../../Data/NISTX";
  // Load NIST attenuation data for lead (using atomic number) and interpolate
  solutio::NistPad Lead(folder);
  Lead.Load(82);
  std::cout << "Lead: " << Lead.MassAttenuation(2.5) << '\n';
  // Load NIST attenuation data for aluminum (using name) and interpolate
  solutio::NistPad Aluminum(folder);
  Aluminum.Load("Aluminum");
  std::cout << "Aluminum: " << Aluminum.MassAttenuation(2.5) << '\n';
  // Load NIST attenuation data for water (using name) and interpolate; print
  // data to terminal
  solutio::NistPad Water(folder);
  Water.Load("Water");
  std::cout << "Water: " << Water.MassAttenuation(2.5) << '\n' << '\n';
  Water.PrintData();
  // Print data to be viewed in Octave/MATLAB using PhysicsDemo.m
  // Compares attenuation for lead vs. water for 0.2 - 20 MeV
  std::ofstream fout("photon_data.txt");
  for(int n = 1; n < 1001; n++)
  {
    double en = 0.001+(10.0*double(n));
    fout << en << '\t';
    fout << Lead.MassAttenuation(en) << '\t';
    fout << Water.MassAttenuation(en) << '\n';
  }
  fout.close();

  //////////////////////////
  // Test NistEstar class //
  //////////////////////////
  std::cout << "Electron total stopping powers @ 2.5 MeV:\n";
  // Set data folder
  std::string folder_e = "../../Data/ESTAR";
  // Load ESTAR data for lead (using atomic number) and interpolate
  solutio::NistEstar Lead_e(folder_e);
  Lead_e.Load(82);
  std::cout << "Lead: " << Lead_e.TotalStoppingPower(2.5) << '\n';
  // Load ESTAR attenuation data for aluminum (using name) and interpolate
  solutio::NistEstar Aluminum_e(folder_e);
  Aluminum_e.Load("Aluminum");
  std::cout << "Aluminum: " << Aluminum_e.TotalStoppingPower(2.5) << '\n';
  // Load NIST attenuation data for water (using name) and interpolate
  solutio::NistEstar Water_e(folder_e);
  Water_e.Load("Water, Liquid");
  std::cout << "Water: " << Water_e.TotalStoppingPower(2.5) << '\n' << '\n';
  Water_e.PrintData();
  // Print data to be viewed in Octave/MATLAB using PhysicsDemo.m
  // Compares stopping powers for lead vs. water for 0.2 - 20 MeV
  fout.open("electron_data.txt");
  for(int n = 1; n < 1001; n++)
  {
    double en = 0.1*double(n);
    fout << en << '\t';
    fout << Lead_e.ColStoppingPower(en) << '\t';
    fout << Lead_e.RadStoppingPower(en) << '\t';
    fout << Water_e.ColStoppingPower(en) << '\t';
    fout << Water_e.RadStoppingPower(en) << '\n';
  }
  fout.close();

  /////////////////
  // Test TASMIP //
  /////////////////
  // Calculate tungsten x-ray spectum using the TASMIP algorithm
  std::vector<double> kVp120 = solutio::Tasmip(120, 0.0, "Aluminum", folder);
  std::vector<double> kVp120_3mm_Al = solutio::Tasmip(120, 3.0, "Aluminum", folder);
  std::vector<double> kVp120_3mm_Cu = solutio::Tasmip(120, 3.0, "Copper", folder);
  // Print spectrum data to file; read using Octave/Matlab and PhysicsResults.m
  fout.open("spectrums.txt");
  for(int n = 0; n < 151; n++)
  {
    fout << kVp120[n] << '\t';
    fout << kVp120_3mm_Al[n] << '\t';
    fout << kVp120_3mm_Cu[n] << '\n';
  }
  fout.close();

  // Return if success
  return 0;
}
