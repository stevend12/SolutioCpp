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
// DICOM Module Classes                                                       //
// (DicomModules.hpp)                                                         //
//                                                                            //
// Steven Dolly                                                               //
// March 13, 2020                                                             //
//                                                                            //
// This file contains the header for a set of classes which define several    //
// common DICOM modules. These modules are combined together to form Image    //
// Object Definitions (IODs).                                                 //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#ifndef DICOMMODULES_HPP
#define DICOMMODULES_HPP

#include "DicomAttributes.hpp"

namespace solutio {
  // Base class
  class DicomModule {
    public:
      DicomModule()
      {
        ie_name = name = "";
      }
      virtual void Read(const gdcm::DataSet& H)
      {
        for(int n = 0; n < attribute_list.size(); n++)
        {
          attribute_list[n]->ReadAttribute(H);
        }
      }
      virtual void Insert(gdcm::DataSet& H)
      {
        for(int n = 0; n < attribute_list.size(); n++)
        {
          attribute_list[n]->InsertAttribute(H);
        }
      }
      std::string GetIE(){ return ie_name; }
      std::string GetName(){ return name; }
    protected:
      std::string ie_name;
      std::string name;
      std::vector<BaseAttribute *> attribute_list;
  };

  // Patient Module
  class PatientModule : public DicomModule {
    public:
      PatientModule()
      {
        ie_name = name = "Patient";
        BaseAttribute * a[4] = {&PatientName, &PatientID, &PatientBirthDate,
            &PatientSex};
        attribute_list.insert(attribute_list.end(), std::begin(a), std::end(a));
      }
      void operator=(const PatientModule &pm)
      {
          PatientName = pm.PatientName;
          PatientID = pm.PatientID;
          PatientBirthDate = pm.PatientBirthDate;
          PatientSex = pm.PatientSex;
      }
      // Patient's name (0010,0010)
      SingleValueAttribute<std::string,0x0010,0x0010> PatientName;
      // Patient ID (0010,0020)
      SingleValueAttribute<std::string,0x0010,0x0020> PatientID;
      // Patient's birth date (0010,0030)
      SingleValueAttribute<std::string,0x0010,0x0030> PatientBirthDate;
      // Patient's sex (0010,0040)
      SingleValueAttribute<std::string,0x0010,0x0040> PatientSex;
  };

  // General Study Module
  class GeneralStudyModule : public DicomModule {
    public:
      GeneralStudyModule()
      {
        ie_name = "Study";
        name = "General Study";
        BaseAttribute * a[4] = {&StudyInstanceUID, &StudyDate, &StudyTime,
            &StudyDescription};
        attribute_list.insert(attribute_list.end(), std::begin(a), std::end(a));
      }
      void operator=(const GeneralStudyModule &gsm)
      {
        StudyInstanceUID = gsm.StudyInstanceUID;
        StudyDate = gsm.StudyDate;
        StudyTime = gsm.StudyTime;
        StudyDescription = gsm.StudyDescription;
      }
      // Study Instance UID (0020,000D)
      SingleValueAttribute<std::string,0x0020,0x000D> StudyInstanceUID;
      // Study Date (0008,0020)
      SingleValueAttribute<std::string,0x0008,0x0020> StudyDate;
      // Study Time (0008,0030)
      SingleValueAttribute<std::string,0x0008,0x0030> StudyTime;
      // Study Description (0008,1030)
      SingleValueAttribute<std::string,0x0008,0x1030> StudyDescription;
  };

  // General Series Module
  class GeneralSeriesModule : public DicomModule {
    public:
      GeneralSeriesModule()
      {
        ie_name = "Series";
        name = "General Series";

        PatientPosition.SetValue("HFS");

        BaseAttribute * a[5] = {&Modality, &SeriesInstanceUID, &SeriesNumber,
            &SeriesDescription, &PatientPosition};
        attribute_list.insert(attribute_list.end(), std::begin(a), std::end(a));
      }
      void operator=(const GeneralSeriesModule &gsm)
      {
        Modality = gsm.Modality;
        SeriesInstanceUID = gsm.SeriesInstanceUID;
        SeriesNumber = gsm.SeriesNumber;
        SeriesDescription = gsm.SeriesDescription;
        PatientPosition = gsm.PatientPosition;
      }
      // Modality (0008,0060)
      SingleValueAttribute<std::string,0x0008,0x0060> Modality;
      // Series Instance UID (0020,000E)
      SingleValueAttribute<std::string,0x0020,0x000e> SeriesInstanceUID;
      // Series Number (0020,0011)
      SingleValueAttribute<std::string,0x0020,0x0011> SeriesNumber;
      // Series Description (0008,103E)
      SingleValueAttribute<std::string,0x0008,0x103e> SeriesDescription;
      // Patient Position (0018,5100)
      SingleValueAttribute<std::string,0x0018,0x5100> PatientPosition;
  };

