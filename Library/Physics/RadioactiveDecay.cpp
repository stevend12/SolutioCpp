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
// (RadioactiveDecay.cpp)                                                     //
//                                                                            //
// Steven Dolly                                                               //
// January 19, 2022                                                           //
//                                                                            //
// This is the main file for the classes/functions that store radioisotope    //
// data and calculate radioactive decay.                                      //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#include "RadioactiveDecay.hpp"

#include <cmath>

namespace solutio
{
  // List of pre-defined radionuclides
  std::vector< std::tuple<std::string, std::string, double, std::string> >
    RadionuclideList
  {
    std::make_tuple("Iodine-131", "I-131", 8.0, "days"),
    std::make_tuple("Iridium-192", "Ir-192", 74.0, "days")
  };

  Radionuclide::Radionuclide(std::string name)
  {
    for(const auto &it : RadionuclideList)
    {
      if(name == std::get<0>(it) || name == std::get<1>(it))
      {
        name = std::get<0>(it);
        abbreviation = std::get<1>(it);
        half_life = std::get<2>(it);
        half_life_units = std::get<3>(it);
      }
    }
  }

  Radionuclide::Radionuclide(std::string n, std::string abbr, double hl,
    std::string hlu)
  {
    name = n;
    abbreviation = abbr;
    half_life = hl;
    half_life_units = hlu;
  }

  double Radionuclide::DecayFactor(double time, std::string units)
  {
    double decay_factor;
    if(units == half_life_units) decay_factor = pow(0.5, time/half_life);
    else decay_factor = 1.0;

    return decay_factor;
  }

  double Radionuclide::DecayFactor(struct tm ref_time, struct tm calc_time)
  {
    double decay_time = difftime(mktime(&calc_time), mktime(&ref_time)); // seconds
    if(half_life_units == "hours") decay_time /= (3600.);
    if(half_life_units == "days") decay_time /= (24.*3600.);
    if(half_life_units == "years") decay_time /= (365.*24.*3600.);
    return pow(0.5, decay_time/half_life);
  }
}
