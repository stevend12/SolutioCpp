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
// (DicomDatabase.hpp)                                                        //
//                                                                            //
// Steven Dolly                                                               //
// March 13, 2020                                                             //
//                                                                            //
// This file contains the header for a class which manages a collection (i.e. //
// database) of DICOM files.                                                  //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#ifndef DICOMDATABASE_HPP
#define DICOMDATABASE_HPP

#include <string>

#include "DicomModules.hpp"

namespace solutio {
  // Struct that contains just enough information from the DICOM file header to properly
  // organize the files hierarchically (patient, study, series, etc.)
  class DicomDatabaseFile
  {
    public:
      DicomDatabaseFile();
      PatientModule Patient;
      GeneralStudyModule Study;
      GeneralSeriesModule Series;
      SOPCommonModule SOPCommon;
      std::string modality_name;
      bool ReadDicomFile(std::string file_name);
      std::string GetPath(){ return file_path; }
    private:
      std::string file_path;
      //bool is_dicom;
  };
  // Struct that contains a DICOM series (i.e. a group of DICOM files)
  class DicomDatabaseSeries
  {
    public:
      void SetInfo(std::string psuid, std::string suid, std::string mn);
      void AddFileID(int id){ file_ids.push_back(id); }
      bool CheckStudy(std::string uid){ return (uid == parent_study_uid); }
      unsigned int GetNumFiles(){ return file_ids.size(); }
      unsigned int GetFileID(unsigned int n){ return file_ids[n]; }
      std::string GetStudyUID(){ return parent_study_uid; }
      std::string GetSeriesUID(){ return series_uid; }
      std::string GetModalityName(){ return modality_name; }
    private:
      std::string parent_study_uid;
      std::string series_uid;
      std::string modality_name;
      std::vector<unsigned int> file_ids;
  };

  // Class to organize the database files hierarchically into a nested list of DICOM objects
  class DicomDatabase
  {
    public:
      void MakeDatabase(std::string database_path);
      DicomDatabaseFile GetFile(unsigned int id){ return dicom_files[id]; }
      DicomDatabaseSeries GetSeries(unsigned int id){ return series_list[id]; }
      std::vector<std::string> GetSeriesFileNames(unsigned int series_id);
      std::vector<std::string> PrintTree();
    private:
      std::vector<DicomDatabaseFile> dicom_files;
      std::vector<std::string> patient_list;
      std::vector< std::pair<std::string, std::string> > study_list;
      std::vector<DicomDatabaseSeries> series_list;
  };
}

#endif
