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

#include <gdcmReader.h>
#include <gdcmWriter.h>

namespace solutio {
  std::vector< std::tuple<std::string, std::string, std::string> >
    SupportedIODList {
      std::make_tuple("1.2.840.10008.5.1.4.1.1.2","CT","CT"),
      std::make_tuple("1.2.840.10008.5.1.4.1.1.481.1","RTIMAGE","RI"),
      std::make_tuple("1.2.840.10008.5.1.4.1.1.481.2","RTDOSE","RD"),
      std::make_tuple("1.2.840.10008.5.1.4.1.1.481.3","RTSTRUCT","RS")
  };

  BaseIOD::BaseIOD()
  {
    Modules.push_back(&Patient);
    Modules.push_back(&SOPCommon);
  }

  bool BaseIOD::IsIODSupported(std::string sop_class)
  {
    bool ans;
    auto it = std::find_if(SupportedIODList.begin(), SupportedIODList.end(),
      [&sop_class](const std::tuple<std::string, std::string, std::string>& e)
        {return std::get<0>(e) == sop_class;});
    if (it != SupportedIODList.end()) ans = true;
    else ans = false;
    return ans;
  }

  std::string BaseIOD::GetFilePrefix(std::string sop_class)
  {
    std::string ans = "";
    auto it = std::find_if(SupportedIODList.begin(), SupportedIODList.end(),
      [&sop_class](const std::tuple<std::string, std::string, std::string>& e)
        {return std::get<0>(e) == sop_class;});
    if (it != SupportedIODList.end()) ans = std::get<2>(*it);
    return ans;
  }

  bool BaseIOD::Read(std::string file_name)
  {
    // Open and read DICOM file using GDCM reader
    gdcm::Reader reader;
    reader.SetFileName(file_name.c_str());
    if(!reader.Read())
    {
      std::cout << "Could not read: " << file_name << std::endl;
      return false;
    }
    gdcm::File &file = reader.GetFile();
    gdcm::DataSet &ds = file.GetDataSet();
    if(ds.IsEmpty())
    {
      std::cout << "DICOM data file " << file_name << " is empty.\n";
      return false;
    }
    // Read all modules
    std::vector<DicomModule *>::iterator it;
    for(it = Modules.begin(); it != Modules.end(); it++) (*it)->Read(ds);
    // Return true for success
    return true;
  }

  bool BaseIOD::Write(std::string file_name = "")
  {
    // Infer file name from SOP UIDs if not given
    if(file_name == "")
    {
      std::string file_pre = "NA";
      if(IsIODSupported(SOPCommon.SOPClassUID.GetValue()))
      {
        file_pre = GetFilePrefix(SOPCommon.SOPClassUID.GetValue());
      }
      file_name = file_pre+"."+SOPCommon.SOPInstanceUID.GetValue()+".dcm";
    }
    // Open file to write using GDCM writer
    gdcm::Writer writer;
    gdcm::File &file = writer.GetFile();
    gdcm::DataSet &ds = file.GetDataSet();
    gdcm::FileMetaInformation &header = file.GetHeader();
    header.SetDataSetTransferSyntax(gdcm::TransferSyntax::ImplicitVRLittleEndian);
    // Insert all modules
    std::vector<DicomModule *>::iterator it;
    for(it = Modules.begin(); it != Modules.end(); it++) (*it)->Insert(ds);
    // Write file
    writer.SetFileName(file_name.c_str());
    if(!writer.Write())
    {
      std::cerr << "Error: unable to write\n";
      return false;
    }
    // Return true for success
    return true;
  }

  BaseImageIOD::BaseImageIOD()
  {
    Modules.push_back(&GeneralStudy);
    Modules.push_back(&GeneralSeries);
    Modules.push_back(&FrameOfReference);
    Modules.push_back(&GeneralEquipment);
    Modules.push_back(&GeneralImage);
    Modules.push_back(&ImagePlane);
    Modules.push_back(&ImagePixel);
  }

  CTImageIOD::CTImageIOD()
  {
    // Set SOP Common class to CT Image IOD
    SOPCommon.SOPClassUID.SetValue("1.2.840.10008.5.1.4.1.1.2");
    // Set modality to CT
    GeneralSeries.Modality.SetValue("CT");
    // Add CT-specific modules
    Modules.push_back(&CTImage);
  }