  // Frame of Reference Module
  class FrameOfReferenceModule : public DicomModule {
    public:
      FrameOfReferenceModule()
      {
        ie_name = "Frame Of Reference";
        name = "Frame Of Reference";
        BaseAttribute * a[2] = {&FrameOfReferenceUID, &PositionReferenceIndicator};
        attribute_list.insert(attribute_list.end(), std::begin(a), std::end(a));
      }
      // Frame of Reference UID (0020,0052)
      SingleValueAttribute<std::string,0x0020,0x0052> FrameOfReferenceUID;
      // Position Reference Indicator (0020,1040)
      SingleValueAttribute<std::string,0x0020,0x1040> PositionReferenceIndicator;
  };

  // General Equipment Module
  class GeneralEquipmentModule : public DicomModule
  {
    public:
      GeneralEquipmentModule()
      {
        ie_name = "Equipment";
        name = "General Equipment";
        BaseAttribute * a[3] = {&Manufacturer, &ManufacturersModelName,
            &SoftwareVersions};
        attribute_list.insert(attribute_list.end(), std::begin(a), std::end(a));
      }
      // Manufacturer (0008,0070)
      SingleValueAttribute<std::string,0x0008,0x0070> Manufacturer;
      // Manufacturer's Model Name (0008,1090)
      SingleValueAttribute<std::string,0x0008,0x1090> ManufacturersModelName;
      // Software Versions (0018,1020)
      SingleValueAttribute<std::string,0x0018,0x1020> SoftwareVersions;
  };

  // General Image Module
  class GeneralImageModule : public DicomModule
  {
    public:
      GeneralImageModule()
      {
        ie_name = "Image";
        name = "General Image";
        BaseAttribute * a[2] = {&InstanceNumber, &PatientOrientation};
        attribute_list.insert(attribute_list.end(), std::begin(a), std::end(a));
      }
      // Instance Number (0020,0013)
      SingleValueAttribute<unsigned int,0x0020,0x0013> InstanceNumber;
      // Patient Orientation (0020,0020)
      SingleValueAttribute<std::string,0x0020,0x0020> PatientOrientation;
  };

  // Image Plane Module
  class ImagePlaneModule : public DicomModule
  {
    public:
      ImagePlaneModule() : PixelSpacing(0.0, 2), ImageOrientation(0.0, 6),
          ImagePosition(0.0, 3)
      {
        ie_name = "Image";
        name = "Image Plane";

        ImageOrientation.SetValue(1.0, 0);
        ImageOrientation.SetValue(0.0, 1);
        ImageOrientation.SetValue(0.0, 2);
        ImageOrientation.SetValue(0.0, 3);
        ImageOrientation.SetValue(1.0, 4);
        ImageOrientation.SetValue(0.0, 5);

        BaseAttribute * a[5] = {&PixelSpacing, &ImageOrientation, &ImagePosition,
            &SliceThickness, &SliceLocation};
        attribute_list.insert(attribute_list.end(), std::begin(a), std::end(a));
      }
      // Pixel Spacing (0028,0030)
      MultiNumberAttribute<0x0028,0x0030> PixelSpacing;
      // Image Orientation (Patient) (0020,0037)
      MultiNumberAttribute<0x0020,0x0037> ImageOrientation;
      // Image Position (Patient) (0020,0032)
      MultiNumberAttribute<0x0020,0x0032> ImagePosition;
      // Slice Thickness (0018,0050)
      SingleValueAttribute<double,0x0018,0x0050> SliceThickness;
      // Slice Location (0020,1041)
      SingleValueAttribute<double,0x0020,0x1041> SliceLocation;
  };

