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
#include <iterator>

namespace solutio {
  std::vector< std::tuple<std::string, std::string, std::string> >
    SupportedIODList {
      std::make_tuple("1.2.840.10008.5.1.4.1.1.2","CT","CT"),
      std::make_tuple("1.2.840.10008.5.1.4.1.1.4","MR","MR"),
      std::make_tuple("1.2.840.10008.5.1.4.1.1.128","PET","PET"),
      std::make_tuple("1.2.840.10008.5.1.4.1.1.481.1","RTIMAGE","RI"),
      std::make_tuple("1.2.840.10008.5.1.4.1.1.481.2","RTDOSE","RD"),
      std::make_tuple("1.2.840.10008.5.1.4.1.1.481.3","RTSTRUCT","RS"),
      std::make_tuple("1.2.840.10008.5.1.4.1.1.481.4","RTBEAMS","RT"),
      std::make_tuple("1.2.840.10008.5.1.4.1.1.481.5","RTPLAN","RP")
      /*
      else if(class_uid == "1.2.840.10008.5.1.4.1.1.2.1") modality_name = "eCT";
      else if(class_uid == "1.2.840.10008.5.1.4.1.1.4.1") modality_name = "eMR";
      else if(class_uid == "1.2.840.10008.5.1.4.1.1.130") modality_name = "ePET";
      else if(class_uid == "1.2.840.10008.5.1.4.1.1.6.1") modality_name = "US";
      else if(class_uid == "1.2.840.10008.5.1.4.1.1.3.1") modality_name = "US";
      */
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
      DcmDataset * data = fileformat.getDataset();
      std::string class_uid = GetDicomValue<std::string>(data, DCM_SOPClassUID);
      if(class_uid == "1.2.840.10008.1.3.10" || class_uid == "")
      {
        std::cout << "Skipping DICOMDIR for now...\n";
        return false;
      }
      patient_name = GetDicomValue<std::string>(data, DCM_PatientName);
      study_uid = GetDicomValue<std::string>(data, DCM_StudyInstanceUID);
      series_uid = GetDicomValue<std::string>(data, DCM_SeriesInstanceUID);
      ref_frame_uid = GetDicomValue<std::string>(data, DCM_FrameOfReferenceUID);
      // Assign modality label based on SOP class UID; match to supported list
      if(class_uid[(class_uid.length()-1)] == '\0')
      {
        class_uid.erase(class_uid.length()-1);
      }
      auto it = std::find_if(SupportedIODList.begin(), SupportedIODList.end(),
        [&class_uid](const std::tuple<std::string, std::string, std::string>& e)
          {return std::get<0>(e) == class_uid;});
      if (it != SupportedIODList.end()) modality_name = std::get<1>(*it);
      else modality_name = "Not Supported";
      // Assign display name based on modality
      if(modality_name == "RTIMAGE")
      {
        display_name = GetDicomValue<std::string>(data, DCM_RTImageLabel);
      }
      else if(modality_name == "RTDOSE")
      {
        display_name = GetDicomValue<std::string>(data, DCM_SeriesDescription);
      }
      else if(modality_name == "RTSTRUCT")
      {
        display_name = GetDicomValue<std::string>(data, DCM_StructureSetLabel);
      }
      else if(modality_name == "RTBEAMS")
      {
        display_name = GetDicomValue<std::string>(data, DCM_SeriesDescription);
      }
      else if(modality_name == "RTPLAN")
      {
        display_name = GetDicomValue<std::string>(data, DCM_RTPlanLabel);
      }
      else
      {
        display_name = GetDicomValue<std::string>(data, DCM_SeriesDescription);
      }
    }
    else
    {
      std::cerr << "Error: cannot read DICOM file (" << file_name << "): " <<
        status.text() << '\n';
      return false;
    }
    // Get absolute file path
    std::filesystem::path p = file_name;
    file_path = std::filesystem::absolute(p).u8string();

