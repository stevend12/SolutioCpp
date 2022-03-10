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
// (RTPlan.cpp)                                                               //
//                                                                            //
// Steven Dolly                                                               //
// March 19, 2021                                                             //
//                                                                            //
// This is the main file for the classes which load, store, and manipulate RT //
// plan data, both for external beam and brachytherapy.                       //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#include "RTPlan.hpp"

#include <iostream>

namespace solutio
{
  void BrachySource::Print()
  {
    std::cout << "Source Number: " << Number << '\n';
    std::cout << "Source Type: " << Type << '\n';
    std::cout << "Isotope: " << IsotopeName << '\n';
    std::cout << "Half Life (Days): " << IsotopeHalfLife << '\n';
    std::cout << "Source Strength Units: " << StrengthUnits << '\n';
    std::cout << "Source Strength: " << Strength << '\n';
    std::cout << "Reference Date-Time: " << asctime(&StrengthReferenceDateTime) << '\n';
  }

  void BrachyChannel::Print()
  {
    std::cout << "Channel ID: " << Number << '\n';
    std::cout << "Channel Time (s): " << TotalTime << '\n';
    std::cout << "Movement Type: " << SourceMovementType << '\n';
    std::cout << "Referenced Source ID: " << ReferencedSourceNumber << '\n';
    std::cout << "Cumulative Weight: " << FinalCumulativeTimeWeight << '\n';
  }

  void BrachyApplicator::Print()
  {
    std::cout << "Applicator ID: " << Number << '\n';
    std::cout << "Applicator Setup Type: " << Type << '\n';
    std::cout << "Applicator Total Air Kerma: " << TotalStrength << '\n';
    std::cout << "Channels:\n";
    for(int c = 0; c < Channels.size(); c++)
    {
      Channels[c].Print();
      for(int p = 0; p < Channels[c].ControlPoints.size(); p++)
      {
        std::cout << Channels[c].ControlPoints[p].Index << ": (" <<
          Channels[c].ControlPoints[p].Position.x << ", " <<
          Channels[c].ControlPoints[p].Position.y << ", " <<
          Channels[c].ControlPoints[p].Position.z << "); " <<
          Channels[c].ControlPoints[p].RelativePosition << "; " <<
          Channels[c].ControlPoints[p].Weight << "\n";
      }
    }
  }

  void BrachyPlan::Print()
  {
    std::cout << "Brachy Source List (" << Sources.size() << " sources)\n";
    for(int n = 0; n < Sources.size(); n++)
    {
      Sources[n].Print();
      std::cout << '\n';
    }
    std::cout << "Brachy Applicator List (" << Applicators.size() << " applicators)\n";
    for(int n = 0; n < Applicators.size(); n++)
    {
      Applicators[n].Print();
      std::cout << '\n';
    }
  }
}
