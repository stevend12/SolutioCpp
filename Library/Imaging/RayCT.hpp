/******************************************************************************/
/*                                                                            */
/* Copyright 2016-2017 Steven Dolly                                           */
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
// Ray Tracing CT Simulation                                                  //
// (RayCT.hpp)                                                                //
//                                                                            //
// Steven Dolly                                                               //
// June 12, 2017                                                              //
//                                                                            //
// This file contains the header for the class which handles the computer     //
// simulation of a CT scanner using ray-tracing and exponential attenuation.  //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

// Header guard
#ifndef RAYCT_HPP
#define RAYCT_HPP

// C++ headers
#include <vector>
#include <string>

// Custom headers
#include "Imaging/ObjectModelXray.hpp"

namespace solutio {
  class RayCT
  {
    public:
      void SetNistDataFolder(std::string folder);
      void SetGeometry(double radius, int n_c, double d_c, int n_r, double d_r);
      void SetAcquisition(int kVp, double photons, int projs);
      double RandNormal(double mean, double stddev);
      void AddPoissonNoise(std::vector<double> &projection);
      std::vector<double> AcquireAirScan();
      std::vector<double> ObjectProjection(ObjectModelXray &M, double angle,
          double z, std::vector<double> spectrum);
      std::vector<double> AcquireAxialProjections(ObjectModelXray &M, double z);
    private:
      // Data folder for NISTX data
      std::string data_folder;
      // Scanner geometry parameters
      double scanner_radius;
      int num_channels;
      double channel_width;
      int num_rows;
      double row_width;
      // Acquisition parameters
      int tube_potential;
      double num_photons;
      int num_projections;
      // Derived parameters
      double fan_angle;
      double d_fan_angle;
      double fov;
  };
}

#endif