  // Image Pixel Module
  class ImagePixelModule : public DicomModule
  {
    public:
      ImagePixelModule()
      {
        ie_name = "Image";
        name = "Image Pixel";

        SamplesPerPixel.SetValue(1);
        PhotometricInterpretation.SetValue("MONOCHROME2");
        BitsAllocated.SetValue(16);
        BitsStored.SetValue(16);
        HighBit.SetValue(15);
        PixelRepresentation.SetValue("0000H");

        BaseAttribute * a[9] = { &SamplesPerPixel, &PhotometricInterpretation,
          &Rows, &Columns, &BitsAllocated, &BitsStored, &HighBit,
          &PixelRepresentation, &PixelData };
        attribute_list.insert(attribute_list.end(), std::begin(a), std::end(a));
      }
      // Samples per Pixel (0028,0002)
      SingleValueAttribute<unsigned int,0x0028,0x0002> SamplesPerPixel;
      // Photometric Interpretation (0028,0004)
      SingleValueAttribute<std::string,0x0028,0x0004> PhotometricInterpretation;
      // Rows (0028,0010)
      SingleValueAttribute<unsigned int,0x0028,0x0010> Rows;
      // Columns (0028,0011)
      SingleValueAttribute<unsigned int,0x0028,0x0011> Columns;
      // Bits Allocated (0028,0100)
      SingleValueAttribute<unsigned int,0x0028,0x0100> BitsAllocated;
      // Bits Stored (0028,0101)
      SingleValueAttribute<unsigned int,0x0028,0x0101> BitsStored;
      // High Bit (0028,0102)
      SingleValueAttribute<unsigned int,0x0028,0x0102> HighBit;
      // Pixel Representation (0028,0103)
      SingleValueAttribute<std::string,0x0028,0x0103> PixelRepresentation;
      // Pixel Data (7FE0,0010)
      PixelDataAttribute PixelData;
  };

  // CT Image Module
  class CTImageModule : public DicomModule
  {
    public:
      CTImageModule()
      {
        ie_name = "Image";
        name = "CT Image";

        ImageType.SetValue("ORIGINAL\\PRIMARY\\AXIAL");
        RescaleIntercept.SetValue(-1000);
        RescaleSlope.SetValue(1);

        BaseAttribute * a[4] = { &ImageType, &RescaleIntercept, &RescaleSlope,
          &KVP };
        attribute_list.insert(attribute_list.end(), std::begin(a), std::end(a));
      }
      // Image Type (0008,0008)
      SingleValueAttribute<std::string,0x0008,0x0008> ImageType;
      // Rescale Intercept (0028,1052)
      SingleValueAttribute<double,0x0028,0x1052> RescaleIntercept;
      // Rescale Slope (0028,1053)
      SingleValueAttribute<double,0x0028,0x1053> RescaleSlope;
      // KVP (0018,0060)
      SingleValueAttribute<double,0x0018,0x0060> KVP;
  };

  // SOP Common Module
  class SOPCommonModule : public DicomModule
  {
    public:
      SOPCommonModule()
      {
        ie_name = "Image";
        name = "SOP Common";
        BaseAttribute * a[2] = {&SOPClassUID, &SOPInstanceUID};
        attribute_list.insert(attribute_list.end(), std::begin(a), std::end(a));
      }
      // SOP Class UID (0008,0016)
      SingleValueAttribute<std::string,0x0008,0x0016> SOPClassUID;
      // SOP Instance UID (0008,0018)
      SingleValueAttribute<std::string,0x0008,0x0018> SOPInstanceUID;
  };

  // RT Series Module
  class RTSeriesModule : public DicomModule
  {
    public:
      RTSeriesModule()
      {
        ie_name = "Series";
        name = "RT Series";
        BaseAttribute * a[4] = {&Modality, &SeriesInstanceUID, &SeriesNumber,
            &SeriesDescription};
        attribute_list.insert(attribute_list.end(), std::begin(a), std::end(a));
      }
      // Modality (0008,0060)
      SingleValueAttribute<std::string,0x0008,0x0060> Modality;
      // Series Instance UID (0020,000E)
      SingleValueAttribute<std::string,0x0020,0x000e> SeriesInstanceUID;
      // Series Number (0020,0011)
      SingleValueAttribute<std::string,0x0020,0x0011> SeriesNumber;
      // Series Description (0008,103E)
      SingleValueAttribute<std::string,0x0008,0x103e> SeriesDescription;
  };

