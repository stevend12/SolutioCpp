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
// interaction simulations.                                                   //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

// C++ headers
#include <iostream>
#include <fstream>

// Solutio library headers
#include "Physics/NistPad.hpp"
#include "Imaging/Tasmip.hpp"

int main()
{
  std::cout << "This program tests photon aspects of the Solutio library.\n";
  
  // Test NistPad class
  std::cout << "Photon mass attenution coefficients @ 2.5 MeV:\n";
  
  // Load NIST attenuation data for lead (using atomic number) and interpolate
  solutio::NistPad Lead;
  Lead.Load(82);
  std::cout << "Lead: " << Lead.MassAttenuation(2.5) << '\n';
  
  // Load NIST attenuation data for water (using name) and interpolate
  solutio::NistPad Aluminum;
  Aluminum.Load("Aluminum");
  std::cout << "Aluminum: " << Aluminum.MassAttenuation(2.5) << '\n';
  
  // Load NIST attenuation data for water (using name) and interpolate
  solutio::NistPad Water;
  Water.Load("Water");
  std::cout << "Water: " << Water.MassAttenuation(2.5) << '\n' << '\n';
  Water.PrintData();
  
  // Calculate tungsten x-ray spectum using the TASMIP algorithm
  std::vector<double> kVp120 = solutio::Tasmip(120, 0.0, "Aluminum");
  std::vector<double> kVp120_3mm_Al = solutio::Tasmip(120, 3.0, "Aluminum");
  std::vector<double> kVp120_3mm_Cu = solutio::Tasmip(120, 3.0, "Copper");
  
  // Print spectrum data to file; read using Octave/Matlab and PhotonResults.m
  std::ofstream fout("spectrums.txt");
  for(int n = 0; n < 151; n++){
    fout << kVp120[n] << '\t';
    fout << kVp120_3mm_Al[n] << '\t';
    fout << kVp120_3mm_Cu[n] << '\n';
  }
  fout.close();

  // Return if success
  return 0;
}
