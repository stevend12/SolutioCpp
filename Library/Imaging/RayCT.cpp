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
// (RayCT.cpp)                                                                //
//                                                                            //
// Steven Dolly                                                               //
// June 12, 2017                                                              //
//                                                                            //
// This file defines the class which handles the computer simulation of a CT  //
// scanner using ray-tracing and exponential attenuation.                     //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

// Class header
#include "RayCT.hpp"

// C++ headers
#include <iostream>
#include <fstream>

// C headers
#include <cstdlib>

// Custom headers
#include "Tasmip.hpp"

namespace solutio
{
  void RayCT::SetNistDataFolder(std::string folder)
  {
    data_folder = folder;
  }
  
  void RayCT::SetGeometry(double radius, int n_c, double d_c, int n_r,
      double d_r)
  {
    // Set base parameters
    scanner_radius = radius;
    num_channels = n_c;
    channel_width = d_c;
    num_rows = n_r;
    row_width = d_r;
    
    // Derive others
    fan_angle = (2.0*channel_width*num_channels) / (2.0*scanner_radius);
    d_fan_angle = (2.0*channel_width) / (2.0*scanner_radius);
    fov = 2.0*scanner_radius*sin(0.5*fan_angle);
    
    std::cout << fan_angle << '\t' << d_fan_angle << '\t' << fov << '\n';
  }
  
  void RayCT::SetAcquisition(int kVp, double photons, int projs)
  {
    tube_potential = kVp;
    num_photons = photons;
    num_projections = projs;
  }
  
  double RayCT::RandNormal(double mean, double stddev)
  {
    static double n2 = 0.0;
    static int n2_cached = 0;
    if(!n2_cached)
    {
      double x, y, r;
      do
      {
        x = 2.0*rand()/RAND_MAX - 1;
        y = 2.0*rand()/RAND_MAX - 1;
        r = x*x + y*y;
      }
      while (r == 0.0 || r > 1.0);
      {
        double d = sqrt(-2.0*log(r)/r);
        double n1 = x*d;
        n2 = y*d;
        double result = n1*stddev + mean;
        n2_cached = 1;
        return result;
      }
    }
    else
    {
      n2_cached = 0;
      return n2*stddev + mean;
    }
  }
  
  void RayCT::AddPoissonNoise(std::vector<double> &projection)
  {
    double input;
    srand(time(0));
    for(int p = 0; p < projection.size(); p++)
    {
      input = projection[p];
      input += RandNormal(input, sqrt(input)); // Photon statistics
      input += RandNormal(0.0, sqrt(10.0));    // Electronic noise
      if(input <= 0.0) input = 0.1;
      projection[p] = input;
    }
  }
  
  std::vector<double> RayCT::AcquireAirScan()
  {
    // Set source spectrum
    std::vector<double> energies;
    for(int e = 0; e < 151; e++){ energies.push_back(double(e)); }
    std::vector<double> source_spectrum = Tasmip(tube_potential, 0.0,
        "Aluminum", data_folder);
    
    // Get attenuation data, from NIST database, and make data table
    std::vector<double> air_data_table;
    air_data_table.push_back(0.0);
    NistPad Air(data_folder, "Air");
    for(int e = 1; e < 151; e++)
    {
      air_data_table.push_back(Air.LinearAttenuation(energies[e]/1000.0));
    }
    
    // Initialize projection data container
    std::vector<double> air_scan_proj;
    
    // Set source position
    double source_angle;
    Vec3<double> source_position;
    source_angle = 0.0*(M_PI/180.0);
    source_position.Set(scanner_radius*cos(source_angle),
        scanner_radius*sin(source_angle), 0.0);
    
    // Acquire mean signal at each detector element from source    
    Ray3 source_ray;
    double gamma, x, y, L, sum;
    Vec3<double> detector_pos;
    for(int r = 0; r < num_rows; r++){
      for(int c = 0; c < num_channels; c++){
        gamma = (-fan_angle/2.0) + ((2.0*c+1.0)*d_fan_angle)/2.0;
        x = -1.0*(2.0*scanner_radius*cos(gamma) - scanner_radius);
        y = 2.0*scanner_radius*sin(gamma);
        detector_pos.x = x*cos(source_angle) - y*sin(source_angle);
        detector_pos.y = x*sin(source_angle) + y*cos(source_angle);
        detector_pos.z = 2.0*row_width * (double(r) - (double(num_rows)/2.0) + 0.5);
        source_ray.SetRay(source_position, detector_pos - source_position);
        L = source_ray.GetLength();
        sum = 0.0;
        for(int e = 0; e < source_spectrum.size(); e++){
          sum += (source_spectrum[e] * exp(-air_data_table[e]*L));
        }
        air_scan_proj.push_back(sum);
      }
    }
    
    // Scale, add noise and spatial blurring
    double value;
    for(int n = 0; n < air_scan_proj.size(); n++)
    {
      value = air_scan_proj[n];
      value *= num_photons;
      air_scan_proj[n] = value;
    }
    AddPoissonNoise(air_scan_proj);
    
    // Return projection data
    return air_scan_proj;
  }
  
