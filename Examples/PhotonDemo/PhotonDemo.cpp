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

// Solutio library headers
#include "Physics/NistPad.hpp"
#include "Imaging/Tasmip.hpp"

int main()
{
  std::cout << "This program tests photon aspects of the Solutio library.\n";
  
  // Load NIST attenuation data for lead and interpolate
  solutio::NistPad Al;
  Al.Load("82-Lead.nistx");
  std::cout << Al.MassAttenuation(2.5) << '\n';
  
  // Calculate tungsten x-ray spectum using the TASMIP algorithm

  // Return if success
  return 0;
}
