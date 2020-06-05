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
// (GenericImage.hpp)                                                         //
//                                                                            //
// Steven Dolly                                                               //
// May 15, 2020                                                               //
//                                                                            //
// This file contains the header for the class which defines a generic image, //
// including image size (in pixels), pixel dimensions, and pixel data.        //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#ifndef GENERICIMAGE_HPP
#define GENERICIMAGE_HPP

#include <iostream>
#include <vector>
#include <algorithm>

namespace solutio
{
  template <class T>
  class GenericImage
  {
    public:
      // Set functions
      void SetImageSize(unsigned int r, unsigned int c, unsigned int ns,
        unsigned int np);
      void SetPixelDimensions(double dx, double dy, double dz);
      void SetPixelOrigin(double ox, double oy, double oz);
      void SetDirectionCosines(double rx, double ry, double rz, double cx,
        double cy, double cz);
      void SetRescaleSlope(double rs){ rescale_slope = rs; }
      void SetRescaleIntercept(double ri){ rescale_intercept = ri; }
      // Get functions
      unsigned int * GetImageSize(){ return image_size; }
      double * GetPixelDimensions(){ return pixel_dimensions; }
      double * GetPixelOrigin(){ return pixel_origin; }
      double * GetDirectionCosines(){ return direction_cosines; }
      double GetRescaleSlope(){ return rescale_slope; }
      double GetRescaleIntercept(){ return rescale_intercept; }
      void SetImage(std::vector<T> input_data){ pixel_data = input_data; }
      std::vector<T> GetImage(){ return pixel_data; }
      std::vector<T> GetImageFrame(unsigned int f);
      T GetMaxValue();
      T GetMinValue();
    private:
      unsigned int image_size[4];
      double pixel_dimensions[3];
      double pixel_origin[3];
      double direction_cosines[6];
      double rescale_slope;
      double rescale_intercept;
      std::vector<T> pixel_data;
  };

  template <class T>
  void GenericImage<T>::SetImageSize(unsigned int r, unsigned int c,
    unsigned int ns, unsigned int np)
  {
    image_size[0] = r;
    image_size[1] = c;
    image_size[2] = ns;
    image_size[3] = np;
  }

  template <class T>
  void GenericImage<T>::SetPixelDimensions(double dx, double dy, double dz)
  {
    pixel_dimensions[0] = dx;
    pixel_dimensions[1] = dy;
    pixel_dimensions[2] = dz;
  }

  template <class T>
  void GenericImage<T>::SetPixelOrigin(double ox, double oy, double oz)
  {
    pixel_origin[0] = ox;
    pixel_origin[1] = oy;
    pixel_origin[2] = oz;
  }

  template <class T>
  void GenericImage<T>::SetDirectionCosines(double rx, double ry, double rz,
    double cx, double cy, double cz)
  {
    direction_cosines[0] = rx;
    direction_cosines[1] = ry;
    direction_cosines[2] = rz;
    direction_cosines[3] = cx;
    direction_cosines[4] = cy;
    direction_cosines[5] = cz;
  }

  template <class T>
  std::vector<T> GenericImage<T>::GetImageFrame(unsigned int frame)
  {
    std::vector<T> image_frame;

    if(frame > image_size[2])
    {
      std::cerr << "Warning: image frame " << frame << " exceeds image size (" <<
        image_size[2] << ")\n";
      return image_frame;
    }

    unsigned long int p_start = image_size[0]*image_size[1]*image_size[3]*frame;
    unsigned long int p_end = image_size[0]*image_size[1]*image_size[3]*(frame+1);
    for(unsigned long int n = p_start; n < p_end; n++)
    {
      image_frame.push_back(pixel_data[n]);
    }
    return image_frame;
  }

  template <class T>
  T GenericImage<T>::GetMaxValue()
  {
    return *std::max_element(pixel_data.begin(), pixel_data.end());
  }

  template <class T>
  T GenericImage<T>::GetMinValue()
  {
    return *std::min_element(pixel_data.begin(), pixel_data.end());
  }
}

#endif
