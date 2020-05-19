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
// DICOM Image Object Definition (IOD) Classes                                //
// (DicomIODs.hpp)                                                            //
//                                                                            //
// Steven Dolly                                                               //
// March 13, 2020                                                             //
//                                                                            //
// This file contains the header for a set of classes which define several    //
// common DICOM IODs uesd in medicine.                                        //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#ifndef DICOMIODS_HPP
#define DICOMIODS_HPP

#include "DicomModules.hpp"
#include "../GenericImage.hpp"

namespace solutio {
  // List of supported modalities
  extern std::vector< std::tuple<std::string, std::string, std::string> > SupportedIODList;
  // Function to load/convert pixel data attribute
  std::vector<float> ConvertPixelBuffer(ImagePixelModule &ipm);
  // Base class for all IODs
  class BaseIOD
  {
    public:
      BaseIOD();
      bool IsIODSupported(std::string sop_class);
      std::string GetFilePrefix(std::string sop_class);
      bool Read(std::string file_name);
      bool Write(std::string file_name);
      std::vector< std::pair<std::string, std::string> > Print();
      PatientModule Patient;
      SOPCommonModule SOPCommon;
      std::vector<DicomModule *> Modules;
  };
  // Base class for all image-based IODs (excluding RT IODs)
  class BaseImageIOD : public BaseIOD
  {
    public:
      BaseImageIOD();
      GenericImageHeader GetGenericImageHeader();
      GenericImage<float> GetGenericImage();
      GeneralStudyModule GeneralStudy;
      GeneralSeriesModule GeneralSeries;
      FrameOfReferenceModule FrameOfReference;
      GeneralEquipmentModule GeneralEquipment;
      GeneralImageModule GeneralImage;
      ImagePlaneModule ImagePlane;
      ImagePixelModule ImagePixel;
  };

  class CTImageIOD : public BaseImageIOD
  {
    public:
      CTImageIOD();
      bool WriteSeriesFromSingle(std::string folder, std::string sopi_base,
        int sopi_start, int num_slices, std::vector<char> volume_pixel_data);
      CTImageModule CTImage;
      std::vector<int16_t> GetHUImage();
  };

  class RTImageIOD : public BaseIOD
  {
    public:
      RTImageIOD();
      GenericImageHeader GetGenericImageHeader();
      GenericImage<float> GetGenericImage();
      GeneralStudyModule GeneralStudy;
      RTSeriesModule RTSeries;
      FrameOfReferenceModule FrameOfReference;
      GeneralEquipmentModule GeneralEquipment;
      GeneralImageModule GeneralImage;
      ImagePixelModule ImagePixel;
      //MultiframeModule Multiframe;
      RTImageModule RTImage;
  };

  class RTDoseIOD : public BaseIOD
  {
    public:
      RTDoseIOD();
      GeneralStudyModule GeneralStudy;
      RTSeriesModule RTSeries;
      FrameOfReferenceModule FrameOfReference;
      GeneralEquipmentModule GeneralEquipment;
      GeneralImageModule GeneralImage;
      ImagePlaneModule ImagePlane;
      ImagePixelModule ImagePixel;
      MultiframeModule Multiframe;
      RTDoseModule RTDose;
  };

  class RTStructureSetIOD : public BaseIOD
  {
    public:
      RTStructureSetIOD();
      GeneralStudyModule GeneralStudy;
      RTSeriesModule RTSeries;
      GeneralEquipmentModule GeneralEquipment;
      StructureSetModule StructureSet;
      ROIContourModule ROIContour;
      RTROIObservationsModule RTROIObservations;
  };
}

#endif
