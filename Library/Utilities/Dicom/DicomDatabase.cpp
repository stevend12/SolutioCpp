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
// DICOM Database Manager                                                     //
// (DicomDatabase.cpp)                                                        //
//                                                                            //
// Steven Dolly                                                               //
// March 13, 2020                                                             //
//                                                                            //
// This file contains the main code for a class which manages a collection    //
// (i.e. database) of DICOM files.                                            //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#include "DicomDatabase.hpp"

#include <iostream>
#include <filesystem>
#include <vector>
#include <iterator>

#include <dcmtk/dcmiod/iodimage.h>
#include <dcmtk/dcmimgle/dcmimage.h>

namespace solutio {
  std::vector< std::tuple<std::string, std::string, std::string> >
    SupportedIODList {
      std::make_tuple("1.2.840.10008.5.1.4.1.1.2","CT","CT"),
      std::make_tuple("1.2.840.10008.5.1.4.1.1.481.1","RTIMAGE","RI"),
      std::make_tuple("1.2.840.10008.5.1.4.1.1.481.2","RTDOSE","RD"),
      std::make_tuple("1.2.840.10008.5.1.4.1.1.481.3","RTSTRUCT","RS")
  };

  DicomDatabaseFile::DicomDatabaseFile()
  {
    modality_name = file_path = "";
  }

  bool DicomDatabaseFile::ReadDicomInfo(std::string file_name)
  {
    // Load DICOM file using DCMTK
    DcmFileFormat fileformat;
    OFCondition status = fileformat.loadFile(file_name.c_str());
    if (status.good())
    {
      // Read DICOM modules
      DcmDataset data = *(fileformat.getDataset());
      Patient.read(data);
      Study.read(data);
      Series.read(data);
      SOPCommon.read(data);
      // Assign modality label based on SOP class UID; match to supported list
      // (DO NOT USE SWITCH!)
      OFString cuid;
      SOPCommon.getSOPClassUID(cuid);
      std::string class_uid(cuid.c_str());
      if(class_uid[(class_uid.length()-1)] == '\0') class_uid.erase(class_uid.length()-1);

      auto it = std::find_if(SupportedIODList.begin(), SupportedIODList.end(),
        [&class_uid](const std::tuple<std::string, std::string, std::string>& e)
          {return std::get<0>(e) == class_uid;});
      if (it != SupportedIODList.end()) modality_name = std::get<2>(*it);
      else modality_name = "Not Supported";
      /*
      // Single and multi-frame CT
      if(class_uid == "1.2.840.10008.5.1.4.1.1.2") modality_name = "CT";
      else if(class_uid == "1.2.840.10008.5.1.4.1.1.2.1") modality_name = "CT";
      // MRI
      else if(class_uid == "1.2.840.10008.5.1.4.1.1.4") modality_name = "MR";
      else if(class_uid == "1.2.840.10008.5.1.4.1.1.4.1") modality_name = "MR";
      // PET
      else if(class_uid == "1.2.840.10008.5.1.4.1.1.128") modality_name = "PET";
      else if(class_uid == "1.2.840.10008.5.1.4.1.1.130") modality_name = "PET";
      // US
      else if(class_uid == "1.2.840.10008.5.1.4.1.1.6.1") modality_name = "US";
      else if(class_uid == "1.2.840.10008.5.1.4.1.1.3.1") modality_name = "US";
      // Radiation therapy data files
      else if(class_uid == "1.2.840.10008.5.1.4.1.1.481.1") modality_name = "RT Image";
      else if(class_uid == "1.2.840.10008.5.1.4.1.1.481.2") modality_name = "RT Dose";
      else if(class_uid == "1.2.840.10008.5.1.4.1.1.481.3") modality_name = "RT Structure Set";
      else if(class_uid == "1.2.840.10008.5.1.4.1.1.481.4") modality_name = "RT Beams Treatment Record";
      else if(class_uid == "1.2.840.10008.5.1.4.1.1.481.5") modality_name = "RT Plan";
      else modality_name = "Not Supported";
      */
    }
    else
    {
      std::cerr << "Error: cannot read DICOM file (" << status.text() << ")" << '\n';
      return false;
    }
    // Get absolute file path
    std::filesystem::path p = file_name;
    file_path = std::filesystem::absolute(p).u8string();

    return true;
  }

