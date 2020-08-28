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
// SNC Read Function                                                          //
// (SncRead.cpp)                                                              //
//                                                                            //
// Steven Dolly                                                               //
// August 10, 2020                                                            //
//                                                                            //
// This is the main file for the function that reads files generated by SNC   //
// products (MapCHECK, ArcCHECK, etc.) and returns the dose as an image.      //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#include "SncRead.hpp"

#include <fstream>
#include <sstream>
#include <iterator>

#include "itkImportImageFilter.h"

#include "FileIO.hpp"

namespace solutio
{
  ItkImageF3::Pointer SncRead(std::string file_name)
  {
    // Create output image pointer
    using ImageType = ItkImageF3;
    ImageType::Pointer dose_image = ImageType::New();

    // Read in pixel data and other info
    std::vector<float> pixel_data;
    unsigned int n_rows = 0, n_cols = 0;
    float x0, y0, x1, y1, dx, dy, dose;

    std::string txt = "";
    std::ifstream fin(file_name.c_str());
    while(txt != "Dose Interpolated") std::getline(fin, txt);
    std::getline(fin, txt); std::getline(fin, txt);
    do
    {
      n_rows++;
      std::vector<std::string> elems = solutio::LineRead(txt, '\t');

      if(n_rows == 1)
      {
        std::stringstream(elems[0]) >> y0;
        std::stringstream(elems[2]) >> dose;
        n_cols = elems.size() - 2;
      }
      if(n_rows == 2)
      {
        std::stringstream(elems[0]) >> y1;
        dy = y1 - y0;
      }

      for(int n = 2; n < elems.size(); n++)
      {
        std::stringstream(elems[n]) >> dose;
        pixel_data.push_back(0.01*dose);
      }
      std::getline(fin, txt);
    } while(txt.find("COL") == std::string::npos);

    std::getline(fin, txt);
    std::vector<std::string> elems = solutio::LineRead(txt, '\t');
    std::stringstream(elems[2]) >> x0;
    std::stringstream(elems[3]) >> x1;
    dx = x1 - x0;

    fin.close();

    // Assign to ITK image
    ImageType::SizeType size;
    size[0] = n_cols;
    size[1] = n_rows;
    size[2] = 1;

    ImageType::IndexType start;
    start.Fill(0);

    ImageType::RegionType region;
    region.SetIndex(start);
    region.SetSize(size);

    dose_image->SetRegions(region);
    dose_image->Allocate();
    dose_image->FillBuffer(itk::NumericTraits<double>::Zero);

    double origin[3];
    origin[0] = 10.0*x0;
    origin[1] = 0.0;
    origin[2] = 10.0*y0;
    dose_image->SetOrigin(origin);

    double spacing[3];
    spacing[0] = fabs(10.0*dx);
    spacing[1] = fabs(10.0*dy);
    spacing[2] = 1.0;
    dose_image->SetSpacing(spacing);

    ImageType::DirectionType direction;
    direction.SetIdentity();
    direction[1][1] = 0.0;
    direction[1][2] = 1.0;
    direction[2][1] = -1.0;
    direction[2][2] = 0.0;
    dose_image->SetDirection(direction);

    for(auto it = pixel_data.begin(); it != pixel_data.end(); ++it)
    {
      unsigned int d = std::distance(pixel_data.begin(), it);
      const ImageType::IndexType pixel_index = {{(d % n_cols), d / n_cols}};
      dose_image->SetPixel(pixel_index, *it);
    }

    return dose_image;
  }
}
