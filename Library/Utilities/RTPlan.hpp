/******************************************************************************/
/*                                                                            */
/* Copyright 2021 Steven Dolly                                                */
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
// RT Plan                                                                    //
// (RTPlan.hpp)                                                               //
//                                                                            //
// Steven Dolly                                                               //
// March 19, 2021                                                             //
//                                                                            //
// This is the header file for the classes which load, store, and manipulate  //
// RT plan data, both for external beam and brachytherapy.                    //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#ifndef RTPLAN_HPP
#define RTPLAN_HPP

#include <string>
#include <ctime>
#include <vector>

#include "../Geometry/Vec3.hpp"

namespace solutio
{
  struct FractionGroup
  {
    int Index;
    int Fractions;
    std::string Type;
    std::vector<double> Dose;
    std::vector<int> DoseID;
  };

  struct ReferenceDosePoint
  {
    int Index;
    Vec3<double> Position;
    double Dose;
  };

  struct BrachySource
  {
    void Print();
    int Number;
    std::string Type;
    std::string IsotopeName;
    double IsotopeHalfLife;
    std::string StrengthUnits;
    double Strength;
    struct tm StrengthReferenceDateTime;
  };

  struct BrachyControlPoint
  {
    int Index;
    Vec3<double> Position;
    double RelativePosition;
    double Weight;
  };

  struct BrachyChannel
  {
    void Print();
    int Number;
    double TotalTime;
    std::string SourceMovementType;
    int ReferencedSourceNumber;
    double FinalCumulativeTimeWeight;
    std::vector<BrachyControlPoint> ControlPoints;
  };

  struct BrachyApplicator
  {
    void Print();
    int Number;
    std::string Type;
    double TotalStrength;
    std::vector<BrachyChannel> Channels;
  };

  struct BrachyPlan
  {
    void Print();
    std::vector<FractionGroup> FractionGroups;
    std::vector<ReferenceDosePoint> DosePoints;
    std::vector<BrachySource> Sources;
    std::vector<BrachyApplicator> Applicators;
    std::string TreatmentMachineName;
    std::string TreatmentTechnique;
    std::string TreatmentType;
  };
}

#endif
