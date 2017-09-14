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
// CBDose.cpp                                                                 //
// Corrections-Based Dose Calculation Class                                   //
// Created September 13, 2017 (Steven Dolly)                                  //
//                                                                            //
// This main file contains a class for dose calculation for a radiotherapy    //
// linac, using the corrections-based methodology. The CBDose class reads in  //
// beam data and calculates the dose to a point for a given beam.             //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#include "CBDose.hpp"

#include <cmath>

#include <iostream>
#include <fstream>
#include <sstream>

#include "/home/steven/C++/SolutioCpp/Library/Utilities/DataInterpolation.hpp"

/////////////////////////////////////
// Class to manage beam setup data //
/////////////////////////////////////

void LinacBeam::SetFieldSize(float x, float y){
  X1 = x;
  X2 = -x;
  Y1 = y;
  Y2 = -y;
}
void LinacBeam::SetFieldSize(float x1, float x2, float y1, float y2){
  X1 = x1;
  X2 = x2;
  Y1 = y1;
  Y2 = y2;
}

////////////////////////////////////////////
// Class to manage calculation point data //
////////////////////////////////////////////

void CalcPoint::SetPoint(float d, float doa){
  depth = d;
  off_axis_distance = doa;
}

///////////////////////
// Utility functions //
///////////////////////

// Calculate equivalent square field sizes
float SquareField(float a, float b){ return ( (2*a*b) / (a+b) ); }
float SquareField(float r){
  
}

// Mayneord F factor for PDD conversion to different SSD
float MayneordF(float f_1, float f_2, float d_0, float d){
  return (pow(((f_2+d_0)/(f_2+d)), 2.0 ) * pow(((f_1+d)/(f_1+d_0)), 2.0 ));
}

float AnalyticPenumbraModel(float oad, float field_size){
  float A = 0.173;
  float B1 = 0.456;
  float B2 = 2.892;
  float T = 0.01;
  return (T + (1.0-T)*( A*((erf(B1*(field_size-oad))+1.0)/2.0) + 
      (1-A)*((erf(B2*(field_size-oad))+1.0)/2.0) ) );
}

///////////////////////////////
// Class to manage beam data //
///////////////////////////////

CBDose::CBDose(){
  SAD = 100.0;
}

