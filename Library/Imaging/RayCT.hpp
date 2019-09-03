/******************************************************************************/
/*                                                                            */
/* Copyright 2016-2018 Steven Dolly                                           */
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
      // Functions to set acquisition/reconstruction parameters
      void SetNistDataFolder(std::string folder);
      void SetGeometry(double radius, int n_c, double d_c, int n_r, double d_r);
      void SetAcquisition(int kVp, double photons, int projs);
      void SetReconstruction(double r_fov, int m_size);
      // Functions to perform acquisition/reconstruction
      void AcquireAirScan();
      void AcquireAxialProjections(ObjectModelXray &M, double z);
      void AcquireHelicalProjections(ObjectModelXray &M, double pitch,
          double z_start, int n_rotations);
      void ReconAxialFBP();
      void HelicalFIFBP(double pitch, double z_start, int n_rotations);
      // Functions to get acquisition/reconstruction data
      std::vector<double> GetAirScanData(){ return air_scan_data; }
      std::vector<double> GetProjectionData(){ return projection_data; }
      std::vector<int> GetImageData(){ return image_data; }
      // Functions to write acquisition/reconstruction data to file(s)
      void WriteProjectionData(std::string file_name, bool split);
      void WriteImageData(std::string file_name, bool split);
    private:
      // Internally-used ancillary functions
      std::vector<double> ObjectProjection(ObjectModelXray &M, double angle,
          double z, std::vector<double> spectrum);
      void AddPoissonNoise(std::vector<double> &projection);
      void NormalizeProjections();
      void TissueBHC(std::vector<double> spectrum, double proj[],
          double recon_energy);
      void FilterProjections1D(double proj[]);
      double CalcLookups(double xy[], double * L_data, double * gamma_data);
      std::vector<int> WeightedBackprojection(double proj[], double xy[],
          double * L_data, double * gamma_data, double proj_gamma[],
          double mu_water, double mu_air);
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
      int proj_per_rotation;
      double fan_angle;
      double d_fan_angle;
      double scan_fov;
      // Recon parameters
      double recon_fov;
      int matrix_size;
      // Stored data
      std::vector<double> air_scan_data;
      std::vector<double> projection_data;
      std::vector<int> image_data;
  };
}

#endif
