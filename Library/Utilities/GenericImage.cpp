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
// Generic Image Class                                                        //
// (GenericImage.cpp)                                                         //
//                                                                            //
// Steven Dolly                                                               //
// May 18, 2020                                                               //
//                                                                            //
// This file contains the main code for the class which defines a generic     //
// image, including image size (in pixels), pixel dimensions, and pixel data. //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#include "GenericImage.hpp"

namespace solutio
{
  void GenericImageHeader::SetImageSize(unsigned int r, unsigned int c,
    unsigned int ns, unsigned int np)
  {
    image_size[0] = r;
    image_size[1] = c;
    image_size[2] = ns;
    image_size[3] = np;
  }

  void GenericImageHeader::SetPixelDimensions(double dx, double dy, double dz)
  {
    pixel_dimensions[0] = dx;
    pixel_dimensions[1] = dy;
    pixel_dimensions[2] = dz;
  }

  void GenericImageHeader::SetPixelOrigin(double ox, double oy, double oz)
  {
    pixel_origin[0] = ox;
    pixel_origin[1] = oy;
    pixel_origin[2] = oz;
  }

  void GenericImageHeader::SetDirectionCosines(double rx, double ry, double rz,
    double cx, double cy, double cz)
  {
    direction_cosines[0] = rx;
    direction_cosines[1] = ry;
    direction_cosines[2] = rz;
    direction_cosines[3] = cx;
    direction_cosines[4] = cy;
    direction_cosines[5] = cz;
  }
}
