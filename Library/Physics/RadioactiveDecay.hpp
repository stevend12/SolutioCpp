/******************************************************************************/
/*                                                                            */
/* Copyright 2022 Steven Dolly                                                */
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
// Radioactive Decay                                                          //
// (RadioactiveDecay.hpp)                                                     //
//                                                                            //
// Steven Dolly                                                               //
// January 19, 2022                                                           //
//                                                                            //
// This is the header file for the classes/functions that store radioisotope  //
// data and calculate radioactive decay.                                      //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#ifndef RADIOACTIVEDECAY_HPP
#define RADIOACTIVEDECAY_HPP

#include <string>
#include <vector>
#include <tuple>

namespace solutio
{
  // List of pre-defined radionuclides
  extern std::vector< std::tuple<std::string, std::string, double,
    std::string> > RadionuclideList;

  class Radionuclide
  {
    public:
      // Constructors
      Radionuclide(std::string name);
      Radionuclide(std::string n, std::string abbr, double hl, std::string hlu);
      // Get/set functions
      std::string GetName(){ return name; }
      std::string GetAbbrevation(){ return abbreviation; }
      double GetHalfLife(){ return half_life; }
      std::string GetHalfLifeUnits(){ return half_life_units; }
      double GetElapsedTime(){ return elapsed_time; }
      std::string GetElapsedTimeUnits(){ return elapsed_time_units; }
      // Decay calculation
      double DecayFactor(double time, std::string units);
      double DecayFactor(struct tm ref_time, struct tm calc_time);
    private:
      std::string name;
      std::string abbreviation;
      double half_life;
      std::string half_life_units;
      double elapsed_time;
      std::string elapsed_time_units;
  };
}

#endif