  std::vector<double> RayCT::ObjectProjection(ObjectModelXray &M, double angle,
      double z, std::vector<double> spectrum)
  {
    std::vector<double> projection;
    double gamma, x0, y0, x1, y1;
    Vec3<double> source_position, detector_pos;
    Ray3 source_ray;
    
    // Set source position (z position always equal to 0)
    x0 = scanner_radius*cos(angle);// + (-1.0*0.5*detector_channel_width*sin(angle));
    y0 = scanner_radius*sin(angle);// + (0.5*detector_channel_width*cos(angle));
    source_position.Set(x0, y0, z);
  
    // Calculate attenuation for each source ray
    for(int r = 0; r < num_rows; r++){
      for(int c = 0; c < num_channels; c++){
        // Set initial detector coordinates
        Ray3 source_ray;
        gamma = (-fan_angle/2.0) + ((2.0*c+1.0)*d_fan_angle)/2.0;
        x1 = -1.0*(2.0*scanner_radius*cos(gamma) - scanner_radius);
        y1 = 2.0*scanner_radius*sin(gamma);
        // Rotate to source angle
        detector_pos.x = x1*cos(angle) - y1*sin(angle);
        detector_pos.y = x1*sin(angle) + y1*cos(angle);
        detector_pos.z = z + (2.0*row_width * (double(r) -
            (double(num_rows)/2.0) + 0.5));
        // Shift for 1/4 detector offset
        //x2 += (-1.0*0.5*detector_channel_width*sin(angle));
        //y2 += (0.5*detector_channel_width*cos(angle));
        // Assign ray parameters
        source_ray.SetRay(source_position, detector_pos - source_position);
        
        // Find path length for each tissue ray passes through  
        double intensity = M.GetRayAttenuation(source_ray, spectrum);
        projection.push_back(intensity);
      }
    }
    
    return projection;
  }
  
  std::vector<double> RayCT::AcquireAxialProjections(ObjectModelXray &M,
      double z)
  {
    // Set source spectrum and attenuation lists
    std::vector<double> energies;
    for(int e = 0; e < 151; e++){ energies.push_back(double(e)/1000.0); }
    std::vector<double> source_spectrum = Tasmip(tube_potential, 0.0,
        "Aluminum", data_folder);
    if(!M.IsListTabulated())
    {
      M.TabulateAttenuationLists(energies, source_spectrum);
    }
    else
    {
      std::cout << "Warning: attenuation list already tabulated!\n";
    }
    
    // Initialize projection data container
    std::vector<double> projection_data;
    
    // Acquire projection at every angle
    double a;
    for(int n = 0; n < num_projections; n++)
    {
      a = (2.0*M_PI*n)/num_projections;
      std::cout << "Simulating projection " << (n+1) << " of " << num_projections << '\n';
      std::vector<double> proj = ObjectProjection(M, a, z, source_spectrum);
      for(int p = 0; p < proj.size(); p++)
      {
        projection_data.push_back(proj[p]);
      }
    }
    
    // Scale, add noise and spatial blurring
    double value;
    for(int n = 0; n < projection_data.size(); n++)
    {
      value = projection_data[n];
      value *= num_photons;
      projection_data[n] = value;
    }
    AddPoissonNoise(projection_data);
    
    // Return projection data
    return projection_data;
  }
}
