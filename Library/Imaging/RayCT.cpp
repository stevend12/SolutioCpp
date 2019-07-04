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
#include <complex>
#include <algorithm>

// C headers
#include <cstdlib>

// Custom headers
#include "Tasmip.hpp"
#include "Utilities/DataInterpolation.hpp"
#include "Utilities/Statistics.hpp"
#include "Utilities/fftw++-2.05/Array.h"
#include "Utilities/fftw++-2.05/fftw++.h"

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
    scan_fov = 2.0*scanner_radius*sin(0.5*fan_angle);
    
    std::cout << fan_angle << '\t' << d_fan_angle << '\t' << scan_fov << '\n';
  }
  
  void RayCT::SetAcquisition(int kVp, double photons, int projs)
  {
    tube_potential = kVp;
    num_photons = photons;
    proj_per_rotation = projs;
  }
  
  void RayCT::SetReconstruction(double r_fov, int m_size)
  {
    if(r_fov > scan_fov)
    {
      std::cout << "Recon FOV too large, setting to scan FOV!\n";
      recon_fov = scan_fov;
    }
    else recon_fov = r_fov;
    
    matrix_size = m_size;
  }
  
  void RayCT::AcquireAirScan()
  {
    // Clear air scan data vector if not empty
    if(air_scan_data.size() != 0) air_scan_data.clear();
    
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
        x = scanner_radius*(2.0*cos((M_PI - fan_angle/2.0 + d_fan_angle/2.0 + c*d_fan_angle)) + 1.0);
        y = 2.0*scanner_radius*sin((M_PI - fan_angle/2.0 + d_fan_angle/2.0 + c*d_fan_angle));
        detector_pos.x = x*cos(source_angle) - y*sin(source_angle);
        detector_pos.y = x*sin(source_angle) + y*cos(source_angle);
        detector_pos.z = 2.0*row_width * (double(r) - (double(num_rows)/2.0) + 0.5);
        source_ray.SetRay(source_position, detector_pos - source_position);
        L = source_ray.GetLength();
        sum = 0.0;
        for(int e = 0; e < source_spectrum.size(); e++){
          sum += (source_spectrum[e] * exp(-air_data_table[e]*L));
        }
        air_scan_data.push_back(sum);
      }
    }
    
    // Scale and add noise (spatial blurring later)
    double value;
    for(int n = 0; n < air_scan_data.size(); n++)
    {
      value = air_scan_data[n];
      value *= num_photons;
      air_scan_data[n] = value;
    }
    AddPoissonNoise(air_scan_data);
  }
  
  void RayCT::AcquireAxialProjections(ObjectModelXray &M,
      double z)
  {
    // Clear projection data vector if not empty
    if(projection_data.size() != 0) projection_data.clear();
    
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
    
    // Acquire projection at every angle
    double a;
    for(int n = 0; n < proj_per_rotation; n++)
    {
      a = (2.0*M_PI*n)/proj_per_rotation;
      std::cout << "Simulating projection " << (n+1) << " of " << proj_per_rotation << '\n';
      std::vector<double> proj = ObjectProjection(M, a, z, source_spectrum);
      for(int p = 0; p < proj.size(); p++)
      {
        projection_data.push_back(proj[p]);
      }
    }
    
    // Scale, add noise, and normalize to air (spatial blurring later)
    double value;
    for(int n = 0; n < projection_data.size(); n++)
    {
      value = projection_data[n];
      value *= num_photons;
      projection_data[n] = value;
    }
    AddPoissonNoise(projection_data);
    NormalizeProjections();
  }
  
  void RayCT::AcquireHelicalProjections(ObjectModelXray &M, double pitch,
      double z_start, int n_rotations)
  {
    // Timing variables
    time_t start_time, end_time;
    double projection_calc_time, total_calc_time = 0.0;
	  
	  // Clear projection data vector if not empty
    if(projection_data.size() != 0) projection_data.clear();
    
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

    // Set scan parameters and allocate memory for projection data
    double angle, z_position;
    double table_motion = pitch * row_width * num_rows;
    int total_projections = proj_per_rotation * n_rotations;
	
    // Main loop over all projection angles
    for(int n = 0; n < total_projections; n++)
    {
      time(&start_time);
      
      // Set source angle and z-position
      angle = (double(n)/double(proj_per_rotation))*2.0*M_PI;
      while(angle >= (2.0*M_PI)) angle -= (2.0*M_PI);
      z_position = z_start + (double(n) * (table_motion/double(proj_per_rotation)));
      
      // Calculate attenuation for each source ray
      std::cout << "Simulating projection " << (n+1) << " of " << total_projections << "... ";
      std::vector<double> proj =
          ObjectProjection(M, angle, z_position, source_spectrum);
      for(int p = 0; p < proj.size(); p++)
      {
        projection_data.push_back(proj[p]);
      }
      
      time(&end_time);
      projection_calc_time = difftime(end_time,start_time) / 60.0;
      total_calc_time += projection_calc_time;
      std::cout << projection_calc_time << " min., " << total_calc_time << " min. total\n";
    }
    
    // Scale, add noise, and normalize to air (spatial blurring later)
    double value;
    for(int n = 0; n < projection_data.size(); n++)
    {
      value = projection_data[n];
      value *= num_photons;
      projection_data[n] = value;
    }
    AddPoissonNoise(projection_data);
    NormalizeProjections();
  }
  
  void RayCT::ReconAxialFBP()
  {
    // Timing variables
    time_t start_time, end_time;
    double time_pre, time_lookup, time_wbp;
    time(&start_time);
  
    // Set source spectrum
    std::vector<double> energies;
    for(int e = 0; e < 151; e++){ energies.push_back(double(e)); }
    std::vector<double> source_spectrum = Tasmip(tube_potential, 0.0,
        "Aluminum", data_folder);
    
    // Get mean attenuation coefficients for air and water
    NistPad NistAir(data_folder, "Air");
    NistPad NistWater(data_folder, "Water");
    double mean_energy = 0.0, mu_air, mu_water;
    for(int e = 0; e < 151; e++) mean_energy += source_spectrum[e]*double(e);
    mean_energy /= 1000.0;
    mu_air = NistAir.LinearAttenuation(mean_energy);
    mu_water = NistWater.LinearAttenuation(mean_energy);
    std::cout << mean_energy << '\t' << mu_air << '\t' << mu_water << '\n';
    
    // Select axial data for reconstruction
    double spatial_proj[(const int)(num_channels*proj_per_rotation)];
    for(int n = 0; n < proj_per_rotation; n++)
    {
      for(int c = 0; c < num_channels; c++)
      {
        spatial_proj[(num_channels*n+c)] = 0.0;
        for(int r = 0; r < num_rows; r++)
        {
          spatial_proj[(num_channels*n+c)] +=
            projection_data[(num_rows*num_channels*n+num_channels*r+c)];
        }
        spatial_proj[(num_channels*n+c)] /= double(num_rows);
      }
    }
    
    // Beam hardening correction, step 1 (soft tissue only)
    TissueBHC(source_spectrum, spatial_proj, mean_energy);
    
    // Angle gamma for a row of projection data
    double p_gamma[(const int)(num_channels)];
    for(int c = 0; c < num_channels; c++)
    {
      p_gamma[c] = (-fan_angle/2.0 + d_fan_angle/2.0 + c*d_fan_angle);
    }
    // Fan-beam projection weighting
    for(int n = 0; n < proj_per_rotation; n++)
    {
      for(int c = 0; c < num_channels; c++)
      {
        spatial_proj[(num_channels*n+c)] *= (scanner_radius*cos(p_gamma[c]));
      }
    }
    
    // Filter projection data
    std::cout << "Filtering projection data for slice...\n";
    FilterProjections1D(spatial_proj);
    
    time(&end_time);
    time_pre = difftime(end_time, start_time) / 60;

    // Weighted backprojection
    int num_images = 1;
    
    // Calculate lookup tables
    time(&start_time);
    std::cout << "Calculating backprojection lookup tables...\n";
    double xy_table[(const int)(matrix_size)];
    double * L = new double [matrix_size*matrix_size*proj_per_rotation];
    double * gamma = new double [matrix_size*matrix_size*proj_per_rotation];
    double pixel_dim = CalcLookups(xy_table, L, gamma);
    time(&end_time);
    time_lookup = difftime(end_time, start_time) / 60;
    
    // Perform backprojection
    time(&start_time);
    std::cout << "Performing weighted backprojection...\n";
    std::vector<int> slice = WeightedBackprojection(spatial_proj, xy_table,
        L, gamma, p_gamma, mu_water, mu_air);
    for(int p = 0; p < (matrix_size*matrix_size); p++)
    {
      image_data.push_back(slice[p]);
    }
    time(&end_time);
    time_wbp = difftime(end_time, start_time) / 60;
    
    std::cout.precision(3);
    std::cout << "Projection processing/filtering time: " << time_pre << " min.\n";
    std::cout << "Lookup table calculation time: " << time_lookup << " min.\n";
    std::cout << "Weighted backprojection time: " << time_wbp << " min.\n";
    std::cout << "Total time: " << (time_pre+time_lookup+time_wbp) << " min.\n";
    
    delete [] L;
    delete [] gamma;
  }
  
  // Reconstruct helically acquired fan beam images FBP and linear interpolation
  void RayCT::HelicalFIFBP(double pitch, double z_start, int n_rotations)
  {
    
    // Initialization
    std::ofstream fout;
    time_t start_time, end_time;
    double TimeLookup, TimeBack, TimeBackTotal;
    omp_set_num_threads(omp_get_max_threads());
    
    /////////////////////
    // Input variables //
    /////////////////////
    
    // Projection z-filtering
    int num_interp_points = 7;
    double FW = 0.3;
    double slice_start = 0.0;
    //double slice_end = 2.0;
    
    // Image reconstruction
    int num_images = 1;//floor((slice_end-slice_start)/FW) - 1; //52
    double sinc_a = 1.0;
    
    /////////////////////////////////////////////////////////////////
    // Preliminary calculations and projection data pre-processing //
    /////////////////////////////////////////////////////////////////
    std::cout << "Preliminary calculations and projection data preprocessing...\n";
    
    // Set source spectrum
    std::vector<double> energies;
    for(int e = 0; e < 151; e++){ energies.push_back(double(e)); }
    std::vector<double> source_spectrum = Tasmip(tube_potential, 0.0,
        "Aluminum", data_folder);
    
    // Calculate mean beam energy for reconstruction
    // and attenuation coefficient for air and water (for CT #'s)
    NistPad NistAir(data_folder, "Air");
    NistPad NistWater(data_folder, "Water");
    double mean_energy = 0.0, mu_air, mu_water;
    for(int e = 0; e < 151; e++) mean_energy += source_spectrum[e]*double(e);
    mean_energy /= 1000.0;
    mu_air = NistAir.LinearAttenuation(mean_energy);
    mu_water = NistWater.LinearAttenuation(mean_energy);
    std::cout << mean_energy << '\t' << mu_air << '\t' << mu_water << '\n';
    
    // Angle gamma for a row of projection data
    double p_gamma[(const int)(num_channels)];
    for(int c = 0; c < num_channels; c++)
    {
      p_gamma[c] = (-fan_angle/2.0 + d_fan_angle/2.0 + c*d_fan_angle);
    }
    
    /////////////////////////////
    // Calculate lookup tables //
    /////////////////////////////
    time(&start_time);
    std::cout << "Calculating backprojection lookup tables...\n";
    
    // Lookup table for helical projection z-values
    int z_index;
    double table_motion = pitch * num_rows * row_width;
    int num_helical_proj = proj_per_rotation * n_rotations;
    double * Z = new double [(num_helical_proj*num_rows)];
    for(int n = 0; n < num_helical_proj; n++)
    {
      for(int r = 0; r < num_rows; r++)
      {
        z_index = num_rows*n + r;
        Z[z_index] = z_start + (table_motion*(double(n)/double(proj_per_rotation))) +
            (row_width*(double(r)-(double(num_rows)/2.0)+0.5));
      }
    }
    
    // Lookup tables for fan-beam reconstruction
    double xy_table[(const int)(matrix_size)];
    double * L = new double [(matrix_size*matrix_size*proj_per_rotation)];
    double * gamma = new double [(matrix_size*matrix_size*proj_per_rotation)];
    double pixel_dim = CalcLookups(xy_table, L, gamma);

    time(&end_time);
    TimeLookup = difftime(end_time,start_time) / 60;
    
    ////////////////////
    // Axial FBP loop //
    ////////////////////
    
    std::cout << "Beginning image reconstruction...\n";
    
    // Select axial slice data for reconstruction, using filtered linear
    // interpolation from the helical data
    for(int n = 0; n < num_images; n++)
    {
      time(&start_time);
      std::cout << "Reconstructing image " << (n+1) << " out of " << num_images << "...\t";
      double slice_z = slice_start + double(n)*FW;
      double interp_f;
      int buffer_size, interp_angle, proj_ind, angle_180;
      double slice_proj[(const int)(num_channels*proj_per_rotation)];
      double current_z, current_p, sample, weight;
      for(int p = 0; p < proj_per_rotation; p++)
      {
        for(int c = 0; c < num_channels; c++)
        {
          // 1. Sample the projection values and z-positions for complementary data
          std::pair<double,double> temp_buffer;
          std::vector< std::pair<double,double> > proj_buffer;
          
          // 1a. Direct data and/or 360 degree complementary data
          interp_angle = p;
          while(interp_angle >= 0) interp_angle -= proj_per_rotation;
          interp_angle += proj_per_rotation;
          while(interp_angle < num_helical_proj)
          {
            for(int r = 0; r < num_rows; r++)
            {
              if(Z[(num_rows*interp_angle + r)] < (slice_z-(FW/2.0)-1.0))
              {
                continue;
              }
              else if(Z[(num_rows*interp_angle + r)] > (slice_z+(FW/2.0)+1.0))
              {
                continue;
              }
              else
              {
                temp_buffer.first = Z[(num_rows*interp_angle + r)];
                temp_buffer.second =
                    projection_data[(interp_angle*num_rows*num_channels + r*num_channels + c)];
                proj_buffer.push_back(temp_buffer);
              }
            }
            interp_angle += proj_per_rotation;
          }
          // 1b. 180 degree complementary data
          interp_angle = (p + proj_per_rotation/2) +
              ceil((2.0*p_gamma[c]*double(proj_per_rotation))/(2.0*M_PI));
          while(interp_angle >= 0) interp_angle -= proj_per_rotation;
          interp_angle += proj_per_rotation;
          interp_f = fabs(ceil((2.0*p_gamma[c]*double(proj_per_rotation))/(2.0*M_PI))
           - ((2.0*p_gamma[c]*double(proj_per_rotation))/(2.0*M_PI)));
          proj_ind = (num_channels-1) - c;
          while(interp_angle < (num_helical_proj-1))
          {
            for(int r = 0; r < num_rows; r++)
            {
              if(Z[(num_rows*interp_angle + r)] < (slice_z-(FW/2.0)-1.0))
              {
                continue;
              }
              else if(Z[(num_rows*interp_angle + r)] > (slice_z+(FW/2.0)+1.0))
              {
                continue;
              }
              else {
                temp_buffer.first = Z[(num_rows*interp_angle + r)];
                temp_buffer.second = 
                    interp_f*(projection_data[((interp_angle+1)*num_rows*num_channels + r*num_channels + proj_ind)]) +
                    (1-interp_f)*(projection_data[(interp_angle*num_rows*num_channels + r*num_channels + proj_ind)]);
                proj_buffer.push_back(temp_buffer);
              }
            }
            interp_angle += proj_per_rotation;
          }
          // 1c. Sort data by z-value
          std::sort(proj_buffer.begin(), proj_buffer.end());
          // 2. Create newly sampled set within filter width (FW) via linear interpolation
          std::vector<double> proj_interp, z_interp;
          if(proj_buffer.size() == 0)
          {
            std::cout << "Error: helical projection z-range does not include current slice!\n";
            std::cout << "(@ Projection " << n << ", channel " << c << ")\n";
            continue;
          }
          for(int ip = 0; ip < num_interp_points; ip++)
          {
            current_z = slice_z - (FW/2.0) + (FW*(double(ip)/double(num_interp_points-1)));
            current_p = LinearInterpolation(proj_buffer, current_z);
            proj_interp.push_back(current_p);
            z_interp.push_back(current_z);
          }
          // 3. Filter set and assign to axial projection data
          sample = 0.0;
          weight = 1.0 / double(num_interp_points);
          for(int ip = 0; ip < proj_interp.size(); ip++)
          {
            sample += (weight*proj_interp[ip]);
          }
          slice_proj[num_channels*p + c] = sample;
        }
      }
      
      fout.open("slice_initial.txt");
      for(int p = 0; p < proj_per_rotation; p++)
      {
        for(int c = 0; c < num_channels; c++)
        {
          fout << slice_proj[(num_channels*p+c)] << '\n';
        }
      }
      fout.close();
      
      // Beam hardening correction, step 1 (soft tissue only)
      TissueBHC(source_spectrum, slice_proj, mean_energy);
      
      // Fan-beam projection weighting
      for(int p = 0; p < proj_per_rotation; p++)
      {
        for(int c = 0; c < num_channels; c++)
        {
          slice_proj[(num_channels*p+c)] *= (scanner_radius*cos(p_gamma[c]));
        }
      }
      
      // Filter projection data
      FilterProjections1D(slice_proj);
      
      fout.open("slice_filtered.txt");
      for(int p = 0; p < proj_per_rotation; p++)
      {
        for(int c = 0; c < num_channels; c++)
        {
          fout << slice_proj[(num_channels*p+c)] << '\n';
        }
      }
      fout.close();
      
      // Perform initial backprojection
      time(&start_time);
      std::vector<int> slice = WeightedBackprojection(slice_proj, xy_table,
          L, gamma, p_gamma, mu_water, mu_air);
      for(int p = 0; p < (matrix_size*matrix_size); p++)
      {
        image_data.push_back(slice[p]);
      }
      
      time(&end_time);
      TimeBack = difftime(end_time,start_time) / 60.0;
      TimeBackTotal += TimeBack;
      
      std::cout << TimeBack << " minutes.\n";
    }
    
    std::cout.precision(3);
    std::cout << "Lookup table calculation time: " << TimeLookup << " min.\n";
    std::cout << "Weighted backprojection time: " << TimeBackTotal << " min.\n";
    std::cout << "Total time: " << (TimeLookup+TimeBackTotal) << " min.\n";
    
    delete [] Z;
    delete [] L;
    delete [] gamma;
  }
  
  void RayCT::WriteProjectionData(std::string file_name, bool split)
  {
    std::ofstream fout;
    if(split)
    {
    
    }
    else
    {
      fout.open(file_name.c_str());
      for(int n = 0; n < projection_data.size(); n++)
      {
        fout << projection_data[n] << '\n';
      }
      fout.close();
    }
  }
  
  void RayCT::WriteImageData(std::string file_name, bool split)
  {
    std::ofstream fout;
    if(split)
    {
    
    }
    else
    {
      fout.open(file_name.c_str());
      for(int n = 0; n < image_data.size(); n++)
      {
        fout << image_data[n] << '\n';
      }
      fout.close();
    }
  }
  
  /////////////////////////
  // Ancillary Functions //
  /////////////////////////
  
  std::vector<double> RayCT::ObjectProjection(ObjectModelXray &M, double angle,
      double z, std::vector<double> spectrum)
  {
    std::vector<double> projection;
    double gamma, x0, y0, x1, y1;
    Vec3<double> source_position, detector_pos;
    Ray3 source_ray;
    
    // Set source position (z position always equal to 0)
    x0 = scanner_radius*cos(angle);// + (-1.0*0.5*channel_width*sin(angle));
    y0 = scanner_radius*sin(angle);// + (0.5*channel_width*cos(angle));
    source_position.Set(x0, y0, z);
  
    // Calculate attenuation for each source ray
    for(int r = 0; r < num_rows; r++)
    {
      for(int c = 0; c < num_channels; c++)
      {
        // Set initial detector coordinates
        Ray3 source_ray;
        x1 = scanner_radius*(2.0*cos((M_PI - fan_angle/2.0 + d_fan_angle/2.0
            + c*d_fan_angle)) + 1.0);
        y1 = 2.0*scanner_radius*sin((M_PI - fan_angle/2.0 + d_fan_angle/2.0
            + c*d_fan_angle));
        // Rotate to source angle
        detector_pos.x = x1*cos(angle) - y1*sin(angle);
        detector_pos.y = x1*sin(angle) + y1*cos(angle);
        detector_pos.z = z + (2.0*row_width * (double(r) -
            (double(num_rows)/2.0) + 0.5));
        // Shift for 1/4 detector offset
        //detector_pos.x += (-1.0*0.5*channel_width*sin(angle));
        //detector_pos.y += (0.5*channel_width*cos(angle));
        // Assign ray parameters
        source_ray.SetRay(source_position, detector_pos - source_position);
        
        // Find path length for each tissue ray passes through  
        double intensity = M.GetRayAttenuation(source_ray, spectrum);
        projection.push_back(intensity);
      }
    }
    
    return projection;
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
  
  void RayCT::NormalizeProjections()
  {
    int n_total = projection_data.size() / (num_rows*num_channels);
    for(int n = 0; n < n_total; n++)
    {
      for(int r = 0; r < num_rows; r++)
      {
        for(int c = 0; c < num_channels; c++)
        {
          double norm = log(
              air_scan_data[(num_channels*r+c)] /
              projection_data[(num_rows*num_channels*n+num_channels*r+c)] );
          projection_data[(num_rows*num_channels*n+num_channels*r+c)] = norm;
        }
      }
    }
  }
  
  void RayCT::TissueBHC(std::vector<double> spectrum, double proj[],
      double recon_energy)
  {
    // Make attenuation lookup table for soft tissue
    int ind;
    double energy, sum, p, T_e, m;
    std::vector<double> dist;
    std::vector<double> table;
    NistPad NistTissue(data_folder, "Tissue4");
    for(int d = 0; d < 100; d++)
    {
      sum = 0.0;
      dist.push_back(double(d));
      for(int e = 0; e < 151; e++)
      {
        if(spectrum[e] == 0.0) continue;
        energy = double(e) / 1000.0;
        sum += (spectrum[e]*exp(-NistTissue.LinearAttenuation(energy)*dist[d]));
      }
      table.push_back(-log(sum));
    }
    // Estimate tissue pathlength and apply correction
    for(int n = 0; n < proj_per_rotation; n++)
    {
      for(int c = 0; c < num_channels; c++)
      {
        p = proj[num_channels*n + c];
        if(p <= 0.0) continue;
        T_e = LinearInterpolation(table, dist, p);
        m = NistTissue.LinearAttenuation(recon_energy)*T_e;
        proj[num_channels*n + c] = m;
      }
    }
  }
  
  void RayCT::FilterProjections1D(double proj[])
  {
    // Set up FFTW
    fftwpp::fftw::maxthreads = get_max_threads();
    int two_power = 0;
    while(pow(2, two_power) < (2*num_channels-1)) two_power++;
    const unsigned int padded_size = pow(2, two_power);
    size_t align = sizeof(Complex);
    
    Array::array1<Complex> ramp_filter(padded_size, align);
    Array::array1<Complex> padded_proj(padded_size, align);
    
    fftwpp::fft1d Forward(padded_size, -1);
    fftwpp::fft1d Backward(padded_size, 1);
    
    // Create ramp filter in spatial domain
    const int filter_size = (2*(num_channels-1)) + 1;
    int nf;
    int filter_padding = (padded_size-filter_size-1)/2;
    double filter_original[filter_size];
    for(int f = 0; f < filter_size; f++)
    {
      nf = (-num_channels+1) + f;
      if(nf == 0) filter_original[f] = 1 / (8*pow(d_fan_angle,2));
      else
      {
        if(nf % 2 == 0) filter_original[f] = 0;
        else filter_original[f] = -0.5 / pow((M_PI*sin(nf*d_fan_angle)),2);
      }
    }
    for(int f = 0; f < padded_size; f++)
    {
      if(f < filter_padding || f > (filter_size+filter_padding-1)) ramp_filter[f] = 0.0;
      else ramp_filter[f] = filter_original[(f-filter_padding)];
    }
    
    // Take FFT
    Forward.fft(ramp_filter);
    for(int f = 0; f < padded_size; f++) ramp_filter[f] = abs(ramp_filter[f]);
    
    // Filter each projection
    int proj_padding = (padded_size-num_channels)/2;
    for(int n = 0; n < proj_per_rotation; n++)
    {
      // Get projection data & add padding
      for(int c = 0; c < 2048; c++){
        if(c < proj_padding || c > (num_channels+proj_padding-1)) padded_proj[c] = 0;
        else padded_proj[c] = proj[((num_channels*n)+(c-proj_padding))];
      }
      // Take Fourier transforms and apply filter
      Forward.fft(padded_proj);
      for(int f = 0; f < padded_size; f++) padded_proj[f] *= ramp_filter[f];
      Backward.fftNormalized(padded_proj);
      // Remove padding
      for(int c = 0; c < num_channels; c++)
      {
        proj[(num_channels*n+c)] =
            d_fan_angle*padded_proj[proj_padding+c].real();
      }
    }
  }
  
  double RayCT::CalcLookups(double xy[], double * L_data, double * gamma_data)
  {  
    double pixel_dim = recon_fov / double(matrix_size);
    
    for(int n = 0; n < matrix_size; n++)
    {
      xy[n] = pixel_dim*(float(n - (float(matrix_size)/2.0)) + 0.5);
    }
    
    omp_set_num_threads(omp_get_max_threads());
    double angle, x_offset, y_offset, x;
    for(int a = 0; a < proj_per_rotation; a++)
    {
      angle = (2.0*M_PI*double(a))/double(proj_per_rotation)-(M_PI/2.0);
      x_offset = 0.0;//(-1.0*0.5*channel_width*sin(angle));
      y_offset = 0.0;//(0.5*channel_width*cos(angle));
      for(int i = 0; i < matrix_size; i++){
        x = xy[i] - x_offset;
        #pragma omp parallel for
        for(int j = 0; j < matrix_size; j++){
          double y = xy[j] - y_offset;
          double radius = sqrt(pow(x,2) + pow(y,2));
          double theta = atan2(y,x);
          int angle_index = matrix_size*matrix_size*a + matrix_size*i + j;
          int image_index = matrix_size*i + j;
          L_data[angle_index] = 
              pow((scanner_radius + radius*sin(angle-theta)),2)
              + pow(radius*cos(angle-theta),2);
          gamma_data[angle_index] =
              atan2((radius*cos(angle-theta)),
              (scanner_radius+radius*sin(angle-theta)));
        }
      }
    }
    return pixel_dim;
  }
  
  std::vector<int> RayCT::WeightedBackprojection(double proj[], double xy[],
          double * L_data, double * gamma_data, double proj_gamma[],
          double mu_water, double mu_air)
  {
    //omp_set_num_threads(omp_get_max_threads());
    std::vector<int> image_slice;
    double sum;
    for(int i = 0; i < matrix_size; i++)
    {
      for(int j = 0; j < matrix_size; j++)
      {
        // Skip if pixel is outside reconstruction FOV
        if(sqrt(pow(xy[i],2.0)+pow(xy[j],2.0)) > (recon_fov/2.0))
        {
          image_slice.push_back(-1000);
          continue;
        }
        // Else begin backprojection
        sum = 0.0;
        //#pragma omp parallel for
        for(int a = 0; a < proj_per_rotation; a++)
        {
          int angle_index = matrix_size*matrix_size*a + matrix_size*i + j;
          if(fabs(gamma_data[angle_index]) > (fan_angle/2.0 - d_fan_angle/2.0))
          {
            std::cout << "Warning: gamma outside of projection data range!\n";
          }
          // Determine projection value from gamma, w/ linear interpolation
          double multiple = (gamma_data[angle_index] - proj_gamma[0]) /
              d_fan_angle;
          int index = int(floor(multiple));
          double f = (gamma_data[angle_index] - proj_gamma[index]) /
              (proj_gamma[index+1] - proj_gamma[index]);
          double p = f*proj[(num_channels*a + index + 1)] + 
              (1-f)*proj[num_channels*a + index];
	              
          // Weighted backprojection, add to sum
          //#pragma omp critical
          sum += (p/L_data[angle_index]);
        }
        // Average contributions from all angles & convert to HU
        sum *= ((2.0*M_PI) / double(proj_per_rotation));
        image_slice.push_back(round(1000*((sum-mu_water)/(mu_water-mu_air))));
      }
    }
    return image_slice;
  }
}