  // Structure Set Module
  class StructureSetModule : public DicomModule
  {
    public:
      StructureSetModule()
      {
        ie_name = name = "Structure Set";
        BaseAttribute * a[6] = {&StructureSetLabel, &StructureSetName,
            &StructureSetDate, &StructureSetTime,
            &ReferencedFrameOfReferenceSequence, &StructureSetROISequence};
        attribute_list.insert(attribute_list.end(), std::begin(a), std::end(a));
      }
      // Structure Set Label (3006,0002)
      SingleValueAttribute<std::string,0x3006,0x0002> StructureSetLabel;
      // Structure Set Label (3006,0004)
      SingleValueAttribute<std::string,0x3006,0x0004> StructureSetName;
      // Structure Set Date (3006,0008)
      SingleValueAttribute<std::string,0x3006,0x0008> StructureSetDate;
      // Structure Set Time (3006,0009)
      SingleValueAttribute<std::string,0x3006,0x0009> StructureSetTime;
      // Referenced Frame of Reference Sequence (3006,0010)
      ReferencedFrameOfReferenceSequenceAttribute ReferencedFrameOfReferenceSequence;
      // Structure Set ROI Sequence (3006,0020)
      StructureSetROISequenceAttribute StructureSetROISequence;
  };

  // ROI Contour Module
  class ROIContourModule : public DicomModule
  {
    public:
      ROIContourModule()
      {
        ie_name = "Structure Set";
        name = "ROI Contour";
        attribute_list.push_back(&ROIContourSequence);
      }
      ROIContourSequenceAttribute ROIContourSequence;
  };

  // RT ROI Observations Module
  class RTROIObservationsModule : public DicomModule
  {
    public:
      RTROIObservationsModule()
      {
        ie_name = "Structure Set";
        name = "RT ROI Observations";
        attribute_list.push_back(&RTROIObservationsSequence);
      }
      RTROIObservationsSequenceAttribute RTROIObservationsSequence;
  };

  // Multi-frame Module
  class MultiframeModule : public DicomModule
  {
    public:
      MultiframeModule()
      {
        ie_name = "Dose";
        name = "Multi-frame";
        BaseAttribute * a[2] = { &NumberOfFrames, &FrameIncrementPointer };
        attribute_list.insert(attribute_list.end(), std::begin(a), std::end(a));
      }
      // Number of Frames (0028,0008)
      SingleValueAttribute<unsigned int,0x0028,0x0008> NumberOfFrames;
      // Frame Increment Pointer (0028,0009)
      TagAttribute<0x0028,0x0009> FrameIncrementPointer;
  };

  class RTDoseModule : public DicomModule
  {
    public:
      RTDoseModule()
      {
        ie_name = "Dose";
        name = "RT Dose";

        DoseUnits.SetValue("GY");
        DoseType.SetValue("PHYSICAL");
        DoseSummationType.SetValue("PLAN");

        BaseAttribute * a[6] = { &DoseUnits, &DoseType, &DoseSummationType,
          &ReferencedRTPlanSequence, &GridFrameOffsetVector, &DoseGridScaling };
        attribute_list.insert(attribute_list.end(), std::begin(a), std::end(a));
      }
      // Dose Units (3004,0002)
      SingleValueAttribute<std::string,0x3004,0x0002> DoseUnits;
      // Dose Type (3004,0004)
      SingleValueAttribute<std::string,0x3004,0x0004> DoseType;
      // Dose Summation Type (3004,000A)
      SingleValueAttribute<std::string,0x3004,0x000a> DoseSummationType;
      // Referenced RT Plan Sequence (300C,0002)
      ReferencedRTPlanSequenceAttribute ReferencedRTPlanSequence;
      // Grid Frame Offset Vector (3004,000C)
      MultiNumberAttribute<0x3004,0x000c> GridFrameOffsetVector;
      // Dose Grid Scaling (3004,000E)
      SingleValueAttribute<double,0x3004,0x000e> DoseGridScaling;
  };
}

#endif