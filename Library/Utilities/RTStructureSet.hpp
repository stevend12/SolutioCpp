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
// (RTStructureSet.hpp)                                                       //
//                                                                            //
// Steven Dolly                                                               //
// October 6, 2020                                                            //
//                                                                            //
// This is the header file for the classes which load, store, and manipulate  //
// individual RT structure as well as sets of RT structures.                  //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#ifndef RTSTRUCTURESET_HPP
#define RTSTRUCTURESET_HPP

#include <string>
#include <vector>

#include "../Geometry/Vec3.hpp"

namespace solutio
{
  class StructureContour
  {
    public:
      Vec3<double> GetPoint(int n){ return points[n]; }
      std::vector< Vec3<double> > GetPoints(){ return points; }
      int GetNumPoints(){ return points.size(); }
      std::string GetGeometricType(){ return geometric_type; }
      void AddPoint(Vec3<double> p){ points.push_back(p); }
      void SetGeometricType(std::string geo_type){ geometric_type = geo_type; }
    private:
      std::vector< Vec3<double> > points;
      std::string geometric_type;
  };

  class RTStructure {
    public:
      std::string GetName(){ return name; }
      StructureContour GetContour(int n){ return contours[n]; }
      std::vector<StructureContour> GetContours(){ return contours; }
      int GetNumContours(){ return contours.size(); }
      float GetColor(int n){ return color[n]; }

      void SetName(std::string n){ name = n; }
      void AddContour(StructureContour p){ contours.push_back(p); }
      void SetColor(float r, float g, float b);
    private:
      std::string name;
      std::vector<StructureContour> contours;
      float color[3];
  };

  class RTStructureSet
  {
    public:
      RTStructure GetStructure(int id);
      RTStructure GetStructure(std::string name);
      std::vector<RTStructure> GetStructures(){ return structures; }
      int GetNumStructures(){ return structures.size(); }
      void AddStructure(RTStructure s){ structures.push_back(s); }
      std::vector<double> SliceVectorZ();
    private:
      std::vector<RTStructure> structures;
  };
}

#endif
