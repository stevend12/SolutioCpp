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

namespace solutio
{
  class GenericImageHeader
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
    protected:
      unsigned int image_size[4];
      double pixel_dimensions[3];
      double pixel_origin[3];
      double direction_cosines[6];
      double rescale_slope;
      double rescale_intercept;
  };

  template <class T>
  class GenericImage : GenericImageHeader
  {
    public:
      void SetHeader(GenericImageHeader header)
      {
        unsigned int * im_size = header.GetImageSize();
        SetImageSize(im_size[0], im_size[1], im_size[2], im_size[3]);

        double * pixel_dim = header.GetPixelDimensions();
        SetPixelDimensions(pixel_dim[0], pixel_dim[1], pixel_dim[2]);

        double * pixel_o = header.GetPixelOrigin();
        SetPixelOrigin(pixel_o[0], pixel_o[1], pixel_o[2]);

        double * dir_cos = header.GetDirectionCosines();
        SetDirectionCosines(dir_cos[0], dir_cos[1], dir_cos[2], dir_cos[3],
          dir_cos[4], dir_cos[5]);
      }
      void SetImage(std::vector<T> input_data){ pixel_data = input_data; }
      std::vector<T> GetImage(){ return pixel_data; }
      std::vector<T> GetImageFrame(unsigned int f);
    private:
      std::vector<T> pixel_data;
  };

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
}

#endif
