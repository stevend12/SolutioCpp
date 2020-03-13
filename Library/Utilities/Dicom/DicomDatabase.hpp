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

namespace solutio {
  // Struct that contains just enough information from the DICOM file header to properly
  // organize the files and series hierarchically
  class DicomDatabaseFile
  {
    public:
      DicomDatabaseFile();
      std::string patient_name;
      std::string study_uid;
      std::string series_uid;
      std::string instance_uid;
      std::string sop_class_uid;
      std::string sop_instance_uid;
      std::string modality;
      std::string file_path;
      //void ScanDicomFile(std::string file_name);
  };

  // Class to organize the database files hierarchically into a nested list of DICOM objects
  class DicomDatabase
  {
    public:
      void MakeDatabase(std::string database_path);
      //void MakeItemList(std::string list_file_name);
      //int GetNumItems(){ return ItemList.size(); }
      //DicomID * GetItem(int l){ return &ItemList[l]; }
    private:
      //std::vector<DicomDatabaseFile> DicomFileList;
  };
}

#endif