    return true;
  }

  void DicomDatabaseSeries::SetInfo(DicomDatabaseFile &ddf)
  {
    parent_study_uid = ddf.GetStudyUID();
    series_uid = ddf.GetSeriesUID();
    modality_name = ddf.GetModality();
    ref_frame_uid = ddf.GetRefFrameUID();
    display_name = ddf.GetDisplayName();
  }

  void DicomDatabase::MakeDatabase(std::string database_path)
  {
    OFCondition result;
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
    unsigned long skipped_files = 0;
    for(file_it = file_list.begin(); file_it != file_list.end(); file_it++)
    {
      // Attempt to read file
      DicomDatabaseFile temp_file;
      if(!temp_file.ReadDicomInfo(*file_it))
      {
        skipped_files++;
        continue;
      }
      dicom_files.push_back(temp_file);
      // Check patient
      std::string pt_text = temp_file.GetPatientName();
      if(std::find(patient_list.begin(), patient_list.end(),
        pt_text) == patient_list.end())
      {
        patient_list.push_back(pt_text);
      }
      // Check study
      std::string st_text = temp_file.GetStudyUID();
      auto study_it = std::find_if(study_list.begin(), study_list.end(),
        [&temp_file,st_text](const std::pair<std::string, std::string>& el){
          return el.first == st_text;} );
      if(study_it == study_list.end())
      {
        std::pair<std::string, std::string> stp(st_text, pt_text);
        study_list.push_back(stp);
      }
      // Check series and assign file(s)
      std::string se_text = temp_file.GetSeriesUID();
      auto series_it = std::find_if(series_list.begin(), series_list.end(),
        [&temp_file,se_text](DicomDatabaseSeries& el){
          return el.GetSeriesUID() == se_text;} );
      if(series_it == series_list.end())
      {
        DicomDatabaseSeries temp_series;
        temp_series.SetInfo(temp_file);
        temp_series.AddFileID(
          std::distance(file_list.begin(), file_it) - skipped_files
        );
        series_list.push_back(temp_series);
      }
      else
      {
        series_list[(std::distance(series_list.begin(), series_it))].AddFileID(
          std::distance(file_list.begin(), file_it) - skipped_files);
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

  ItkImageF3::Pointer DicomDatabase::GetImageSeries(unsigned int series_id,
    std::function<void(float)> progress_function)
  {
    std::vector<std::string> file_list = GetSeriesFileNames(series_id);
    return ReadImageSeries(file_list, progress_function);
  }

  ItkImageF3::Pointer DicomDatabase::GetRTDose(unsigned int series_id)
  {
    std::vector<std::string> file_list = GetSeriesFileNames(series_id);
    return ReadRTDose(file_list[0]);
  }

  RTStructureSet DicomDatabase::GetRTS(unsigned int series_id)
  {
    std::vector<std::string> file_list = GetSeriesFileNames(series_id);
    return ReadRTS(file_list[0]);
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
              std::string txt = "    "+series_list[se].GetDisplayName()+
              " (" + series_list[se].GetModalityName()+
                ", "+std::to_string(series_list[se].GetNumFiles())+" file";
              if(series_list[se].GetNumFiles() == 1) txt += ")";
              else txt += "s)";
              print_text.push_back(txt);
            }
          }
        }
      }
    }

    return print_text;
  }

  std::vector< std::pair<std::string,int> > DicomDatabase::GetTree()
  {
    std::vector< std::pair<std::string,int> > tree;
    std::pair<std::string,int> temp;
    for(unsigned int p = 0; p < patient_list.size(); p++)
    {
      temp.first = patient_list[p];
      temp.second = 0;
      tree.push_back(temp);
      for(unsigned int st = 0; st < study_list.size(); st++)
      {
        if(study_list[st].second == patient_list[p])
        {
          temp.first = study_list[st].first;
          temp.second = 1;
          tree.push_back(temp);
          for(unsigned int se = 0; se < series_list.size(); se++)
          {
            if(series_list[se].CheckStudy(study_list[st].first))
            {
              std::string txt = series_list[se].GetDisplayName() + " ("
                + series_list[se].GetModalityName() +
                ", "+std::to_string(series_list[se].GetNumFiles())+" file";
              if(series_list[se].GetNumFiles() == 1) txt += ")";
              else txt += "s)";
              temp.first = txt;
              temp.second = 2;
              tree.push_back(temp);
            }
          }
        }
      }
    }
    return tree;
  }
}