void CBDose::LoadData(std::string file_name){
  // Initialization and open file
  std::ifstream fin;
  std::string input, str;
  size_t p1, p2;
  bool reading = true;
  float temp;
  
  fin.open(file_name.c_str());
  
  // Get first line and display
  std::getline(fin, input);
  std::cout << input << '\n';
  for(int n = 0; n < 3; n++){ std::getline(fin, input); }
  
  // Get calibration constant, SSD, and depth
  std::getline(fin, input);
  p1 = input.find(':'); p1++; p2 = input.find('c');
  str = input.substr(p1,p2-p1);
  std::stringstream(str) >> k;
  
  std::getline(fin, input);
  p1 = input.find(':'); p1++; p2 = input.find('c');
  str = input.substr(p1,p2-p1);
  std::stringstream(str) >> d_0;
  
  std::getline(fin, input);
  p1 = input.find(':'); p1++; p2 = input.find('c');
  str = input.substr(p1,p2-p1);
  std::stringstream(str) >> SSD_0;
  
  std::getline(fin, input);
  p1 = input.find(':'); p1++; p2 = input.find('c');
  str = input.substr(p1,p2-p1);
  std::stringstream(str) >> SSD_PDD;
  
  std::cout << "Calibration constant (k) = " << k << " cGy/MU @ " << d_0 << " cm\n";
  std::cout << "PDD measured using " << SSD_PDD << " cm SSD\n";
  for(int n = 0; n < 4; n++){ std::getline(fin, input); }
  
  // Get scatter factor tables for S_c and S_p
  int ind = 0;
  while(reading){
    std::getline(fin, input);
    if(input == "end scatter factors"){
      reading = false;
    }
    else {
      p1 = input.find(' ');
      std::stringstream(input.substr(0,p1)) >> temp;
      r_scatter.push_back(temp);
      
      p2 = input.find(' ',p1+1);
      std::stringstream(input.substr(p1,p2-p1)) >> temp;
      S_c_data.push_back(temp);
      
      std::stringstream(input.substr(p2)) >> temp;
      S_p_data.push_back(temp);
    }
  }
  for(int n = 0; n < 3; n++){ std::getline(fin, input); }
  
  // Get PDD table
  std::getline(fin, input);
  p1 = 0; p2 = input.find(' ');
  while(p1 != std::string::npos){
    std::stringstream(input.substr(p1,p2-p1)) >> temp;
    r_pdd.push_back(temp);
    p1 = p2; p2 = input.find(' ',p1+1);
  }
  
  reading = true;
  while(reading){
    std::getline(fin, input);
    
    if(input == "end pdd table"){
      reading = false;
      continue;
    }
    
    std::vector<float> buffer;
    p1 = 0; p2 = input.find(' ');
    std::stringstream(input.substr(p1,p2-p1)) >> temp;
    d_pdd.push_back(temp);
    //std::cout << temp << '\n';
    p1 = p2; p2 = input.find(' ',p1+1);
    while(p1 != std::string::npos){
      std::stringstream(input.substr(p1,p2-p1)) >> temp;
      buffer.push_back(temp);
      p1 = p2; p2 = input.find(' ',p1+1);
      //std::cout << temp << '\n';
    }
    
    pdd_data.push_back(buffer);
  }
  for(int n = 0; n < 2; n++){ std::getline(fin, input); }
  
  // Get TPR table
  
  // Read if TPR data is available, else calculate from PDD
  if(input == "TPR Table"){
    for(int n = 0; n < 2; n++){ std::getline(fin, input); }
    p1 = 0; p2 = input.find(' ');
    while(p1 != std::string::npos){
      std::stringstream(input.substr(p1,p2-p1)) >> temp;
      r_tpr.push_back(temp);
      p1 = p2; p2 = input.find(' ',p1+1);
    }
    
    reading = true;
    while(reading){
      std::getline(fin, input);
      
      if(input == "end tpr table"){
        reading = false;
        continue;
      }
      
      std::vector<float> buffer;
      p1 = 0; p2 = input.find(' ');
      std::stringstream(input.substr(p1,p2-p1)) >> temp;
      d_tpr.push_back(temp);
      //std::cout << temp << '\n';
      p1 = p2; p2 = input.find(' ',p1+1);
      while(p1 != std::string::npos){
        std::stringstream(input.substr(p1,p2-p1)) >> temp;
        buffer.push_back(temp);
        p1 = p2; p2 = input.find(' ',p1+1);
        //std::cout << temp << '\n';
      }
      
      tpr_data.push_back(buffer);
    }
    for(int n = 0; n < 3; n++){ std::getline(fin, input); }
  }
  else if(input == "no tpr table"){
    std::cout << "No TPR data found, calculating from PDD data...\n";
    for(int n = 1; n < r_pdd.size(); n++) r_tpr.push_back(r_pdd[n]);
    for(int n = 0; n < d_pdd.size(); n++) d_tpr.push_back(d_pdd[n]);
    for(int n_d = 0; n_d < d_tpr.size(); n_d++){
      std::vector<float> buffer;
      for(int n_r = 0; n_r < r_tpr.size(); n_r++){
        buffer.push_back(PDDToTPR(d_tpr[n_d], r_tpr[n_r]));
      }
      tpr_data.push_back(buffer);
    }
    for(int n = 0; n < 4; n++){ std::getline(fin, input); }
  }
  else {
    std::cout << "Error in reading/calculating TPR data!\n";
    for(int n = 0; n < 2; n++){ std::getline(fin, input); }
  }
  
  // Get OAR table
  std::getline(fin, input);
  std::cout << input << '\n';
  p1 = 0; p2 = input.find(' ');
  while(p1 != std::string::npos){
    std::stringstream(input.substr(p1,p2-p1)) >> temp;
    oad_oar.push_back(temp);
    p1 = p2; p2 = input.find(' ',p1+1);
  }
  
  reading = true;
  while(reading){
    std::getline(fin, input);
    
    if(input == "end oar table"){
      reading = false;
      continue;
    }
    
    std::vector<float> buffer;
    p1 = 0; p2 = input.find(' ');
    std::stringstream(input.substr(p1,p2-p1)) >> temp;
    d_oar.push_back(temp);
    p1 = p2; p2 = input.find(' ',p1+1);
    while(p1 != std::string::npos){
      std::stringstream(input.substr(p1,p2-p1)) >> temp;
      buffer.push_back(temp);
      p1 = p2; p2 = input.find(' ',p1+1);
    }
    
    oar_data.push_back(buffer);
  }
  
  // Close file
  fin.close();
}

