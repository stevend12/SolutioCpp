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
// DCMTK Read Functions                                                       //
// (DcmtkRead.hpp)                                                            //
//                                                                            //
// Steven Dolly                                                               //
// June 4, 2020                                                               //
//                                                                            //
// This file contains utility functions to read DICOM files using DCMTK.      //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#ifndef DCMTKREAD_HPP
#define DCMTKREAD_HPP

#include <string>

#include <dcmtk/config/osconfig.h>
#include <dcmtk/dcmdata/dctk.h>
#include <dcmtk/dcmimgle/dcmimage.h>
#include <dcmtk/dcmiod/iodcommn.h>
#include <dcmtk/dcmrt/drmdose.h>

#include "../GenericImage.hpp"

namespace solutio
{
  // DCMTK helper functions
  template<class T>
  T GetDicomValue(DcmDataset * data, DcmTagKey key)
  {
    T output;
    OFString v;
    if(data->findAndGetOFString(key, v).good()) std::stringstream(v.c_str()) >> output;
    return output;
  }

  template<class T>
  std::vector<T> GetDicomArray(DcmDataset * data, DcmTagKey key, int length)
  {
    std::vector<T> output;
    for(int l = 0; l < length; l++)
    {
      T val;
      OFString s_val;
      data->findAndGetOFString(key, s_val, l);
      std::stringstream(s_val.c_str()) >> val;
      output.push_back(val);
    }
    return output;
  }

  template<class T>
  GenericImage<T> ReadRTDose(std::string file_name)
  {
    // Output image
    GenericImage<T> output_image;

    std::vector<float> vals;
    float val;

    // Read in DICOM image modules using DCMTK
    DcmFileFormat fileformat;
    OFCondition status = fileformat.loadFile(file_name.c_str());
    if(status.good())
    {
      // Read DICOM image and data
      DcmDataset * data = fileformat.getDataset();
      DRTDose * rtd = new DRTDose();
      status = rtd->read(*data);
      if(rtd->isValid())
      {
        // Read image info
        val = GetDicomValue<int>(data, DCM_NumberOfFrames);
        output_image.SetImageSize(
          rtd->getDoseImageHeight(), rtd->getDoseImageWidth(), val, 1
        );

        vals = GetDicomArray<float>(data, DCM_PixelSpacing, 2);
        std::vector<float> gfov = GetDicomArray<float>(data, DCM_GridFrameOffsetVector, 2);
        output_image.SetPixelDimensions(vals[0], vals[1], fabs(gfov[1]-gfov[0]));
        vals.clear();

        vals = GetDicomArray<float>(data, DCM_ImagePositionPatient, 3);
        output_image.SetPixelOrigin(vals[0], vals[1], vals[2]);
        vals.clear();

        vals = GetDicomArray<float>(data, DCM_ImageOrientationPatient, 6);
        output_image.SetDirectionCosines(vals[0], vals[1], vals[2], vals[3], vals[4], vals[5]);
        vals.clear();

        // Read pixel data
        OFVector< OFVector<double> > of_buffer;
        status = rtd->getDoseImages(of_buffer);
        if(status.good())
        {
          std::vector<T> buffer;
          for(int a = 0; a < of_buffer.size(); a++)
          {
            for(int b = 0; b < of_buffer[a].size(); b++)
            {
              buffer.push_back(of_buffer[a][b]);
            }
          }
          output_image.SetImage(buffer);
        }
        else
        {
          std::cerr << "Error: Could not read RTDose images, returning blank image\n";
          return output_image;
        }
      }
      else
      {
        std::cerr << "Error: RTDose is not valid, returning blank image\n";
        delete rtd;
        return output_image;
      }

      delete rtd;
    }
    else
    {
      std::cerr << "Error: Dose from file " << file_name <<
        " could not be read, returning blank image\n";
      return output_image;
    }

    return output_image;
  }

}

#endif
