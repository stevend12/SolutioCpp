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
//#include <filesystem>
#include <vector>

namespace solutio {
  DicomDatabaseFile::DicomDatabaseFile()
  {
    patient_name = "";
    study_uid = "";
    series_uid = "";
    instance_uid = "";
    sop_class_uid = "";
    sop_instance_uid = "";
    file_path = "";
  }
  /*
  void DicomDatabase::MakeDatabase(std::string database_path)
  {
    std::vector<std::string> file_list;
    std::string path = "Data";
    for(const auto & entry : std::filesystem::recursive_directory_iterator(path))
    {
      file_list.push_back(entry.path().string());
    }
    //std::vector<string>::iterator it;
    //it = std::unique(file_list.begin(), file_list.end());
    //file_list =
  }

  void DicomDatabaseFile::ScanDicomFile(std::string file_name)
  {
    // Check to see if it's DICOM
    size_t dcm = file_name.find(".dcm");
    size_t dcmdir = file_name.find("DICOMDIR");
    size_t blank = file_name.find(".");
    if((dcm == std::string::npos) && (blank != std::string::npos)){
      std::cout << "File is not DICOM.\n";
      D.is_dicom = false;
      return D;
    }
    if(dcmdir != std::string::npos){
      std::cout << "Skipping DICOMDIR.\n";
      D.is_dicom = false;
      return D;
    }

    // Set file name and check reader
    gdcm::Reader RTReader;
    RTReader.SetFileName(file_name.c_str());
    if(!RTReader.Read()){
      std::cout << "File is not DICOM or cannot be read.\n";
      D.is_dicom = false;
      return D;
    }

    // If it is DICOM, get header data
    const gdcm::DataSet& Header = RTReader.GetFile().GetDataSet();
    if(Header.IsEmpty()){
      std::cout << "DICOM data file is empty.\n";
      D.is_dicom = false;
      return D;
    }

    // Set directory from file name
    const char pathSeparator =
      #ifdef _WIN32
        '\\';
      #else
        '/';
      #endif
    D.dir = file_name.substr(0, file_name.find_last_of(pathSeparator));
    //std::cout << D.dir << '\n';

    gdcm::Attribute<0x0010,0x0010> pt_name_att;
    pt_name_att.Set(Header);
    D.name = pt_name_att.GetValue();

    // Study UID
    gdcm::Attribute<0x0020,0x000d> study_uid_att;
    study_uid_att.Set(Header);
    D.study_uid = study_uid_att.GetValue();
    D.study_uid.erase(std::remove(D.study_uid.begin(),
        D.study_uid.end(), '\0'), D.study_uid.end());

    // Series UID
    gdcm::Attribute<0x0020,0x000e> series_uid_att;
    series_uid_att.Set(Header);
    D.series_uid = series_uid_att.GetValue();
    //D.series_uid.erase(std::remove(D.series_uid.begin(),
        //D.series_uid.end(), '\0'), D.series_uid.end());

    // Get DICOM file type/modality

    // SOP Class UID
    gdcm::Attribute<0x0008,0x0016> class_uid_att;
    class_uid_att.Set(Header);
    std::string class_uid = class_uid_att.GetValue();
    class_uid.erase(std::remove(class_uid.begin(), class_uid.end(), '\0'), class_uid.end());
    // Match to list (DO NOT USE SWITCH!)
    // Single and multi-frame CT
    if(class_uid == "1.2.840.10008.5.1.4.1.1.2") D.file_type = "CT";
    else if(class_uid == "1.2.840.10008.5.1.4.1.1.2.1") D.file_type = "CT";
    // MRI
    else if(class_uid == "1.2.840.10008.5.1.4.1.1.4") D.file_type = "MR";
    else if(class_uid == "1.2.840.10008.5.1.4.1.1.4.1") D.file_type = "MR";
    // PET
    else if(class_uid == "1.2.840.10008.5.1.4.1.1.128") D.file_type = "PET";
    else if(class_uid == "1.2.840.10008.5.1.4.1.1.130") D.file_type = "PET";
    // US
    else if(class_uid == "1.2.840.10008.5.1.4.1.1.6.1") D.file_type = "US";
    else if(class_uid == "1.2.840.10008.5.1.4.1.1.3.1") D.file_type = "US";
    // Radiation therapy data files
    else if(class_uid == "1.2.840.10008.5.1.4.1.1.481.1") D.file_type = "RT Image";
    else if(class_uid == "1.2.840.10008.5.1.4.1.1.481.2") D.file_type = "RT Dose";
    else if(class_uid == "1.2.840.10008.5.1.4.1.1.481.3") D.file_type = "RT Structure Set";
    else if(class_uid == "1.2.840.10008.5.1.4.1.1.481.4") D.file_type = "RT Beams";
    else if(class_uid == "1.2.840.10008.5.1.4.1.1.481.5") D.file_type = "RT Plan";
    else D.file_type = "Not Supported";
    D.sop_class_uid = class_uid;

    // SOP Instance UID
    gdcm::Attribute<0x0008,0x0018> sop_instance_uid_att;
    sop_instance_uid_att.Set(Header);
    std::string sop_in_uid = sop_instance_uid_att.GetValue();
    sop_in_uid.erase(std::remove(sop_in_uid.begin(), sop_in_uid.end(), '\0'), sop_in_uid.end());
    D.sop_instance_uid = sop_in_uid;

    return D;
  }
  */
}
