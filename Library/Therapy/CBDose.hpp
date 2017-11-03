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
// CBDose.hpp                                                                 //
// Corrections-Based Dose Calculation Class                                   //
// Created September 13, 2017 (Steven Dolly)                                  //
//                                                                            //
// This header file defines a class for dose calculation for a radiotherapy   //
// linac, using the corrections-based methodology. The CBDose class reads in  //
// beam data and calculates the dose to a point for a given beam.             //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

// Header guards
#ifndef CBDOSE_HPP
#define CBDOSE_HPP

// Standard C++ header files
#include <string>
#include <vector>

namespace solutio
{
  // Class to represent a linac beam
  class LinacBeam {
    public:
      // Set and get functions
      void SetFieldSize(float x, float y);
      void SetFieldSize(float x1, float x2, float y1, float y2);
      void SetSSD(float ssd){ SSD = ssd; }
      float GetX1(){ return X1; }
      float GetX2(){ return X2; }
      float GetY1(){ return Y1; }
      float GetY2(){ return Y2; }
      float GetX(){ return (X1-X2); }
      float GetY(){ return (Y1-Y2); }
      float GetSSD(){ return SSD; }
    private:
      // Beam setup data
      float X1; // X1 jaw position
      float X2; // X2 jaw position
      float Y1; // Y1 jaw position
      float Y2; // Y2 jaw position
      float SSD;
  };
  
  // Class to represent calculation point
  class CalcPoint {
    public:
      void SetPoint(float d, float doa);
      float GetDepth(){ return depth; }
      float GetOAD(){ return off_axis_distance; }
    private:
      float depth;
      float off_axis_distance;
  };
  
  // Utility calculation equations
  float SquareField(float a, float b);
  float SquareField(float r);
  float MayneordF(float f_1, float f_2, float d_0, float d);
  float AnalyticPenumbraModel(float oad, float field_size);
  
  // CBDose algorithm class:
  // 1) Loads/stores linac beam data
  // 2) Calculates dose for a given LinacBeam at a given CalcPoint, using the
  //    corrections-based formalism
  class CBDose {
    public:
      CBDose();
      // Load beam data from text file
      void LoadData(std::string file_name);
      // Get data from memory
      float Getk(){ return k; };
      float Getd_0(){ return d_0; };
      float GetSSD_0(){ return SSD_0; };
      float GetSAD(){ return SAD; };
      float GetS_c(float r);
      float GetS_p(float r);
      float GetPDD(float d, float r, float f);
      float GetTPR(float d, float r);
      float GetOAR(float d, float oad);
      // Calculation functions
      float PDDToTPR(float d, float r_d);
      float CalcDose(float mu, LinacBeam &beam, CalcPoint &point, 
          std::string type = "SAD");
      float CalcMU(float dose, LinacBeam &beam, CalcPoint &point, 
          std::string type = "SAD");
    private:
      float k; // Calibration constant in cGy/MU
      float d_0; // Depth of calibration in cm
      float SSD_0; // Calibration SSD, in cm
      float SAD; // Source-to-isocenter distance, in cm (usually 100 cm)
      float SSD_PDD; // SSD for PDD measurements, in cm
      
      std::vector<float> r_scatter;
      std::vector<float> S_c_data;
      std::vector<float> S_p_data;
      
      std::vector<float> r_pdd;
      std::vector<float> d_pdd;
      std::vector< std::vector<float> > pdd_data;
      
      std::vector<float> r_tpr;
      std::vector<float> d_tpr;
      std::vector< std::vector<float> > tpr_data;
      
      std::vector<float> oad_oar;
      std::vector<float> d_oar;
      std::vector< std::vector<float> > oar_data;
  };
}

#endif
