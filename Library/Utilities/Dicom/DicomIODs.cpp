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
// (DicomIODs.cpp)                                                            //
//                                                                            //
// Steven Dolly                                                               //
// March 13, 2020                                                             //
//                                                                            //
// This file contains the main code for a set of classes which define several //
// common DICOM IODs uesd in medicine.                                        //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#include "DicomIODs.hpp"

#include <gdcmWriter.h>

namespace solutio {
  CTImageIOD::CTImageIOD()
  {
    // Set SOP Common class to CT Image IOD
    SOPCommon.SOPClassUID.SetValue("1.2.840.10008.5.1.4.1.1.2");
    // Set modality to CT
    GeneralSeries.Modality.SetValue("CT");
  }

  bool CTImageIOD::ReadSeries()
  {
    return true;
  }

  bool CTImageIOD::WriteSeries(std::string folder, std::string sopi_base,
    int sopi_start, int num_slices)
  {
    // Calculation variables for CT slices
    double ct_origin[3] = {ImagePlane.ImagePosition.GetValue(0),
      ImagePlane.ImagePosition.GetValue(1), ImagePlane.ImagePosition.GetValue(2)};
    double slice_thickness = ImagePlane.SliceThickness.GetValue();
    unsigned long int slice_buffer_size = ImagePixel.Rows.GetValue() *
      ImagePixel.Columns.GetValue()*2;

    for(int n = 0; n < num_slices; n++)
    {
      std::string current_uid = sopi_base + "." + std::to_string(sopi_start+n);

      gdcm::Writer writer;
      gdcm::File &file = writer.GetFile();
      gdcm::DataSet &ds = file.GetDataSet();
      gdcm::FileMetaInformation &header = file.GetHeader();

      header.SetDataSetTransferSyntax(gdcm::TransferSyntax::ImplicitVRLittleEndian);

      Patient.Insert(ds);
      GeneralStudy.Insert(ds);
      GeneralSeries.Insert(ds);
      FrameOfReference.Insert(ds);
      GeneralEquipment.Insert(ds);

      GeneralImage.InstanceNumber.SetValue(sopi_start+n);
      GeneralImage.Insert(ds);

      ImagePlane.ImagePosition.SetValue(ct_origin[0], 0);
      ImagePlane.ImagePosition.SetValue(ct_origin[1], 1);
      ImagePlane.ImagePosition.SetValue(ct_origin[2]-((n-1)*slice_thickness), 2);
      ImagePlane.Insert(ds);

      std::vector<char>::const_iterator first =
        volume_pixel_data.begin() + n*slice_buffer_size;
      std::vector<char>::const_iterator last =
        volume_pixel_data.begin() + (n+1)*slice_buffer_size;
      std::vector<char> slice_buffer(first, last);
      ImagePixel.PixelData.SetValue(slice_buffer);
      ImagePixel.Insert(ds);

      CTImage.Insert(ds);

      SOPCommon.SOPInstanceUID.SetValue(current_uid);
      SOPCommon.Insert(ds);

      // Set the filename
      std::string series_filename = "CT." + current_uid + ".dcm";
      writer.SetFileName((folder+series_filename).c_str());
      if(!writer.Write())
      {
        std::cerr << "Error: unable to write\n";
        return false;
      }
    }
    return true;
  }

  RTStructureSetIOD::RTStructureSetIOD()
  {
    // Set SOP Common class to CT Image IOD
    SOPCommon.SOPClassUID.SetValue("1.2.840.10008.5.1.4.1.1.481.3");
    // Set modality to RTSTRUCT
    RTSeries.Modality.SetValue("RTSTRUCT");
  }

  bool RTStructureSetIOD::Write(std::string filename = "")
  {
    if(filename == "") filename = "RS."+SOPCommon.SOPInstanceUID.GetValue()+".dcm";

    gdcm::Writer writer;
    gdcm::File &file = writer.GetFile();
    gdcm::DataSet &ds = file.GetDataSet();

    gdcm::FileMetaInformation &header = file.GetHeader();
    header.SetDataSetTransferSyntax(gdcm::TransferSyntax::ImplicitVRLittleEndian);

    Patient.Insert(ds);
    GeneralStudy.Insert(ds);
    RTSeries.Insert(ds);
    GeneralEquipment.Insert(ds);
    StructureSet.Insert(ds);
    ROIContour.Insert(ds);
    RTROIObservations.Insert(ds);
    SOPCommon.Insert(ds);

    writer.SetFileName(filename.c_str());
    if(!writer.Write())
    {
      std::cerr << "Error: unable to write\n";
      return false;
    }

    return true;
  }

  RTDoseIOD::RTDoseIOD()
  {
    // Set SOP Common class to CT Image IOD
    SOPCommon.SOPClassUID.SetValue("1.2.840.10008.5.1.4.1.1.481.2");
    // Set modality to RTSTRUCT
    RTSeries.Modality.SetValue("RTDOSE");
  }

  bool RTDoseIOD::Write(std::string filename = "")
  {
    if(filename == "") filename = "RD."+SOPCommon.SOPInstanceUID.GetValue()+".dcm";

    gdcm::Writer writer;
    gdcm::File &file = writer.GetFile();
    gdcm::DataSet &ds = file.GetDataSet();

    gdcm::FileMetaInformation &header = file.GetHeader();
    header.SetDataSetTransferSyntax(gdcm::TransferSyntax::ImplicitVRLittleEndian);

    Patient.Insert(ds);
    GeneralStudy.Insert(ds);
    RTSeries.Insert(ds);
    FrameOfReference.Insert(ds);
    GeneralEquipment.Insert(ds);
    GeneralImage.Insert(ds);
    ImagePlane.Insert(ds);
    ImagePixel.Insert(ds);
    Multiframe.Insert(ds);
    RTDose.Insert(ds);
    SOPCommon.Insert(ds);

    writer.SetFileName(filename.c_str());
    if(!writer.Write())
    {
      std::cerr << "Error: unable to write\n";
      return false;
    }

    return true;
  }
}