  bool CTImageIOD::WriteSeriesFromSingle(std::string folder, std::string sopi_base,
    int sopi_start, int num_slices, std::vector<char> volume_pixel_data)
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

      GeneralImage.InstanceNumber.SetValue(sopi_start+n);

      ImagePlane.ImagePosition.SetValue(ct_origin[0], 0);
      ImagePlane.ImagePosition.SetValue(ct_origin[1], 1);
      ImagePlane.ImagePosition.SetValue(ct_origin[2]-((n-1)*slice_thickness), 2);

      std::vector<char>::const_iterator first =
        volume_pixel_data.begin() + n*slice_buffer_size;
      std::vector<char>::const_iterator last =
        volume_pixel_data.begin() + (n+1)*slice_buffer_size;
      std::vector<char> slice_buffer(first, last);
      ImagePixel.PixelData.SetValue(slice_buffer);

      SOPCommon.SOPInstanceUID.SetValue(current_uid);

      // Set the filename
      std::string series_file_name = "CT." + current_uid + ".dcm";
      bool did_write = Write(folder+series_file_name);
      if(!did_write)
      {
        std::cerr << "Error: unable to write\n";
        return false;
      }
    }
    return true;
  }

  std::vector<int16_t> CTImageIOD::GetHUImage()
  {
    double slope = CTImage.RescaleSlope.GetValue();
    double intercept = CTImage.RescaleIntercept.GetValue();
    std::vector<char> image_buffer = ImagePixel.PixelData.GetValue();
    std::vector<int16_t> hu_buffer;
    for(unsigned long int n = 0; n < image_buffer.size(); n+=2)
    {
      uint16_t raw;
      *((char*)(&raw) + 1) = image_buffer[(n+1)];
      *((char*)(&raw) + 0) = image_buffer[n];
      hu_buffer.push_back(round(slope*double(raw) + intercept));
    }
    return hu_buffer;
  }

  RTImageIOD::RTImageIOD()
  {
    // Set SOP Common class to CT Image IOD
    SOPCommon.SOPClassUID.SetValue("1.2.840.10008.5.1.4.1.1.481.1");
    // Set modality to RTSTRUCT
    RTSeries.Modality.SetValue("RTIMAGE");
    // Add RT-image specific modules
    Modules.push_back(&GeneralStudy);
    Modules.push_back(&RTSeries);
    Modules.push_back(&FrameOfReference);
    Modules.push_back(&GeneralEquipment);
    Modules.push_back(&GeneralImage);
    Modules.push_back(&ImagePixel);
    //Modules.push_back(&Multiframe);
    Modules.push_back(&RTImage);
  }

  RTDoseIOD::RTDoseIOD()
  {
    // Set SOP Common class to CT Image IOD
    SOPCommon.SOPClassUID.SetValue("1.2.840.10008.5.1.4.1.1.481.2");
    // Set modality to RTSTRUCT
    RTSeries.Modality.SetValue("RTDOSE");
    // Add RT-dose specific modules
    Modules.push_back(&GeneralStudy);
    Modules.push_back(&RTSeries);
    Modules.push_back(&FrameOfReference);
    Modules.push_back(&GeneralEquipment);
    Modules.push_back(&GeneralImage);
    Modules.push_back(&ImagePlane);
    Modules.push_back(&ImagePixel);
    Modules.push_back(&Multiframe);
    Modules.push_back(&RTDose);
  }

  RTStructureSetIOD::RTStructureSetIOD()
  {
    // Set SOP Common class to CT Image IOD
    SOPCommon.SOPClassUID.SetValue("1.2.840.10008.5.1.4.1.1.481.3");
    // Set modality to RTSTRUCT
    RTSeries.Modality.SetValue("RTSTRUCT");
    // Add RT-structure set specific modules
    Modules.push_back(&GeneralStudy);
    Modules.push_back(&RTSeries);
    Modules.push_back(&GeneralEquipment);
    Modules.push_back(&StructureSet);
    Modules.push_back(&ROIContour);
    Modules.push_back(&RTROIObservations);
  }
}
