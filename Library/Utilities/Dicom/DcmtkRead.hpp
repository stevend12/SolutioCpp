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
  GenericImage<T> ReadImageSeries(std::vector<std::string> file_list,
    void (*progress_function)(float) =
      [](float p){ std::cout << 100.0*p << "%\n"; })
  {
    // Output image
    GenericImage<T> output_image;

    // Global variables
    std::vector< std::pair<float,int> > image_z;
    std::vector<float> vals;
    float val;
    std::vector<float> temp_buffer;

    for(int n = 0; n < file_list.size(); n++)
    {
      // Read in DICOM image modules using DCMTK
      DcmFileFormat fileformat;
      OFCondition status = fileformat.loadFile(file_list[n].c_str());
      if (status.good())
      {
        // Read DICOM image and data
        DcmDataset * data = fileformat.getDataset();
        DicomImage Im(data, EXS_Unknown);
        // Assign/check image properties

        // In future, check for multiframe before determining slices

        if(n == 0)
        {
          output_image.SetImageSize(Im.getHeight(), Im.getWidth(), file_list.size(), 1);

          vals = GetDicomArray<float>(data, DCM_PixelSpacing, 2);
          val = GetDicomValue<float>(data, DCM_SliceThickness);
          output_image.SetPixelDimensions(vals[0], vals[1], val);
          vals.clear();

          vals = GetDicomArray<float>(data, DCM_ImagePositionPatient, 3);
          output_image.SetPixelOrigin(vals[0], vals[1], vals[2]);
          std::pair<float,int> zp(vals[2], n);
          image_z.push_back(zp);
          vals.clear();

          vals = GetDicomArray<float>(data, DCM_ImageOrientationPatient, 6);
          output_image.SetDirectionCosines(vals[0], vals[1], vals[2], vals[3], vals[4], vals[5]);
          vals.clear();
        }
        else
        {
          unsigned int * ims = output_image.GetImageSize();
          if(Im.getHeight() != ims[0] || Im.getWidth() != ims[1])
          {
            std::cerr << "Error: Image " << n << " size does not match "
              << "initial values\n";
            return output_image;
          }
          /* Other checks (implement later)
          double * pd = header.GetPixelDimensions();
          if(Im.getHeight() != ims[0] || Im.getWidth() != ims[1])
          {
            std::cerr << "Error: Image " << n << " size does not match "
              << "initial values\n";
            return output_image;
          }

          double * po = header.GetPixelOrigin();

          double * dc = GetDirectionCosines();
          */
          vals = GetDicomArray<float>(data, DCM_ImagePositionPatient, 3);
          std::pair<float,int> zp(vals[2], n);
          image_z.push_back(zp);
          vals.clear();
        }

        // Assign pixel data
        unsigned long np;
        if(Im.isMonochrome())
        {
          const DiPixel * pixel_obj = Im.getInterData();
          if(pixel_obj != NULL)
          {
            np = pixel_obj->getCount();
            EP_Representation rep = pixel_obj->getRepresentation();
            if(rep == EPR_Uint8)
            {
              Uint8 * pixel_data = (Uint8 *)pixel_obj->getData();
              for(unsigned long p = 0; p < np; p++)
              {
                temp_buffer.push_back(T(pixel_data[p]));
              }
            }
            if(rep == EPR_Sint8)
            {
              Sint8 * pixel_data = (Sint8 *)pixel_obj->getData();
              for(unsigned long p = 0; p < np; p++)
              {
                temp_buffer.push_back(T(pixel_data[p]));
              }
            }
            if(rep == EPR_Uint16)
            {
              Uint16 * pixel_data = (Uint16 *)pixel_obj->getData();
              for(unsigned long p = 0; p < np; p++)
              {
                temp_buffer.push_back(T(pixel_data[p]));
              }
            }
            if(rep == EPR_Sint16)
            {
              Sint16 * pixel_data = (Sint16 *)pixel_obj->getData();
              for(unsigned long p = 0; p < np; p++)
              {
                temp_buffer.push_back(T(pixel_data[p]));
              }
            }
            if(rep == EPR_Uint32)
            {
              Uint32 * pixel_data = (Uint32 *)pixel_obj->getData();
              for(unsigned long p = 0; p < np; p++)
              {
                temp_buffer.push_back(T(pixel_data[p]));
              }
            }
            if(rep == EPR_Sint32)
            {
              Sint32 * pixel_data = (Sint32 *)pixel_obj->getData();
              for(unsigned long p = 0; p < np; p++)
              {
                temp_buffer.push_back(T(pixel_data[p]));
              }
            }
          }
        }
        progress_function(float(n) / float(2*file_list.size()));
      }
      else
      {
        std::cerr << "Image " << n << " from file " << file_list[n] <<
          " could not be read, returning blank image\n";
        return output_image;
      }
    }
    // Sort and reorder
    std::vector<float> image_buffer;

    std::sort(image_z.begin(), image_z.end(), [&image_z](
      std::pair<float,int>& l, std::pair<float,int>& r)
        { return l.first < r.first; });

    double * po = output_image.GetPixelOrigin();
    output_image.SetPixelOrigin(po[0], po[1], image_z[0].first);

    unsigned int * im_size = output_image.GetImageSize();
    for(int n = 0; n < image_z.size(); n++)
    {
      unsigned long int p_start =
        im_size[0]*im_size[1]*im_size[3]*image_z[n].second;
      unsigned long int p_end =
        im_size[0]*im_size[1]*im_size[3]*(image_z[n].second+1);
      for(unsigned long int p = p_start; p < p_end; p++)
      {
        image_buffer.push_back(temp_buffer[p]);
      }
      progress_function(float(n+file_list.size()) / float(2*file_list.size()));
    }
    output_image.SetImage(image_buffer);

    return output_image;
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
