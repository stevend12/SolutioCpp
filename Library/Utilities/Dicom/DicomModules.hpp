/******************************************************************************/
/*                                                                            */
/* Copyright 2021 Steven Dolly                                                */
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
// April 22, 2021                                                             //
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
      std::string GetIE(){ return ie_name; }
      std::string GetName(){ return name; }
      void Read(DcmDataset * data)
      {
        for(int n = 0; n < attribute_list.size(); n++)
        {
          attribute_list[n]->ReadAttribute(data);
        }
      }
      void Read(std::string file_name)
      {
        DcmFileFormat fileformat;
        OFCondition status = fileformat.loadFile(file_name.c_str());
        if(status.good())
        {
          DcmDataset * data = fileformat.getDataset();
          Read(data);
        }
      }
      void Insert(DcmDataset * data)
      {
        for(int n = 0; n < attribute_list.size(); n++)
        {
          attribute_list[n]->InsertAttribute(data);
        }
      }
      std::vector< std::pair<std::string, std::string> > Print()
      {
        std::vector< std::pair<std::string, std::string> > mod_list;
        for(int n = 0; n < attribute_list.size(); n++)
        {
          std::pair<std::string, std::string> a = attribute_list[n]->Print();
          mod_list.push_back(a);
        }
        return mod_list;
      }
    protected:
      std::string ie_name;
      std::string name;
      std::vector<BaseAttribute *> attribute_list;
  };

  // Patient Module
  class PatientModule : public DicomModule
  {
    public:
      PatientModule() : PatientName(DCM_PatientName), PatientID(DCM_PatientID),
        PatientBirthDate(DCM_PatientBirthDate), PatientSex(DCM_PatientSex)
      {
        ie_name = name = "Patient";
        attribute_list.clear();
        BaseAttribute * a[4] = {&PatientName, &PatientID, &PatientBirthDate,
            &PatientSex};
        attribute_list.insert(attribute_list.end(), std::begin(a), std::end(a));
      }
      PatientModule(const PatientModule &pm) : PatientModule()
      {
        (*this) = pm;
      }
      void operator=(const PatientModule &pm)
      {
          PatientName = pm.PatientName;
          PatientID = pm.PatientID;
          PatientBirthDate = pm.PatientBirthDate;
          PatientSex = pm.PatientSex;
      }
      // Patient's Name (0010,0010)
      SingleValueAttribute<std::string> PatientName;
      // Patient ID (0010,0020)
      SingleValueAttribute<std::string> PatientID;
      // Patient's birth date (0010,0030)
      SingleValueAttribute<std::string> PatientBirthDate;
      // Patient's sex (0010,0040)
      SingleValueAttribute<std::string> PatientSex;
  };

  ////////////////
  // RT Modules //
  ////////////////

  // RT General Plan Module
  class RTGeneralPlanModule : public DicomModule
  {
    public:
      RTGeneralPlanModule() : RTPlanLabel(DCM_RTPlanLabel),
        RTPlanName(DCM_RTPlanName), RTPlanDescription(DCM_RTPlanDescription),
        RTPlanDate(DCM_RTPlanDate), RTPlanTime(DCM_RTPlanTime),
        RTPlanGeometry(DCM_RTPlanGeometry),
        ReferencedStructureSetSequence(DCM_ReferencedStructureSetSequence)
      {
        ie_name = "Plan";
        name = "RT General Plan";
        attribute_list.clear();
        BaseAttribute * a[7] = {&RTPlanLabel, &RTPlanName, &RTPlanDescription,
          &RTPlanDate, &RTPlanTime, &RTPlanGeometry,
          &ReferencedStructureSetSequence};
        attribute_list.insert(attribute_list.end(), std::begin(a), std::end(a));
      }
      RTGeneralPlanModule(const RTGeneralPlanModule &gpm) : RTGeneralPlanModule()
      {
        (*this) = gpm;
      }
      void operator=(const RTGeneralPlanModule &gpm)
      {
          RTPlanLabel = gpm.RTPlanLabel;
          RTPlanName = gpm.RTPlanName;
          RTPlanDescription = gpm.RTPlanDescription;
          RTPlanDate = gpm.RTPlanDate;
          RTPlanTime = gpm.RTPlanTime;
          RTPlanGeometry = gpm.RTPlanGeometry;
          ReferencedStructureSetSequence = gpm.ReferencedStructureSetSequence;
      }
      // RT Plan Label (300A,0002)
      SingleValueAttribute<std::string> RTPlanLabel;
      // RT Plan Name (300A,0003)
      SingleValueAttribute<std::string> RTPlanName;
      // RT Plan Description (300A,0004)
      SingleValueAttribute<std::string> RTPlanDescription;
      // RT Plan Date (300A,0006)
      SingleValueAttribute<std::string> RTPlanDate;
      // RT Plan Time (300A,0007)
      SingleValueAttribute<std::string> RTPlanTime;
      // RT Plan Geometry (300A,000C)
      SingleValueAttribute<std::string> RTPlanGeometry;
      // Referenced Structure Set Sequence (300C,0060)
      SingleValueAttribute<std::string> ReferencedStructureSetSequence;
  };
}

#endif