// Get data from tables using linear interpolation
float CBDose::GetS_c(float r){
  return solutio::LinearInterpolation(r_scatter, S_c_data, r);
}
float CBDose::GetS_p(float r){
  return solutio::LinearInterpolation(r_scatter, S_p_data, r);
}
float CBDose::GetPDD(float d, float r, float f){
  float pdd_1 = solutio::LinearInterpolation(d_pdd, r_pdd, pdd_data, d, r);
  float pdd_2;
  if(f == SSD_PDD) pdd_2 = pdd_1;
  else {
    float r_1 = r*((SSD_PDD + d)/SSD_PDD);
    float r_2 = r*((f+d)/f);
    float r_10 = r*((SSD_PDD + d_0)/SSD_PDD);
    float r_20 = r*((f+d_0)/f);
    float tpr_ratio = GetTPR(d, r_2) / GetTPR(d, r_1);
    float Sp_ratio = (GetS_p(r_10)/GetS_p(r_1)) * (GetS_p(r_2)/GetS_p(r_20));
    pdd_2 = pdd_1 * MayneordF(SSD_PDD, f, d_0, d) * tpr_ratio * Sp_ratio;
  }
  return pdd_2;
}
float CBDose::GetTPR(float d, float r){
  return solutio::LinearInterpolation(d_tpr, r_tpr, tpr_data, d, r);
}

float CBDose::GetOAR(float d, float oad){
  return solutio::LinearInterpolation(d_oar, oad_oar, oar_data, d, oad);
}
// Convert PDD(d, r, f) to TPR(d, r_d)
float CBDose::PDDToTPR(float d, float r_d){
  float r = r_d*(SSD_PDD/(SSD_PDD+d));
  float r_d0 = r*((SSD_PDD+d_0)/SSD_PDD);
  return ( (GetPDD(d,r,SSD_PDD)/100.0) * pow(((SSD_PDD+d)/(SSD_PDD+d_0)),2.0) * (GetS_p(r_d0)/GetS_p(r_d)) );
}
// Calculate dose or monitor units, depending on variable "type"
float CBDose::CalcDose(float mu, LinacBeam &beam, CalcPoint &point, 
    std::string type){
  // Calculate source to point distance
  float SPD = beam.GetSSD() + point.GetDepth();
  // Calculate field sizes
  float r_c = SquareField(beam.GetX(), beam.GetY());
  float r = r_c*(beam.GetSSD() / GetSAD());
  float r_0 = r_c*((beam.GetSSD() + Getd_0()) / GetSAD());
  float r_d = r_c*(SPD / GetSAD());
  // Get scatter factors
  float S_c = GetS_c(r_c);
  float S_p;
  if(type == "SAD") S_p = GetS_p(r_d);
  else GetS_p(r_0);
  // Get PDD/TPR
  float depth_dose;
  if(type == "SAD") depth_dose = GetTPR(point.GetDepth(), r_d);
  else depth_dose = GetPDD(point.GetDepth(), r, beam.GetSSD()) / 100.0;
  // Calculate inverse square factor
  float isf;
  if(type == "SAD") isf = pow(((GetSSD_0()+Getd_0())/SPD),2.0);
  else isf = pow(((GetSSD_0()+Getd_0()) / (beam.GetSSD()+Getd_0())), 2.0);
  // Get off-axis factor
  float OAR = GetOAR(point.GetDepth(), point.GetOAD());
  // Calculate dose
  return ( mu * (Getk()*S_c*S_p*depth_dose*isf*OAR) );
}

float CBDose::CalcMU(float dose, LinacBeam &beam, CalcPoint &point, 
    std::string type){
  return ( dose / CalcDose(1.0, beam, point, type) );
}
