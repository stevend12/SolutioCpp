/******************************************************************************/
/*                                                                            */
/* Copyright 2018 Steven Dolly                                                */
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
// RayCtDemo.cpp                                                              //
// Demonstration of RayCT CT simulation class                                 //
// Created March 9, 2018 (Steven Dolly)                                       //
//                                                                            //
// This file demonstrates use of the RayCT class to simulate CT scan          //
// acquisition and reconstruction.                                            //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

// C++ header files
#include <iostream>
#include <fstream>

// SolutioCpp library headers
#include "Geometry/Cylinder.hpp"
#include "Imaging/ObjectModelXray.hpp"
#include "Imaging/RayCT.hpp"

int main(){
  // Object model geometries
  solutio::Vec3<double> C0;
  solutio::Vec3<double> C1(5.0, 5.0, 0.0);
  solutio::Cylinder World(C0, 40.0, 20.0);
  solutio::Cylinder Phantom(C0, 10.0, 10.0);
  // Object model construction
  std::string folder = "/home/steven/Cpp/SolutioCpp/Data/NISTX";
  solutio::ObjectModelXray Model;
  Model.AddMaterial(folder, "Air", "Air");
  Model.AddMaterial(folder, "Water", "Water");
  Model.AddObject("World", World, "None", "Air");
  Model.AddObject("Phantom", Phantom, "World", "Water");
  Model.MakeTree();
  // Initialize RayCT
  solutio::RayCT MyScanner;
  MyScanner.SetNistDataFolder(folder);
  MyScanner.SetGeometry(40.0, 672, 0.0625, 1, 0.0625);
  // CT scan
  MyScanner.SetAcquisition(120, 5.0e5, 500);
  MyScanner.AcquireAirScan();
  MyScanner.AcquireAxialProjections(Model, 0.0);
  // CT recon
  MyScanner.SetReconstruction(40.0, 512);
  MyScanner.ReconAxialFBP();
  // Write data to file
  std::vector<double> obj_scan = MyScanner.GetProjectionData();
  std::ofstream fout;
  fout.open("proj.txt");
  for(int n = 0; n < obj_scan.size(); n++) fout << obj_scan[n] << '\n';
  fout.close();
  std::vector<int> img_scan = MyScanner.GetImageData();
  fout.open("image.txt");
  for(int i = 0; i < 512; i++)
  {
    for(int j = 0; j < 512; j++) fout << img_scan[(512*i + j)] << '\n';
  }
  fout.close();
  
  return 0;
}