  void DicomDatabaseSeries::SetInfo(std::string psuid, std::string suid, std::string mn)
  {
    parent_study_uid = psuid;
    series_uid = suid;
    modality_name = mn;
  }

  void DicomDatabase::MakeDatabase(std::string database_path)
  {
    // Make list of files in directory and sub-directories
    std::vector<std::string> file_list;
    for(const auto & entry : std::filesystem::recursive_directory_iterator(database_path))
    {
      if(!std::filesystem::is_directory(entry.path()))
      {
        file_list.push_back(entry.path().string());
      }
    }
    // Read and organize each file
    std::vector<std::string>::iterator file_it;
    for(file_it = file_list.begin(); file_it != file_list.end(); file_it++)
    {
      // Attempt to read file
      DicomDatabaseFile temp_file;
      if(!temp_file.ReadDicomInfo(*file_it)) continue;
      dicom_files.push_back(temp_file);
      OFString text;
      // Check patient
      temp_file.Patient.getPatientName(text);
      std::string pt_text(text.c_str());
      if(std::find(patient_list.begin(), patient_list.end(),
        pt_text) == patient_list.end())
      {
        patient_list.push_back(pt_text);
      }
      // Check study
      temp_file.Study.getStudyInstanceUID(text);
      std::string st_text(text.c_str());
      auto study_it = std::find_if(study_list.begin(), study_list.end(),
        [&temp_file,st_text](const std::pair<std::string, std::string>& el){
          return el.first == st_text;} );
      if(study_it == study_list.end())
      {
        std::pair<std::string, std::string> stp(st_text, pt_text);
        study_list.push_back(stp);
      }
      // Check series and assign file
      temp_file.Series.getSeriesInstanceUID(text);
      std::string se_text(text.c_str());
      auto series_it = std::find_if(series_list.begin(), series_list.end(),
        [&temp_file,se_text](DicomDatabaseSeries& el){
          return el.GetSeriesUID() == se_text;} );
      if(series_it == series_list.end())
      {
        DicomDatabaseSeries temp_series;
        temp_series.SetInfo(st_text, se_text, temp_file.modality_name);
        temp_series.AddFileID(std::distance(file_list.begin(), file_it));
        series_list.push_back(temp_series);
      }
      else
      {
        series_list[(std::distance(series_list.begin(), series_it))].AddFileID(
          std::distance(file_list.begin(), file_it));
      }
    }
  }

  std::vector<std::string> DicomDatabase::GetSeriesFileNames(unsigned int series_id)
  {
    std::vector<std::string> file_list;
    if(series_id >= series_list.size())
    {
      std::cerr << "Warning: series number " << series_id << " is not available\n";
      return file_list;
    }

    DicomDatabaseSeries dds = GetSeries(series_id);
    for(unsigned int n = 0; n < dds.GetNumFiles(); n++)
    {
      DicomDatabaseFile ddf = GetFile(dds.GetFileID(n));
      file_list.push_back(ddf.GetPath());
    }
    return file_list;
  }

