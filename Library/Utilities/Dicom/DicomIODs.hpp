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

namespace solutio {
  class BaseIOD
  {
    public:
      PatientModule Patient;
      SOPCommonModule SOPCommon;
  };

  class BaseImageIOD : public BaseIOD
  {
    public:
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
      void SetVolumePixelData(std::vector<char> vpd){ volume_pixel_data = vpd; }
      bool ReadSeries();
      bool WriteSeries(std::string folder, std::string sopi_base,
        int sopi_start, int num_slices);
      CTImageModule CTImage;
    private:
      // Pixel data for entire volume (not just one slice)
      std::vector<char> volume_pixel_data;
  };

  class RTStructureSetIOD : public BaseIOD
  {
    public:
      RTStructureSetIOD();
      bool Read();
      bool Write(std::string filename);
      GeneralStudyModule GeneralStudy;
      RTSeriesModule RTSeries;
      GeneralEquipmentModule GeneralEquipment;
      StructureSetModule StructureSet;
      ROIContourModule ROIContour;
      RTROIObservationsModule RTROIObservations;
  };

  class RTDoseIOD : public BaseIOD
  {
    public:
      RTDoseIOD();
      bool Read();
      bool Write(std::string filename);
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
}

#endif
