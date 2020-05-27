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

#include <gdcmReader.h>

namespace solutio {
  DicomDatabaseFile::DicomDatabaseFile()
  {
    modality_name = file_path = "";
  }

  bool DicomDatabaseFile::ReadDicomFile(std::string file_name)
  {
    // Load DICOM file using GDCM
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
    // Read DICOM modules
    Patient.Read(ds);
    //std::cout << "Patient Name: " << Patient.PatientName.GetValue() << '\n';
    Study.Read(ds);
    //std::cout << "Study UID: " << Study.StudyInstanceUID.GetValue() << '\n';
    Series.Read(ds);
    //std::cout << "Series UID: " << Series.SeriesInstanceUID.GetValue() << '\n';
    SOPCommon.Read(ds);
    //std::cout << "SOP Class UID: " << SOPCommon.SOPClassUID.GetValue() << '\n';
    // Assign modality label based on SOP class UID; match to list
    // (DO NOT USE SWITCH!)
    // Single and multi-frame CT
    std::string class_uid = SOPCommon.SOPClassUID.GetValue();
    if(class_uid[(class_uid.length()-1)] == '\0') class_uid.erase(class_uid.length()-1);
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
      if(!temp_file.ReadDicomFile(*file_it)) continue;
      dicom_files.push_back(temp_file);
      // Check patient
      if(std::find(patient_list.begin(), patient_list.end(),
        temp_file.Patient.PatientName.GetValue()) == patient_list.end())
      {
        patient_list.push_back(temp_file.Patient.PatientName.GetValue());
      }
      // Check study
      auto study_it = std::find_if(study_list.begin(), study_list.end(),
        [&temp_file](const std::pair<std::string, std::string>& el){
          return el.first == temp_file.Study.StudyInstanceUID.GetValue();} );
      if(study_it == study_list.end())
      {
        std::pair<std::string, std::string> stp(temp_file.Study.StudyInstanceUID.GetValue(),
          temp_file.Patient.PatientName.GetValue());
        study_list.push_back(stp);
      }
      // Check series and assign file
      auto series_it = std::find_if(series_list.begin(), series_list.end(),
        [&temp_file](DicomDatabaseSeries& el){
          return el.GetSeriesUID() == temp_file.Series.SeriesInstanceUID.GetValue();} );
      if(series_it == series_list.end())
      {
        DicomDatabaseSeries temp_series;
        temp_series.SetInfo(temp_file.Study.StudyInstanceUID.GetValue(),
          temp_file.Series.SeriesInstanceUID.GetValue(), temp_file.modality_name);
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
      std::cout << "Warning: series could not be accessed\n";
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