  std::vector< GenericImage<float> > DicomDatabase::GetImageSeries(unsigned int series_id)
  {
    std::vector< GenericImage<float> > output_image;

    std::vector<std::string> file_list = GetSeriesFileNames(series_id);
    for(int n = 0; n < 1 /*file_list.size()*/; n++)
    {
      // Read in DICOM image modules using DCMTK
      DcmFileFormat fileformat;
      OFCondition status = fileformat.loadFile(file_list[n].c_str());
      if (status.good())
      {
        // Read DICOM modules
        DcmDataset * data = fileformat.getDataset();
        DicomImage Im(data, EXS_Unknown);
        // Assign/check image properties
        GenericImageHeader header;
        // In future, check for multiframe before determining slices
        header.SetImageSize(
          Im.getHeight(),
          Im.getWidth(),
          1,1
        );

        std::vector<float> vals;
        float val;

        vals = GetDicomArray<float>(data, DCM_PixelSpacing, 2);
        val = GetDicomValue<float>(data, DCM_SliceThickness);
        header.SetPixelDimensions(vals[0], vals[1], val);
        std::cout << "Dimensions: (" << vals[0] << ", " << vals[1] << ", " <<
          val << ")\n";
        vals.clear();

        vals = GetDicomArray<float>(data, DCM_ImagePositionPatient, 3);
        header.SetPixelOrigin(vals[0], vals[1], vals[2]);
        std::cout << "Origin: (" << vals[0] << ", " << vals[1] << ", " <<
          vals[2] << ")\n";
        vals.clear();

        vals = GetDicomArray<float>(data, DCM_ImageOrientationPatient, 6);
        header.SetDirectionCosines(vals[0], vals[1], vals[2], vals[3], vals[4], vals[5]);
        std::cout << "Dimensions: (" << vals[0] << ", " << vals[1] << ", " <<
          vals[2] << ", " << vals[3] << ", " << vals[4] << ", " <<
          vals[5] << ")\n";
        vals.clear();
        // Assign pixel data
        /*
        GenericImage<float> temp_image;
        temp_image.SetHeader(header);
        if(Im->isMonochrome())
        {
          Im->setMinMaxWindow();
          Uint8 * pixel_data = (Uint8 *)(Im->getOutputData(8));
          if(pixelData != NULL)
          {
            for(int v = 0; v < )
          }
        }
        std::vector<char> byte_buffer;// = ipm.PixelData.GetValue();
        std::vector<float> image_buffer;
        int p_bytes = 0;//ipm.BitsAllocated.GetValue() / 8;
        if(p_bytes < 1 || p_bytes == 3 || p_bytes > 4)
        {
          std::cerr << "Image with " << p_bytes << " bytes/pixel is not supported\n";
          return output_image;
        }

        for(unsigned long int n = 0; n < byte_buffer.size(); n += p_bytes)
        {
          if(p_bytes == 1)
          {
            uint8_t raw8 = byte_buffer[n];
            image_buffer.push_back(float(raw8));
          }
          if(p_bytes == 2)
          {
            uint16_t raw16;
            for(int b = 0; b < p_bytes; b++)
            {
              *((char*)(&raw16) + b) = byte_buffer[(n+b)];
            }
            image_buffer.push_back(float(raw16));
          }
          else
          {
            uint32_t raw32;
            for(int b = 0; b < p_bytes; b++)
            {
              *((char*)(&raw32) + b) = byte_buffer[(n+b)];
            }
            image_buffer.push_back(float(raw32));
          }
        }
        */
      }
      else
      {
        std::cerr << "Image " << n << " from file " << file_list[n] <<
          " could not be read, returning " << (n-1) << '/' << file_list.size() <<
          " images\n";
        return output_image;
      }
    }

    return output_image;
  }

  std::vector<std::string> DicomDatabase::PrintTree()
  {
    std::vector<std::string> print_text;
    for(unsigned int p = 0; p < patient_list.size(); p++)
    {
      print_text.push_back(patient_list[p]);
      for(unsigned int st = 0; st < study_list.size(); st++)
      {
        if(study_list[st].second == patient_list[p])
        {
          print_text.push_back("  "+study_list[st].first);
          for(unsigned int se = 0; se < series_list.size(); se++)
          {
            if(series_list[se].CheckStudy(study_list[st].first))
            {
              print_text.push_back("    "+series_list[se].GetModalityName()+
                " ("+std::to_string(series_list[se].GetNumFiles())+" files)");
            }
          }
        }
      }
    }

    return print_text;
  }
}
