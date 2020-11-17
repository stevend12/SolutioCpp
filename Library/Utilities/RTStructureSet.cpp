/******************************************************************************/
/*                                                                            */
/* Copyright 2020 Steven Dolly                                                */
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
// RT Structure Set                                                           //
// (RTStructureSet.cpp)                                                       //
//                                                                            //
// Steven Dolly                                                               //
// October 6, 2020                                                            //
//                                                                            //
// This is the main for the classes which load, store, and manipulate         //
// individual RT structure as well as sets of RT structures.                  //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#include "RTStructureSet.hpp"

#include <algorithm>

namespace solutio
{
  void RTStructure::SetColor(float r, float g, float b)
  {
    color[0] = r; color[1] = g; color[2] = b;
  }

  RTStructure RTStructureSet::GetStructure(int id)
  {
    if(id < structures.size()) return structures[id];
    else
    {
      RTStructure s;
      return s;
    }
  }

  RTStructure RTStructureSet::GetStructure(std::string name)
  {
    for(int n = 0; n < structures.size(); n++)
    {
      if(name == structures[n].GetName()) return structures[n];
    }
    RTStructure s;
    return s;
  }

  std::vector<double> RTStructureSet::SliceVectorZ()
  {
    std::vector<double> slice_z;
    for(int n = 0; n < structures.size(); n++)
    {
      for(int c = 0; c < structures[n].GetNumContours(); c++)
      {
        StructureContour con = structures[n].GetContour(c);
        Vec3<double> p = con.GetPoint(0);
        if(slice_z.size() == 0) slice_z.push_back(p.z);
        else
        {
          bool add_to = true;
          for(int z = 0; z < slice_z.size(); z++)
          {
            if(fabs(slice_z[z] - p.z) < 1.0e-05)
            {
              add_to = false;
              break;
            }
          }
          if(add_to) slice_z.push_back(p.z);
        }
      }
    }
    std::sort(slice_z.begin(), slice_z.end());
    return slice_z;
  }
}
